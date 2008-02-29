/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "appearance.h"
#ifndef QTOPIA_PHONE
# include "samplewindow.h"
#endif
#include "themeparser.h"
#include <QSettings>
#include <qcontent.h>
#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qpluginmanager.h>
#include <qthumbnail.h>
#include <qimagedocumentselector.h>
#include <qwindowdecorationinterface.h>
#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#ifdef QTOPIA_PHONE
# include <qtopia/private/phonedecoration_p.h>
# include <qtopia/private/qtopiaresource_p.h>
# include <qtopia/qphonestyle.h>
#else
# include <qtopia/qtopiastyle.h>
#endif
#include <Qt>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTabWidget>
#include <QSlider>
#include <QFile>
#include <QAction>
#include <QTextStream>
#include <QDataStream>
#include <QMessageBox>
#include <QComboBox>
#include <QListWidget>
#include <QDir>
#include <QGroupBox>
#include <QWindowsStyle>
#include <QLayout>
#include <QMenuBar>
#include <QPushButton>
#include <QCheckBox>
#include <QWhatsThis>
#include <QPixmapCache>
#include <QPixmap>
#include <QStyleFactory>
#include <QDesktopWidget>
#include <QTimer>
#include <QPainter>
#include <QFontDatabase>
#include <QMenu>
#include <QKeyEvent>
#include <qtranslatablesettings.h>
#include <qtopiaipcenvelope.h>
#include <qdrmcontent.h>
#include <qwaitwidget.h>

#define SAMPLE_HEIGHT   115


class PluginItem : public QListWidgetItem
{
public:
    PluginItem( const QString &t, QListWidget *lb ) : QListWidgetItem(t, lb) {}
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

class DefaultWindowDecoration : public QWindowDecorationInterface
{
public:
    DefaultWindowDecoration() {}
    QString name() const {
        return qApp->translate("WindowDecoration", "Default",
        "List box text for default window decoration");
    }
    QPixmap icon() const {
        return QPixmap();
    }
};

//===========================================================================

AppearanceSettings::AppearanceSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog(parent, fl), isClosing( false ), defaultColor( 0 )
#ifdef QTOPIA_PHONE
, isThemeLoaded( true ), isShowPreview( false )
, isStatusView( false ), defaultTheme( 0 )
#endif
, currColor(-1), currTheme(-1), bgChanged(false)
{
    setupUi(this);
    wdiface = 0;
    wdLoader = new QPluginManager( "decorations" );
    wdIsPlugin = false;
#ifdef DEBUG
    maxFontSize = 24;
#else
    maxFontSize = qApp->desktop()->width() >= 640 ? 24 : 12;
#endif

#ifdef QTOPIA_PHONE
    contextMenu = QSoftMenuBar::menuFor( this );
    QAction *actionCapture = new QAction( QIcon( ":icon/Note" ), tr( "Add to profile" ), this );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pushSettingStatus()) );
    contextMenu->addAction( actionCapture );

    tabWidget->setFocusPolicy(Qt::NoFocus);
    vboxLayout->setMargin(0);

    tabWidget->setTabIcon(0, QIcon(":icon/theme"));
    tabWidget->setTabText(0, QString());
    tabWidget->setTabIcon(1, QIcon(":icon/color"));
    tabWidget->setTabText(1, QString());
    tabWidget->setTabIcon(2, QIcon(":icon/info"));
    tabWidget->setTabText(2, QString());
    tabWidget->setTabIcon(3, QIcon(":icon/background"));
    tabWidget->setTabText(3, QString());

    connect( tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );

    themeList->setFrameStyle(QFrame::NoFrame);
    colorList->setFrameStyle(QFrame::NoFrame);
    smbLabelTypeList->setFrameStyle(QFrame::NoFrame);

    QSoftMenuBar::setLabel(themeList, Qt::Key_Select, QSoftMenuBar::View);
    QSoftMenuBar::setLabel(colorList, Qt::Key_Select, QSoftMenuBar::View);
    QSoftMenuBar::setLabel(smbLabelTypeList, Qt::Key_Select, QSoftMenuBar::View);

    waitWidget = new QWaitWidget( this );
#else
    // in landscape mode, change layout to QHBoxLayout
    if ( isWide() ) {
        delete vboxLayout;
        hBoxLayout = new QHBoxLayout( this );
        hBoxLayout->addWidget( tabWidget );
    }

    sample = new SampleWindow( this );
    vboxLayout->addWidget( sample );
    populateStyleList();
    populateDecorationList();
