/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qtopia/qpeglobal.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/resource.h>
#include <qtopia/applnk.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/mimetype.h>
#include <qtopia/storage.h>
#include <qtopia/categories.h>
#include <qtopia/services.h>
#include <qtopia/version.h>

#include <qdir.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <qtimer.h>
#include <qcombobox.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qmessagebox.h>
#include <qframe.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qpopupmenu.h>

#include "launcherview.h"
#include "launcher.h"
#include "syncdialog.h"
#include "desktop.h"
#include <qtopia/docproperties.h>
#include "mrulist.h"
#include "qrsync.h"
#include <stdlib.h>
#include <unistd.h>

#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <stdio.h>
#include <sys/vfs.h>
#include <mntent.h>
#endif

#ifdef Q_WS_QWS
#include <qkeyboard_qws.h>
#endif

//#define SHOW_ALL

class CategoryTab : public QTab
{
public:
    CategoryTab( const QIconSet &icon, const QString &text=QString::null )
	: QTab( icon, text )
    {
    }

    QColor bgColor;
    QColor fgColor;
};

//===========================================================================

CategoryTabWidget::CategoryTabWidget( QWidget* parent ) :
    QVBox( parent )
{
    categoryBar = 0;
    stack = 0;
}

void CategoryTabWidget::prevTab()
{
    if ( categoryBar ) {
	int n = categoryBar->count();
	int tab = categoryBar->currentTab();
	if ( tab >= 0 )
            categoryBar->setCurrentTab( (tab - 1 + n)%n );
    }
}

void CategoryTabWidget::nextTab()
{
    if ( categoryBar ) {
	int n = categoryBar->count();
        int tab = categoryBar->currentTab();
	categoryBar->setCurrentTab( (tab + 1)%n );
    }
}

void CategoryTabWidget::addItem( const QString& linkfile )
{
    int i=0;
    AppLnk *app = new AppLnk(linkfile);
    if ( !app->isValid() ) {
	delete app;
	app=0;
    }
    if ( !app || !app->file().isEmpty() ) {
	// A document
	delete app;
	app = new DocLnk(linkfile);
	if ( app->fileKnown() ) {
	    ((LauncherView*)(stack->widget(ids.count()-1)))->addItem(app);
	} else {
	    ((LauncherView*)(stack->widget(ids.count()-1)))->sort();
	    delete app;
	}
	return;
    }
    // An application
    for ( QStringList::Iterator it=ids.begin(); it!=ids.end(); ++it) {
	if ( !(*it).isEmpty() ) {
	    QRegExp tf(*it,FALSE,TRUE);
	    if ( tf.match(app->type()) >= 0 ) {
		((LauncherView*)stack->widget(i))->addItem(app);
		return;
	    }
	    i++;
	}
    }

    QCopEnvelope e("QPE/TaskBar","reloadApps()");
}

void CategoryTabWidget::initializeCategories(AppLnkSet* rootFolder,
	AppLnkSet* docFolder, const QList<FileSystem> &fs)
{
    delete categoryBar;
    categoryBar = new CategoryTabBar( this );
    QPalette pal = categoryBar->palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );

    delete stack;
    stack = new QWidgetStack(this);
    tabs=0;

    ids.clear();

    Config cfg("Launcher");

    QStringList types = rootFolder->types();
    for ( QStringList::Iterator ittypes=types.begin(); ittypes!=types.end(); ++ittypes) {
	if ( !(*ittypes).isEmpty() ) {
	    (void)newView(*ittypes,rootFolder->typePixmap(*ittypes),rootFolder->typeName(*ittypes));
	    setTabAppearance( *ittypes, cfg );
	}
    }
    QListIterator<AppLnk> itapp( rootFolder->children() );
    AppLnk* l;
    while ( (l=itapp.current()) ) {
	if ( l->type() == "Separator" ) { // No tr
	    rootFolder->remove(l);
	    delete l;
	} else {
	    int i=0;
	    for ( QStringList::Iterator itstring=types.begin(); itstring!=types.end(); ++itstring) {
		if ( *itstring == l->type() )
		    ((LauncherView*)stack->widget(i))->addItem(l,FALSE);
		i++;
	    }
	}
	++itapp;
    }
    rootFolder->detachChildren();
    for (int i=0; i<tabs; i++)
	((LauncherView*)stack->widget(i))->sort();

    // all documents
    QImage img( Resource::loadImage( "DocsIcon" ) );
    QPixmap pm;
    pm = img.smoothScale( AppLnk::smallIconSize(), AppLnk::smallIconSize() );
    docview = newView( "Documents", // No tr
	pm, tr("Documents"));
    docview->populate( docFolder, QString::null );
    docFolder->detachChildren();
    docview->setFileSystems(fs);
    docview->setToolsEnabled(TRUE);
    setTabAppearance( "Documents", cfg ); // No tr

    connect( categoryBar, SIGNAL(selected(int)), stack, SLOT(raiseWidget(int)) );

    ((LauncherView*)stack->widget(0))->setFocus();

    categoryBar->show();
    stack->show();
}

