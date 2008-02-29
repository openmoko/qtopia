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

#include "appearance.h"
#include <qtopia/global.h>
#include <qtopia/fontdatabase.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/pluginloader.h>
#include <qtopia/qpestyle.h>
#include <qtopia/styleinterface.h>
#include <qtopia/windowdecorationinterface.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qgroupbox.h>
#include <qwindowsstyle.h>
#include <qobjectlist.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpixmapcache.h>
#if QT_VERSION >= 300
#include <qstylefactory.h>
#endif

//#if defined(QT_QWS_IPAQ) || defined(QT_QWS_SL5XXX)
//#include <unistd.h>
//#include <linux/fb.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#endif
#include <stdlib.h>

#define SAMPLE_HEIGHT	115

class PluginItem : public QListBoxText
{
public:
    PluginItem( QListBox *lb, const QString &t ) : QListBoxText( lb, t ) {}
    void setFilename( const QString fn ) { file = fn; }
    const QString &filename() const { return file; }

private:
    QString file;
};

class SampleText : public QWidget
{
public:
    SampleText( const QString &t, bool h, QWidget *parent )
	: QWidget( parent ), hl(h), text(t)
    {
	if ( hl )
	    setBackgroundMode( PaletteHighlight );
	else
	    setBackgroundMode( PaletteBase );
    }

    QSize sizeHint() const
    {
	QFontMetrics fm(font());
	return QSize( fm.width(text)+10, fm.height()+4 );
    }

    void paintEvent( QPaintEvent * )
    {
	QPainter p(this);
	if ( hl )
	    p.setPen( colorGroup().highlightedText() );
	else
	    p.setPen( colorGroup().text() );
	p.drawText( rect(), AlignCenter, text );
    }

private:
    bool hl;
    QString text;
};

