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

#include "appearance.h"
#include "themeparser.h"

#include <QtopiaApplication>
#include <qtopianamespace.h>
#include <QSoftMenuBar>
#include <QKeyEvent>
#include <qtopialog.h>
#include <qtopiaservices.h>
#include <QMenu>
#include <QDesktopWidget>
#include <QTimer>
#include <QtopiaIpcEnvelope>
#include <QPhoneProfileManager>

AppearanceSettings::AppearanceSettings( QWidget* parent, Qt::WFlags fl )
    : QDialog(parent, fl)
#ifdef QTOPIA_PHONE
, bgControl(0), isStatusView(false), isFromActiveProfile(false)
#endif
{
    setupUi(this);

#ifdef QTOPIA_PHONE
    tabWidget->setTabIcon(0, QIcon(":icon/theme"));
    tabWidget->setTabText(0, QString());
    tabWidget->setTabIcon(1, QIcon(":icon/color"));
    tabWidget->setTabText(1, QString());
    tabWidget->setTabIcon(2, QIcon(":icon/info"));
    tabWidget->setTabText(2, QString());
    tabWidget->setTabIcon(3, QIcon(":icon/background"));
    tabWidget->setTabText(3, QString());

    connect( tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)) );

    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
        this, SLOT(receive(const QString&,const QByteArray&)) );

    QAction *actionCapture = new QAction( QIcon( ":icon/Note" ), tr( "Add to profile" ), this );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pushSettingStatus()) );
    QSoftMenuBar::menuFor( this )->addAction( actionCapture );

    // create controls
    itemControls[themeList] = new ThemeAppearanceItemControl(themeList);
    itemControls[colorList] = new ColorAppearanceItemControl(colorList);
    itemControls[smbLabelTypeList] = new LabelAppearanceItemControl(smbLabelTypeList);
    bgControl = new BackgroundAppearanceItemControl(previewLabel);
    itemControls[previewLabel] = bgControl;

    // set buttons to background control
    bgControl->mPreviewLabel = previewLabel;
    bgControl->mMyPictures = myPictures;
    bgControl->mWallpaper = wallpaper;
    bgControl->mClearButton = clearButton;

    themeList->installEventFilter( this );
    colorList->installEventFilter( this );
    smbLabelTypeList->installEventFilter( this );
    myPictures->installEventFilter( this );
    wallpaper->installEventFilter( this );
    clearButton->installEventFilter( this );
#else
    // in landscape mode, change layout to QHBoxLayout
    if ( isWide() ) {
        delete vboxLayout;
        hBoxLayout = new QHBoxLayout( this );
        hBoxLayout->addWidget( tabWidget );
    }

    sample = new SampleWindow( this );
    vboxLayout->addWidget( sample );

    // create controls
    itemControls[colorList] = new ColorControl(colorList);
    itemControls[styleList] = new StyleControl(styleList);
    itemControls[decorationList] = new DecorationControl(decorationList);
    itemControls[fontList] = new FontConrol(fontList);
#endif
    QTimer::singleShot( 0, this, SLOT(populate()) );
}

AppearanceSettings::~AppearanceSettings()
{
}


void AppearanceSettings::populate()
{
    // populate items
    QMapIterator<QWidget*,AppearanceItemControl*> i(itemControls);
    while (i.hasNext()) {
        i.next();
        i.value()->run();
    }
}

void AppearanceSettings::accept()
{
#ifdef QTOPIA_PHONE
    QPhoneProfileManager profileManager;
    QPhoneProfile activeProfile = profileManager.activeProfile();

    AppearanceItemControl::ApplyItemType applyType = AppearanceItemControl::CurrentItem;

    if ( !isStatusView ) { // normal appearance setting operation
        QPhoneProfile::Setting setting = activeProfile.applicationSetting("appearance");
        if ( setting != QPhoneProfile::Setting() )
            pushSettingStatus();
    } else { // status view from profiles
        // save current status to the profile
        pushSettingStatus();
        // not from an active profile, revert to active status
        if (!isFromActiveProfile)
            applyType = AppearanceItemControl::ActiveItem;
    }
#endif
    applyStyle(applyType);

    QDialog::accept();
}

void AppearanceSettings::reject()
{
#ifdef QTOPIA_PHONE
    applyStyle(AppearanceItemControl::ActiveItem);
#endif
    QDialog::reject();
}

void AppearanceSettings::applyStyle(AppearanceItemControl::ApplyItemType itemType)
{
    QMapIterator<QWidget*,AppearanceItemControl*> i(itemControls);
    while (i.hasNext()) {
        i.next();
        i.value()->apply(itemType);
    }
}

static bool processKeyRelease = false;
static int nextRowUp = 0;
static int nextRowDown = 0;

