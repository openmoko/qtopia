/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "launcherview.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/applnk.h>
#include <qtopia/qpedebug.h>
#include <qtopia/categories.h>
#include <qtopia/categoryselect.h>
#include <qtopia/menubutton.h>
#include <qtopia/mimetype.h>
#include <qtopia/resource.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
//#include <qtopia/private/palmtoprecord.h>

#include <qaction.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qdict.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhbox.h>
#include <qiconview.h>
#include <qwidgetstack.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qtoolbutton.h>
#include <qimage.h>
#include <qlabel.h>
#include <qobjectlist.h>


// These define how the busy icon is animated and highlighted
#define BRIGHTEN_BUSY_ICON
//#define ALPHA_FADE_BUSY_ICON
//#define USE_ANIMATED_BUSY_ICON_OVERLAY
#define BOUNCE_BUSY_ICON


class BgPixmap
{
public:
    BgPixmap( const QPixmap &p ) : pm(p), ref(1) {}
    QPixmap pm;
    int ref;
};

static QMap<QString,BgPixmap*> *bgCache = 0;

static void cleanup_cache()
{
    QMap<QString,BgPixmap*>::Iterator it = bgCache->begin();
    while ( it != bgCache->end() ) {
	QMap<QString,BgPixmap*>::Iterator curr = it;
	++it;
	delete (*curr);
	bgCache->remove( curr );
    }
    delete bgCache;
    bgCache = 0;
}


class LauncherItem : public QIconViewItem
{
public:
    LauncherItem( QIconView *parent, AppLnk* applnk, bool bigIcon=TRUE );
    ~LauncherItem();

    AppLnk *appLnk() const { return app; }
    AppLnk *takeAppLnk() { AppLnk* r=app; app=0; return r; }

    void animateIcon();
    void resetIcon();

    virtual int compare ( QIconViewItem * i ) const;
    void paintItem( QPainter *p, const QColorGroup &cg );
    void paintFocus( QPainter *, const QColorGroup & ) {}

protected:
    bool isBigIcon;
    int iteration;
    AppLnk* app;
private:
    void paintAnimatedIcon( QPainter *p );
};

#ifdef QTOPIA_PHONE
// XXX share me
static int phoneKeyMatchN(const QString& haystack, const QString& needle, int n)
{
    // XXX cache me / load me
    QString phonekey[8];
    phonekey[0] = "abcABC";
    phonekey[1] = "defDEF";
    phonekey[2] = "ghiGHI";
    phonekey[3] = "jklJKL";
    phonekey[4] = "mnoMNO";
    phonekey[5] = "pqrsQPRS";
    phonekey[6] = "tuvTUV";
    phonekey[7] = "wxyzWXYZ";

    for (int i=0; i<n; i++) {
	int pk = needle[i]-'2';
	if ( pk >= 0 && pk < 8 ) {
	    QChar ch = haystack[i].lower();
	    if ( !phonekey[pk].contains(ch) )
		return ch.unicode() - phonekey[pk][0].unicode();
	}
    }

    return 0; // match!
}
#endif

class LauncherIconView : public QIconView {
    Q_OBJECT
public:
    LauncherIconView( QWidget* parent, const char* name=0 ) :
	QIconView(parent,name),
	tf(""),
	cf(-2),
	bsy(0),
	busyTimer(0),
	bigIcns(TRUE),
	bgColor(white)
    {
#ifdef QTOPIA_PHONE
	keytimer = new QTimer(this);
	connect(keytimer, SIGNAL(timeout()), this, SLOT(keyTimeout()));
#endif
	sortmeth = Name;
	hidden.setAutoDelete(TRUE);
	ike = FALSE;
	calculateGrid( Bottom );
	disconnect(verticalScrollBar(), SIGNAL(valueChanged(int)),
	    this, SLOT(vslide(int)));
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
	    this, SLOT(vertScroll(int)));
	connect(this, SIGNAL(myvslide(int)), this, SLOT(vslide(int)));
    }

    ~LauncherIconView()
    {
#if 0 // debuggery
	QListIterator<AppLnk> it(hidden);
	AppLnk* l;
	while ((l=it.current())) {
	    ++it;
	    //qDebug("%p: hidden (should remove)",l);
	}
#endif
    }

    QIconViewItem* busyItem() const { return bsy; }
#ifdef USE_ANIMATED_BUSY_ICON_OVERLAY
    QPixmap busyPixmap() const { return busyPix; }