void CategoryTabWidget::setTabAppearance( const QString &id, Config &cfg )
{
    QString grp( "Tab %1" ); // No tr
    cfg.setGroup( grp.arg(id) );
    LauncherView *v = view( id );
    int idx = ids.findIndex( id );
    CategoryTab *tab = (CategoryTab *)categoryBar->tab( idx );

    // View
    QString view = cfg.readEntry( "View", "Icon" );
    if ( view == "List" ) // No tr
	v->setViewMode( LauncherView::List );
    QString bgType = cfg.readEntry( "BackgroundType", "Ruled" );
    if ( bgType == "Image" ) { // No tr
	QString pm = cfg.readEntry( "BackgroundImage", "wallpaper/marble" );
	v->setBackgroundType( LauncherView::Image, pm );
    } else if ( bgType == "SolidColor" ) {
	QString c = cfg.readEntry( "BackgroundColor" );
	v->setBackgroundType( LauncherView::SolidColor, c );
    }
    QString textCol = cfg.readEntry( "TextColor" );
    if ( textCol.isEmpty() )
	v->setTextColor( QColor() );
    else
	v->setTextColor( QColor(textCol) );
    bool customFont = cfg.readBoolEntry( "CustomFont", FALSE );
    if ( customFont ) {
	QStringList font = cfg.readListEntry( "Font", ',' );
	if ( font.count() == 4 )
	    v->setViewFont( QFont(font[0], font[1].toInt(), font[2].toInt(), font[3].toInt()!=0) );
    } else {
	v->clearViewFont();
    }

    // Tabs
    QString tabCol = cfg.readEntry( "TabColor" );
    if ( tabCol.isEmpty() )
	tab->bgColor = QColor();
    else
	tab->bgColor = QColor(tabCol);
    QString tabTextCol = cfg.readEntry( "TabTextColor" );
    if ( tabTextCol.isEmpty() )
	tab->fgColor = QColor();
    else
	tab->fgColor = QColor(tabTextCol);
}

void CategoryTabWidget::updateDocs(AppLnkSet* docFolder, const QList<FileSystem> &fs)
{
    docview->populate( docFolder, QString::null );
    docFolder->detachChildren();
    docview->setFileSystems(fs);
    docview->updateTools();
}

void CategoryTabWidget::tabProperties()
{
    LauncherView *view = (LauncherView*)stack->widget( categoryBar->currentTab() );
    QPopupMenu *m = new QPopupMenu( this );
    m->insertItem( tr("Icon View"), LauncherView::Icon );
    m->insertItem( tr("List View"), LauncherView::List );
    m->setItemChecked( (int)view->viewMode(), TRUE );
    int rv = m->exec( QCursor::pos() );
    if ( rv >= 0 && rv != view->viewMode() ) {
	view->setViewMode( (LauncherView::ViewMode)rv );
    }

    delete m;
}

QString CategoryTabWidget::getAllDocLinkInfo() const
{
    return docview->getAllDocLinkInfo();
}

LauncherView* CategoryTabWidget::newView( const QString& id, const QPixmap& pm, const QString& label )
{
    LauncherView* view = new LauncherView( stack );
    connect( view, SIGNAL(clicked(const AppLnk*)),
	    this, SIGNAL(clicked(const AppLnk*)));
    connect( view, SIGNAL(rightPressed(AppLnk*)),
	    this, SIGNAL(rightPressed(AppLnk*)));
    ids.append(id);
    categoryBar->addTab( new CategoryTab( pm, label ) );
    stack->addWidget( view, tabs++ );
    return view;
}

void CategoryTabWidget::updateLink(const QString& linkfile)
{
    int i=0;
    LauncherView* view;
    while ((view = (LauncherView*)stack->widget(i++))) {
	if ( view->removeLink(linkfile) )
	    break;
    }
    addItem(linkfile);
    docview->updateTools();
}