bool AppearanceSettings::eventFilter(QObject *o, QEvent *e)
{
#ifdef QTOPIA_PHONE
    if ( e->type() == QEvent::KeyPress ) {
        QKeyEvent *ke = (QKeyEvent*) e;
        int key = ke->key();
        if ( o == themeList || o == colorList || o == smbLabelTypeList
        || o == myPictures || o == wallpaper || o == clearButton ) {
            int currentIndex = tabWidget->currentIndex();
            int count = tabWidget->count();
            switch ( key ) {
                case Qt::Key_Back:
                    e->ignore();
                    break;
                case Qt::Key_Left:
                    if ( layoutDirection() == Qt::RightToLeft && currentIndex < count - 1 )
                        ++currentIndex;
                    else if ( !layoutDirection() == Qt::RightToLeft && currentIndex > 0 )
                        --currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                    break;
                case Qt::Key_Right:
                    if ( layoutDirection() == Qt::RightToLeft && currentIndex > 0 )
                        --currentIndex;
                    else if ( !layoutDirection() == Qt::RightToLeft && currentIndex < count - 1 )
                        ++currentIndex;
                    tabWidget->setCurrentIndex( currentIndex );
                    return true;
                    break;
                case Qt::Key_Select:
                    if ( o == themeList || o == colorList || o == smbLabelTypeList ) {
                        itemControls[qobject_cast<QWidget*>(o)]->apply();
                    }
                    break;
                default: break;
            }
        }
        if ( ( o == themeList || o == colorList || o == smbLabelTypeList )
        && ( key == Qt::Key_Down || key == Qt::Key_Up ) ) {
            QListWidget *list = qobject_cast<QListWidget*>(o);
            // focus moves from the first item to the tab
            if ( list->currentRow() == 0 && key == Qt::Key_Up ) {
                nextRowUp = list->count() - 1; // item when Key_Up is pressed again from the tab
                nextRowDown = list->currentRow(); // item when Key_Down is pressed from the tab
                list->clearSelection();
                list->setEditFocus(false);
            }
            // focus moves from the last item to the tab
            else if ( list->currentRow() == list->count() - 1 && key == Qt::Key_Down ) {
                nextRowDown = 0; // item when Key_Down is pressed again from the tab
                nextRowUp = list->currentRow(); // item when Key_Up is pressed from the tab
                list->clearSelection();
                list->setEditFocus(false);
            }
            // focus moves between list items.
            else {
                list->setEditFocus(true);
            }
            return false;
        }
    }
    // when list widget gaining focus again from the tab
    // cannot give the widget edit focus from here
    // because key might be either Key_Down or Key_Up
    // so set flag to process the edit focus when key release event is received.
    if ( e->type() == QEvent::FocusIn ) {
        QListWidget *list = qobject_cast<QListWidget*>(o);
        if ( list && list->count() > 0 ) { // list should be populated.
            processKeyRelease = true;
            return true;
        }
    }
    // when focus moves from the tab to the list
    // select different item depending on kye type.
    if ( e->type() == QEvent::KeyRelease && processKeyRelease ) {
        QKeyEvent *ke = (QKeyEvent*) e;
        QListWidget *list = qobject_cast<QListWidget*>(o);
        if ( list ) {
            if ( ke->key() == Qt::Key_Up ) {
                list->setCurrentItem( list->item(nextRowUp) );
            } else if ( ke->key() == Qt::Key_Down ) {
                list->setCurrentItem( list->item(nextRowDown) );
            }
            list->setEditFocus( true );
        }
        processKeyRelease = false;
        return true;
    }
#endif
    return false;
}

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

#ifndef QTOPIA_PHONE
void AppearanceSettings::fixSampleGeometry()
{
    sample->fixGeometry();
    sample->setUpdatesEnabled( true );
    sample->repaint();  // repaint decoration
}

bool AppearanceSettings::isWide()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QSize dSize = desktop->availableGeometry(desktop->primaryScreen()).size();
    return (dSize.height() < 300 && dSize.height() < dSize.width());
}
#endif

#ifdef QTOPIA_PHONE
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
    status += QString::number( themeList->currentRow() ) + ",";
    status += QString::number( colorList->currentRow() ) + ",";
    status += bgControl->bgImgName + ",";
    status += QString::number( bgControl->bgDisplayMode ) + ",";
    status += QString::number( smbLabelTypeList->currentRow() ) + ",";
    return status;
}

void AppearanceSettings::setStatus( const QString details )
{
    QStringList s = details.split( ',' );
    themeList->setCurrentItem( themeList->item( s.at( 0 ).toInt() ) );
    colorList->setCurrentItem( colorList->item( s.at( 1 ).toInt() ) );
    bgControl->bgImgName = s.at( 2 );
    bgControl->bgDisplayMode = s.at( 3 ).toInt();
    smbLabelTypeList->setCurrentItem( smbLabelTypeList->item( s.at( 4 ).toInt() ) );
    if ( !bgControl->bgImgName.isEmpty() )
        bgControl->updatePreview();
}

void AppearanceSettings::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds( data );
    if ( msg == "Settings::setStatus(bool,QString)" ) {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        isStatusView = true;
        QSoftMenuBar::removeMenuFrom( this, QSoftMenuBar::menuFor( this ) );
        QString details;
        ds >> isFromActiveProfile;
        ds >> details;
        setStatus( details );
        applyStyle();
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        QString details;
        ds >> details;
        setStatus( details );
        applyStyle();
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        pullSettingStatus();
    } else if ( msg == "Settings::activateDefault()" ) {
        applyStyle(AppearanceItemControl::DefaultItem);
    }
}

void AppearanceSettings::tabChanged( int curIndex )
{
    if ( curIndex == 3 )
        bgControl->updatePreview();
    else {
        QListWidget *list = 0;
        switch ( curIndex ) {
        case 0: list = themeList; break;
        case 1: list = colorList; break;
        case 2: list = smbLabelTypeList; break;
        default: break;
        }
        if ( list ) {
            list->setEditFocus( true );
            // tab, not the item,  might have been selected before
            // set selection to the last selected item.
            list->setItemSelected( list->currentItem(), true );
        }
    }
}
#endif

