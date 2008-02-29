/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include "themedview.h"

#include <QtopiaApplication>
#include <qtopiaservices.h>
#include <QMenu>
#include <QTimer>
#include <QPhoneProfileManager>
#include <QFormLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QTranslatableSettings>
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>
#include <QPixmap>
#include <QDesktopWidget>
#include <QScrollArea>

ThemePreview::ThemePreview(const QString &name, int width, int height)
    : QObject(0), m_name(name), themedView(0),
      m_button0Icon(true),
      m_button1Icon(true),
      m_button2Icon(true),
      m_width(width),
      m_height(height)
{
}

ThemePreview::~ThemePreview()
{
    delete themedView;
}

void ThemePreview::setXmlFilename(const QString &filename)
{
    if (filename == m_filename)
        return;
    m_filename = filename;
    reload();
}

void ThemePreview::setButton(bool icon, ThemeTextItem *textItem, ThemeImageItem *imageItem,
                             const QString &text, const QString &image)
{
    if (!icon) {
        if (textItem) {
           textItem->setVisible(true);
           textItem->setText(text);
        }
        if (imageItem) {
            imageItem->setVisible(false);
        }
    } else {
        if (textItem) {
            textItem->setVisible(false);
        }
        if (imageItem) {
            imageItem->setVisible(true);
            imageItem->setImage(QPixmap(image));
        }
    }
}

void ThemePreview::reload()
{
    if (m_color.isEmpty() || m_filename.isEmpty())
        return;
    delete themedView;
    themedView = new ThemedView;
    themedView->setPalette(m_palette);
    themedView->loadSource(m_filename);
    themedView->resize(m_width, m_height);

    if (m_name == "contextbar") {
        ThemeTextItem *tbutton0 = (ThemeTextItem *)themedView->findItem("button0", ThemedView::Text);
        ThemeImageItem *ibutton0 = (ThemeImageItem *)themedView->findItem("button0", ThemedView::Image);
        setButton(m_button0Icon, tbutton0, ibutton0, "Options", ":icon/options");
        ThemeTextItem *tbutton1 = (ThemeTextItem *)themedView->findItem("button1", ThemedView::Text);
        ThemeImageItem *ibutton1 = (ThemeImageItem *)themedView->findItem("button1", ThemedView::Image);
        setButton(m_button1Icon, tbutton1, ibutton1, "Select", ":icon/select");
        ThemeTextItem *tbutton2 = (ThemeTextItem *)themedView->findItem("button2", ThemedView::Text);
        ThemeImageItem *ibutton2 = (ThemeImageItem *)themedView->findItem("button2", ThemedView::Image);
        setButton(m_button2Icon, tbutton2, ibutton2, "Back", ":icon/cancel");
    }
}

void ThemePreview::setColor(const QString &color)
{
    if (color == m_color)
        return;
    QSettings scheme(color, QSettings::IniFormat);
    scheme.beginGroup("Colors");
    m_palette.setColor(QPalette::Normal, QPalette::Window, scheme.value("Background").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Button, scheme.value("Button").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Highlight, scheme.value("Highlight").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Text, scheme.value("Text").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Base, scheme.value("Base").toString());
    m_color = color;
    reload();
}

void ThemePreview::requestPreview()
{
    QTimer::singleShot(0, this, SLOT(doPreview()));
}

void ThemePreview::doPreview()
{
    if (!themedView)
        return;
    ThemeItem *page = themedView->findItem(m_name, ThemedView::Page);
    m_preview = QPixmap::grabWidget(themedView, page->rect());
    emit previewReady(m_preview.scaledToWidth((int)(m_width * 0.55), Qt::SmoothTransformation));
}

void ThemePreview::setIconLabel(bool enable)
{
    m_button0Icon = enable;
    m_button1Icon = enable;
    m_button2Icon = enable;

    if (!themedView)
        return;

   reload();
}

static QSettings gConfig("Trolltech", "qpe");

// ThemeInfo ----------------------------------------------------
ThemeInfo::ThemeInfo( const QString &tn )
    : name( tn ) {}