#endif
    populateColorList();

    QSettings config("Trolltech","qpe");
    config.beginGroup( "Appearance" );
    QString s = config.value( "Scheme", "Qtopia" ).toString();
    connect( colorList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(colorSelected(QListWidgetItem*)) );
    colorList->setCurrentItem( colorList->item(colorListIDs.indexOf(s)) );
    currColor = colorList->currentRow();
#ifdef QTOPIA_PHONE
    QString t = config.value( "Theme").toString();
    populateThemeList( t );
#else
    s = config.value( "Style", "Qtopia" ).toString();
    int i;
    if ( s == "QPE" ) s = "Qtopia";
    for ( i = 0; i < styleList->count(); i++ ) {
        PluginItem *item = (PluginItem*)styleList->item(i);
        if ( item->filename() == s || item->text() == s ) {
            styleList->setCurrentItem( item );
            break;
        }
    }
    connect( styleList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(styleSelected(QListWidgetItem*)) );

    s = config.value( "Decoration" ).toString();
    for ( i = 0; i < decorationList->count(); i++ ) {
        PluginItem *item = (PluginItem*)decorationList->item(i);
        if ( item->filename() == s || item->text() == s ) {
            decorationList->setCurrentItem( item );
            break;
        }
    }
    connect( decorationList, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem * ) ),
            this, SLOT( decorationSelected(QListWidgetItem * ) ) );
    decorationSelected( decorationList->currentItem() );
#endif
#ifndef QTOPIA_PHONE
    s = config.value( "FontFamily", "Helvetica" ).toString();
    prefFontSize = config.value( "FontSize", font().pointSize() ).toInt();
    populateFontList(s,prefFontSize);
    connect( fontList, SIGNAL(currentTextChanged(const QString&)),
            this, SLOT(fontSelected(const QString&)) );
    connect( fontSize, SIGNAL(activated(const QString&)),
            this, SLOT(fontSizeSelected(const QString&)) );
#else
    previewLabel->setAlignment( Qt::AlignCenter );
    previewLabel->setWordWrap( true );
#endif


#ifdef QTOPIA_PHONE
    bgImgName = config.value("BackgroundImage").toString();
    // bg display mode 1 = standard
    // mode 2 = maximized
    // mode 3 = tiled
    bgDisplayMode = config.value("BackgroundMode", 0).toInt();
    if ( !bgImgName.isEmpty() && bgImgName[0]!='/' )
        bgImgName = QString(":image/wallpaper/"+bgImgName);
    connect(myPictures, SIGNAL(clicked()), this, SLOT(selectImage()));
    connect(wallpaper, SIGNAL(clicked()), this, SLOT(selectWallpaper()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearBackground()));

    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
        this, SLOT(receive(const QString&,const QByteArray&)) );

    config.endGroup();
    config.beginGroup( "ContextMenu" );
    populateLabelTypeList( (QSoftMenuBar::LabelType)config.value( "LabelType", QSoftMenuBar::IconLabel ).toInt() );

    initStatus = status();

#endif

#ifdef QTOPIA_PHONE
    themeList->installEventFilter( this );
    colorList->installEventFilter( this );
    smbLabelTypeList->installEventFilter( this );
    myPictures->installEventFilter( this );
    wallpaper->installEventFilter( this );
    clearButton->installEventFilter( this );

    themeList->setEditFocus( true );
#endif
    rtl = layoutDirection() == Qt::RightToLeft;
}

AppearanceSettings::~AppearanceSettings()
{
    delete wdLoader;
}

void AppearanceSettings::accept()
{
    isClosing = true;
#ifdef QTOPIA_PHONE
    if ( isStatusView ) { // status view from profiles
        if ( initStatus != status() ) { // if changed
            int result = QMessageBox::warning( this, tr("Appearance"),
                            tr("<qt>Would you like to save changes to Profile?</qt>"),
                            QMessageBox::Yes, QMessageBox::No);
            if ( result == QMessageBox::No ) { // do not save
                if ( isFromActiveProfile ) // activate initial status
                    setStatus( initStatus );
            } else { // save changes to the selected profile
                pushSettingStatus();
            }
        }

        if ( !isFromActiveProfile )
            setStatus( activeDetails );
    } else { // normal appearance setting operation
        if ( initStatus != status() ) { // if changed
            int result = QMessageBox::warning( this, tr("Appearance"),
                            tr("<qt>Would you like to apply new appearance?</qt>"),
                            QMessageBox::Yes, QMessageBox::No);
            if ( result == QMessageBox::No ) { // apply initial appearance
                setStatus( initStatus );
            } else { // if active profile contatins this property udpate details
                QSettings cfg( "Trolltech", "PhoneProfile" );
                cfg.beginGroup( "Profiles" );
                QString activeProfile = cfg.value( "Selected", 1 ).toString();
                cfg.endGroup();
                cfg.beginGroup( "Profile " + activeProfile );
                QString settings = cfg.value( "SettingList" ).toString();
                if ( settings.contains( "appearance" ) )
                    pushSettingStatus();
            }
        }
    }
#endif
    applyStyle();
    if ( waitWidget->isVisible() )
        waitWidget->hide();
    QDialog::accept();
}