#endif
    void setBigIcons( bool bi ) {
	bigIcns = bi;
#ifdef USE_ANIMATED_BUSY_ICON_OVERLAY
	busyPix.resize(0,0);
#endif
    }

    void updateCategoriesAndMimeTypes();

    void doAutoScroll()
    {
	// We don't want rubberbanding (yet)
    }

    void setBusy(bool on)
    {
#ifdef USE_ANIMATED_BUSY_ICON_OVERLAY
	if ( busyPix.isNull() ) {
	    int size = ( bigIcns ) ? AppLnk::bigIconSize() : AppLnk::smallIconSize();
	    busyPix.convertFromImage( Resource::loadImage( "busy" ).smoothScale( size * 16, size ) );
	}
#endif

	if ( on ) {
	    busyTimer = startTimer( 100 );
	} else {
	    if ( busyTimer ) {
	        killTimer( busyTimer );
		busyTimer = 0;
	    }
	}

	LauncherItem *c = on ? (LauncherItem*)currentItem() : 0;

	if ( bsy != c ) {
	    LauncherItem *oldBusy = bsy;
	    bsy = c;
	    if ( oldBusy )
		oldBusy->resetIcon();
	    if ( bsy )
		bsy->animateIcon();
	}
    }

    bool inKeyEvent() const { return ike; }

    void keyPressEvent(QKeyEvent* e)
    {
	ike = TRUE;
	if ( e->key() == Key_F33 /* OK button */ || e->key() == Key_Space ) {
	    if ( (e->state() & ShiftButton) )
		emit mouseButtonPressed(ShiftButton, currentItem(), QPoint() );
	    else
		returnPressed(currentItem());
#ifdef QTOPIA_PHONE
	} else if ( e->ascii() >= '2' && e->ascii() <= '9' ) {
	    static const int ABC_AUTOEND_TIME=700;
	    keytimer->start(ABC_AUTOEND_TIME, TRUE);
	    keychars += e->ascii();
	    QIconViewItem* i = firstItem();
	    QIconViewItem* sel = 0;
	    while (i) {
		QString n = i->text();
		int m = phoneKeyMatchN(n,keychars,keychars.length());
		if ( m == 0 ) {
		    sel = i;
		    break;
		} else if ( m > 0 ) {
		    if ( !sel )
			sel = i;
		}
		i = i->nextItem();
	    }
	    if ( sel )
		setCurrentItem(sel);
	} else {
	    keychars = "";
#endif
	}
	QIconView::keyPressEvent(e);
	ike = FALSE;
    }

private:
#ifdef QTOPIA_PHONE
    QString keychars;
    QTimer *keytimer;
#endif

private slots:
    void keyTimeout()
    {
#ifdef QTOPIA_PHONE
	keychars = "";
#endif
    }

signals:
    void myvslide(int pos);

public:

    LauncherItem *addItem(AppLnk* app, bool resort=TRUE);
    bool removeLink(const QString& linkfile);

    QStringList mimeTypes() const;
    QStringList categories() const;

    void clear()
    {
	bsy = 0;
	mimes.clear();
	QIconView::clear();
	hidden.clear();
    }

    void addCatsAndMimes(AppLnk* app)
    {
	// No longer...
	// 	QStringList c = app->categories();
	// 	for (QStringList::ConstIterator cit=c.begin(); cit!=c.end(); ++cit) {
	// 	    cats.replace(*cit,(void*)1);
	// 	}
	QString maj=app->type();
	int sl=maj.find('/');
	if (sl>=0) {
	    QString k;
	    k = maj.left(12) == "application/" ? maj : maj.left(sl);
	    mimes.replace(k,(void*)1);
	}
    }

    void setBackgroundOrigin( QWidget::BackgroundOrigin ) {
    }

    void setBackgroundPixmap( const QPixmap &pm ) {
	QIconView::setBackgroundPixmap(pm);
	bgPixmap = pm;
    }

    void setBackgroundColor( const QColor &c ) {
	bgColor = c;
    }

    void drawBackground( QPainter *p, const QRect &r )
    {
	if ( !bgPixmap.isNull() ) {
	    if (!hasStaticBackground()) {
		p->drawTiledPixmap( r, bgPixmap,
			QPoint( (r.x() + contentsX()) % bgPixmap.width(),
			    (r.y() + contentsY()) % bgPixmap.height() ) );
	    } else {
		QPoint offs(0, 0);
		offs = mapToGlobal(offs);
		p->drawPixmap(r.x(), r.y(), bgPixmap,
			    r.x()+offs.x(), r.y()+offs.y(), r.width(), r.height());
	    }
	} else {
#ifdef QTOPIA_PHONE
	    QIconView::drawBackground(p, r);
#else
	    p->fillRect( r, bgColor );
#endif
	}
    }

    void setItemTextPos( ItemTextPos pos )
    {
	calculateGrid( pos );
	QIconView::setItemTextPos( pos );
    }

    void hideOrShowItems(bool resort);

    void setTypeFilter(const QString& typefilter, bool resort)
    {
	tf = QRegExp(typefilter,FALSE,TRUE);
	hideOrShowItems(resort);
    }

    void setCategoryFilter( int catfilter, bool resort )
    {
	Categories cat;
	cat.load( categoryFileName() );
	QString str;
	cf = catfilter;
	hideOrShowItems(resort);
    }

    enum SortMethod { Name, Date, Type };

    void setSortMethod( SortMethod m )
    {
	if ( sortmeth != m ) {
	    sortmeth = m;
	    sort();
	}
    }

    int compare(const AppLnk* a, const AppLnk* b)
    {
	switch (sortmeth) {
	case Name:
	    return Qtopia::dehyphenate(a->name()).lower()
          .compare(Qtopia::dehyphenate(b->name()).lower());
	case Date: {
	    QFileInfo fa(a->linkFileKnown() ? a->linkFile() : a->file());
	    QFileInfo fb(b->linkFileKnown() ? b->linkFile() : b->file());
	    return fa.lastModified().secsTo(fb.lastModified());
	}
	case Type:
	    return a->type().compare(b->type());
	}
	return 0;
    }