void CategoryTabWidget::paletteChange( const QPalette &p )
{
    QVBox::paletteChange( p );
    QPalette pal = palette();
    pal.setColor( QColorGroup::Light, pal.color(QPalette::Active,QColorGroup::Shadow) );
    pal.setColor( QColorGroup::Background, pal.active().background().light(110) );
    categoryBar->setPalette( pal );
    categoryBar->update();
}

void CategoryTabWidget::setBusy(bool on)
{
    if ( on )
	((LauncherView*)stack->visibleWidget())->setBusy(TRUE);
    else
	for (int i=0; i<tabs; i++)
	    ((LauncherView*)stack->widget(i))->setBusy(FALSE);
}

LauncherView *CategoryTabWidget::view( const QString &id )
{
    int idx = ids.findIndex( id );
    return (LauncherView *)stack->widget(idx);
}

//===========================================================================

CategoryTabBar::CategoryTabBar( QWidget *parent, const char *name )
    : QTabBar( parent, name )
{
    setFocusPolicy( NoFocus );
    connect( this, SIGNAL( selected(int) ), this, SLOT( layoutTabs() ) );
}

CategoryTabBar::~CategoryTabBar()
{
}

void CategoryTabBar::layoutTabs()
{
    if ( !count() )
	return;

//    int percentFalloffTable[] = { 100, 70, 40, 12, 6, 3, 1, 0 };
    int available = width()-1;
    QFontMetrics fm = fontMetrics();
    int hiddenTabWidth = -12;
    int middleTab = currentTab();
    int hframe, vframe, overlap;
    style().tabbarMetrics( this, hframe, vframe, overlap );
    int x = 0;
    QRect r;
    QTab *t;
    int required = 0;
    int eventabwidth = (width()-1)/count();
    int i;
    enum Mode { HideBackText, Pack, Even } mode=Even;
    for ( i = 0; i < count(); i++ ) {
	t = tab(i);
	int iw = fm.width( t->text() ) + hframe - overlap;
	if ( i != middleTab ) {
	    available -= hiddenTabWidth + hframe - overlap;
	    if ( t->iconSet() != 0 )
		available -= t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
	}
	if ( t->iconSet() != 0 )
	    iw += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
	required += iw;
	// As space gets tight, packed looks better than even. "10" must be at least 0.
	if ( iw >= eventabwidth-10 )
	    mode = Pack;
    }
    if ( mode == Pack && required > width()-1 )
	mode = HideBackText;
    for ( i = 0; i < count(); i++ ) {
	t = tab(i);
	if ( mode != HideBackText ) {
	    int w = fm.width( t->text() );
	    int ih = 0;
	    if ( t->iconSet() != 0 ) {
		w += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
		ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int h = QMAX( fm.height(), ih );
	    h = QMAX( h, QApplication::globalStrut().height() );

	    h += vframe;
	    w += hframe;

	    QRect tr(x, 0,
		mode == Even ? eventabwidth : w * (width()-1)/required, h);
	    t->setRect(tr);
	    x += tr.width() - overlap;
	    r = r.unite(tr);
	} else if ( i != middleTab ) {
	    int w = hiddenTabWidth;
	    int ih = 0;
	    if ( t->iconSet() != 0 ) {
		w += t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width();
		ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int h = QMAX( fm.height(), ih );
	    h = QMAX( h, QApplication::globalStrut().height() );

	    h += vframe;
	    w += hframe;

	    t->setRect( QRect(x, 0, w, h) );
	    x += t->rect().width() - overlap;
	    r = r.unite( t->rect() );
	} else {
	    int ih = 0;
	    if ( t->iconSet() != 0 ) {
		ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int h = QMAX( fm.height(), ih );
	    h = QMAX( h, QApplication::globalStrut().height() );

	    h += vframe;

	    t->setRect( QRect(x, 0, available, h) );
	    x += t->rect().width() - overlap;
	    r = r.unite( t->rect() );
	}
    }

    QRect rr = tab(count()-1)->rect();
    rr.setRight(width()-1);
    tab(count()-1)->setRect( rr );

    for ( t = tabList()->first(); t; t = tabList()->next() ) {
       QRect tr = t->rect();
       tr.setHeight( r.height() );
       t->setRect( tr );
    }

    update();
}


void CategoryTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
    CategoryTabBar *that = (CategoryTabBar *) this;
    CategoryTab *ct = (CategoryTab *)t;
    QPalette pal = palette();
    bool setPal = FALSE;
    if ( ct->bgColor.isValid() ) {
	pal.setColor( QPalette::Active, QColorGroup::Background, ct->bgColor );
	pal.setColor( QPalette::Active, QColorGroup::Button, ct->bgColor );
	pal.setColor( QPalette::Inactive, QColorGroup::Background, ct->bgColor );
	pal.setColor( QPalette::Inactive, QColorGroup::Button, ct->bgColor );
	that->setUpdatesEnabled( FALSE );
	that->setPalette( pal );
	setPal = TRUE;
    }
#if QT_VERSION >= 300
    QStyle::SFlags flags = QStyle::Style_Default;
    if ( selected )
        flags |= QStyle::Style_Selected;
    style().drawControl( QStyle::CE_TabBarTab, p, this, t->rect(),
                         colorGroup(), flags, QStyleOption(t) );
#else
    style().drawTab( p, this, t, selected );
#endif

    QRect r( t->rect() );
    QFont f( font() );
    if ( selected )
	f.setBold( TRUE );
    p->setFont( f );

    if ( ct->fgColor.isValid() ) {
	pal.setColor( QPalette::Active, QColorGroup::Foreground, ct->fgColor );
	pal.setColor( QPalette::Inactive, QColorGroup::Foreground, ct->fgColor );
	that->setUpdatesEnabled( FALSE );
	that->setPalette( pal );
	setPal = TRUE;
    }
    int iw = 0;
    int ih = 0;
    if ( t->iconSet() != 0 ) {
	iw = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
	ih = t->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height();
    }
    int w = iw + p->fontMetrics().width( t->text() ) + 4;
    int h = QMAX(p->fontMetrics().height() + 4, ih );
    paintLabel( p, QRect( r.left() + (r.width()-w)/2 - 3,
			  r.top() + (r.height()-h)/2, w, h ), t,
#if QT_VERSION >= 300
			    t->identifier() == keyboardFocusTab()
#else
			    t->identitifer() == keyboardFocusTab()
#endif
		);
    if ( setPal ) {
	that->unsetPalette();
	that->setUpdatesEnabled( TRUE );
    }
}


void CategoryTabBar::paintLabel( QPainter* p, const QRect&,
			  QTab* t, bool has_focus ) const
{
    QRect r = t->rect();
    //    if ( t->id != currentTab() )
    //r.moveBy( 1, 1 );
    //
    if ( t->iconSet() ) {
	// the tab has an iconset, draw it in the right mode
	QIconSet::Mode mode = (t->isEnabled() && isEnabled()) ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && has_focus )
	    mode = QIconSet::Active;
	QPixmap pixmap = t->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	p->drawPixmap( r.left() + 6, r.center().y() - pixh / 2 + 1, pixmap );
	r.setLeft( r.left() + pixw + 5 );
    }

    QRect tr = r;

    if ( r.width() < 20 )
	return;

    if ( t->isEnabled() && isEnabled()  ) {
#if defined(_WS_WIN32_)
	if ( colorGroup().brush( QColorGroup::Button ) == colorGroup().brush( QColorGroup::Background ) )
	    p->setPen( colorGroup().buttonText() );
	else
	    p->setPen( colorGroup().foreground() );
#else
	p->setPen( colorGroup().foreground() );
#endif
	p->drawText( tr, AlignCenter | AlignVCenter | ShowPrefix, t->text() );
    } else {
	p->setPen( palette().disabled().foreground() );
	p->drawText( tr, AlignCenter | AlignVCenter | ShowPrefix, t->text() );
    }
}

//---------------------------------------------------------------------------

Launcher::Launcher( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    setCaption( tr("Launcher") );

    syncDialog = 0;

    // we have a pretty good idea how big we'll be
    setGeometry( 0, 0, qApp->desktop()->width(), qApp->desktop()->height() );

    tabs = 0;
    rootFolder = 0;
    docsFolder = 0;

    tabs = new CategoryTabWidget( this );
    setCentralWidget( tabs );

    connect( tabs, SIGNAL(selected(const QString&)),
	this, SLOT(viewSelected(const QString&)) );
    connect( tabs, SIGNAL(clicked(const AppLnk*)),
	this, SLOT(select(const AppLnk*)));
    connect( tabs, SIGNAL(rightPressed(AppLnk*)),
	this, SLOT(properties(AppLnk*)));

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
    QCopChannel* sysChannel = new QCopChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(const QCString &, const QByteArray &)),
             this, SLOT(systemMessage( const QCString &, const QByteArray &)) );
    QCopChannel *channel = new QCopChannel( "QPE/Launcher", this );
    connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
             this, SLOT(launcherMessage(const QCString&, const QByteArray&)) );