void AppearanceSettings::reject()
{
    isClosing = true;
#ifdef QTOPIA_PHONE
    if ( initStatus != status() ) {
        setStatus( initStatus );
        applyStyle();
        if ( waitWidget->isVisible() )
            waitWidget->hide();
    }
#endif
    QDialog::reject();
}

void AppearanceSettings::done(int r)
{
    QDialog::done(r);
    close();
}

void AppearanceSettings::applyStyle()
{
#ifdef QTOPIA_PHONE
    if ( !isThemeLoaded || !isShowPreview )
        return;

    isThemeLoaded = false;
#endif

    bool updateTheme = currColor != colorList->currentRow()
                        || currTheme != themeList->currentRow();

    QSettings config("Trolltech","qpe");
    config.beginGroup( "Appearance" );

    if (updateTheme) {
        waitWidget->show();
        QString s;

#ifdef QTOPIA_PHONE
        PluginItem *item = (PluginItem *)themeList->currentItem();
        if ( item ) {
            if (!item->themeFilename().isEmpty() && (item->themeFilename() != item->text()) ){
                config.setValue( "Style", item->filename() );
                config.setValue("Theme", item->themeFilename());
                config.setValue("DecorationTheme", item->decorationFilename());
                qLog(UI) << "Write config theme select" << item->filename().toLatin1().data() <<
                    item->text().toLatin1().data();
            } else {
                s = item->themeFilename().isEmpty() ? item->text() : item->themeFilename();
                qLog(UI) << "Write simple config theme select" << item->filename().toLatin1().data() <<
                    item->text().toLatin1().data();
                config.setValue( "Style", s );
                config.setValue( "Theme", "");
                config.setValue( "DecorationTheme", "");
            }
        }
#else

        PluginItem *item = (PluginItem *)styleList->currentItem();
        if ( item ) {
            s = item->filename().isEmpty() ? item->text() : item->filename();
            config.setValue( "Style", s );
        }
        item = (PluginItem *)decorationList->currentItem();
        if ( item ) {
            s = item->filename().isEmpty() ? item->text() : item->filename();
            config.setValue( "Decoration", s );
        }
#endif

        s = colorListIDs[colorList->row(colorList->currentItem())];
        config.setValue( "Scheme", s );

        QSettings scheme(Qtopia::qtopiaDir() + "etc/colors/" + s + ".scheme", QSettings::IniFormat);
        if (scheme.status()==QSettings::NoError){
            scheme.beginGroup("Colors");
            QString color = scheme.value( "Background", "#EEEEEE" ).toString();
            config.setValue( "Background", color );
            color = scheme.value( "Foreground", "#000000" ).toString();
            config.setValue( "Foreground", color );
            color = scheme.value( "Button", "#F0F0F0" ).toString();
            config.setValue( "Button", color );
            color = scheme.value( "Highlight", "#8BAF31" ).toString();
            config.setValue( "Highlight", color );

            if ( waitWidget ) {
                QColor col( color );
                waitWidget->setColor( col );
            }

            color = scheme.value( "HighlightedText", "#FFFFFF" ).toString();
            config.setValue( "HighlightedText", color );
            color = scheme.value( "Text", "#000000" ).toString();
            config.setValue( "Text", color );
            color = scheme.value( "ButtonText", "#000000" ).toString();
            config.setValue( "ButtonText", color );
            color = scheme.value( "Base", "#FFFFFF" ).toString();
            config.setValue( "Base", color );
            color = scheme.value( "AlternateBase", "#CBEF71" ).toString();
            config.setValue( "AlternateBase", color );
            color = scheme.value( "Text_disabled", "" ).toString();
            config.setValue("Text_disabled", color);
            color = scheme.value( "Foreground_disabled", "" ).toString();
            config.setValue("Foreground_disabled", color);
            color = scheme.value( "Shadow", "" ).toString();
            config.setValue("Shadow", color);
        }
    }

#ifndef QTOPIA_PHONE
    QFont font(fontList->currentItem()->text(), fontSize->currentText().toInt());
    config.setValue( "FontFamily", fontList->currentItem()->text() );
    config.setValue( "FontSize", fontSize->currentText().toDouble() );
#else
    config.setValue("BackgroundImage", bgImgName);
    config.setValue("BackgroundMode", bgDisplayMode );
#endif

#ifdef QTOPIA_PHONE
    config.endGroup();
    config.beginGroup( "ContextMenu" );
    config.setValue( "LabelType", smbLabelTypeList->currentRow() );
#endif

    config.sync(); // need to flush the config info first
    if (updateTheme)
        QtopiaChannel::send("QPE/System", "applyStyle()");
#ifdef QTOPIA_PHONE
    QtopiaChannel::send("QPE/System", "updateContextLabels()");
    if (updateTheme) {
        if ( isClosing )
            QtopiaChannel::send("QPE/System", "applyStyleSplash()");
        else
            QtopiaChannel::send("QPE/System", "applyStyleNoSplash()");
    } else if (bgChanged) {
        QtopiaChannel::send("QPE/System", "applyBackgroundImage()");
    }
#endif
}

