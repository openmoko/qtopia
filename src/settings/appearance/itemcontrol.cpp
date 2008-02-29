/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "itemcontrol.h"

#include <QObject>
#include <QWindowsStyle>
#include <QApplication>
#include <QSoftMenuBar>
#include <QPalette>
#include <QLabel>
#include <QPushButton>
#include <QThumbnail>
#include <QDialog>
#include <QVBoxLayout>
#include <QTranslatableSettings>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtopiaApplication>
#include <QContentFilter>
#include <QContentSet>
#include <QImageDocumentSelectorDialog>
#include <QDrmContent>
#include <QtopiaChannel>
#include <qtopianamespace.h>
#include <qtopialog.h>
#ifndef QTOPIA_PHONE
# include <qtopia/qtopiastyle.h>
#endif
#include <QStorageMetaInfo>

// PluginItem ----------------------------------------------------
PluginItem::PluginItem( const QString &t, QListWidget *lb )
    : QListWidgetItem(t, lb) {}
void PluginItem::setFilename( const QString fn ) { file = fn; }
const QString & PluginItem::filename() const { return file; }
void PluginItem::setThemeFilename( const QString fn ) { themeFile = fn; }
const QString & PluginItem::themeFilename() const { return themeFile; }
void PluginItem::setDecorationFilename( const QString fn ) { decorationFile = fn; }
const QString & PluginItem::decorationFilename() const { return decorationFile; }

// DefaultWindowDecoration ----------------------------------------------------
DefaultWindowDecoration::DefaultWindowDecoration() {}

QString DefaultWindowDecoration:: name() const
{
    return qApp->translate("WindowDecoration", "Default",
    "List box text for default window decoration");
}

QPixmap DefaultWindowDecoration::icon() const
{
    return QPixmap();
}

static QSettings gConfig("Trolltech", "qpe");

// AppearanceItemControl ----------------------------------------------------
AppearanceItemControl::AppearanceItemControl(QWidget *view)
    : QThread(view), mView(view), mList(0)
, mDefaultItem(0), mActiveItem(0), mLastAppliedItem(0), mApplied(false)
{
    mList = qobject_cast<QListWidget*>(view);
    if (mList) {
        connect(mList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(currentItemChanged(QListWidgetItem*)));
        mList->setFrameStyle(QFrame::NoFrame);
        // preview label
        QSoftMenuBar::setLabel(mList, Qt::Key_Select, QSoftMenuBar::View);
    }
}

AppearanceItemControl::~AppearanceItemControl()
{}

void AppearanceItemControl::run()
{
    populate();
}

// read config and populate the list or do something equivalent
// call this after list is populated
void AppearanceItemControl::populate()
{
    mLastAppliedItem = mList->currentItem();

    QFont fnt(mList->font());
    fnt.setBold( true );
    mLastAppliedItem->setFont(fnt);

    mApplied = true;
}

// set asked item to apply
// call this first before doing anything specific
void AppearanceItemControl::apply(ApplyItemType itemType)
{
    if (!mList)
        return;

    switch (itemType) {
    case DefaultItem:
        mList->setCurrentItem(mDefaultItem);
        break;
    case ActiveItem:
        mList->setCurrentItem(mActiveItem);
        break;
    case CurrentItem:
        break;
    }

    if (mLastAppliedItem != mList->currentItem()) {
        mLastAppliedItem = mList->currentItem();
        mApplied = false;
    } else {
        mApplied = true;
    }
}

// override this to apply the change immediately
void AppearanceItemControl::currentItemChanged(QListWidgetItem *)
{
    mApplied = false;
}

// ColorAppearanceItemControl ----------------------------------------------------
ColorAppearanceItemControl::ColorAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{}

ColorAppearanceItemControl::~ColorAppearanceItemControl()
{}

void ColorAppearanceItemControl::populate()
{
    if (!mList)
        return;

    QDir dir( Qtopia::qtopiaDir() + "etc/colors" );
    QStringList list = dir.entryList( QStringList("*.scheme") ); // No tr
    QStringList::Iterator it;
    colorListIDs.clear();
    for ( it = list.begin(); it != list.end(); ++it ) {
        QTranslatableSettings scheme(Qtopia::qtopiaDir() + "etc/colors/" + *it, QSettings::IniFormat);
        QString name = (*it).left( (*it).indexOf( ".scheme" ) );
        colorListIDs.append(name);
        scheme.beginGroup("Global");
        QListWidgetItem *item = new QListWidgetItem(scheme.value("Name",name+"-DEF").toString(), mList);
        // set default
        if ( name == tr( "Qtopia" ) )
            mDefaultItem = item;
    }

    // set current
    gConfig.beginGroup( "Appearance" );
    QString cur = gConfig.value( "Scheme", "Qtopia" ).toString();
    gConfig.endGroup();
    gConfig.sync();

    mList->setCurrentItem( mList->item(colorListIDs.indexOf(cur)) );
    mActiveItem = mList->currentItem();

    AppearanceItemControl::populate();
}