const QString & ThemeInfo::themeName() { return name; }
void ThemeInfo::setStyleName( const QString sn ) { style = sn; }
const QString & ThemeInfo::styleName() const { return style; }
void ThemeInfo::setThemeFileName( const QString fn ) { themeFile = fn; }
const QString & ThemeInfo::themeFileName() const { return themeFile; }
void ThemeInfo::setDecorationFileName( const QString fn ) { decorationFile = fn; }
const QString & ThemeInfo::decorationFileName() const { return decorationFile; }
void ThemeInfo::setServerWidget(const QString &sw) { srvWidget = sw; }
QString ThemeInfo::serverWidget() const { return srvWidget; }
void ThemeInfo::setColorSchemes(const QStringList &cs) { colSchemes = cs; }
QStringList ThemeInfo::colorSchemes() const { return colSchemes; }
void ThemeInfo::setBackgroundImages(const QStringList &bg) { bgImages = bg; }
QStringList ThemeInfo::backgroundImages() const { return bgImages; }

// AppearanceSettings ----------------------------------------------------
AppearanceSettings::AppearanceSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog(parent, fl),
    isStatusView(false), isFromActiveProfile(false)
{
    setupUi();
    readThemeInfo();
    readColorScheme();

    // Populate the first combo box (theme).
    populate();

    // Select the theme to populate the other combo boxes.
    // Note that they are not connected yet, so no preview is requested.
    themeSelected(mActiveThemeIndex);

    // Connect the combo boxes.
    connect( themeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(themeSelected(int)) );
    connect( colorCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(colorSelected(int)) );
    connect( bgCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(backgroundSelected(QString)) );
    connect( labelCkBox, SIGNAL(toggled(bool)),
            this, SLOT(labelToggled(bool)) );

    connect( qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(receive(QString,QByteArray)) );

    // Re-select the theme to request the previews.
    themeSelected(mActiveThemeIndex);

    QSoftMenuBar::menuFor( this )->addAction
        ( QIcon( ":icon/Note" ), tr( "Add to current profile" ), this, SLOT(pushSettingStatus()) );
    QSoftMenuBar::menuFor( this )->addAction
        ( QIcon( ":image/homescreen/homescreen" ), tr( "Homescreen Settings..." ), this, SLOT(openHomescreenSettings()) );
}

AppearanceSettings::~AppearanceSettings()
{
}

void AppearanceSettings::populate()
{
    // current theme & color
    gConfig.beginGroup( "Appearance" );
    mActiveTheme = gConfig.value( "Theme", "qtopia.conf" ).toString();
    mActiveColor = gConfig.value( "Scheme", "Qtopia" ).toString();
    mActiveBackground = gConfig.value( "BackgroundImage", "" ).toString();
    gConfig.endGroup();

    // current label type
    gConfig.beginGroup( "ContextMenu" );
    mActiveLabelType = (QSoftMenuBar::LabelType)gConfig.value( "LabelType", QSoftMenuBar::TextLabel ).toInt();
    gConfig.endGroup();
    labelCkBox->setCheckState( mActiveLabelType == QSoftMenuBar::IconLabel ? Qt::Checked : Qt::Unchecked );
    labelToggled( mActiveLabelType == QSoftMenuBar::IconLabel );

    // current server widgets
    QSettings cfg( "Trolltech", "ServerWidgets" );
    cfg.beginGroup( "Mapping" );
    if (!cfg.childKeys().isEmpty())
        mServerWidgets = cfg.value("Default", "Phone").toString();

    // populate theme combo box
    foreach ( ThemeInfo theme, themes ) {
        themeCombo->addItem( theme.themeName() );
        if ( theme.themeFileName() == mActiveTheme ) {
            mActiveThemeIndex = themeCombo->count() - 1;
            themeCombo->setCurrentIndex(mActiveThemeIndex);
        }
    }
    contextPreview->setIconLabel(mActiveLabelType == QSoftMenuBar::IconLabel);
}