#ifdef QTOPIA_PHONE
bool AppearanceSettings::eventFilter(QObject *o, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*) e;
        int key = ke->key();
        if ( o == themeList || o == colorList || o == smbLabelTypeList
        || o == myPictures || o == wallpaper || o == clearButton ) {
            int currentIndex = tabWidget->currentIndex();
            int count = tabWidget->count();
            switch ( key ) {
                case Qt::Key_Hangup:
                    accept();
                    break;
                case Qt::Key_Back:
                    // normal exit after theme is loaded
                    if ( isThemeLoaded ) {
                        isClosing = true;
                        e->ignore();
                    }
                    // item selection changed but don't want to apply
                    else if ( initStatus != status() && isThemeLoaded ) {
                        setStatus( initStatus );
                        isClosing = true;
                        e->ignore();
                    }
                    // item selection changed and the theme is currently applying
                    else if ( initStatus != status() && !isThemeLoaded ) {
                        return true;
                    }
                    break;
                case Qt::Key_Left:
                    if ( rtl && currentIndex < count - 1 )
                        ++currentIndex;
                    else if ( !rtl && currentIndex > 0 )
                        --currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                    break;
                case Qt::Key_Right:
                    if ( rtl && currentIndex > 0 )
                        --currentIndex;
                    else if ( !rtl && currentIndex < count - 1 )
                        ++currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                    break;
                case Qt::Key_Select:
                    if ( o == themeList || o == colorList )
                        applyStyle();
                    else if ( o == smbLabelTypeList )
                        updateContextLabels();
                    break;
                default: break;
            }
        }
        if ( o == clearButton && key == Qt::Key_Down ) {
            myPictures->setFocus();
            return true;
        } else if ( o == myPictures && key == Qt::Key_Up ) {
            clearButton->setFocus();
            return true;
        }
#ifdef QTOPIA_PHONE
        if ( ( o == themeList || o == colorList || o == smbLabelTypeList )
        && ( key == Qt::Key_Down || key == Qt::Key_Up ) )
            qobject_cast<QWidget*>(o)->setEditFocus( true );
#endif
    }
    return false;
}
#endif

void AppearanceSettings::resizeEvent( QResizeEvent *e )
{
#ifndef QTOPIA_PHONE
    static bool wide = false;
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
    QDialog::resizeEvent( e );
}

void AppearanceSettings::showEvent( QShowEvent *e )
{
    QFont fnt(font());

    // update font and background of currentItems
    fnt.setBold( true );

#ifdef QTOPIA_PHONE
    themeList->currentItem()->setFont( fnt );
    smbLabelTypeList->currentItem()->setFont( fnt );
    isShowPreview = true;
#else
    styleList->currentItem()->setFont( fnt );
    decorationList->currentItem()->setFont( fnt );
    fontList->currentItem()->setFont( fnt );
#endif
    colorList->currentItem()->setFont( fnt );

    QDialog::showEvent( e );
}

void AppearanceSettings::colorSelected( QListWidgetItem *colorItem )
{
#ifndef QTOPIA_PHONE
    int id = colorList->row(colorItem);
    QPalette pal = readColorScheme(id);
    sample->setPalette( pal );
#else
    Q_UNUSED( colorItem );
#endif
}