void ColorAppearanceItemControl::currentItemChanged(QListWidgetItem *colorItem)
{
#ifndef QTOPIA_PHONE
    int id = mList->row(colorItem);
    QPalette pal = readColorScheme(id);
    mSample->setPalette( pal );
#else
    Q_UNUSED( colorItem );
#endif
    AppearanceItemControl::currentItemChanged(colorItem);
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

QPalette ColorAppearanceItemControl::readColorScheme(int id)
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

void ColorAppearanceItemControl::apply(ApplyItemType itemType)
{
    AppearanceItemControl::apply(itemType);

    if (mApplied)
        return;

    gConfig.beginGroup( "Appearance" );

    QString s = colorListIDs[mList->row(mList->currentItem())];
    gConfig.setValue( "Scheme", s );

    QSettings scheme(Qtopia::qtopiaDir() + "etc/colors/" + s + ".scheme", QSettings::IniFormat);
    if (scheme.status()==QSettings::NoError){
        scheme.beginGroup("Colors");
        QString color = scheme.value( "Background", "#EEEEEE" ).toString();
        gConfig.setValue( "Background", color );
        color = scheme.value( "Foreground", "#000000" ).toString();
        gConfig.setValue( "Foreground", color );
        color = scheme.value( "Button", "#F0F0F0" ).toString();
        gConfig.setValue( "Button", color );
        color = scheme.value( "Highlight", "#8BAF31" ).toString();
        gConfig.setValue( "Highlight", color );
        color = scheme.value( "HighlightedText", "#FFFFFF" ).toString();
        gConfig.setValue( "HighlightedText", color );
        color = scheme.value( "Text", "#000000" ).toString();
        gConfig.setValue( "Text", color );
        color = scheme.value( "ButtonText", "#000000" ).toString();
        gConfig.setValue( "ButtonText", color );
        color = scheme.value( "Base", "#FFFFFF" ).toString();
        gConfig.setValue( "Base", color );
        color = scheme.value( "AlternateBase", "#CBEF71" ).toString();
        gConfig.setValue( "AlternateBase", color );
        color = scheme.value( "Text_disabled", "" ).toString();
        gConfig.setValue("Text_disabled", color);
        color = scheme.value( "Foreground_disabled", "" ).toString();
        gConfig.setValue("Foreground_disabled", color);
        color = scheme.value( "Shadow", "" ).toString();
        gConfig.setValue("Shadow", color);
    }
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "applyStyle()");
#ifdef QTOPIA_PHONE
    // colorize theme areas
    QtopiaChannel::send("QPE/System", "applyStyleNoSplash()");
#endif

    mApplied = true;
}

#ifdef QTOPIA_PHONE
// ThemeAppearanceItemControl ----------------------------------------------------
ThemeAppearanceItemControl::ThemeAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{}

ThemeAppearanceItemControl::~ThemeAppearanceItemControl()
{}

void ThemeAppearanceItemControl::populate()
{
    if (!mList)
        return;

    gConfig.beginGroup( "Appearance" );
    QString current = gConfig.value("Theme").toString();
    gConfig.endGroup();
    gConfig.sync();

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
                    PluginItem *item = new PluginItem( themeName, mList );

                    // set default
                    if ( themeName == tr( "Qtopia" ) )
                        mDefaultItem = item;

                    item->setFilename( styleName );
                    item->setThemeFilename( dir[index] );
                    item->setDecorationFilename( decorationName );

                    // set current
                    if ( configFileName.contains( current ) ) {
                        mList->setCurrentItem( item );
                        mActiveItem = item;
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

    AppearanceItemControl::populate();
}

void ThemeAppearanceItemControl::apply(ApplyItemType itemType)
{
    AppearanceItemControl::apply(itemType);

    if (mApplied)
        return;

    gConfig.beginGroup( "Appearance" );

    PluginItem *item = (PluginItem *)mList->currentItem();
    if ( item ) {
        if (!item->themeFilename().isEmpty() && (item->themeFilename() != item->text()) ){
            gConfig.setValue("Style", item->filename());
            gConfig.setValue("Theme", item->themeFilename());
            gConfig.setValue("DecorationTheme", item->decorationFilename());
            qLog(UI) << "Write config theme select" << item->filename().toLatin1().data() <<
                item->text().toLatin1().data();
        } else {
            QString s = item->themeFilename().isEmpty() ? item->text() : item->themeFilename();
            qLog(UI) << "Write simple config theme select" << item->filename().toLatin1().data() <<
                item->text().toLatin1().data();
            gConfig.setValue( "Style", s );
            gConfig.setValue( "Theme", "");
            gConfig.setValue( "DecorationTheme", "");
        }
    }
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "applyStyleSplash()");

    mApplied = true;
}

