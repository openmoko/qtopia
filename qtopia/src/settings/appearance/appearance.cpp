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

#include "appearance.h"
#ifndef QTOPIA_PHONE
# include "samplewindow.h"
#endif
#include "themeparser.h"
#include <qtopia/global.h>
#include <qtopia/fontdatabase.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/pluginloader.h>
#include <qtopia/styleinterface.h>
#include <qtopia/resource.h>
#include <qtopia/image.h>
#include <qtopia/imageselector.h>
#include <qtopia/windowdecorationinterface.h>
#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_PHONE
# include <qtopia/private/phonedecoration_p.h>
# include <qtopia/contextbar.h>
# include <qtopia/phonestyle.h>
#else
# include <qtopia/qpestyle.h>
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
#if QT_VERSION >= 0x030000
#include <qstylefactory.h>
#endif
#include <qaccel.h>
#include <qpixmap.h>

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
    void setThemeFilename( const QString fn ) { themeFile = fn; }
    const QString &themeFilename() const { return themeFile; }
    void setDecorationFilename( const QString fn ) { decorationFile = fn; }
    const QString &decorationFilename() const { return decorationFile; }

private:
    QString file;
    QString themeFile;
    QString decorationFile;
};

class FontItem : public QListBoxText
{
public:
    FontItem(QListBox *lb, const QString &t, const QFont &f)
	: QListBoxText( lb, t ), fnt(f) {}

    int height(const QListBox *) const { return 20; }

protected:
    void paint(QPainter *p) {
	p->setFont(fnt);
	QListBoxText::paint(p);
    }

private:
    QFont fnt;
};

//===========================================================================