#define setPaletteEntry(pal, cfg, role, defaultVal) \
    setPalEntry(pal, cfg, #role, QPalette::role, defaultVal)
static void setPalEntry( QPalette &pal, const QSettings &config, const QString &entry,
            QPalette::ColorRole role, const QString &defaultVal )
{
    QString value = config.value( entry, defaultVal ).toString();
    if ( value[0] == '#' )
        pal.setColor( role, QColor(value) );
    else
        pal.setBrush( role, QBrush(QColor(defaultVal), QPixmap(":image/"+value)) );
}


QPalette AppearanceSettings::readColorScheme(int id)
{
    QSettings config(Qtopia::qtopiaDir() + "etc/colors/" + colorListIDs[id] + ".scheme", QSettings::IniFormat);
    config.beginGroup( "Colors" );

    QPalette tempPal;
    setPaletteEntry( tempPal, config, Button, "#F0F0F0" );
    setPaletteEntry( tempPal, config, Background, "#EEEEEE" );
    QPalette pal( tempPal.color(QPalette::Button), tempPal.color(QPalette::Background) );
    setPaletteEntry( pal, config, Button, "#F0F0F0" );
    setPaletteEntry( pal, config, Background, "#EEEEEE" );
    setPaletteEntry( pal, config, Base, "#FFFFFF" );
    setPaletteEntry( pal, config, Highlight, "#8BAF31" );
    setPaletteEntry( pal, config, Foreground, "#000000" );
    QString color = config.value( "HighlightedText", "#FFFFFF" ).toString();
    pal.setColor( QPalette::HighlightedText, QColor(color) );
    color = config.value( "Text", "#000000" ).toString();
    pal.setColor( QPalette::Text, QColor(color) );
    color = config.value( "ButtonText", "#000000" ).toString();
    pal.setColor( QPalette::Active, QPalette::ButtonText, QColor(color) );

    pal.setColor( QPalette::Disabled, QPalette::Text, pal.color(QPalette::Active, QPalette::Background).dark() );

    return pal;
}

void AppearanceSettings::styleSelected( QListWidgetItem *styleItem )
{
#ifndef QTOPIA_PHONE
    QString style("Qtopia");
    PluginItem *item = (PluginItem *)styleItem;
    if ( item )
        style = item->filename().isEmpty() ? item->text() : item->filename();

    // XXX delete old style
    QStyle *newStyle = 0;
    if ( style == "Windows" ) { // No tr
        newStyle = new QWindowsStyle;
    } else if ( style == "QPE" || style == "Qtopia" ) {
#ifdef QTOPIA4_TODO
        newStyle = new QPEStyle;
#endif
    } else {
        newStyle = QStyleFactory::create(style);
    }

#ifdef QTOPIA4_TODO
    if (!newStyle)
        newStyle = new QPEStyle;
#else
    if (!newStyle)
        newStyle = new QWindowsStyle;
#endif

    sample->setUpdatesEnabled( false );
    QPixmapCache::clear();
    setStyle( sample, newStyle );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );

#else
    Q_UNUSED(styleItem);
#endif
}

void AppearanceSettings::decorationSelected( QListWidgetItem *decItem )
{
#ifndef QTOPIA_PHONE
    wdiface = 0;
    wdIsPlugin = false;

    QString dec("Qtopia");
    PluginItem *item = (PluginItem *)decItem;
    if ( item )
        dec = item->filename().isEmpty() ? item->text() : item->filename();

    if ( dec != "Qtopia" ) {
        QObject *instance = wdLoader->instance(dec);
        QWindowDecorationFactoryInterface *iface = 0;
        iface = qobject_cast<QWindowDecorationFactoryInterface*>(instance);
        if (iface) {
            wdiface = iface->decoration(iface->keys()[0]);
            wdIsPlugin = true;
        }
    }

    if ( !wdiface )
        wdiface = new DefaultWindowDecoration;
    sample->setDecoration( wdiface );
    sample->repaint();
#else
    Q_UNUSED(decItem);
#endif
}

QFont AppearanceSettings::fontSelected( const QString &name )
{
    QString selFontFamily = name;
    QFont font;

#ifndef QTOPIA_PHONE
    int selFontSize = prefFontSize;

    sample->setUpdatesEnabled( false );
    fontSize->clear();

    int diff = 1000;
    QFontDatabase fd;
    QList<int> pointSizes = fd.pointSizes( name.toLower() );
    QList<int>::Iterator it;
    for ( it = pointSizes.begin(); it != pointSizes.end(); ++it ) {
        if ( *it <= maxFontSize ) {
            fontSize->addItem( QString::number( *it ) );
            if ( qAbs(*it-prefFontSize) < diff ) {
                diff = qAbs(*it - prefFontSize);
                fontSize->setCurrentIndex( fontSize->count()-1 );
            }
        }
    }

    selFontSize = fontSize->currentText().toInt();
    QFont f( selFontFamily, selFontSize );
    font = f;

    sample->setFont( font );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
#endif
    return font;
}