protected:

    void timerEvent( QTimerEvent *te )
    {
	if ( te->timerId() == busyTimer ) {
	    if ( bsy )
		bsy->animateIcon();
	} else {
	    QIconView::timerEvent( te );
	}
    }

    void styleChange( QStyle &old )
    {
	QIconView::styleChange( old );
	calculateGrid( itemTextPos() );
    }

    void calculateGrid( ItemTextPos pos )
    {
	int dw = QApplication::desktop()->width();
	int viewerWidth = dw-style().scrollBarExtent().width();
	if ( pos == Bottom ) {
	    int cols = 3;
	    if ( viewerWidth <= 200 )
		cols = 2;
	    else if ( viewerWidth >= 400 )
		 cols = viewerWidth/96;
	    setSpacing( 4 );
	    setGridX( (viewerWidth-(cols+1)*spacing())/cols );
	    setGridY( fontMetrics().height()*2+24 );
	} else {
	    int cols = 2;
#ifdef QTOPIA_PHONE
	    if ( viewerWidth < 240 )
#else
	    if ( viewerWidth < 150 )
#endif
		cols = 1;
	    else if ( viewerWidth >= 400 )
		 cols = viewerWidth/150;
	    setSpacing( 2 );
	    setGridX( (viewerWidth-(cols+1)*spacing())/cols );
	    setGridY( fontMetrics().height()+2 );
	}
    }

    void resizeEvent( QResizeEvent *e )
    {
	QIconView::resizeEvent(e);
	if ((e->size().width() != e->oldSize().width()) || (e->size().height() != e->oldSize().height())) {
	    calculateGrid(itemTextPos());
	    hideOrShowItems(FALSE);
	}
    }

    void focusInEvent( QFocusEvent * ) {}
    void focusOutEvent( QFocusEvent * ) {}

private slots:
    void vertScroll(int pos) {
	if (hasStaticBackground()) {
	    if (pos < verticalScrollBar()->maxValue()) {
		int step = gridY()+spacing();
		pos = (pos + step/2) / step * step;
	    }
	}
	emit myvslide(pos);
    }

private:
    QList<AppLnk> hidden;
    QDict<void> mimes;
    SortMethod sortmeth;
    QRegExp tf;
    int cf;
    LauncherItem* bsy;
    int busyTimer;
    bool ike;
    bool bigIcns;
    QPixmap bgPixmap;
    QColor bgColor;
#ifdef USE_ANIMATED_BUSY_ICON_OVERLAY
    QPixmap busyPix;
#endif
};


bool LauncherView::bsy=FALSE;

void LauncherView::setBusy(bool on)
{
    icons->setBusy(on);
}

LauncherItem::LauncherItem( QIconView *parent, AppLnk *applnk, bool bigIcon )
    : QIconViewItem( parent, applnk->name(),
           bigIcon ? applnk->bigPixmap() :applnk->pixmap() ),
	isBigIcon( bigIcon ),
	iteration(0),
	app(applnk) // Takes ownership
{
}

LauncherItem::~LauncherItem()
{
    LauncherIconView* liv = (LauncherIconView*)iconView();
    if ( liv->busyItem() == this )
	liv->setBusy(FALSE);
    delete app;
}