void AppearanceSettings::accept()
{
    QPhoneProfileManager profileManager;
    QPhoneProfile activeProfile = profileManager.activeProfile();

    if ( !isStatusView ) { // normal appearance setting operation
        QPhoneProfile::Setting setting = activeProfile.applicationSetting("appearance");
        if ( setting != QPhoneProfile::Setting() )
            pushSettingStatus();
    } else { // status view from profiles
        // save current status to the profile
        pushSettingStatus();
    }

    applyStyle();

    QDialog::accept();
}

void AppearanceSettings::applyStyle()
{
    ThemeInfo *theme = &themes[themeCombo->currentIndex()];
    if ( !theme )
        return;

    if ( mServerWidgets != theme->serverWidget() ) {
        int ret = QMessageBox::warning( this, tr( "Restart?" ),
                tr( "Device will be restarted for theme to be fully applied.<br>Apply Now?" ),
                QMessageBox::Yes, QMessageBox::No );
        if ( ret != QMessageBox::Yes )
            return;
    }

    // apply only when changed
    if ( theme->themeFileName() != mActiveTheme )
        applyTheme();
    if ( colorCombo->itemData(colorCombo->currentIndex()).toString() != mActiveColor )
        applyColorScheme();
    if ( bgCombo->currentText() != mActiveBackground )
        applyBackgroundImage();
    if ( ( labelCkBox->isChecked()
                ? QSoftMenuBar::IconLabel
                : QSoftMenuBar::TextLabel ) != mActiveLabelType )
        applyLabel();
}

void AppearanceSettings::setupUi()
{
    // title string
    setWindowTitle( tr( "Appearance" ) );

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    QWidget *appearance = new QWidget;

    QScrollArea *appearanceWrapper = new QScrollArea;
    appearanceWrapper->setFocusPolicy(Qt::NoFocus);
    appearanceWrapper->setFrameStyle(QFrame::NoFrame);
    appearanceWrapper->setWidget(appearance);
    appearanceWrapper->setWidgetResizable(true);
    appearanceWrapper->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 

    // create layout
    QFormLayout *formLayout = new QFormLayout(appearance);

    // create widgets
    themeCombo = new QComboBox( this );
    colorCombo = new QComboBox( this );
    bgCombo = new QComboBox( this );

    // add widgets
    formLayout->addRow( tr( "Theme" ), themeCombo );
    formLayout->addRow( tr( "Color" ), colorCombo );
    formLayout->addRow( tr( "Background" ), bgCombo );

    // label option checkbox
    labelCkBox = new QCheckBox( tr( "Use icons for soft keys" ), this );
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget( labelCkBox, 0, Qt::AlignHCenter );
    formLayout->addRow( hLayout );

    // preview
    groupBox = new QGroupBox(this);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(10);
    groupBox->setLayout(layout);
    groupBox->hide();

    QVBoxLayout *previewLayout = new QVBoxLayout();
    previewLayout->setSpacing(0);

    QDesktopWidget *desktop = QApplication::desktop();
    int width = desktop->screenGeometry(desktop->primaryScreen()).width();
    int height = desktop->screenGeometry(desktop->primaryScreen()).height();

    titlePreview = new ThemePreview("title", width, height);
    connect(titlePreview, SIGNAL(previewReady(QPixmap)), this, SLOT(setTitlePixmap(QPixmap)));

    contextPreview = new ThemePreview("contextbar", width, height);
    connect(contextPreview, SIGNAL(previewReady(QPixmap)), this, SLOT(setContextPixmap(QPixmap)));

    previewLayout->addWidget( &titleLabel, 0, Qt::AlignHCenter );
    previewLayout->addWidget( &contextLabel, 0, Qt::AlignHCenter );

    layout->addStretch(1);
    layout->addLayout(previewLayout);
    layout->addWidget(&backgroundLabel);
    layout->addStretch(1);
    formLayout->addRow( groupBox );

    l->addWidget(appearanceWrapper);
}