#endif

    storage = new StorageInfo( this );
    connect( storage, SIGNAL( disksChanged() ), SLOT( storageChanged() ) );

    updateTabs();

    preloadApps();

    in_lnk_props = FALSE;
    got_lnk_change = FALSE;
}

Launcher::~Launcher()
{
    delete rootFolder;
    delete docsFolder;
}

static bool isVisibleWindow(int wid)
{
#ifdef QWS
    const QList<QWSWindow> &list = qwsServer->clientWindows();
    QWSWindow* w;
    for (QListIterator<QWSWindow> it(list); (w=it.current()); ++it) {
	if ( w->winId() == wid )
	    return !w->isFullyObscured();
    }
#endif
    return FALSE;
}

void Launcher::showMaximized()
{
    if ( isVisibleWindow( winId() ) )
	doMaximize();
    else
	QTimer::singleShot( 20, this, SLOT(doMaximize()) );
}

void Launcher::doMaximize()
{
    QMainWindow::showMaximized();
    tabs->setMaximumWidth( qApp->desktop()->width() );
}

void Launcher::updateMimeTypes()
{
    MimeType::clear();
    updateMimeTypes(rootFolder);
}

void Launcher::updateMimeTypes(AppLnkSet* folder)
{
    for ( QListIterator<AppLnk> it( folder->children() ); it.current(); ++it ) {
	AppLnk *app = it.current();
	if ( app->type() == "Folder" ) // No tr
	    updateMimeTypes((AppLnkSet *)app);
	else {
	    MimeType::registerApp(*app);
	}
    }
}