void AppearanceSettings::fontSizeSelected( const QString &sz )
{
    prefFontSize = sz.toInt();
#ifndef QTOPIA_PHONE
    sample->setUpdatesEnabled( false );
    sample->setFont( QFont(fontList->currentItem()->text(),prefFontSize) );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
#endif
}

#ifndef QTOPIA_PHONE
void AppearanceSettings::setStyle( QWidget *w, QStyle *s )
{
    if (w->style() != s)
        w->setStyle( s );
    QObjectList childObjects = w->children();
    QObjectList::ConstIterator it;
    for (it = childObjects.begin(); it != childObjects.end(); ++it) {
        if( (*it)->isWidgetType() ) {
            setStyle((QWidget *)(*it),s);
        }
    }
}

void AppearanceSettings::populateStyleList()
{
    (void)new PluginItem( "Qtopia", styleList );
    QStringList list = QStyleFactory::keys();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        //PluginItem *item = new PluginItem( *it, styleList );
    }
}
#endif

#ifdef QTOPIA_PHONE
void AppearanceSettings::populateThemeList( QString current )
{
    // Look for themed styles
    QString themeDataPath( Qtopia::qtopiaDir() + "etc/themes/" );
    QString configFileName, themeName, decorationName;
    QDir dir;
    if (dir.exists(themeDataPath)){
        bool valid;
        dir.setPath( themeDataPath );
        dir.setNameFilters( QStringList( "*.conf" )); // No tr
        for (int index = 0; index < (int)dir.count(); index++) {
            valid = true;
            configFileName = themeDataPath + dir[index];
            QTranslatableSettings themeConfig(configFileName, QSettings::IniFormat);
            // Ensure that we only provide valid theme choices.
            if (themeConfig.status()==QSettings::NoError){
                themeConfig.beginGroup( "Theme" ); // No tr
                QString styleName = themeConfig.value("Style", "Qtopia").toString();
                QStringList list;
                list << "TitleConfig" << "HomeConfig" << "ContextConfig" << "DialerConfig" << "CallScreenConfig" << "DecorationConfig";
                for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); it++){
                    if (themeConfig.contains(*it)){
                        QFileInfo info(Qtopia::qtopiaDir() + "etc/themes/" + themeConfig.value(*it).toString());
                        if (!info.isFile()){
                            qLog(UI) << "QSettings entry" << (*it).toLocal8Bit().data()
                                    << "in" << configFileName.toLocal8Bit().data()
                                    << "points to non-existant file" << info.filePath().toLocal8Bit().data();
                            valid = false;
                            break;
                        }
                    }
                }
                if (valid && themeConfig.contains("Name")) {
                    themeName = themeConfig.value( "Name" ).toString();
                    decorationName = themeConfig.value("DecorationConfig").toString();
                    PluginItem *item = new PluginItem( themeName, themeList );

                    if ( themeName == tr( "Qtopia" ) )
                        defaultTheme = item;

                    item->setFilename( styleName );
                    item->setThemeFilename( dir[index] );
                    item->setDecorationFilename( decorationName );
                    if ( configFileName.contains( current ) ) {
                        themeList->setCurrentItem( item );
                        currTheme = themeList->currentRow();
                    }
                } else {
                    valid = false;
                }
            } else {
                valid = false;
            }

            if (!valid)
                qLog(UI) << "Ignoring invalid theme conf file" << configFileName.toLocal8Bit().data();
        }

    } else {
        qLog(UI) << "Theme style configuration path not found" << themeDataPath.toLocal8Bit().data(); // No tr
    }
}
#endif

void AppearanceSettings::populateColorList()
{
    QDir dir( Qtopia::qtopiaDir() + "etc/colors" );
    QStringList list = dir.entryList( QStringList("*.scheme") ); // No tr
    QStringList::Iterator it;
    colorListIDs.clear();
    for ( it = list.begin(); it != list.end(); ++it ) {
        QTranslatableSettings scheme(Qtopia::qtopiaDir() + "etc/colors/" + *it, QSettings::IniFormat);
        QString name = (*it).left( (*it).indexOf( ".scheme" ) );
        colorListIDs.append(name);
        scheme.beginGroup("Global");
        QListWidgetItem *item = new QListWidgetItem(scheme.value("Name",name+"-DEF").toString(), colorList);

        if ( name == tr( "Qtopia" ) )
            defaultColor = item;
    }
}