void AppearanceSettings::readThemeInfo()
{
    QDir dir;
    QString themeDataPath( Qtopia::qtopiaDir() + "etc/themes/" );
    if ( !dir.exists( themeDataPath ) ) {
        qLog(UI) << "Theme style configuration path not found" << themeDataPath.toLocal8Bit().data(); // No tr
        return;
    }

    QString configFileName, themeName, decorationName;
    bool valid;

    // read theme.conf files
    dir.setPath( themeDataPath );
    dir.setNameFilters( QStringList( "*.conf" )); // No tr

    for (int index = 0; index < (int)dir.count(); index++) {
        valid = true;
        configFileName = themeDataPath + dir[index];
        QTranslatableSettings themeConfig(configFileName, QSettings::IniFormat);
        // Ensure that we only provide valid theme choices.
        if ( themeConfig.status() != QSettings::NoError ) {
            // failed to read theme.conf. try next one
            qLog(UI) << "Failed to read, ignore" << configFileName.toLocal8Bit().data();
            continue;
        } else {
            themeConfig.beginGroup( "Theme" ); // No tr
            QString styleName = themeConfig.value("Style", "Qtopia").toString();
            QString serverWidgetName = themeConfig.value("ServerWidget", "Phone").toString();
            QStringList colSchemes = themeConfig.value("ColorScheme").toString().split( "|", QString::SkipEmptyParts );
            QStringList bg = themeConfig.value("Backgrounds").toString().split( "|", QString::SkipEmptyParts );

//          FIXME: We don't check the validity of the theme for performance issues...
//             QStringList list;
//             list << "TitleConfig" << "HomeConfig"
//                 << "ContextConfig" << "DialerConfig"
//                 << "CallScreenConfig" << "DecorationConfig"
//                 << "SecondaryTitleConfig" << "SecondaryHomeConfig"
//                 << "BootChargerConfig";
//             for (QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); it++){
//                 if (themeConfig.contains(*it)){
//                     QFileInfo info(Qtopia::qtopiaDir() + "etc/themes/" + themeConfig.value(*it).toString());
//                     if (!info.isFile()){
//                         qLog(UI) << "QSettings entry" << (*it).toLocal8Bit().data()
//                                 << "in" << configFileName.toLocal8Bit().data()
//                                 << "points to non-existant file" << info.filePath().toLocal8Bit().data();
//                         valid = false;
//                         break;
//                     }
//                 }
//             }
//             if ( !valid ) {
//                 // this theme does not have all the necessary files, try next one
//                 qLog(UI) << "Missing files, ignore" << configFileName.toLocal8Bit().data();
//                 continue;
//             }

            if ( !themeConfig.contains( "Name" ) ) {
                // this theme does not have a name, try next one
                qLog(UI) << "No name, ignore" << configFileName.toLocal8Bit().data();
                continue;
            }

            themeName = themeConfig.value( "Name" ).toString();
            decorationName = themeConfig.value("DecorationConfig").toString();

            ThemeInfo theme( themeName );
            theme.setStyleName( styleName );
            theme.setThemeFileName( dir[index] );
            theme.setDecorationFileName( decorationName );
            theme.setServerWidget( serverWidgetName );
            theme.setColorSchemes( colSchemes );
            theme.setBackgroundImages( bg );
            theme.extendedFocusHighlight = themeConfig.value("ExtendedFocusHighlight", "1").toString();
            theme.formStyle = themeConfig.value("FormStyle", "QtopiaDefaultStyle").toString();
            theme.popupShadows = themeConfig.value("PopupShadows", "0").toString();
            theme.hideMenuIcons = themeConfig.value("HideMenuIcons", "1").toString();
            theme.fullWidthMenu = themeConfig.value("FullWidthMenu", "1").toString();
            themes.append( theme );
        }
    }
}

void AppearanceSettings::readColorScheme()
{
    QDir dir;
    QString colorSchemePath( Qtopia::qtopiaDir() + "etc/colors/" );
    if ( !dir.exists( colorSchemePath ) ) {
        qLog(UI) << "Color scheme configuration path not found" << colorSchemePath.toLocal8Bit().data(); // No tr
        return;
    }

    dir.setPath( colorSchemePath );
    dir.setNameFilters( QStringList( "*.scheme" )); // No tr

    for (int index = 0; index < (int)dir.count(); index++) {
        QString name = dir[index].left( dir[index].indexOf( ".scheme" ) );
        colorListIDs.append(name);
    }
}