void Launcher::loadDocs()
{
    delete docsFolder;
    docsFolder = new DocLnkSet;
    Global::findDocuments(docsFolder);
}

void Launcher::updateTabs()
{
    MimeType::updateApplications(); // ### reads all applnks twice

    delete rootFolder;
    rootFolder = new AppLnkSet( MimeType::appsFolderName() );

    loadDocs();

    tabs->initializeCategories(rootFolder, docsFolder, storage->fileSystems());
}

void Launcher::updateDocs()
{
    loadDocs();
    tabs->updateDocs(docsFolder,storage->fileSystems());
}

void Launcher::viewSelected(const QString& s)
{
    setCaption( s + tr(" - Launcher") );
}

void Launcher::nextView()
{
    tabs->nextTab();
}


void Launcher::select( const AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
	// Not supported: flat is simpler for the user
    } else {
	if ( appLnk->exec().isNull() ) {
	    int i = QMessageBox::information(this,tr("No application"),
		tr("<p>No application is defined for this document."
		"<p>Type is %1.").arg(appLnk->type()), tr("OK"), tr("View as text"), 0, 0, 1);
	    
	    if ( i == 1 ) {
		QCopEnvelope e(Service::channel("Open/text/*"), "setDocument(QString)" );
		e << appLnk->file();
	    }
	    
	    return;
	}
	tabs->setBusy(TRUE);
	emit executing( appLnk );
	appLnk->execute();
    }
}

void Launcher::properties( AppLnk *appLnk )
{
    if ( appLnk->type() == "Folder" ) { // No tr
	// Not supported: flat is simpler for the user
    } else {
	in_lnk_props = TRUE;
	got_lnk_change = FALSE;
	DocPropertiesDialog prop(appLnk);
	prop.showMaximized();
	prop.exec();
	in_lnk_props = FALSE;
	if ( got_lnk_change ) {
	    updateLink(lnk_change);
	}
    }
}

void Launcher::updateLink(const QString& link)
{
    if (link.isNull())
	updateTabs();
    else if (link.isEmpty())
	updateDocs();
    else
	tabs->updateLink(link);
}


#if 1 //### quick demo hack
typedef struct KeyOverride {
    ushort scan_code;
    QWSServer::KeyMap map;
};