int LauncherItem::compare ( QIconViewItem * i ) const
{
    LauncherIconView* view = (LauncherIconView*)iconView();
    return view->compare(app,((LauncherItem *)i)->appLnk());
}

void LauncherItem::paintItem( QPainter *p, const QColorGroup &cg )
{
    LauncherIconView* liv = (LauncherIconView*)iconView();
    QBrush oldBrush( liv->itemTextBackground() );
    QColorGroup mycg( cg );
    if ( liv->currentItem() == this ) {
	liv->setItemTextBackground( cg.brush( QColorGroup::Highlight ) );
	mycg.setColor( QColorGroup::Text, cg.color( QColorGroup::HighlightedText ) );
    }

    QIconViewItem::paintItem(p,mycg);

    // Paint animation overlay
    if ( liv->busyItem() == this ) 
	paintAnimatedIcon(p);

    if ( liv->currentItem() == this )
	liv->setItemTextBackground( oldBrush );
}

void LauncherItem::paintAnimatedIcon( QPainter *p )
{
    LauncherIconView* liv = (LauncherIconView*)iconView();
    int w = pixmap()->width(), h = pixmap()->height();
    QPixmap dblBuf( w, h + 4 );
    QPainter p2( &dblBuf );
    int x1, y1;
    if ( liv->itemTextPos() == QIconView::Bottom ) {
	x1 = x() + (width() - w) / 2 - liv->contentsX();
	y1 = y() - liv->contentsY();
    } else {
	x1 = x() - liv->contentsX();
	y1 = y() + (height() - h) / 2 - liv->contentsY();
    }
    y1 -= 2;
    p2.translate(-x1,-y1);
    liv->drawBackground( &p2, QRect(x1,y1,w,h+4) );
    int bounceY = 2;
#ifdef BOUNCE_BUSY_ICON
    bounceY = 4 - ((iteration+2)%8);
    bounceY = bounceY < 0 ? -bounceY : bounceY;
#endif
    p2.drawPixmap( x1, y1 + bounceY, *pixmap() );
#ifdef USE_ANIMATED_BUSY_ICON_OVERLAY
    int pic = iteration % 16;
    p2.drawPixmap( x1, y1 + bounceY, liv->busyPixmap(), w * pic, 0, w, h );
#endif
    p->drawPixmap( x1, y1, dblBuf );
}

void LauncherItem::animateIcon()
{
    LauncherIconView* liv = (LauncherIconView*)iconView();

    if ( liv->busyItem() != this || !app )
	return;

    // Highlight the icon
    if ( iteration == 0 ) {
	QPixmap src = isBigIcon ? app->bigPixmap() : app->pixmap();
	QImage img = src.convertToImage();
	QRgb *rgb;
	int count;
	if ( img.depth() == 32 ) {
	    rgb = (QRgb*)img.bits();
	    count = img.bytesPerLine()/sizeof(QRgb)*img.height();
	} else {
	    rgb = img.colorTable();
	    count = img.numColors();
	}
	for ( int r = 0; r < count; r++, rgb++ ) {
#if defined(BRIGHTEN_BUSY_ICON)
	    QColor c(*rgb);
	    int h, s, v;
	    c.hsv(&h,&s,&v);
	    c.setHsv(h,QMAX(s-24,0),QMIN(v+48,255));
	    *rgb = qRgba(c.red(),c.green(),c.blue(),qAlpha(*rgb));
#elif defined(ALPHA_FADE_BUSY_ICON)
	    *rgb = qRgba(qRed(*rgb),qGreen(*rgb),qBlue(*rgb),qAlpha(*rgb)/2);
#endif
	}
	src.convertFromImage( img );
	setPixmap( src );
    }

    iteration++;

    // Paint animation overlay
    QPainter p( liv->viewport() );
    paintAnimatedIcon( &p );
}

void LauncherItem::resetIcon()
{
    iteration = 0;
    setPixmap( isBigIcon ? app->bigPixmap() : app->pixmap() );
}

//===========================================================================

QStringList LauncherIconView::mimeTypes() const
{
    QStringList r;
    QDictIterator<void> it(mimes);
    while (it.current()) {
	r.append(it.currentKey());
	++it;
    }
    r.sort();
    return r;
}