class SampleWindow : public QWidget
{
    Q_OBJECT
public:
    SampleWindow( QWidget *parent ) : QWidget(parent), iface(0)
    {
	setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum) );
	init();
    }

    QSize sizeHint() const
    {
	int w = container->sizeHint().width() + 10;
	int h = container->sizeHint().height() + 30;
	return QSize( w, QMAX(SAMPLE_HEIGHT,h) );
    }

    void setFont( const QFont &f )
    {
	QWidget::setFont( f );
	popup->setFont( f );
    }

    void setDecoration( WindowDecorationInterface *i )
    {
	iface = i;
	wd.rect = QRect( 0, 0, 150, 75 );
	wd.caption = tr("Sample");
	wd.palette = palette();
	wd.flags = WindowDecorationInterface::WindowData::Dialog |
		   WindowDecorationInterface::WindowData::Active;
	wd.reserved = 1;

	th = iface->metric(WindowDecorationInterface::TitleHeight, &wd);
	tb = iface->metric(WindowDecorationInterface::TopBorder, &wd);
	lb = iface->metric(WindowDecorationInterface::LeftBorder, &wd);
	rb = iface->metric(WindowDecorationInterface::RightBorder, &wd);
	bb = iface->metric(WindowDecorationInterface::BottomBorder, &wd);

	int yoff = th + tb;
	int xoff = lb;

	wd.rect.setX( 0 );
	wd.rect.setWidth( width() - lb - rb );
	wd.rect.setY( 0 );
	wd.rect.setHeight( height() - yoff - bb );

	container->setGeometry( xoff, yoff, wd.rect.width(), wd.rect.height() );
	setMinimumSize( container->sizeHint().width()+lb+rb,
			QMAX(SAMPLE_HEIGHT,container->sizeHint().height()+tb+th+bb) );
    }

    void paintEvent( QPaintEvent * )
    {
	if ( !iface )
	    return;

	QPainter p( this );

	p.translate( lb, th+tb );

	iface->drawArea(WindowDecorationInterface::Border, &p, &wd);
	iface->drawArea(WindowDecorationInterface::Title, &p, &wd);

	p.setPen(palette().active().color(QColorGroup::HighlightedText));
	QFont f( font() );
	f.setWeight( QFont::Bold );
	p.setFont(f);
	iface->drawArea(WindowDecorationInterface::TitleText, &p, &wd);

	QRect brect( 0, -th, iface->metric(WindowDecorationInterface::HelpWidth,&wd), th );
	iface->drawButton( WindowDecorationInterface::Help, &p, &wd,
	    brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
	brect.moveBy( wd.rect.width() -
	    iface->metric(WindowDecorationInterface::OKWidth,&wd) -
	    iface->metric(WindowDecorationInterface::CloseWidth,&wd), 0 );
	iface->drawButton( WindowDecorationInterface::Close, &p, &wd,
	    brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
	brect.moveBy( iface->metric(WindowDecorationInterface::CloseWidth,&wd), 0 );
	iface->drawButton( WindowDecorationInterface::OK, &p, &wd,
	    brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)0 );
    }

    void init()
    {
	container = new QVBox( this );
	popup = new QPopupMenu( this );
	popup->insertItem( tr("Normal Item"), 1 );
	popup->insertItem( tr("Disabled Item"), 2 );
	popup->setItemEnabled(2, FALSE);
	QMenuBar *mb = new QMenuBar( container );
	mb->insertItem( tr("Menu"), popup );
	QHBox *hb = new QHBox( container );
	QWidget *w = new QWidget( hb );
	(void)new QScrollBar( 0, 0, 0, 0, 0, Vertical, hb );

	QGridLayout *gl = new QGridLayout( w, 2, 2, 4 );
	SampleText *l = new SampleText( tr("Normal Text"), FALSE, w );
	gl->addWidget( l, 0, 0 );

	l = new SampleText( tr("Highlighted Text"), TRUE, w );
	gl->addWidget( l, 1, 0 );

	QPushButton *pb = new QPushButton( tr("Button"), w );
	gl->addWidget( pb, 0, 1 );
	pb->setFocusPolicy( NoFocus );

	QCheckBox *cb = new QCheckBox( tr("Check Box"), w );
	gl->addWidget( cb, 1, 1 );
	cb->setFocusPolicy( NoFocus );
	cb->setChecked( TRUE );

	QWhatsThis::add( this, tr("Sample window using the selected settings.") );
    }

    bool eventFilter( QObject *, QEvent *e )
    {
	switch ( e->type() ) {
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonRelease:
	    case QEvent::MouseButtonDblClick:
	    case QEvent::MouseMove:
	    case QEvent::KeyPress:
	    case QEvent::KeyRelease:
		return TRUE;
	    default:
		break;
	}

	return FALSE;
    }

    void paletteChange( const QPalette &old )
    {
	QWidget::paletteChange( old );
	wd.palette = palette();
	popup->setPalette( palette() );
    }

    void resizeEvent( QResizeEvent *re )
    {
	wd.rect.setX( 0 );
	wd.rect.setWidth( width() - lb - rb );
	wd.rect.setY( 0 );
	wd.rect.setHeight( height() - th - tb - bb );

	container->setGeometry( lb, th+tb, wd.rect.width(), wd.rect.height() );
	QWidget::resizeEvent( re );
    }

    void setUpdatesEnabled( bool e ) {
	QWidget::setUpdatesEnabled( e );
	const QObjectList *ol = children();
	if ( ol) {
	    QObjectListIt it( *ol );
	    for ( ; it.current(); ++it ) {
		QObject *o = *it;
		if( o->isWidgetType() ) {
		    ((QWidget *)o)->setUpdatesEnabled( e );
		}
	    }
	}
    }

public:
    void fixGeometry()
    {
	int w = container->sizeHint().width()+lb+rb;
	int h = container->sizeHint().height()+tb+th+bb;
	setMinimumSize( w, QMAX(SAMPLE_HEIGHT,h) );
    }

protected:
    WindowDecorationInterface *iface;
    WindowDecorationInterface::WindowData wd;
    QVBox *container;
    QPopupMenu *popup;
    int th;
    int tb;
    int lb;
    int rb;
    int bb;
};

//===========================================================================