// LabelAppearanceItemControl ----------------------------------------------------
LabelAppearanceItemControl::LabelAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{}

LabelAppearanceItemControl::~LabelAppearanceItemControl()
{}

void LabelAppearanceItemControl::populate()
{
    if (!mList)
        return;

    gConfig.beginGroup( "ContextMenu" );
    int type = gConfig.value( "LabelType", QSoftMenuBar::IconLabel ).toInt();
    gConfig.endGroup();
    gConfig.sync();

    mList->addItem( new QListWidgetItem( tr( "Icon Label" ) ) );
    mList->addItem( new QListWidgetItem( tr( "Text Label" ) ) );
    mList->setCurrentItem( mList->item( type ) );

    mDefaultItem = mList->item(0);
    mActiveItem = mList->currentItem();

    AppearanceItemControl::populate();
}

void LabelAppearanceItemControl::apply(ApplyItemType itemType)
{
    AppearanceItemControl::apply(itemType);

    if (mApplied)
        return;

    gConfig.beginGroup( "ContextMenu" );
    gConfig.setValue( "LabelType", mList->currentRow() );
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "updateContextLabels()");

    mApplied = true;
}

// BackgroundAppearanceItemControl ----------------------------------------------------

// a hidden file name, used to copy an image from any removable storage
#define EXPORTED_BG_NAME ".ExportedBGImage"
#define EXPORTED_BG_PATH Qtopia::documentDir() + EXPORTED_BG_NAME

BackgroundAppearanceItemControl::BackgroundAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view), mPreviewLabel(0), mMyPictures(0), mWallpaper(0), mClearButton(0)
,bgDisplayMode(0)
{}

BackgroundAppearanceItemControl::~BackgroundAppearanceItemControl()
{}

void BackgroundAppearanceItemControl::populate()
{
    // initialize
    if (mPreviewLabel) {
        mPreviewLabel->setAlignment( Qt::AlignCenter );
        mPreviewLabel->setWordWrap( true );
    }

    gConfig.beginGroup( "Appearance" );
    bgImgName = gConfig.value("BackgroundImage").toString();
    activeImgName = bgImgName; // cache active image
    // bg display mode 1 = standard
    // mode 2 = maximized
    // mode 3 = tiled
    bgDisplayMode = gConfig.value("BackgroundMode", 0).toInt();
    gConfig.endGroup();
    gConfig.sync();

    if ( !bgImgName.isEmpty() && bgImgName[0]!='/' )
        bgImgName = QString(":image/wallpaper/"+bgImgName);

    connect(mMyPictures, SIGNAL(clicked()), this, SLOT(selectImage()));
    connect(mWallpaper, SIGNAL(clicked()), this, SLOT(selectWallpaper()));
    connect(mClearButton, SIGNAL(clicked()), this, SLOT(clearBackground()));
}

void BackgroundAppearanceItemControl::selectImage(void)
{
    selectBackground(false);
}

void BackgroundAppearanceItemControl::selectWallpaper(void)
{
    selectBackground(true);
}

void BackgroundAppearanceItemControl::selectBackground(bool wallpapers)
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

    QImageDocumentSelectorDialog sel;
    sel.setFilter( locations );
    sel.setMandatoryPermissions( QDrmRights::Automated );
    if (QtopiaApplication::execDialog(&sel)) {
        QContent doc = sel.selectedDocument();

        if(doc.fileKnown() && displayMode() ) { // display mode must be selected
            qLog(Resource) << "Selected Document:" << doc.file();
            // remove old image
            QFile::remove( EXPORTED_BG_PATH );
            checkFileSystem( doc );
            mPreviewLabel->setText(tr("Loading..."));
            updatePreview();
        }
    }
}