LauncherItem* LauncherIconView::addItem(AppLnk* app, bool resort)
{
    addCatsAndMimes(app);
    LauncherItem *r=0;

    if ( (tf.isEmpty() || tf.match(app->type()) >= 0)
	 && (cf == -2 || app->categories().contains(cf)
	     || cf == -1 && app->categories().count() == 0 ) )
	r = new LauncherItem( this, app, bigIcns );
    else
	hidden.append(app);
    if ( resort )
	sort();
    return r;
}

void LauncherIconView::updateCategoriesAndMimeTypes()
{
    mimes.clear();
    LauncherItem* item = (LauncherItem*)firstItem();
    while (item) {
	addCatsAndMimes(item->appLnk());
	item = (LauncherItem*)item->nextItem();
    }
    QListIterator<AppLnk> it(hidden);
    AppLnk* l;
    while ((l=it.current())) {
	addCatsAndMimes(l);
	++it;
    }
}

void LauncherIconView::hideOrShowItems(bool resort)
{
    viewport()->setUpdatesEnabled( FALSE );
    hidden.setAutoDelete(FALSE);
    QList<AppLnk> links=hidden;
    hidden.clear();
    hidden.setAutoDelete(TRUE);
    LauncherItem* item = (LauncherItem*)firstItem();
    LauncherItem* cur = (LauncherItem*)currentItem();
    AppLnk *cura = cur ? cur->appLnk() : 0;
    while (item) {
	links.append(item->takeAppLnk());
	item = (LauncherItem*)item->nextItem();
    }
    clear();
    cur = 0;
    QListIterator<AppLnk> it(links);
    AppLnk* l;
    while ((l=it.current())) {
	LauncherItem *i = addItem(l,FALSE);
	if ( l == cura )
	    cur = i;
	++it;
    }
    if ( resort && !autoArrange() )
	sort();
    setCurrentItem(cur);
    viewport()->setUpdatesEnabled( TRUE );
}

bool LauncherIconView::removeLink(const QString& linkfile)
{
    LauncherItem* item = (LauncherItem*)firstItem();
    AppLnk* l;
    bool did = FALSE;
    DocLnk dl(linkfile);
    while (item) {
	l = item->appLnk();
	LauncherItem *nextItem = (LauncherItem *)item->nextItem();
	if (  l->linkFileKnown() && l->linkFile() == linkfile
		|| l->fileKnown() && (
		    l->file() == linkfile
		    || dl.isValid() && dl.file() == l->file() ) ) {
	    delete item;
	    did = TRUE;
	}
	item = nextItem;
    }
    QListIterator<AppLnk> it(hidden);
    while ((l=it.current())) {
	++it;
	if ( l->linkFileKnown() && l->linkFile() == linkfile
		|| l->file() == linkfile
		|| dl.isValid() && dl.file() == l->file() ) {
	    hidden.removeRef(l);
	   did = TRUE;
	}
    }
    return did;
}

//===========================================================================

LauncherView::LauncherView( QWidget* parent, const char* name, WFlags fl )
    : QVBox( parent, name, fl )
{
    catmb = 0;
    icons = new LauncherIconView( this );
    setFocusProxy(icons);
    QPEApplication::setStylusOperation( icons->viewport(), QPEApplication::RightOnHold );

    icons->setItemsMovable( FALSE );
    icons->setAutoArrange( TRUE );
    icons->setSorting( TRUE );
    icons->setFrameStyle( QFrame::NoFrame );
    icons->setMargin( 0 );
    icons->setSelectionMode( QIconView::NoSelection );
    icons->setBackgroundMode( PaletteBase );
    icons->setResizeMode( QIconView::Fixed );
    vmode = (ViewMode)-1;
    setViewMode( Icon );

    connect( icons, SIGNAL(mouseButtonClicked(int,QIconViewItem*,const QPoint&)),
		   SLOT(itemClicked(int,QIconViewItem*)) );
    connect( icons, SIGNAL(selectionChanged()),
		   SLOT(selectionChanged()) );
    connect( icons, SIGNAL(returnPressed(QIconViewItem*)),
		   SLOT(returnPressed(QIconViewItem*)) );
    connect( icons, SIGNAL(mouseButtonPressed(int,QIconViewItem*,const QPoint&)),
		   SLOT(itemPressed(int,QIconViewItem*)) );

    tools = 0;
    setBackgroundType( Ruled, QString::null );
}

LauncherView::~LauncherView()
{
    if ( bgCache && bgCache->contains( bgName ) )
	(*bgCache)[bgName]->ref--;
}

void LauncherView::hideIcons()
{
    icons->hide();
}