void AppearanceSettings::themeSelected( int index )
{
    QString dir = themes[index].themeFileName();
    dir.chop(5);

    titlePreview->setXmlFilename(Qtopia::qtopiaDir() + "etc/themes/" + dir + "/title.xml");
    contextPreview->setXmlFilename(Qtopia::qtopiaDir() + "etc/themes/" + dir + "/context.xml");

    bgCombo->clear();
    if ( themes[index].backgroundImages().count() ) {
        foreach ( QString bg, themes[index].backgroundImages() ) {
            bgCombo->addItem( bg );
            if ( bg == mActiveBackground )
                bgCombo->setCurrentIndex( bgCombo->count() - 1 );
        }
    }

    if ( !colorListIDs.count() )
        return;

    colorCombo->clear();

    if ( themes[index].colorSchemes().count() ) {
        foreach ( QString colorId, themes[index].colorSchemes() ) {
            QString col = colorId.left( colorId.indexOf( ".scheme" ) );
            if ( colorListIDs.contains( col ) ) {
                QTranslatableSettings scheme(Qtopia::qtopiaDir() + 
                        "etc/colors/" + col + ".scheme", QSettings::IniFormat);
                colorCombo->addItem( scheme.value("Global/Name", col).toString(), col );
                if ( col == mActiveColor )
                    colorCombo->setCurrentIndex( colorCombo->count() - 1 );
            }
        }
        return;
    }

    // theme doesn't have preference, show all color schemes
    int defaultIdx = 0;
    foreach ( QString colorId, colorListIDs ) {
        QTranslatableSettings scheme(Qtopia::qtopiaDir() + 
                "etc/colors/" + colorId + ".scheme", QSettings::IniFormat);
        colorCombo->addItem( scheme.value("Global/Name", colorId).toString(), colorId );
        if ( colorId == mActiveColor )
            colorCombo->setCurrentIndex( colorCombo->count() - 1 );
        if ( colorId == "Qtopia" )
            defaultIdx = colorCombo->count() - 1;
    }
    if ( !colorCombo->currentIndex() )
        colorCombo->setCurrentIndex( defaultIdx );
}

void AppearanceSettings::colorSelected( int index )
{
    QString text = colorCombo->itemData( index ).toString();
    if (!text.isNull()) {
        titlePreview->setColor(Qtopia::qtopiaDir() + "etc/colors/" + text + ".scheme");
        contextPreview->setColor(Qtopia::qtopiaDir() + "etc/colors/" + text + ".scheme");
        titlePreview->requestPreview();
        contextPreview->requestPreview();
    }
}

void AppearanceSettings::labelToggled( bool toggled )
{
    contextPreview->setIconLabel(toggled);
    contextPreview->requestPreview();
}

void AppearanceSettings::backgroundSelected( const QString &text )
{
    if (text.isNull()) {
        backgroundLabel.setPixmap(QPixmap());
        return;
    }

    QString themeFileName;

    foreach(ThemeInfo info, themes) {
        if (info.themeName() == themeCombo->currentText())
            themeFileName = info.themeFileName();
    }
    themeFileName.chop(5);

    QDesktopWidget *desktop = QApplication::desktop();
    int width = desktop->screenGeometry(desktop->primaryScreen()).width();
    QPixmap back(Qtopia::qtopiaDir() + "pics/themes/" + themeFileName + '/' + text + ".png");
    backgroundLabel.setPixmap(back.scaledToWidth((int)(width * 0.2), Qt::SmoothTransformation));
}

void AppearanceSettings::setTitlePixmap(const QPixmap &pixmap)
{
    titleLabel.setPixmap(pixmap);
    groupBox->show();
}