#ifndef QTOPIA_PHONE
void AppearanceSettings::populateDecorationList()
{
    (void)new PluginItem( "Qtopia", decorationList );
    QStringList list = wdLoader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        QWindowDecorationFactoryInterface *iface = 0;
        QObject *instance = wdLoader->instance(*it);
        iface = qobject_cast<QWindowDecorationFactoryInterface*>(instance);
        if (iface) {
            PluginItem *item = new PluginItem( iface->decoration(iface->keys()[0])->name(), decorationList );
            item->setFilename(*it);
        }
        if (instance)
            delete instance;
    }
}

void AppearanceSettings::populateFontList(const QString& cur, int cursz)
{
    QFontDatabase fd;
    QStringList f = fd.families();
    for ( QStringList::const_iterator it=f.constBegin(); it!=f.constEnd(); ++it ) {
        QString n = *it;
        n[0] = n[0].toUpper();
        fontList->addItem(n);
        if ( (*it).toLower() == cur.toLower() )
            fontList->setCurrentItem(fontList->item(fontList->count()-1));
    }
    if ( !fontList->currentItem() )
        fontList->setCurrentRow(0);
    fontSize->addItem( QString::number( cursz ) );
    fontSelected( fontList->currentItem()->text() );
}
#endif

#ifdef QTOPIA_PHONE
void AppearanceSettings::populateLabelTypeList(const QSoftMenuBar::LabelType type)
{
    smbLabelTypeList->addItem( new QListWidgetItem( tr( "Icon Label" ) ) );
    smbLabelTypeList->addItem( new QListWidgetItem( tr( "Text Label" ) ) );
    smbLabelTypeList->setCurrentItem( smbLabelTypeList->item( (int)type ) );
}
#endif

void AppearanceSettings::fixSampleGeometry()
{
#ifndef QTOPIA_PHONE
    sample->fixGeometry();
    sample->setUpdatesEnabled( true );
    sample->repaint();  // repaint decoration
#endif
}

#ifdef QTOPIA_PHONE
void AppearanceSettings::selectBackground(bool wallpapers)
{
    QContentFilter locations;
    if( wallpapers) {
        QStringList qtopia_paths = Qtopia::installPaths();
        QStringList::Iterator it;
        for ( it = qtopia_paths.begin(); it != qtopia_paths.end(); it++ ) {
            locations |= QContentFilter( QContentFilter::Directory, (*it) + QLatin1String("pics/wallpaper") );
            QContentSet::scan( (*it) + "pics/wallpaper" );
        }
    }
    else
        locations = QContentFilter( QContent::Document );

    QDrmContent drm( QDrmRights::Display );

    QImageDocumentSelectorDialog sel( this );
    sel.setFilter( locations );
    sel.setMandatoryPermissions( QDrmRights::Automated );
    if (QtopiaApplication::execDialog(&sel)) {
        QContent doc = sel.selectedDocument();

        if(doc.fileKnown() && displayMode() ) { // display mode must be selected
            qLog(Resource) << "Selected Document:" << doc.file();
            bgChanged = true;
            previewLabel->setText("");
            bgImgName = doc.file();
            previewLabel->setText(tr("Loading..."));
            updateBackground();
            if ( isThemeLoaded && isShowPreview )
                QtopiaChannel::send("QPE/System", "applyBackgroundImage()");
        }
    }
}
#endif

void AppearanceSettings::selectImage(void)
{
#ifdef QTOPIA_PHONE
    selectBackground(false);
#endif
}

void AppearanceSettings::selectWallpaper(void)
{
#ifdef QTOPIA_PHONE
    selectBackground(true);
#endif
}

void AppearanceSettings::clearBackground()
{
#ifdef QTOPIA_PHONE
    bgImgName = QString();
    bgDisplayMode = 0;
    bgChanged = true;
    updateBackground();
    applyStyle();
#endif
}

void AppearanceSettings::updateBackground()
{
#ifdef QTOPIA_PHONE
    if (bgImgName.isEmpty()) {
        previewLabel->setText(tr("No image"));
    } else if (previewLabel->isVisible()) {
        QDrmContent imgContent( QDrmRights::Display, QDrmContent::NoLicenseOptions );
        if( imgContent.requestLicense( QContent( bgImgName ) ) )
        {
            QThumbnail thumbnail( bgImgName );
            int margin = previewLabel->margin();
            QSize sz = previewLabel->contentsRect().size()
                        - QSize(margin,margin)
                        - QSize(1,1); // prevents weird Qt bug.
            QPixmap bgImg = thumbnail.pixmap(sz);
            if (!bgImg.isNull()) {
                previewLabel->setPixmap(bgImg);
                return;
            }
        }
        previewLabel->setText(tr("Cannot load image"));
    }
#endif
}