void LauncherView::setToolsEnabled(bool y)
{
    if ( !y != !tools ) {
	if ( y ) {
	    tools = new QHBox(this);

	    // Type filter
	    typemb = new QComboBox(tools);
	    QSizePolicy p = typemb->sizePolicy();
	    p.setHorData(QSizePolicy::Expanding);
	    typemb->setSizePolicy(p);

	    // Category filter
	    updateTools();
	    tools->show();

	    // Always show vscrollbar
	    icons->setVScrollBarMode( QScrollView::AlwaysOn );
	} else {
	    delete tools;
	    tools = 0;
	}
    }
}

//needed for lupdate
static const char* translatedMimeTypes[] = {
    QT_TRANSLATE_NOOP("MimeType", "Image"),
    QT_TRANSLATE_NOOP("MimeType", "Audio"),
    QT_TRANSLATE_NOOP("MimeType", "Text"),
    QT_TRANSLATE_NOOP("MimeType", "Video"),
};

void LauncherView::updateTools()
{
    disconnect( typemb, SIGNAL(activated(int)),
	        this, SLOT(showType(int)) );
    if ( catmb ) disconnect( catmb, SIGNAL(signalSelected(int)),
	        this, SLOT(showCategory(int)) );

    // ### I want to remove this
    icons->updateCategoriesAndMimeTypes();

    QString prev;

    // Type filter
    QStringList types;
    typelist = icons->mimeTypes();
    for (QStringList::ConstIterator it = typelist.begin(); it!=typelist.end(); ++it) {
	QString t = *it;
	if ( t.left(12) == "application/" ) {
	    MimeType mt(t);
	    const AppLnk* app = mt.application();
	    if ( app )
		t = Qtopia::dehyphenate(app->name());
	    else
		t = t.mid(12);
	} else {
	    t[0] = t[0].upper();
            t = qApp->translate("MimeType", t.latin1());
	}
	types += t;
    }
    types << tr("All types");
    prev = typemb->currentText();
    typemb->clear();
    typemb->insertStringList(types);
    for (int i=0; i<typemb->count(); i++) {
	if ( typemb->text(i) == prev ) {
	    typemb->setCurrentItem(i);
	    break;
	}
    }
    if ( prev.isNull() )
	typemb->setCurrentItem(typemb->count()-1);

    int pcat = catmb ? catmb->currentCategory() : -2;
    if ( !catmb )
	catmb = new CategorySelect(tools);
    Categories cats( 0 );
    cats.load( categoryFileName() );
    QArray<int> vl( 0 );
    catmb->setCategories( vl, "Document View", // No tr
	tr("Document View") );
    catmb->setRemoveCategoryEdit( TRUE );
    catmb->setAllCategories( TRUE );
    catmb->setCurrentCategory(pcat);

    // if type has changed we need to redisplay
    if ( typemb->currentText() != prev )
	showType( typemb->currentItem() );

    connect(typemb, SIGNAL(activated(int)), this, SLOT(showType(int)));
    connect(catmb, SIGNAL(signalSelected(int)), this, SLOT(showCategory(int)));
}

void LauncherView::sortBy(int s)
{
    icons->setSortMethod((LauncherIconView::SortMethod)s);
}

void LauncherView::showType(int t)
{
    if ( t >= (int)typelist.count() ) {
	icons->setTypeFilter("",TRUE);
    } else {
	QString ty = typelist[t];
	if ( !ty.contains('/') )
	    ty += "/*";
	icons->setTypeFilter(ty,TRUE);
    }
}

void LauncherView::showType(const QString & t)
{
    if (t.isEmpty()) {
	icons->setTypeFilter("",TRUE);
    } else {
	QString ty(t);
	if ( !ty.contains('/') )
	    ty += "/*";
	icons->setTypeFilter(ty,TRUE);
    }
}

void LauncherView::showCategory( int c )
{
    icons->setCategoryFilter( c, TRUE );
}

void LauncherView::setViewMode( ViewMode m )
{
    if ( vmode != m ) {
	bool bigIcons = m == Icon;
	icons->viewport()->setUpdatesEnabled( FALSE );
	icons->setBigIcons( bigIcons );
	switch ( m ) {
	    case List:
		icons->setItemTextPos( QIconView::Right );
		break;
	    case Icon:
		icons->setItemTextPos( QIconView::Bottom );
		break;
	}
	icons->hideOrShowItems( FALSE );
	icons->viewport()->setUpdatesEnabled( TRUE );
	vmode = m;
    }
}