class DefaultWindowDecoration : public WindowDecorationInterface
{
public:
    DefaultWindowDecoration() : ref(0) {}
    QString name() const {
	return "Default";
    }
    QPixmap icon() const {
	return QPixmap();
    }
    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface ) {
	*iface = 0;
	if ( uuid == IID_QUnknown )
	    *iface = this;
	else if ( uuid == IID_WindowDecoration )
	    *iface = this;

	if ( *iface )
	    (*iface)->addRef();
	return QS_OK;
    }
    Q_REFCOUNT

private:
	ulong ref;
};

//===========================================================================

AppearanceSettings::AppearanceSettings( QWidget* parent,  const char* name, WFlags fl )
    : AppearanceSettingsBase( parent, name, TRUE, fl )
{
    sample = new SampleWindow( this );
    AppearanceSettingsBaseLayout->addWidget( sample );

    wdiface = 0;
    styleiface = 0;
    wdLoader = new PluginLoader( "decorations" );
    styleLoader = new PluginLoader( "styles" );
    wdIsPlugin = FALSE;

    maxFontSize = qApp->desktop()->width() >= 640 ? 14 : 12;

    populateStyleList();
    populateColorList();
    populateDecorationList();

    Config config("qpe");
    config.setGroup( "Appearance" );
    QString s = config.readEntry( "Style", "Qtopia" );
    unsigned i;
    if ( s == "QPE" ) s = "Qtopia";
    for ( i = 0; i < styleList->count(); i++ ) {
	PluginItem *item = (PluginItem*)styleList->item(i);
	if ( item->filename() == s || item->text() == s ) {
	    styleList->setCurrentItem( i );
	    break;
	}
    }
    connect( styleList, SIGNAL(highlighted(int)),
	this, SLOT(styleSelected(int)) );

    s = config.readEntry( "Scheme", "Desert" );
    connect( colorList, SIGNAL(highlighted(int)),
	this, SLOT(colorSelected(int)) );
    colorList->setCurrentItem( colorListIDs.findIndex(s) );

    s = config.readEntry( "Decoration" );
    for ( i = 0; i < decorationList->count(); i++ ) {
	PluginItem *item = (PluginItem*)decorationList->item(i);
	if ( item->filename() == s || item->text() == s ) {
	    decorationList->setCurrentItem( i );
	    break;
	}
    }
    connect( decorationList, SIGNAL(highlighted(int)),
	this, SLOT(decorationSelected(int)) );
    decorationSelected( decorationList->currentItem() );

    s = config.readEntry( "FontFamily", "Helvetica" );
    prefFontSize = config.readNumEntry( "FontSize", font().pointSize() );
    populateFontList(s,prefFontSize);
    connect( fontList, SIGNAL(highlighted(const QString&)),
	this, SLOT(fontSelected(const QString&)) );
    connect( fontSize, SIGNAL(activated(const QString&)),
	this, SLOT(fontSizeSelected(const QString&)) );
}

AppearanceSettings::~AppearanceSettings()
{
    delete styleLoader;
    delete wdLoader;
}

void AppearanceSettings::accept()
{
    Config config("qpe");
    config.setGroup( "Appearance" );

    PluginItem *item = (PluginItem *)styleList->item( styleList->currentItem() );
    QString s = item->filename().isEmpty() ? item->text() : item->filename();
    config.writeEntry( "Style", s );

    item = (PluginItem *)decorationList->item( decorationList->currentItem() );
    s = item->filename().isEmpty() ? item->text() : item->filename();
    config.writeEntry( "Decoration", s );

    s = colorListIDs[colorList->currentItem()];
    config.writeEntry( "Scheme", s );

    Config scheme( QPEApplication::qpeDir() + "etc/colors/" + s + ".scheme",
        Config::File );

    scheme.setGroup("Colors");
    QString color = scheme.readEntry( "Background", "#E5E1D5" );
    config.writeEntry( "Background", color );
    color = scheme.readEntry( "Button", "#D6CDBB" );
    config.writeEntry( "Button", color );
    color = scheme.readEntry( "Highlight", "#800000" );
    config.writeEntry( "Highlight", color );
    color = scheme.readEntry( "HighlightedText", "#FFFFFF" );
    config.writeEntry( "HighlightedText", color );
    color = scheme.readEntry( "Text", "#000000" );
    config.writeEntry( "Text", color );
    color = scheme.readEntry( "ButtonText", "#000000" );
    config.writeEntry( "ButtonText", color );
    color = scheme.readEntry( "Base", "#FFFFFF" );
    config.writeEntry( "Base", color );

    QFontMetrics fm(sample->fontMetrics());
    config.writeEntry( "FontFamily", fontList->currentText() );
    config.writeEntry( "FontSize", fontSize->currentText().toInt() );

#ifndef QPE_FONT_HEIGHT_TO_ICONSIZE
#define QPE_FONT_HEIGHT_TO_ICONSIZE(x) (x+1)
#endif
    config.writeEntry( "IconSize", QPE_FONT_HEIGHT_TO_ICONSIZE(fm.height()) );

    config.write(); // need to flush the config info first
    Global::applyStyle();
    QDialog::accept();
}