void BackgroundAppearanceItemControl::checkFileSystem( const QContent doc )
{
    // check if the file is from a removable storage
    QStorageMetaInfo storageInfo;
    const QFileSystem *fs = storageInfo.fileSystemOf(doc.file());

    if (fs && fs->isRemovable()) {
        qLog(Resource) << doc.file() << "is from a removable storage," << doc.media();

        // check if the file has Distribute right
        QDrmContent imgContent( QDrmRights::Distribute, QDrmContent::NoLicenseOptions );
        if ( imgContent.requestLicense( doc ) ) {
            qLog(Resource) << "File has Distribute right";
            copyImage( doc );
        } else {
            qLog(Resource) << "File does not have Distribute right";
            bgImgName = doc.file();
        }
    } else {
        bgImgName = doc.file();
    }
}

void BackgroundAppearanceItemControl::copyImage( QContent doc )
{
    // save a copy
    QFile oldFile( doc.file() );
    if ( oldFile.copy( EXPORTED_BG_PATH ) ) {
        bgImgName = EXPORTED_BG_PATH;
        qLog(Resource) << "Successfully copied" << doc.file() << "to" << EXPORTED_BG_PATH;
    } else {
        qLog(Resource) << "Failed to copy" << doc.file() << ". Need to use as is.";
        bgImgName = doc.file();
    }
}

void BackgroundAppearanceItemControl::updatePreview()
{
    if (bgImgName.isEmpty()) {
        mPreviewLabel->setText(tr("No image"));
        return;
    }

    if (mPreviewLabel->isVisible()) {
        QDrmContent imgContent( QDrmRights::Display, QDrmContent::NoLicenseOptions );
        if( imgContent.requestLicense( QContent( bgImgName ) ) )
        {
            QThumbnail thumbnail( bgImgName );
            int margin = mPreviewLabel->margin();
            QSize sz = mPreviewLabel->contentsRect().size()
                        - QSize(margin,margin)
                        - QSize(1,1); // prevents weird Qt bug.
            QPixmap bgImg = thumbnail.pixmap(sz);
            if (!bgImg.isNull()) {
                mPreviewLabel->setPixmap(bgImg);
                return;
            }
        }
        mPreviewLabel->setText(tr("Cannot load image"));
    }
}

void BackgroundAppearanceItemControl::clearBackground()
{
    bgImgName = QString();
    bgDisplayMode = 0;

    // remove copied image
    QFile::remove( EXPORTED_BG_PATH );

    updatePreview();
}

bool BackgroundAppearanceItemControl::displayMode()
{
    QDialog dlg;
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

void BackgroundAppearanceItemControl::apply(ApplyItemType itemType)
{
    switch (itemType) {
    case DefaultItem:
        bgImgName.clear();
        break;
    case ActiveItem:
        bgImgName = activeImgName;
        break;
    case CurrentItem:
        break;
    }

    gConfig.beginGroup( "Appearance" );
    gConfig.setValue("BackgroundImage", bgImgName);
    gConfig.setValue("BackgroundMode", bgDisplayMode );
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "applyBackgroundImage()");
}

#else // !QTOPIA_PHONE

// FontAppearanceItemControl ----------------------------------------------------
FontAppearanceItemControl::FontAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{
#ifdef DEBUG
    maxFontSize = 24;
#else
    maxFontSize = qApp->desktop()->width() >= 640 ? 24 : 12;
#endif

    if (mList) {
        connect(mList, SIGNAL(currentTextChanged(const QString&)),
            this, SIGNAL(fontSelected(const QString&)));
    }
}

FontAppearanceItemControl::~FontAppearanceItemControl()
{}

void FontAppearanceItemControl::populate()
{
    if (!mList)
        return;

    gConfig.beginGroup( "Appearance" );
    QString cur = gConfig.value( "FontFamily", "Helvetica" ).toString();
    prefFontSize = gConfig.value( "FontSize", font().pointSize() ).toInt();
    gConfig.endGroup();
    gConfig.sync();

    QFontDatabase fd;
    QStringList f = fd.families();
    for ( QStringList::const_iterator it=f.constBegin(); it!=f.constEnd(); ++it ) {
        QString n = *it;
        n[0] = n[0].toUpper();
        mList->addItem(n);
        if ( (*it).toLower() == cur.toLower() )
            fontList->setCurrentItem(fontList->item(fontList->count()-1));
    }
    if ( !fontList->currentItem() )
        fontList->setCurrentRow(0);
    fontSize->addItem( QString::number( prefFontSize ) );
    fontSelected( fontList->currentItem()->text() );
}

QFont FontAppearanceItemControl::fontSelected(const QString &name)
{
    QString selFontFamily = name;
    QFont font;

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

    return font;
}

void FontAppearanceItemControl::fontSizeSelected( const QString &sz )
{
    prefFontSize = sz.toInt();
    sample->setUpdatesEnabled( false );
    sample->setFont( QFont(fontList->currentItem()->text(),prefFontSize) );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );
}