//
// User images may require scaling.
//
QImage LauncherView::loadBackgroundImage(QString &bgName)
{
    QImageIO imgio;
    QSize   ds = qApp->desktop()->size();   // should be launcher, not desktop
    bool    further_scaling = TRUE;

    imgio.setFileName( bgName );
    imgio.setParameters("GetHeaderInformation");

    if (imgio.read() == FALSE) {
	return imgio.image();
    }

    if (imgio.image().width() < ds.width() &&
	    imgio.image().height() < ds.height()) {
	further_scaling = FALSE;
    }

    if (!imgio.image().bits()) {
	//
	// Scale and load.  Note we don't scale up.
	//
	QString param( "Scale( %1, %2, ScaleMin )" ); // No tr
	imgio.setParameters(further_scaling ?
	    param.arg(ds.width()).arg(ds.height()).latin1() :
	    "");
	imgio.read();
    } else {
	if (further_scaling) {
	    int	t1 = imgio.image().width() * ds.height();
	    int t2 = imgio.image().height() * ds.width();
	    int dsth = ds.height();
	    int dstw = ds.width();

	    if (t1 > t2) {
		dsth = t2 / imgio.image().width();
	    } else {
		dstw = t1 / imgio.image().height();
	    }

	    //
	    // Loader didn't scale for us.  Do it manually.
	    //
	    return imgio.image().smoothScale(dstw, dsth);
	}
    }

    return imgio.image();
}

void LauncherView::setBackgroundType( BackgroundType t, const QString &val )
{
    bgType = t;
    bgValue = val;

    if (icons->hasStaticBackground())
	return;
    if ( !bgCache ) {
	bgCache = new QMap<QString,BgPixmap*>;
	qAddPostRoutine( cleanup_cache );
    }

    if ( bgCache->contains( bgName ) )
	(*bgCache)[bgName]->ref--;
    bgName = "";

    QPixmap bg;

    switch ( t ) {
	case Ruled: {
	    bgName = QString("Ruled_%1").arg(colorGroup().background().name()); // No tr
	    if ( bgCache->contains( bgName ) ) {
		(*bgCache)[bgName]->ref++;
		bg = (*bgCache)[bgName]->pm;
	    } else {
		bg.resize( width(), 9 );
		QPainter painter( &bg );
		for ( int i = 0; i < 3; i++ ) {
		    painter.setPen( colorGroup().base() );
		    painter.drawLine( 0, i*3, width()-1, i*3 );
		    painter.drawLine( 0, i*3+1, width()-1, i*3+1 );
		    painter.setPen( colorGroup().background() );
		    painter.drawLine( 0, i*3+2, width()-1, i*3+2 );
		}
		painter.end();
		bgCache->insert( bgName, new BgPixmap(bg) );
	    }
	    break;
	}

	case Image:
	    if (!val.isEmpty()) {
		bgName = val;
		if ( bgCache->contains( bgName ) ) {
		    (*bgCache)[bgName]->ref++;
		    bg = (*bgCache)[bgName]->pm;
		} else {
		    QString imgFile = bgName;
		    bool tile = FALSE;
		    if ( imgFile[0]!='/' || !QFile::exists(imgFile) ) {
			imgFile = Resource::findPixmap( "wallpaper/" + imgFile );
			tile = TRUE;
		    }
		    QImage img = loadBackgroundImage(imgFile);


		    if ( img.depth() == 1 )
			img = img.convertDepth(8);
		    img.setAlphaBuffer(FALSE);
		    bg.convertFromImage(img);
		    bgCache->insert( bgName, new BgPixmap(bg) );
		}
	    }
	    break;

	case SolidColor:
	default:
	    break;
    }

    const QObjectList *list = queryList( "QWidget", 0, FALSE );
    QObject *obj;
    for ( QObjectListIt it( *list ); (obj=it.current()); ++it ) {
	if ( obj->isWidgetType() ) {
	    QWidget *w = (QWidget*)obj;
	    w->setBackgroundPixmap( bg );
	    if ( bgName.isEmpty() ) {
		// Solid Color
		if ( val.isEmpty() ) 
		    w->setBackgroundColor( colorGroup().base() );
		else 
		    w->setBackgroundColor( val );
	    } else {
		// Ruled or Image pixmap
		w->setBackgroundOrigin( ParentOrigin );
	    }
	}
    }
    delete list;

    icons->viewport()->update();

    QTimer::singleShot( 1000, this, SLOT(flushBgCache()) );
}

void LauncherView::setStaticBackground(const QPixmap &pm)
{
    if (pm.isNull()) {
	if (icons->hasStaticBackground()) {
	    icons->setStaticBackground(FALSE);
	    setBackgroundType(bgType, bgValue);
	}
    } else {
	icons->setStaticBackground(TRUE);
	icons->setBackgroundPixmap(pm);
	icons->viewport()->setBackgroundMode(NoBackground);
	icons->viewport()->update();
    }
}