void AppearanceSettings::done(int r) { 
  QDialog::done(r);
  close();
}

void AppearanceSettings::colorSelected( int id )
{
    Config config( QPEApplication::qpeDir() + "etc/colors/" + colorListIDs[id] + ".scheme",
        Config::File );

    config.setGroup( "Colors" );
    QColor bgcolor( config.readEntry( "Background", "#E5E1D5" ) );
    QColor btncolor( config.readEntry( "Button", "#D6CDBB" ) );
    QPalette pal( btncolor, bgcolor );
    QString color = config.readEntry( "Highlight", "#800000" );
    pal.setColor( QColorGroup::Highlight, QColor(color) );
    color = config.readEntry( "HighlightedText", "#FFFFFF" );
    pal.setColor( QColorGroup::HighlightedText, QColor(color) );
    color = config.readEntry( "Text", "#000000" );
    pal.setColor( QColorGroup::Text, QColor(color) );
    color = config.readEntry( "ButtonText", "#000000" );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, QColor(color) );
    color = config.readEntry( "Base", "#FFFFFF" );
    pal.setColor( QColorGroup::Base, QColor(color) );

    pal.setColor( QPalette::Disabled, QColorGroup::Text,
		  pal.color(QPalette::Active, QColorGroup::Background).dark() );

    sample->setPalette( pal );
}

void AppearanceSettings::styleSelected( int idx )
{
    QString style("Qtopia");
    PluginItem *item = (PluginItem *)styleList->item( idx );
    if ( item )
	style = item->filename().isEmpty() ? item->text() : item->filename();

    StyleInterface *oldIface = styleiface;
    QStyle *newStyle = 0;
    styleiface = 0;

    if ( style == "Windows" ) { // No tr
	newStyle = new QWindowsStyle;
    } else if ( style == "QPE" || style == "Qtopia" ) {
	newStyle = new QPEStyle;
    } else {
	StyleInterface *iface = 0;
	if ( styleLoader->queryInterface( style, IID_Style, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    newStyle = iface->style();
	    styleiface = iface;
	}
    }

    if (!newStyle)
	newStyle = new QPEStyle;

    sample->setUpdatesEnabled( FALSE );
    QPixmapCache::clear();
    setStyle( sample, newStyle );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );

    if ( oldIface )
	styleLoader->releaseInterface( oldIface );
}

void AppearanceSettings::decorationSelected( int idx )
{
    if ( wdIsPlugin ) {
	wdLoader->releaseInterface( wdiface );
    } else {
	delete wdiface;
    }
    wdiface = 0;
    wdIsPlugin = FALSE;

    QString dec("Qtopia");
    PluginItem *item = (PluginItem *)decorationList->item( idx );
    if ( item )
	dec = item->filename().isEmpty() ? item->text() : item->filename();

    if ( dec != "Qtopia" ) {
	if ( wdLoader->queryInterface( dec, IID_WindowDecoration, (QUnknownInterface**)&wdiface ) == QS_OK && wdiface )
	    wdIsPlugin = TRUE;
    }

    if ( !wdiface )
	wdiface = new DefaultWindowDecoration;

    sample->setDecoration( wdiface );
    sample->repaint();
}