static const KeyOverride jp109keys[] = {
   { 0x03, {   Qt::Key_2,      '2'     , 0x22     , 0x0000  } },
   { 0x07, {   Qt::Key_6,      '6'     , '&'     , 0x0000  } },
   { 0x08, {   Qt::Key_7,      '7'     , '\''     , 0x0000  } },
   { 0x09, {   Qt::Key_8,      '8'     , '('     , 0x0000  } },
   { 0x0a, {   Qt::Key_9,      '9'     , ')'     , 0x0000  } },
   { 0x0b, {   Qt::Key_0,      '0'     , 0x0000  , 0x0000  } },
   { 0x0c, {   Qt::Key_Minus,      '-'     , '='     , 0x0000  } },
   { 0x0d, {   Qt::Key_AsciiCircum,'^'     , '~'     , '^' - 64  } },
   { 0x1a, {   Qt::Key_At,     '@'     , '`'     , 0x0000  } },
   { 0x1b, {   Qt::Key_BraceLeft, '['     , '{'  , '[' - 64  } },
   { 0x27, {   Qt::Key_Semicolon,  ';'     , '+'     , 0x0000  } },
   { 0x28, {   Qt::Key_Colon,  ':'    ,  '*'     , 0x0000  } },
   { 0x29, {   Qt::Key_Zenkaku_Hankaku,  0x0000  , 0x0000     , 0x0000  } },
   { 0x2b, {   Qt::Key_BraceRight,  ']'    , '}'     , ']'-64  } },
   { 0x70, {   Qt::Key_Hiragana_Katakana,    0x0000  , 0x0000  , 0x0000  } },
   { 0x73, {   Qt::Key_Backslash,  '\\'    , '_'  ,    0x0000  } },
   { 0x79, {   Qt::Key_Henkan,     0x0000  , 0x0000  , 0x0000  } },
   { 0x7b, {   Qt::Key_Muhenkan,   0x0000  , 0x0000  , 0x0000  } },
   { 0x7d, {   Qt::Key_yen,        0x00a5  , '|'     , 0x0000  } },
   { 0x00, {   0,          0x0000  , 0x0000  , 0x0000  } }
};

static void setJP109Keys() 
{
    QIntDict<QWSServer::KeyMap> *om = new QIntDict<QWSServer::KeyMap>(37);
    const KeyOverride *k = jp109keys;
    while ( k->scan_code ) {
	om->insert( k->scan_code, &k->map ); 
	k++;
    }
    QWSServer::setOverrideKeys( om );
}

#endif

void Launcher::systemMessage( const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "linkChanged(QString)" ) {
	QString link;
	stream >> link;
	if ( in_lnk_props ) {
	    got_lnk_change = TRUE;
	    lnk_change = link;
	} else {
	    updateLink(link);
	}
    } else if ( msg == "busy()" ) {
	emit busy();
    } else if ( msg == "notBusy(QString)" ) {
	QString app;
	stream >> app;
	tabs->setBusy(FALSE);
	emit notBusy(app);
    } else if ( msg == "mkdir(QString)" ) {
	QString dir;
	stream >> dir;
	if ( !dir.isEmpty() )
	    mkdir( dir );
    } else if ( msg == "rdiffGenSig(QString,QString)" ) {
	QString baseFile, sigFile;
	stream >> baseFile >> sigFile;
	QRsync::generateSignature( baseFile, sigFile );
    } else if ( msg == "rdiffGenDiff(QString,QString,QString)" ) {
	QString baseFile, sigFile, deltaFile;
	stream >> baseFile >> sigFile >> deltaFile;
	QRsync::generateDiff( baseFile, sigFile, deltaFile );
    } else if ( msg == "rdiffApplyPatch(QString,QString)" ) {
	QString baseFile, deltaFile;
	stream >> baseFile >> deltaFile;
	if ( !QFile::exists( baseFile ) ) {
	    QFile f( baseFile );
	    f.open( IO_WriteOnly );
	    f.close();
	}
	QRsync::applyDiff( baseFile, deltaFile );
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "patchApplied(QString)" );
	e << baseFile;
#endif
    } else if ( msg == "rdiffCleanup()" ) {
	mkdir( "/tmp/rdiff" );
	QDir dir;
	dir.setPath( "/tmp/rdiff" );
	QStringList entries = dir.entryList();
	for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
	    dir.remove( *it );
    } else if ( msg == "sendHandshakeInfo()" ) {
	QString home = getenv( "HOME" );
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "handshakeInfo(QString,bool)" );
	e << home;
	int locked = (int) Desktop::screenLocked();
	e << locked;