void LauncherView::setTextColor( const QColor &tc )
{
    textCol = tc;
    QColorGroup cg = icons->colorGroup();
    cg.setColor( QColorGroup::Text, tc );
    icons->setPalette( QPalette(cg,cg,cg) );
    icons->viewport()->update();
}

void LauncherView::setViewFont( const QFont &f )
{
    icons->setFont( f );
    icons->hideOrShowItems( FALSE );
}

void LauncherView::clearViewFont()
{
    icons->unsetFont();
    icons->hideOrShowItems( FALSE );
}

void LauncherView::resizeEvent(QResizeEvent *e)
{
    QVBox::resizeEvent( e );
    if ( e->size().width() != e->oldSize().width() )
	sort();
}

void LauncherView::selectionChanged()
{
    QIconViewItem* item = icons->currentItem();
    if ( item && item->isSelected() ) {
	AppLnk *appLnk = ((LauncherItem *)item)->appLnk();
	if ( icons->inKeyEvent() ) // not for mouse press
	    emit clicked( appLnk );
	item->setSelected(FALSE);
    }
}

void LauncherView::returnPressed( QIconViewItem *item )
{
    if ( item ) {
	AppLnk *appLnk = ((LauncherItem *)item)->appLnk();
	emit clicked( appLnk );
    }
}

void LauncherView::itemClicked( int btn, QIconViewItem *item )
{
    if ( item ) {
	AppLnk *appLnk = ((LauncherItem *)item)->appLnk();
	if ( btn == LeftButton ) {
	    QTOPIA_PROFILE("icon clicked");
	    // Make sure it's the item we execute that gets highlighted
	    icons->setCurrentItem( item );
	    emit clicked( appLnk );
	}
	item->setSelected(FALSE);
    }
}

void LauncherView::itemPressed( int btn, QIconViewItem *item )
{
    if ( item ) {
	AppLnk *appLnk = ((LauncherItem *)item)->appLnk();
	item->setSelected(FALSE);
	if ( btn == RightButton )
	    emit rightPressed( appLnk );
	else if ( btn == ShiftButton )
	    emit rightPressed( appLnk );
    }
}

void LauncherView::removeAllItems()
{
    icons->clear();
}

bool LauncherView::removeLink(const QString& linkfile)
{
    return icons->removeLink(linkfile);
}

void LauncherView::setSortEnabled( bool v )
{
    icons->setSorting( v );
    if ( v )
	sort();
}

void LauncherView::setUpdatesEnabled( bool u )
{
    icons->setUpdatesEnabled( u );
}

void LauncherView::sort()
{
    icons->sort();
#ifdef QTOPIA_PHONE
    if ( !icons->currentItem() )
	icons->setCurrentItem(icons->firstItem());
#endif
}

void LauncherView::addItem(AppLnk* app, bool resort)
{
    icons->addItem(app,resort);
}

void LauncherView::paletteChange( const QPalette &p )
{
    icons->unsetPalette();
    QVBox::paletteChange( p );
    if ( bgType == Ruled )
	setBackgroundType( Ruled, QString::null );
    QColorGroup cg = colorGroup();
    cg.setColor( QColorGroup::Text, textCol );
    icons->setPalette( QPalette(cg,cg,cg) );
}

void LauncherView::fontChange(const QFont&)
{
    icons->hideOrShowItems( FALSE );
}

void LauncherView::relayout(void)
{
    icons->hideOrShowItems(FALSE);
}

void LauncherView::flushBgCache()
{
    if ( !bgCache )
	return;
    // remove unreferenced backgrounds.
    QMap<QString,BgPixmap*>::Iterator it = bgCache->begin();
    while ( it != bgCache->end() ) {
	QMap<QString,BgPixmap*>::Iterator curr = it;
	++it;
	if ( (*curr)->ref == 0 ) {
	    delete (*curr);
	    bgCache->remove( curr );
	}
    }
}

void LauncherView::resetSelection()
{
    if (icons->firstItem()) {
	icons->setCurrentItem(icons->firstItem());
	icons->setContentsPos(0,0);
    }
}

QStringList LauncherView::mimeTypes() const
{
    return icons->mimeTypes();
}

const AppLnk *LauncherView::currentItem() const
{
    QIconViewItem* item = icons->currentItem();
    if (item)
	return ((LauncherItem *)item)->appLnk();
    return 0;
}

uint LauncherView::count() const
{
    return icons->count();
}

#include "launcherview.moc"