void AppearanceSettings::setContextPixmap(const QPixmap &pixmap)
{
    contextLabel.setPixmap(pixmap);
    groupBox->show();
}

void AppearanceSettings::applyTheme()
{
    ThemeInfo *theme = &themes[themeCombo->currentIndex()];
    if ( !theme )
        return;

    gConfig.beginGroup( "Appearance" );
    if (!theme->themeFileName().isEmpty() && (theme->themeFileName() != themeCombo->currentText()) ){
        gConfig.setValue("Style", theme->styleName());
        gConfig.setValue("Theme", theme->themeFileName());
        gConfig.setValue("DecorationTheme", theme->decorationFileName());
        qLog(UI) << "Write config theme select" << theme->styleName().toLatin1().data() <<
            themeCombo->currentText().toLatin1().data();
    } else {
        QString s = theme->themeFileName().isEmpty() ? themeCombo->currentText() : theme->themeFileName();
        qLog(UI) << "Write simple config theme select" << theme->styleName().toLatin1().data() <<
                themeCombo->currentText().toLatin1().data();
        gConfig.setValue( "Style", s );
        gConfig.setValue( "Theme", "");
        gConfig.setValue( "DecorationTheme", "");
    }
    gConfig.endGroup();

    gConfig.beginGroup("Style");
    gConfig.setValue("ExtendedFocusHighlight", theme->extendedFocusHighlight);
    gConfig.setValue("FormStyle", theme->formStyle);
    gConfig.setValue("PopupShadows", theme->popupShadows);
    gConfig.setValue("HideMenuIcons", theme->hideMenuIcons);
    gConfig.setValue("FullWidthMenu", theme->fullWidthMenu);
    gConfig.endGroup();

    gConfig.sync();

    if ( !theme->serverWidget().isEmpty() ) {
        QSettings cfg( "Trolltech", "ServerWidgets" );
        cfg.beginGroup( "Mapping" );
        cfg.remove(""); //delete all entries in current grp
        cfg.setValue("Default", theme->serverWidget());

        if (mServerWidgets != theme->serverWidget()) {
            QtopiaIpcEnvelope env( "QPE/System", "restart()" );
            QtopiaApplication::quit();
        }
    }

    QtopiaChannel::send("QPE/System", "applyStyleSplash()");
}

void AppearanceSettings::applyBackgroundImage()
{
    QString s = bgCombo->currentText();
    gConfig.beginGroup( "Appearance" );
    gConfig.setValue( "BackgroundImage", s );
    gConfig.endGroup();
    gConfig.sync();
    QtopiaChannel::send("QPE/System", "applyHomeScreenImage()");
}

void AppearanceSettings::applyColorScheme()
{
    QString s = colorCombo->itemData( colorCombo->currentIndex() ).toString();
    gConfig.beginGroup( "Appearance" );
    gConfig.setValue( "Scheme", s );

    QSettings scheme(Qtopia::qtopiaDir() + "etc/colors/" + s + ".scheme", QSettings::IniFormat);
    if (scheme.status()==QSettings::NoError){
        scheme.beginGroup("Colors");
        QString color = scheme.value( "Background", "#EEEEEE" ).toString();
        gConfig.setValue( "Background", color );
        QString alpha = scheme.value( "Background_alpha", "64" ).toString();
        gConfig.setValue( "Background_alpha", alpha );
        color = scheme.value( "Foreground", "#000000" ).toString();
        gConfig.setValue( "Foreground", color );
        color = scheme.value( "Button", "#F0F0F0" ).toString();
        gConfig.setValue( "Button", color );
        alpha = scheme.value( "Button_alpha", "176" ).toString();
        gConfig.setValue( "Button_alpha", alpha );
        color = scheme.value( "Highlight", "#8BAF31" ).toString();
        gConfig.setValue( "Highlight", color );
        alpha = scheme.value( "Highlight_alpha", "176" ).toString();
        gConfig.setValue( "Highlight_alpha", alpha );
        color = scheme.value( "HighlightedText", "#FFFFFF" ).toString();
        gConfig.setValue( "HighlightedText", color );
        color = scheme.value( "Text", "#000000" ).toString();
        gConfig.setValue( "Text", color );
        color = scheme.value( "ButtonText", "#000000" ).toString();
        gConfig.setValue( "ButtonText", color );
        color = scheme.value( "Base", "#FFFFFF" ).toString();
        gConfig.setValue( "Base", color );
        alpha = scheme.value( "Base_alpha", "176" ).toString();
        gConfig.setValue( "Base_alpha", alpha );
        color = scheme.value( "AlternateBase", "#CBEF71" ).toString();
        gConfig.setValue( "AlternateBase", color );
        alpha = scheme.value( "AlternateBase_alpha", "176" ).toString();
        gConfig.setValue( "AlternateBase_alpha", alpha );
        color = scheme.value( "Text_disabled", "" ).toString();
        gConfig.setValue("Text_disabled", color);
        color = scheme.value( "Foreground_disabled", "" ).toString();
        gConfig.setValue("Foreground_disabled", color);
        color = scheme.value( "Shadow", "" ).toString();
        gConfig.setValue("Shadow", color);
        color = scheme.value( "Link", "#0000FF" ).toString();
        gConfig.setValue( "Link", color );
        color = scheme.value( "LinkVisited", "#FF00FF" ).toString();
        gConfig.setValue( "LinkVisited", color );
    }
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "applyStyle()");

    // colorize theme areas
    QtopiaChannel::send("QPE/System", "applyStyleNoSplash()");
}