void AppearanceSettings::fontSelected( const QString &name )
{
    sample->setUpdatesEnabled( FALSE );
    QFont f( name, fontSize->currentText().toInt() );
    sample->setFont( f );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );

    fontSize->clear();

    int diff = 1000;

    FontDatabase fd;
    QValueList<int> pointSizes = fd.pointSizes( name.lower() );
    QValueList<int>::Iterator it;
    for ( it = pointSizes.begin(); it != pointSizes.end(); ++it ) {
	if ( *it <= maxFontSize ) {
	    fontSize->insertItem( QString::number( *it ) );
	    if ( QABS(*it-prefFontSize) < diff ) {
		diff = QABS(*it - prefFontSize);
		fontSize->setCurrentItem( fontSize->count()-1 );
	    }
	}
    }
}

void AppearanceSettings::fontSizeSelected( const QString &sz )
{
    prefFontSize = sz.toInt();
    sample->setUpdatesEnabled( FALSE );
    sample->setFont( QFont(fontList->currentText(),prefFontSize) );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
}

void AppearanceSettings::setStyle( QWidget *w, QStyle *s )
{
    w->setStyle( s );
    QObjectList *childObjects=(QObjectList*)w->children();
    if ( childObjects ) {
	QObject * o;
	for(o=childObjects->first();o!=0;o=childObjects->next()) {
	    if( o->isWidgetType() ) {
		setStyle((QWidget *)o,s);
	    }
	}
    }
}

void AppearanceSettings::populateStyleList()
{
    (void)new PluginItem( styleList, "Qtopia");
#if QT_VERSION >= 300
//    styleList->insertStringList(QStyleFactory::styles());
#else
    (void)new PluginItem( styleList, "Windows"); // No tr
    QStringList list = styleLoader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	StyleInterface *iface = 0;
	if (  styleLoader->queryInterface( *it, IID_Style, (QUnknownInterface**)&iface ) == QS_OK ) {
	    PluginItem *item = new PluginItem( styleList, iface->name() );
	    item->setFilename( *it );
	    styleLoader->releaseInterface( iface );
	}
    }
#endif
}

void AppearanceSettings::populateColorList()
{
    QDir dir( QPEApplication::qpeDir() + "etc/colors" );
    QStringList list = dir.entryList( "*.scheme" ); // No tr
    colorListIDs.clear();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	Config scheme( QPEApplication::qpeDir() + "etc/colors/" + *it, Config::File );
        QString name = (*it).left( (*it).find( ".scheme" ) );
	colorListIDs.append(name);
	scheme.setGroup("Global");
        colorList->insertItem( scheme.readEntry("Name",name+"-DEF") );
    }
}

void AppearanceSettings::populateDecorationList()
{
    (void)new PluginItem( decorationList, "Qtopia" );
    QStringList list = wdLoader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	WindowDecorationInterface *iface = 0;
	if ( wdLoader->queryInterface( *it, IID_WindowDecoration, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    PluginItem *item = new PluginItem( decorationList, iface->name() );
	    item->setFilename( *it );
	    wdLoader->releaseInterface( iface );
	}
    }
}

void AppearanceSettings::populateFontList(const QString& cur, int cursz)
{
    FontDatabase fd;
    QStringList f = fd.families();
    for ( QStringList::ConstIterator it=f.begin(); it!=f.end(); ++it ) {
	QString n = *it;
	n[0] = n[0].upper();
	fontList->insertItem(n);
	if ( (*it).lower() == cur.lower() )
	    fontList->setCurrentItem(fontList->count()-1);
    }
    fontSize->insertItem( QString::number( cursz ) );
    fontSelected( fontList->currentText() );
}

void AppearanceSettings::fixSampleGeometry()
{
    sample->fixGeometry();
    sample->setUpdatesEnabled( TRUE );
    sample->repaint( FALSE );  // repaint decoration
}

#include "appearance.moc"