#endif

    } else if ( msg == "sendVersionInfo()" ) {
	QCopEnvelope e( "QPE/Desktop", "versionInfo(QString)" );
	QString v = QPE_VERSION;
	QStringList l = QStringList::split( '.', v );
	QString v2 = l[0] + '.' + l[1];
	e << v2;
	//qDebug("version %s\n", line.latin1());
    } else if ( msg == "sendCardInfo()" ) {
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "cardInfo(QString)" );
#endif
	const QList<FileSystem> &fs = storage->fileSystems();
	QListIterator<FileSystem> it ( fs );
	QString s;
	QString homeDir = getenv("HOME");
	QString hardDiskHome, hardDiskHomePath;
	for ( ; it.current(); ++it ) {
	    int k4 = (*it)->blockSize()/256;
	    if ( (*it)->isRemovable() ) {
		s += (*it)->name() + "=" + (*it)->path() + "/Documents "
		     + QString::number( (*it)->availBlocks() * k4/4 )
		     + "K " + (*it)->options() + ";";
	    } else if ( (*it)->disk() == "/dev/mtdblock1" ||
		      (*it)->disk() == "/dev/mtdblock/1" ) {
		s += (*it)->name() + "=" + homeDir + "/Documents "
		     + QString::number( (*it)->availBlocks() * k4/4 )
		     + "K " + (*it)->options() + ";";
	    } else if ( (*it)->name().contains( "Hard Disk") &&
		      homeDir.contains( (*it)->path() ) &&
		      (*it)->path().length() > hardDiskHomePath.length() ) {
		hardDiskHomePath = (*it)->path();
		hardDiskHome =
		    (*it)->name() + "=" + homeDir + "/Documents "
		    + QString::number( (*it)->availBlocks() * k4/4 )
		    + "K " + (*it)->options() + ";";
	    }
	}
	if ( !hardDiskHome.isEmpty() )
	    s += hardDiskHome;

#ifndef QT_NO_COP
	e << s;
#endif
    } else if ( msg == "sendSyncDate(QString)" ) {
	QString app;
	stream >> app;
	Config cfg( "qpe" );
	cfg.setGroup("SyncDate");
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "syncDate(QString,QString)" );
	e  << app  << cfg.readEntry( app );
#endif
	//qDebug("QPE/System sendSyncDate for %s: response %s", app.latin1(),
	//cfg.readEntry( app ).latin1() );
    } else if ( msg == "setSyncDate(QString,QString)" ) {
	QString app, date;
	stream >> app >> date;
	Config cfg( "qpe" );
	cfg.setGroup("SyncDate");
	cfg.writeEntry( app, date );
	//qDebug("setSyncDate(QString,QString) %s %s", app.latin1(), date.latin1());
    } else if ( msg == "startSync(QString)" ) {
	QString what;
	stream >> what;
	delete syncDialog; syncDialog = 0;
	syncDialog = new SyncDialog( this, "syncProgress", FALSE,
				     WStyle_Tool | WStyle_Customize |
				     Qt::WStyle_StaysOnTop );
	syncDialog->showMaximized();
	syncDialog->whatLabel->setText( "<b>" + what + "</b>" );
	connect( syncDialog->buttonCancel, SIGNAL( clicked() ),
		 SLOT( cancelSync() ) );	
    } else if ( msg == "stopSync()") {
	delete syncDialog; syncDialog = 0;
    } else if ( msg == "getAllDocLinks()" ) {
	loadDocs();

	QString contents;

	Categories cats;
	for ( QListIterator<DocLnk> it( docsFolder->children() ); it.current(); ++it ) {
	    DocLnk *doc = it.current();
	    QFileInfo fi( doc->file() );
	    if ( !fi.exists() )
		continue;

	    bool fake = !doc->linkFileKnown();
	    if ( !fake ) {
		QFile f( doc->linkFile() );
		if ( f.open( IO_ReadOnly ) ) {
		    QTextStream ts( &f );
		    ts.setEncoding( QTextStream::UnicodeUTF8 );
		    contents += ts.read();
		    f.close();
		} else
		    fake = TRUE;
	    }
	    if (fake) {
		contents += "[Desktop Entry]\n";
		contents += "Categories = " + // No tr
		    cats.labels("Document View",doc->categories()).join(";") + "\n"; // No tr
		contents += "Name = "+doc->name()+"\n"; // No tr
		contents += "Type = "+doc->type()+"\n"; // No tr
	    }
	    contents += "File = "+doc->file()+"\n"; // No tr // (resolves path)
	    contents += QString("Size = %1\n").arg( fi.size() ); // No tr
	}

	//qDebug( "sending length %d", contents.length() );
#ifndef QT_NO_COP
	QCopEnvelope e( "QPE/Desktop", "docLinks(QString)" );
	e << contents;
#endif
	
 	//qDebug( "================ \n\n%s\n\n===============",
	//contents.latin1() );

	delete docsFolder;
	docsFolder = 0;
#ifdef QWS
    } else if ( msg == "setMouseProto(QString)" ) {
	QString mice;
	stream >> mice;
	setenv("QWS_MOUSE_PROTO",mice.latin1(),1);
	qwsServer->openMouse();
    } else if ( msg == "setKeyboard(QString)" ) {
	QString kb;
	stream >> kb;
	setenv("QWS_KEYBOARD",kb.latin1(),1);
	qwsServer->openKeyboard();

    } else if ( msg == "setKeyboardAutoRepeat(int,int)" ) {
	int delay, period;
	stream >> delay >> period;
	qwsSetKeyboardAutoRepeat( delay, period );
	Config cfg( "qpe" );
	cfg.setGroup("Keyboard");
	cfg.writeEntry( "RepeatDelay", delay );
	cfg.writeEntry( "RepeatPeriod", period );
    } else if ( msg == "setKeyboardLayout(QString)" ) {
	QString kb;
	stream >> kb;
	//### quick demo hack that we can extend later 
	if ( kb == "us101" )
	    QWSServer::setOverrideKeys( 0 );
	else
	    setJP109Keys();
#endif
    }
}