bool AppearanceSettings::displayMode()
{
    QDialog dlg( this );
    dlg.setWindowTitle( tr( "Display Mode" ) );
    QVBoxLayout layout( &dlg );
    QRadioButton centerButton( tr( "Standard" ), &dlg );
    QRadioButton scaleButton( tr( "Maximize" ), &dlg );
    QRadioButton tileButton( tr( "Tile" ), &dlg );
    QButtonGroup group( &dlg );
    group.addButton( &centerButton, 0 );
    group.addButton( &scaleButton, 1 );
    group.addButton( &tileButton, 2 );
    centerButton.setChecked( true );
    layout.setMargin( 2 );
    layout.setSpacing( 2 );
    layout.addWidget( &centerButton );
    layout.addWidget( &scaleButton );
    layout.addWidget( &tileButton );
    layout.addStretch( 0 );

    if ( QtopiaApplication::execDialog( &dlg ) ) {
        bgDisplayMode = group.checkedId();
        return true;
    }
    return false;
}

bool AppearanceSettings::isWide()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QSize dSize = desktop->availableGeometry(desktop->primaryScreen()).size();
    return (dSize.height() < 300 && dSize.height() < dSize.width());
}

#ifdef QTOPIA_PHONE
void AppearanceSettings::pushSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pushSettingStatus(QString,QString,QString)" );
    e << QString( "appearance" ) << QString( windowTitle() ) << status();
    e.send();
    initStatus = status();
}

void AppearanceSettings::pullSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pullSettingStatus(QString,QString,QString)" );
    e << QString( "appearance" ) << QString( windowTitle() ) << status();
    e.send();
}

QString AppearanceSettings::status()
{
    QString status;
    status += QString::number( themeList->currentRow() ) + ",";
    status += QString::number( colorList->currentRow() ) + ",";
    status += bgImgName + ",";
    status += QString::number( bgDisplayMode ) + ",";
    status += QString::number( smbLabelTypeList->currentRow() ) + ",";
    return status;
}

void AppearanceSettings::setStatus( const QString details )
{
#ifdef QTOPIA_PHONE
    isShowPreview = false;
#endif
    QStringList s = details.split( ',' );
    themeList->setCurrentRow( s.at( 0 ).toInt() );
    colorList->setCurrentRow( s.at( 1 ).toInt() );
    bgImgName = s.at( 2 );
    bgDisplayMode = s.at( 3 ).toInt();
    smbLabelTypeList->setCurrentRow( s.at( 4 ).toInt() );
    if ( !bgImgName.isEmpty() )
        updateBackground();
#ifdef QTOPIA_PHONE
    isShowPreview = true;
#endif
}

void AppearanceSettings::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds( data );
    if ( msg == "Settings::setStatus(bool,QString)" ) {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        isStatusView = true;
#ifdef QTOPIA_PHONE
        QSoftMenuBar::removeMenuFrom( this, contextMenu );
        delete contextMenu;
#endif
        QString details;
        ds >> isFromActiveProfile;
        ds >> details;
        activeDetails = initStatus;
        initStatus = details;
        setStatus( details );
        applyStyle();
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        isClosing = true;
        QString details;
        ds >> details;
        setStatus( details );
        applyStyle();
        QTimer::singleShot( 500, waitWidget, SLOT(hide()) );
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        pullSettingStatus();
    } else if ( msg == "Settings::activateDefault()" ) {
        colorList->setCurrentItem( defaultColor );
#ifdef QTOPIA_PHONE
        isShowPreview = false;
        themeList->setCurrentItem( defaultTheme );
        bgImgName = "";
        smbLabelTypeList->setCurrentRow( (int)QSoftMenuBar::IconLabel );
        isShowPreview = true;
#endif
        isClosing = true;
        applyStyle();
        QTimer::singleShot( 500, waitWidget, SLOT(hide()) );
    } else if ( msg == "Settings::themeLoaded()" ) {
        isThemeLoaded = true;
        QTimer::singleShot( 500, waitWidget, SLOT(hide()) );
    }
}

void AppearanceSettings::tabChanged( int curIndex )
{
    if ( curIndex == 0 )
        themeList->setEditFocus( true );
    else if ( curIndex == 1 )
        colorList->setEditFocus( true );
    else if ( curIndex == 2 )
        smbLabelTypeList->setEditFocus( true );
    else if ( curIndex == 3 )
        updateBackground();
}

void AppearanceSettings::updateContextLabels()
{
#ifdef QTOPIA_PHONE
    if ( isThemeLoaded &&  isShowPreview ) {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "ContextMenu" );
        config.setValue( "LabelType", smbLabelTypeList->currentRow() );

        QtopiaChannel::send("QPE/System", "updateContextLabels()");
    }
#endif
}

#endif