void AppearanceSettings::applyLabel()
{
    gConfig.beginGroup( "ContextMenu" );
    gConfig.setValue( "LabelType", labelCkBox->isChecked() ? (int)QSoftMenuBar::IconLabel : (int)QSoftMenuBar::TextLabel );
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "updateContextLabels()");
}

void AppearanceSettings::pushSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pushSettingStatus(QString,QString,QString)" );
    e << QString( "appearance" ) << QString( windowTitle() ) << status();
    e.send();
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
    status += QString::number( themeCombo->currentIndex() ) + ",";
    status += QString::number( colorCombo->currentIndex() ) + ",";
    status += QString::number( bgCombo->currentIndex() ) + ",";
    status += QString::number( labelCkBox->isChecked() ) + ",";
    return status;
}

void AppearanceSettings::setStatus( const QString details )
{
    QStringList s = details.split( ',' );
    themeCombo->setCurrentIndex( s.at( 0 ).toInt() );
    colorCombo->setCurrentIndex( s.at( 1 ).toInt() );
    bgCombo->setCurrentIndex( s.at( 2 ).toInt() );
    labelCkBox->setCheckState( s.at( 3 ).toInt() ? Qt::Checked : Qt::Unchecked );
}

void AppearanceSettings::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds( data );
    if ( msg == "Settings::setStatus(bool,QString)" ) {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        isStatusView = true;
        QSoftMenuBar::removeMenuFrom( this, QSoftMenuBar::menuFor( this ) );
        QSoftMenuBar::menuFor(this);
        QString details;
        ds >> isFromActiveProfile;
        ds >> details;
        setStatus( details );
        applyStyle();
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        hide();
        QString details;
        ds >> details;
        setStatus( details );
        applyStyle();
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        hide();
        pullSettingStatus();
    } else if ( msg == "Settings::activateDefault()" ) {
        hide();
        int i = 0;
        foreach ( ThemeInfo theme, themes ) {
            if ( theme.themeFileName() == "qtopia.conf" ) {
                themeCombo->setCurrentIndex( i );
                break;
            }
            i++;
        }
        for ( i = 0; i < colorCombo->count(); i++ ) {
            if ( colorCombo->itemData( i ).toString() == "Qtopia" ) {
                colorCombo->setCurrentIndex( i );
                break;
            }
        }
        labelCkBox->setCheckState( Qt::Checked );
        applyStyle();
    }
}

void AppearanceSettings::openHomescreenSettings()
{
    QtopiaIpcEnvelope env( "QPE/Application/homescreen", "HomescreenSettings::configure()" );
}