void Launcher::cancelSync()
{
#ifndef QT_NO_COP
    QCopEnvelope e( "QPE/Desktop", "cancelSync()" );
#endif
}

void Launcher::launcherMessage( const QCString &msg, const QByteArray &data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "setTabView(QString,int)" ) {
	QString id;
	stream >> id;
	int mode;
	stream >> mode;
	if ( tabs->view(id) )
	    tabs->view(id)->setViewMode( (LauncherView::ViewMode)mode );
    } else if ( msg == "setTabBackground(QString,int,QString)" ) {
	QString id;
	stream >> id;
	int mode;
	stream >> mode;
	QString pixmapOrColor;
	stream >> pixmapOrColor;
	if ( tabs->view(id) )
	    tabs->view(id)->setBackgroundType( (LauncherView::BackgroundType)mode, pixmapOrColor );
    } else if ( msg == "setTextColor(QString,QString)" ) {
	QString id;
	stream >> id;
	QString color;
	stream >> color;
	if ( tabs->view(id) )
	    tabs->view(id)->setTextColor( QColor(color) );
    } else if ( msg == "setFont(QString,QString,int,int,int)" ) {
	QString id;
	stream >> id;
	QString fam;
	stream >> fam;
	int size;
	stream >> size;
	int weight;
	stream >> weight;
	int italic;
	stream >> italic;
	if ( tabs->view(id) ) {
	    if ( !fam.isEmpty() ) {
		tabs->view(id)->setViewFont( QFont(fam, size, weight, italic!=0) );
		qDebug( "setFont: %s, %d, %d, %d", fam.latin1(), size, weight, italic );
	    } else {
		tabs->view(id)->clearViewFont();
	    }
	}
    }
}

void Launcher::storageChanged()
{
    if ( in_lnk_props ) {
	got_lnk_change = TRUE;
	lnk_change = QString::null;
    } else {
	updateLink( QString::null );
    }
}


bool Launcher::mkdir(const QString &localPath)
{
    QDir fullDir(localPath);
    if (fullDir.exists())
	return true;

    // at this point the directory doesn't exist
    // go through the directory tree and start creating the direcotories
    // that don't exist; if we can't create the directories, return false

    QString dirSeps = "/";
    int dirIndex = localPath.find(dirSeps);
    QString checkedPath;

    // didn't find any seps; weird, use the cur dir instead
    if (dirIndex == -1) {
	//qDebug("No seperators found in path %s", localPath.latin1());
	checkedPath = QDir::currentDirPath();
    }

    while (checkedPath != localPath) {
	// no more seperators found, use the local path
	if (dirIndex == -1)
	    checkedPath = localPath;
	else {
	    // the next directory to check
	    checkedPath = localPath.left(dirIndex) + "/";
	    // advance the iterator; the next dir seperator
	    dirIndex = localPath.find(dirSeps, dirIndex+1);
	}

	QDir checkDir(checkedPath);
	if (!checkDir.exists()) {
	    //qDebug("mkdir making dir %s", checkedPath.latin1());

	    if (!checkDir.mkdir(checkedPath)) {
		qDebug("Unable to make directory %s", checkedPath.latin1());
		return FALSE;
	    }
	}

    }
    return TRUE;
}

void Launcher::preloadApps()
{
    Config cfg("Launcher");
    cfg.setGroup("Preload");
    QStringList apps = cfg.readListEntry("Apps",',');
    for (QStringList::ConstIterator it=apps.begin(); it!=apps.end(); ++it) {
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/Application/"+(*it).local8Bit(), "enablePreload()");
#endif
    }
}