void FonAppearanceItemControl::apply(ApplyItemType itemType)
{
    gConfig.beginGroup( "Appearance" );
    QFont font(mList->currentItem()->text(), mList->currentText().toInt());
    gConfig.setValue( "FontFamily", mList->currentItem()->text() );
    gConfig.setValue( "FontSize", mList->currentText().toDouble() );
    gConfig.endGroup();
    gConfig.sync();
}

// StyleAppearanceItemControl ----------------------------------------------------
StyleAppearanceItemControl::StyleAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{}

StyleAppearanceItemControl::~StyleAppearanceItemControl()
{}

void StyleAppearanceItemControl::populate()
{
    if (!mList)
        return;

    (void)new PluginItem( "Qtopia", mList );
    QStringList keys = QStyleFactory::keys();
    QStringList::Iterator it;
    for ( it = keys.begin(); it != keys.end(); ++it ) {
        //PluginItem *item = new PluginItem( *it, mList );
    }

    gConfig.beginGroup( "Appearance" );
    QString current = gConfig.value( "Style", "Qtopia" ).toString();
    gConfig.endGroup();
    gConfig.sync();

    if ( current == "QPE" ) current = "Qtopia";
    for ( int i = 0; i < mList->count(); i++ ) {
        PluginItem *item = (PluginItem*)mList->item(i);
        if ( item->filename() == current || item->text() == current ) {
            mList->setCurrentItem( item );
            break;
        }
    }
}

void StyleAppearanceItemControl::currentItemChanged(QListWidgetItem *styleItem)
{
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

    mSample->setUpdatesEnabled( false );
    QPixmapCache::clear();
    setStyle( mSample, newStyle );
    QTimer::singleShot( 0, this, SLOT(fixSampleGeometry()) );

    AppearanceItemControl::currentItemChanged(item);
}

void StyleAppearanceItemControl::setStyle( QWidget *w, QStyle *s )
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

void StyleAppearanceItemControl::apply(ApplyItemType itemType)
{
    gConfig.beginGroup( "Appearance" );

    PluginItem *item = (PluginItem *)mList->currentItem();
    if ( item ) {
        QString s = item->filename().isEmpty() ? item->text() : item->filename();
        gConfig.setValue( "Style", s );
    }
    gConfig.endGroup();
    gConfig.sync();
}

// DecorationAppearanceItemControl ----------------------------------------------------
DocorationAppearanceItemControl::DocorationAppearanceItemControl(QWidget *view)
    : AppearanceItemControl(view)
{
    wdiface = 0;
    wdLoader = new QPluginManager( "decorations" );
    wdIsPlugin = false;
}

DocorationAppearanceItemControl::~DocorationControl()
{
    delete wdLoader;
}

void DecorationAppearanceItemControl::populate()
{
    if (!mList)
        return;

    (void)new PluginItem( "Qtopia", mList );
    QStringList pluginList = wdLoader->list();
    QStringList::Iterator it;
    for ( it = pluginList.begin(); it != pluginList.end(); ++it ) {
        QWindowDecorationFactoryInterface *iface = 0;
        QObject *instance = wdLoader->instance(*it);
        iface = qobject_cast<QWindowDecorationFactoryInterface*>(instance);
        if (iface) {
            PluginItem *item = new PluginItem( iface->decoration(iface->keys()[0])->name(), mList );
            item->setFilename(*it);
        }
        if (instance)
            delete instance;
    }

    // set current
    gConfig.beginGroup( "Appearance" );
    QString current = gConfig.value( "Decoration" ).toString();
    gConfig.endGroup();
    gConfig.sync();
    for ( int i = 0; i < mList->count(); i++ ) {
        PluginItem *item = (PluginItem*)mList->item(i);
        if ( item->filename() == current || item->text() == current ) {
            mList->setCurrentItem( item );
            break;
        }
    }
    currentItemChanged( mList->currentItem() );
}

void DecorationAppearanceItemControl::currentItemChanged(QListWidgetItem *decItem)
{
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
    mSample->setDecoration( wdiface );
    mSample->repaint();

    AppearanceItemControl::currentItemChanged(item);
}

void DecorationAppearanceItemControl::apply(ApplyItemType itemType)
{
    gConfig.beginGroup( "Appearance" );

    PluginItem *item = (PluginItem *)mList->currentItem();
    if ( item ) {
        QString s = item->filename().isEmpty() ? item->text() : item->filename();
        gConfig.setValue( "Decoration", s );
    }
    gConfig.endGroup();
    gConfig.sync();
}
#endif