class DefaultWindowDecoration : public WindowDecorationInterface
{
public:
    DefaultWindowDecoration() : ref(0) {}
    QString name() const {
	return qApp->translate("WindowDecoration", "Default", 
		"List box text for default window decoration");
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
    : AppearanceSettingsBase( parent, name, TRUE, fl|Qt::WStyle_ContextHelp )
{
    wdiface = 0;
    styleiface = 0;
    wdLoader = new PluginLoader( "decorations" );
    styleLoader = new PluginLoader( "styles" );
    wdIsPlugin = FALSE;

#ifdef DEBUG
    maxFontSize = 24;
#else
    maxFontSize = qApp->desktop()->width() >= 640 ? 24 : 12;
#endif

#ifdef QTOPIA_PHONE
    tabWidget->removePage(tab_3);
    tabWidget->removePage(tab_4);
    tabWidget->setFocusPolicy(NoFocus);
    AppearanceSettingsBaseLayout->setMargin(0);

    tabWidget->changeTab(tab, Resource::loadIconSet("theme"), QString::null);
    tabWidget->changeTab(tab_2, Resource::loadIconSet("color"), QString::null);
    tabWidget->changeTab(tab_5, Resource::loadIconSet("font"), QString::null);
    tabWidget->changeTab(tab_6, Resource::loadIconSet("background"), QString::null);

    tabLayout->setMargin(0);
    tabLayout_2->setMargin(0);
    tabLayout_5->setMargin(0);

#ifdef QTOPIA_PHONE
    themeList->setFrameStyle(QFrame::NoFrame);
#endif
    colorList->setFrameStyle(QFrame::NoFrame);
    fontList->setFrameStyle(QFrame::NoFrame);

    QAccel *accel = new QAccel(this);
    connect(accel, SIGNAL(activated(int)), this, SLOT(accelerator(int)));
    accel->insertItem(Key_Left, Key_Left);
    accel->insertItem(Key_Right, Key_Right);
#else
    // in landscape mode, change layout to QHBoxLayout
    if ( isWide() ) {
        delete AppearanceSettingsBaseLayout;
        AppearanceSettingsBaseLayoutH = new QHBoxLayout( this );
        AppearanceSettingsBaseLayoutH->addWidget( tabWidget );
    }

    sample = new SampleWindow( this );
    AppearanceSettingsBaseLayout->addWidget( sample );
    populateStyleList();
    populateDecorationList();
#endif

#ifdef QTOPIA_PHONE
    populateThemeList();
#endif
    populateColorList();

    Config config("qpe");
    config.setGroup( "Appearance" );
    QString s = config.readEntry( "Scheme", "Qtopia" );
    connect( colorList, SIGNAL(highlighted(int)),
	this, SLOT(colorSelected(int)) );
    colorList->setCurrentItem( colorListIDs.findIndex(s) );

#ifdef QTOPIA_PHONE
    QString t = config.readEntry( "Theme");
    int themeMatchIndex = -1;
    for ( unsigned i = 0; i < themeList->count(); i++ ) {
	PluginItem *item = (PluginItem*)themeList->item(i);
	if (item->themeFilename() == t && themeMatchIndex == -1)
	    themeMatchIndex = i;
    }
    if (themeMatchIndex != -1){
	themeList->setSelected( themeMatchIndex, TRUE );
	themeSelected(themeMatchIndex);
    }
    connect( themeList, SIGNAL(highlighted(int)),
	this, SLOT(themeSelected(int)) );
#else
    s = config.readEntry( "Style", "Qtopia" );
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
#endif

    s = config.readEntry( "FontFamily", "Helvetica" );
    prefFontSize = config.readNumEntry( "FontSize", font().pointSize() );
    populateFontList(s,prefFontSize);
    connect( fontList, SIGNAL(highlighted(const QString&)),
	this, SLOT(fontSelected(const QString&)) );
#ifndef QTOPIA_PHONE
    connect( fontSize, SIGNAL(activated(const QString&)),
	this, SLOT(fontSizeSelected(const QString&)) );

#else
    fontSizeLabel->hide();
    fontSize->hide();

    ContextBar::setLabel(themeList, Key_Select, ContextBar::NoLabel);
    ContextBar::setLabel(colorList, Key_Select, ContextBar::NoLabel);
    ContextBar::setLabel(styleList, Key_Select, ContextBar::NoLabel);
    ContextBar::setLabel(fontList, Key_Select, ContextBar::NoLabel);

    previewLabel->installEventFilter(this);
    bgTimer = new QTimer(this);
    connect(bgTimer, SIGNAL(timeout()), this, SLOT(updateBackground()));
#endif


#ifdef QTOPIA_PHONE
    bgImgName = config.readEntry("BackgroundImage");
    if ( !bgImgName.isEmpty() && bgImgName[0]!='/' )
	bgImgName = Resource::findPixmap("wallpaper/"+bgImgName);
    connect(myPictures, SIGNAL(clicked()), this, SLOT(selectImage()));
    connect(wallpaper, SIGNAL(clicked()), this, SLOT(selectWallpaper()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearBackground()));
#endif
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
    QString s;

#ifdef QTOPIA_PHONE
    PluginItem *item = (PluginItem *)themeList->item( themeList->currentItem() );
    if ( item ) {
	if (!item->themeFilename().isEmpty() && (item->themeFilename() != item->text()) ){
	    config.writeEntry( "Style", item->filename() );
	    config.writeEntry("Theme", item->themeFilename());
	    config.writeEntry("DecorationTheme", item->decorationFilename());
	    qDebug("Write config theme select %s %s", item->filename().latin1(), item->text().latin1());
	}else{
	    s = item->themeFilename().isEmpty() ? item->text() : item->themeFilename();
	    qDebug("Write simple config theme select %s %s", item->filename().latin1(), item->text().latin1());
	    config.writeEntry( "Style", s );
	    config.writeEntry( "Theme", "");
	    config.writeEntry( "DecorationTheme", "");
	}
    }
#else
    PluginItem *item = (PluginItem *)styleList->item( styleList->currentItem() );
    if ( item ) {
	s = item->filename().isEmpty() ? item->text() : item->filename();
	config.writeEntry( "Style", s );
    }
#endif

    item = (PluginItem *)decorationList->item( decorationList->currentItem() );
    if ( item ) {
	s = item->filename().isEmpty() ? item->text() : item->filename();
	config.writeEntry( "Decoration", s );
    }

    s = colorListIDs[colorList->currentItem()];
    config.writeEntry( "Scheme", s );

    Config scheme( QPEApplication::qpeDir() + "etc/colors/" + s + ".scheme",
        Config::File );
    if (scheme.isValid()){
	scheme.setGroup("Colors");
	QString color = scheme.readEntry( "Background", "#EEEEEE" );
	config.writeEntry( "Background", color );
	color = scheme.readEntry( "Foreground", "#000000" );
	config.writeEntry( "Foreground", color );
	color = scheme.readEntry( "Button", "#F0F0F0" );
	config.writeEntry( "Button", color );
	color = scheme.readEntry( "Highlight", "#8BAF31" );
	config.writeEntry( "Highlight", color );
	color = scheme.readEntry( "HighlightedText", "#FFFFFF" );
	config.writeEntry( "HighlightedText", color );
	color = scheme.readEntry( "Text", "#000000" );
	config.writeEntry( "Text", color );
	color = scheme.readEntry( "ButtonText", "#000000" );
	config.writeEntry( "ButtonText", color );
	color = scheme.readEntry( "Base", "#FFFFFF" );
	config.writeEntry( "Base", color );
	color = scheme.readEntry( "AlternateBase", "#CBEF71" );
	config.writeEntry( "AlternateBase", color );
	color = scheme.readEntry( "Text_disabled", "" );
	config.writeEntry("Text_disabled", color);
	color = scheme.readEntry( "Foreground_disabled", "" );
	config.writeEntry("Foreground_disabled", color);
	color = scheme.readEntry( "Shadow", "" );
	config.writeEntry("Shadow", color);
    } else {
	QString themeConfigName(QPEApplication::qpeDir() + "etc/themes/" + s+ ".conf");
	Config themeConfig(themeConfigName, Config::File);
	if (themeConfig.isValid()){
	    themeConfig.setGroup( "Theme" ); 
	    if (themeConfig.hasKey( "Name" ) && themeConfig.hasKey( "Style" ) && themeConfig.hasKey( "WidgetsConfig")) {
		QString themeXMLFile = themeConfig.readEntry( "WidgetsConfig");
		WidgetThemeParser parser;
		if (parser.parse(QPEApplication::qpeDir() + "etc/themes/" + themeXMLFile)){
		    QPalette pal = parser.palette();
		    config.writeEntry("Button", pal.color(QPalette::Normal, QColorGroup::Button).name());
		    config.writeEntry("Background", pal.color(QPalette::Normal, QColorGroup::Background).name());
		    config.writeEntry("Foreground", pal.color(QPalette::Normal, QColorGroup::Foreground).name());
		    config.writeEntry("Highlight", pal.color(QPalette::Normal, QColorGroup::Highlight).name());
		    config.writeEntry("HighlightedText", pal.color(QPalette::Normal, QColorGroup::HighlightedText).name());
		    config.writeEntry("Text", pal.color(QPalette::Normal, QColorGroup::Text).name());
		    config.writeEntry("ButtonText", pal.color(QPalette::Normal, QColorGroup::ButtonText).name());
		    config.writeEntry("Base", pal.color(QPalette::Normal, QColorGroup::Base).name());
		    config.writeEntry("Text_disabled", "");
		    config.writeEntry("Foreground_disabled", "");
		    config.writeEntry("Shadow", "");
		}	
	    }else{
		qWarning("Theme config %s is invalid", themeConfigName.local8Bit().data());
	    }
	}else{
	    qWarning("Unable to read config file %s", themeConfigName.local8Bit().data());
	}
    }

#ifndef QTOPIA_PHONE
    QFont font(fontList->currentText(), fontSize->currentText().toInt());
    config.writeEntry( "FontFamily", fontList->currentText() );
    config.writeEntry( "FontSize", fontSize->currentText().toInt() );
#else
    FontMap::Font f = fontMap.current();
    QFont font(f.family,f.size);
    fontMap.write(config);

    config.writeEntry("BackgroundImage", bgImgName);
#endif

#ifndef QPE_FONT_HEIGHT_TO_ICONSIZE
#define QPE_FONT_HEIGHT_TO_ICONSIZE(x) (x+1)
#endif
    QFontMetrics fm(font);
    config.writeEntry( "IconSize", QPE_FONT_HEIGHT_TO_ICONSIZE(fm.height()) );

    config.write(); // need to flush the config info first
    Global::applyStyle();
    QDialog::accept();
}

void AppearanceSettings::done(int r)
{ 
    QDialog::done(r);
    close();
}

bool AppearanceSettings::eventFilter(QObject *
#ifdef QTOPIA_PHONE
o, QEvent *e
#else
,QEvent *
#endif
)
{
#ifdef QTOPIA_PHONE
    if (o == previewLabel && e->type() == QEvent::Show) {
	bgTimer->start(0, TRUE);
    }
#endif

    return FALSE;
}

void AppearanceSettings::resizeEvent( QResizeEvent *e )
{
#ifndef QTOPIA_PHONE
    static bool wide = FALSE;
    bool w = isWide();
    if ( w != wide ) {
        wide = w;
        delete layout();

        QBoxLayout *l;
        if ( wide )
            l = new QHBoxLayout( this );
        else
            l = new QVBoxLayout( this );

        l->setMargin( 4 );
        l->setSpacing( 6 );
        l->addWidget( tabWidget );

        delete sample;
        sample = new SampleWindow( this );

        if ( wdiface )
            sample->setDecoration( wdiface );
        else
            sample->setDecoration( new DefaultWindowDecoration );

        l->addWidget( sample );
        sample->show();
    }
#endif
    AppearanceSettingsBase::resizeEvent( e );
}

bool AppearanceSettings::isWide()
{
    int dheight = QApplication::desktop()->height();
    int dwidth = QApplication::desktop()->width();
    return ( dheight < 300 && dheight < dwidth );
}

void AppearanceSettings::colorSelected( int id )
{
#ifndef QTOPIA_PHONE
    QPalette pal = readColorScheme(id);
    sample->setPalette( pal );
#else
    Q_UNUSED(id);
#endif
}


#define setPaletteEntry(pal, cfg, role, defaultVal) \
    setPalEntry(pal, cfg, #role, QColorGroup::role, defaultVal)
static void setPalEntry( QPalette &pal, const Config &config, const QString &entry,
				QColorGroup::ColorRole role, const QString &defaultVal )
{
    QString value = config.readEntry( entry, defaultVal );
    if ( value[0] == '#' )
	pal.setColor( role, QColor(value) );
    else
	pal.setBrush( role, QBrush(QColor(defaultVal), Resource::loadPixmap(value)) );
}


QPalette AppearanceSettings::readColorScheme(int id)
{
    Config config( QPEApplication::qpeDir() + "etc/colors/" + colorListIDs[id] + ".scheme",
        Config::File );
    config.setGroup( "Colors" );

    QPalette tempPal;
    setPaletteEntry( tempPal, config, Button, "#F0F0F0" );
    setPaletteEntry( tempPal, config, Background, "#EEEEEE" );
    QPalette pal( tempPal.normal().button(), tempPal.normal().background() );
    setPaletteEntry( pal, config, Button, "#F0F0F0" );
    setPaletteEntry( pal, config, Background, "#EEEEEE" );
    setPaletteEntry( pal, config, Base, "#FFFFFF" );
    setPaletteEntry( pal, config, Highlight, "#8BAF31" );
    setPaletteEntry( pal, config, Foreground, "#000000" );
    QString color = config.readEntry( "HighlightedText", "#FFFFFF" );
    pal.setColor( QColorGroup::HighlightedText, QColor(color) );
    color = config.readEntry( "Text", "#000000" );
    pal.setColor( QColorGroup::Text, QColor(color) );
    color = config.readEntry( "ButtonText", "#000000" );
    pal.setColor( QPalette::Active, QColorGroup::ButtonText, QColor(color) );

    pal.setColor( QPalette::Disabled, QColorGroup::Text,
		  pal.color(QPalette::Active, QColorGroup::Background).dark() );

    return pal;
}

void AppearanceSettings::styleSelected( int idx )
{
#ifndef QTOPIA_PHONE
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
#else
    Q_UNUSED(idx);
#endif
}

void AppearanceSettings::themeSelected( int idx )
{
#ifdef QTOPIA_PHONE
    QString theme("Qtopia");
    PluginItem *item = (PluginItem *)themeList->item( idx );
    if ( item )
	theme = item->themeFilename().isEmpty() ? item->text() : item->themeFilename();

    QString readStyleName, configFileName;
    // Check if a themed style was selected.
    QString themeDataPath( QPEApplication::qpeDir() + "etc/themes/" );
    QDir dir;
    bool validTheme, skipTheme;

    qDebug( "Find the themed definition" );
    if ( item && !item->text().isEmpty() && dir.exists( themeDataPath ) ){
	dir.setPath( themeDataPath );
	dir.setNameFilter( "*.conf" ); // No tr
	for (int index = 0; index < (int)dir.count(); index++){
	    QString configFileName(themeDataPath + dir[index]);
	    Config themeConfig(configFileName, Config::File );
	    validTheme = TRUE;
	    skipTheme = FALSE;
	    qDebug( "Reading config %s", configFileName.local8Bit().data() );
	    themeConfig.setGroup( "Theme" ); // no Tr
	    if ( themeConfig.isValid() ){
		if ( themeConfig.hasKey( "Name" ) && themeConfig.hasKey( "Style" ) ){	// No tr 
		    readStyleName = themeConfig.readEntry( "Name" );
		    if ( readStyleName == item->text() ){ 
			qDebug( "Adopting theme %s", item->text().local8Bit().data());
			theme = item->themeFilename();
			break;
		    }else{
			skipTheme = TRUE;
			qDebug( "Skipping themed style %s when looking for %s", 
				readStyleName.local8Bit().data(), item->text().local8Bit().data() );
		    }
		}else{
		    validTheme = FALSE;
		}
	    }

	    if (!validTheme && !skipTheme)
		qDebug("Ignoring invalid theme file %s", configFileName.local8Bit().data()); 
	}
    }
#else
    Q_UNUSED(idx);
#endif
}

void AppearanceSettings::decorationSelected( int idx )
{
#ifndef QTOPIA_PHONE
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
#else
    Q_UNUSED(idx);
#endif
}

void AppearanceSettings::fontSelected( const QString &name )
{

    QString selFontFamily = name;

#ifndef QTOPIA_PHONE
    int selFontSize = prefFontSize;

    sample->setUpdatesEnabled( FALSE );
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

    selFontSize = fontSize->currentText().toInt();

    qDebug( "Setfont: %s %d", selFontFamily.latin1(), selFontSize );

    QFont f( selFontFamily, selFontSize );

    sample->setFont( f );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
#else
    fontMap.select(name);
#endif
}

void AppearanceSettings::fontSizeSelected( const QString &sz )
{
    prefFontSize = sz.toInt();
#ifndef QTOPIA_PHONE
    sample->setUpdatesEnabled( FALSE );
    sample->setFont( QFont(fontList->currentText(),prefFontSize) );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
#endif
}

void AppearanceSettings::setStyle( QWidget *w, QStyle *s )
{
    if (&w->style() != s)
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
#if QT_VERSION >= 0x030000
//    styleList->insertStringList(QStyleFactory::styles());
#else
# ifndef QTOPIA_PHONE
    (void)new PluginItem( styleList, "Windows"); // No tr
# endif
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

#ifdef QTOPIA_PHONE
void AppearanceSettings::populateThemeList()
{
    // Look for themed styles
    QString themeDataPath( QPEApplication::qpeDir() + "etc/themes/" );
    QString configFileName, themeName, decorationName;
    QDir dir;
    if (dir.exists(themeDataPath)){
	bool valid;
	dir.setPath( themeDataPath );
	dir.setNameFilter( "*.conf" ); // No tr
	for (int index = 0; index < (int)dir.count(); index++){
	    valid = TRUE;
	    configFileName = themeDataPath + dir[index];
	    Config themeConfig( configFileName, Config::File );
	    // Ensure that we only provide valid theme choices.
	    if (themeConfig.isValid()){
		themeConfig.setGroup( "Theme" ); // No tr
		QString styleName = themeConfig.readEntry("Style", "Qtopia");
		QStringList list;
		list << "TitleConfig" << "HomeConfig" << "DecorationsConfig" << "ContextConfig" << "WidgetsConfig";
		for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++){
		    if (themeConfig.hasKey(*it)){
			QFileInfo info(QPEApplication::qpeDir() + "etc/themes/" + themeConfig.readEntry(*it));
			if (!info.isFile()){
			    qWarning("Config entry %s in %s points to non-existant file  %s", 
					(*it).local8Bit().data(), configFileName.local8Bit().data(), info.filePath().local8Bit().data()); 
			    valid = FALSE; 
			    break; 
			}
		    }
		}
		if (valid && themeConfig.hasKey( "Name" )) {
		    themeName = themeConfig.readEntry( "Name" );
		    decorationName = themeConfig.readEntry("DecorationConfig");
		    PluginItem *item = new PluginItem( themeList, themeName );
		    item->setFilename( styleName );
		    item->setThemeFilename( dir[index] );
		    item->setDecorationFilename( decorationName );
		}else{
		    valid = FALSE;
		}
	    }else{
		valid = FALSE;
	    }
	    if (!valid)
		qWarning("Ignoring invalid theme conf file %s", configFileName.local8Bit().data()); 
	}

    } else {
	qWarning("Theme style configuration path not found %s", themeDataPath.local8Bit().data()); // No tr
    }
}
#endif

void AppearanceSettings::populateColorList()
{
    int lineHeight = fontMetrics().lineSpacing();
    QDir dir( QPEApplication::qpeDir() + "etc/colors" );
    QStringList list = dir.entryList( "*.scheme" ); // No tr
    QStringList::Iterator it;
    colorListIDs.clear();
    for ( it = list.begin(); it != list.end(); ++it ) {
	Config scheme( QPEApplication::qpeDir() + "etc/colors/" + *it, Config::File );
        QString name = (*it).left( (*it).find( ".scheme" ) );
	colorListIDs.append(name);
	scheme.setGroup("Global");
	QPalette pal = readColorScheme(colorListIDs.count()-1);
	QPixmap pm(32, lineHeight);
	QPainter ppm(&pm);
	ppm.setPen(black);
	ppm.setBrush(QBrush::NoBrush);
	ppm.drawRect(0, 0, 32, lineHeight);
	ppm.fillRect(1, 1, 10, lineHeight-2, pal.active().background());
	ppm.fillRect(11, 1, 10, lineHeight-2, pal.active().base());
	ppm.fillRect(21, 1, 10, lineHeight-2, pal.active().highlight());
	ppm.end();
        colorList->insertItem( pm, scheme.readEntry("Name",name+"-DEF") );
    }
    
    dir = QDir( QPEApplication::qpeDir() + "etc/themes" );
    list = dir.entryList( "*.conf" ); // No tr
    for ( it = list.begin(); it != list.end(); ++it ) {
	Config themeConfig( QPEApplication::qpeDir() + "etc/themes/" + *it, Config::File );
	if ( themeConfig.isValid() ){
	    themeConfig.setGroup("Theme");
	    if ( themeConfig.hasKey( "Name" ) && themeConfig.hasKey( "Style" )	// No tr 
		    && themeConfig.hasKey( "WidgetsConfig" ) ) {			// No tr
		QString name = (*it).left( (*it).find( ".conf" ) );
		colorListIDs.append(name);
		colorList->insertItem( themeConfig.readEntry("Name",name+"-DEF") );
	    }
	}
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
#ifndef QTOPIA_PHONE
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
#else
    for (int i=0; i<fontMap.count(); ++i) {
	(void)new FontItem(fontList, fontMap[i].name, QFont(fontMap[i].family, fontMap[i].size));
	if ( fontMap[i].family.lower()==cur.lower() && fontMap[i].size == cursz )
	    fontList->setCurrentItem(fontList->count()-1);
    }
    if (fontList->currentText().isNull())
	fontList->setCurrentItem(1);
    fontSelected( fontList->currentText() );
#endif
}

void AppearanceSettings::fixSampleGeometry()
{
#ifndef QTOPIA_PHONE
    qDebug( "update sample");
    sample->fixGeometry();
    sample->setUpdatesEnabled( TRUE );
    sample->repaint( FALSE );  // repaint decoration
#endif
}

void AppearanceSettings::accelerator(int 
#ifdef QTOPIA_PHONE        
        key
#endif
        )
{
#ifdef QTOPIA_PHONE
    if (key == Key_Left) {
	int idx = tabWidget->currentPageIndex()-1;
	if (idx < 0)
	    idx = 5;
	if (idx == 3) // the removed decoration page
	    idx--;
	if (idx == 2) // the removed style page
	    idx--;
	tabWidget->setCurrentPage(idx);
    } else if (key == Key_Right) {
	int idx = tabWidget->currentPageIndex()+1;
	if (idx > 5)
	    idx = 0;
	if (idx == 2) // the removed style page
	    idx++;
	if (idx == 3) // the removed decoration page
	    idx++;
	tabWidget->setCurrentPage(idx);
    }
#endif
}

#ifdef QTOPIA_PHONE
void AppearanceSettings::selectBackground(bool wallpapers)
{
    QStringList locations;
    if( wallpapers ) {
        QStringList qtopia_paths = Global::qtopiaPaths();
        QStringList::Iterator it;
        for ( it = qtopia_paths.begin(); it != qtopia_paths.end(); it++ )
            locations.append( (*it) + "pics/wallpaper" );
    }
    
    ImageSelectorDialog sel( locations, this );
    if (QPEApplication::execDialog(&sel)) {
	if (!sel.selectedFilename().isNull()) {
	    previewLabel->setText("");
	    bgImgName = sel.selectedFilename();
	    previewLabel->setText(tr("Loading..."));
	    updateBackground();
	}
    }
}
#endif

void AppearanceSettings::selectImage(void)
{
#ifdef QTOPIA_PHONE
    selectBackground(FALSE);
#endif
}

void AppearanceSettings::selectWallpaper(void)
{
#ifdef QTOPIA_PHONE
    selectBackground(TRUE);
#endif
}

void AppearanceSettings::clearBackground()
{
#ifdef QTOPIA_PHONE
    bgImgName = QString::null;
    updateBackground();
#endif
}

void AppearanceSettings::updateBackground()
{
#ifdef QTOPIA_PHONE
    //QPixmap *currPm = previewLabel->pixmap();
    if (bgImgName.isEmpty()) {
	previewLabel->setText(tr("No image"));
    } else if (previewLabel->isVisible()) {
	QImage bgImg = Image::loadScaled(bgImgName, previewLabel->width(), previewLabel->height());
	if (!bgImg.isNull()) {
	    QPixmap pm;
	    pm.convertFromImage(bgImg);
	    previewLabel->setPixmap(pm);
	} else {
	    previewLabel->setText(tr("Cannot load image"));
	}
    }
#endif
}
