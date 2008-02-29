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
#include "light.h"

#include <qtopiaapplication.h>

#include <custom.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#include <QDebug>
#include <QLayout>
#include <QScrollArea>
#include <QSettings>
#include <QTabWidget>

#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <QMenu>
#include <QAction>
#include <QTimer>
#endif

static const int S_PER_MIN = 60; // 60 seconds per minute

extern int qpe_sysBrightnessSteps();

LightSettings::LightSettings( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl), isStatusView( false )
{
    setWindowTitle(tr("Power Management"));
    QVBoxLayout * baseLayout = new QVBoxLayout( this );
    baseLayout->setMargin( 0 );

#ifdef QTOPIA_PHONE
    QTabWidget *tabWidget = new QTabWidget( this );
    baseLayout->addWidget( tabWidget );
    QScrollArea *sView = new QScrollArea(0);
    sView->setFocusPolicy(Qt::NoFocus);
    sView->setFrameStyle(QFrame::NoFrame);
    sView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget * container = new QWidget();
    QVBoxLayout *lightLayout = new QVBoxLayout(container);
    lightLayout->setMargin( 0 );
    b = new LightSettingsContainer();
    QtopiaApplication::setInputMethodHint( b->interval_dim, QtopiaApplication::AlwaysOff );
    QtopiaApplication::setInputMethodHint( b->interval_lightoff, QtopiaApplication::AlwaysOff );
    QtopiaApplication::setInputMethodHint( b->interval_suspend, QtopiaApplication::AlwaysOff );

    lightLayout->addWidget(b);
    sView->setWidget( container );
    sView->setWidgetResizable( true );
    baseLayout->addWidget(sView);
    tabWidget->addTab( sView, tr("Power Saving"));

    sView = new QScrollArea(0);
    sView->setFocusPolicy(Qt::NoFocus);
    sView->setFrameStyle(QFrame::NoFrame);
    sView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *displayWidget = new QWidget();
    QVBoxLayout *displayLayout = new QVBoxLayout( displayWidget );
    displayLayout->setMargin( 0 );
    d = new DisplaySettingsContainer(displayWidget);
    displayLayout->addWidget(d);
    QtopiaApplication::setInputMethodHint( d->interval_home, QtopiaApplication::AlwaysOff );
    sView->setWidget( displayWidget );
    sView->setWidgetResizable( true );
    tabWidget->addTab( sView, tr("Security") );

    // add context menu to push its status to Profiles
    contextMenu = QSoftMenuBar::menuFor( this );
    QAction* actionCapture = new QAction( QIcon( ":icon/Note" ), tr( "Add to profile" ), this );
    contextMenu->addAction( actionCapture );
    connect( actionCapture, SIGNAL(triggered()), this, SLOT(pushSettingStatus()) );
    connect( qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
        this, SLOT(receive(const QString&,const QByteArray&)) );

#else
    QScrollArea *sView = new QScrollArea( this );
    sView->setFocusPolicy(Qt::NoFocus);
    sView->setFrameStyle(QFrame::NoFrame);
    baseLayout->addWidget(sView);
    QWidget * container = new QWidget;
    QVBoxLayout *lightLayout = new QVBoxLayout(container);
    lightLayout->setMargin( 2 );
    b = new LightSettingsContainer(container);
    lightLayout->addWidget(b);
    sView->setWidget(container);
    sView->setWidgetResizable( true );
#endif

    connect( b->screensaver_dim, SIGNAL(stateChanged(int)), this, SLOT(updateEditBoxes()) );
    connect( b->screensaver_lightoff, SIGNAL(stateChanged(int)), this, SLOT(updateEditBoxes()) );
    connect( b->screensaver_suspend, SIGNAL(stateChanged(int)), this, SLOT(updateEditBoxes()) );
    connect( b->interval_dim, SIGNAL(valueChanged(int)), this, SLOT(updateLightOffMinValue(int)) );

    connect( d->display_home, SIGNAL(stateChanged(int)), this, SLOT(updateEditBoxes()) );
    connect( b->interval_dim, SIGNAL(valueChanged(int)), this, SLOT(updateSuspendMinValue(int)) );
    connect( b->interval_lightoff, SIGNAL(valueChanged(int)), this, SLOT(updateSuspendMinValue(int)) );
    connect( d->interval_home, SIGNAL(valueChanged(int)), this, SLOT(updateSuspendMinValue(int)) );

    b->powerSource = new QButtonGroup(this);
    b->powerSource->addButton(b->batteryButton);
    b->powerSource->addButton(b->externalButton);
    b->powerSource->setExclusive( true );
    b->batteryButton->setChecked( true );

    b->officon->setPixmap(QPixmap(":image/light-off"));
    b->brighticon->setPixmap(QPixmap(":image/light-on"));

#ifdef QTOPIA_PHONE
    QSettings hwConfig("Trolltech", "Hardware");
    hwConfig.beginGroup("PowerManagement");
    batteryMode.canSuspend = hwConfig.value("CanSuspend", false).toBool();
    externalMode.canSuspend = hwConfig.value("CanSuspendAC", false).toBool();
    hwConfig.endGroup();

    b->notnetworkedsuspend->hide();

    if (batteryMode.canSuspend || externalMode.canSuspend) {
        b->screensaver_suspend->setEnabled(true);
    } else {
        b->screensaver_suspend->hide();
        b->interval_suspend->hide();
    }

    b->biginfo->hide();
    if (Qtopia::mousePreferred())
        d->autoPhoneLock->hide();
#else
    batteryMode.canSuspend = true;
    externalMode.canSuspend = false;

    b->screensaver_suspend->setEnabled(true);
    if (!QPowerStatusManager::APMEnabled()) {
        b->powerSourceGroupBox->hide();
        b->notnetworkedsuspend->hide();
        b->screensaver_suspend->hide();
        b->interval_suspend->hide();
    }
#endif

    QSettings config("Trolltech","qpe");

    config.beginGroup("BatteryPower");
    batteryMode.intervalDim = config.value( "Interval_Dim", 20 ).toInt();
    batteryMode.intervalLightOff = config.value("Interval_LightOff", 30).toInt();
    batteryMode.intervalSuspend = config.value("Interval", 60).toInt();
    batteryMode.initbright = config.value("Brightness", 255).toInt();
    batteryMode.dim = config.value("Dim", true).toBool();
    batteryMode.lightoff = config.value("LightOff", false).toBool();
    batteryMode.suspend = config.value("Suspend", true).toBool();
    batteryMode.networkedsuspend = config.value("NetworkedSuspend", true).toBool();
    config.endGroup();

    config.beginGroup("ExternalPower");
    externalMode.intervalDim = config.value( "Interval_Dim", 20 ).toInt();
    externalMode.intervalLightOff = config.value("Interval_LightOff", 30).toInt();
    externalMode.intervalSuspend = config.value("Interval", 240).toInt();
    externalMode.initbright = config.value("Brightness", 255).toInt();
    externalMode.dim = config.value("Dim", true).toBool();
    externalMode.lightoff = config.value("LightOff", false).toBool();   //default to leave on
    externalMode.suspend = config.value("Suspend", true).toBool();
    externalMode.networkedsuspend = config.value("NetworkedSuspend",false).toBool();
    config.endGroup();

    //must set min > 0 the screen will become completely black
    int maxbright = qpe_sysBrightnessSteps();
    b->brightness->setMaximum( maxbright );
    b->brightness->setMinimum( 1 );
    b->brightness->setTickInterval( qMax(1,maxbright/16) );
    b->brightness->setSingleStep( qMax(1,maxbright/16) );
    b->brightness->setPageStep( qMax(1,maxbright/16) );

    currentMode = &batteryMode;

#ifdef QTOPIA_PHONE
    config.beginGroup("HomeScreen");
    showHomeScreen = config.value("ShowHomeScreen", false).toBool();
    intervalShowHomeScreen = config.value("Interval_HomeScreen", 300 ).toInt();
    if (!Qtopia::mousePreferred())
        keyLock = config.value("AutoKeyLock", "Disabled").toString();
    config.endGroup();
    connect( d->display_home, SIGNAL(stateChanged(int)),
            this, SLOT(backToHomeScreenClicked(int)) );
#endif

    applyMode();

    powerStatus = QPowerStatusManager::readStatus();
    connect(b->powerSource, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(powerTypeClicked(QAbstractButton*)));
    if ( powerStatus.acStatus() == QPowerStatus::Online ) {
        b->externalButton->setChecked(true);
        powerTypeClicked(b->externalButton);
    }

    initbright = currentMode->initbright;

    connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));

    QtopiaChannel *channel = new QtopiaChannel("Qtopia/PowerStatus", this);
    connect(channel, SIGNAL(received(const QString&, const QByteArray&)),
            this, SLOT(sysMessage(const QString&, const QByteArray&)));
}

LightSettings::~LightSettings()
{
}

static void set_fl(int bright)
{
    QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)" );
    e << bright;
    e.send();
}

void LightSettings::reject()
{
    set_fl(initbright);

    QDialog::reject();
    close();
}

void LightSettings::accept()
{
    if ( qApp->focusWidget() )
        qApp->focusWidget()->clearFocus();

    if ( isStatusView ) {
#ifdef QTOPIA_PHONE
        if ( isFromActiveProfile )
            saveConfig();
        pushSettingStatus();
#endif
    } else {
        saveConfig();
#ifdef QTOPIA_PHONE
        // if the current profile has power management item
        // update the details with the new setting
        QSettings cfg( "Trolltech", "PhoneProfile" );
        cfg.beginGroup( "Profiles" );
        QString activeProfile = cfg.value( "Selected", 1 ).toString();
        cfg.endGroup();
        cfg.beginGroup( "Profile " + activeProfile );
        QString settings = cfg.value( "SettingList" ).toString();
        if ( settings.contains( "light-and-power" ) )
            pushSettingStatus();
#endif
    }
    QDialog::accept();
    close();
}

void LightSettings::saveConfig()
{
    // safe call, always one selected.
    powerTypeClicked( b->powerSource->checkedButton() );

    // Set settings for current power source
    powerStatus = QPowerStatusManager::readStatus();
    if ( powerStatus.acStatus() == QPowerStatus::Online )
        currentMode = &externalMode;
    else
        currentMode = &batteryMode;

    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup("BatteryPower");
    writeMode(cfg, &batteryMode);
    cfg.endGroup();
    cfg.beginGroup("ExternalPower");
    writeMode(cfg, &externalMode);
    cfg.sync();

    int i_dim =      (currentMode->dim ? currentMode->intervalDim : 0);
    int i_lightoff = (currentMode->lightoff ? currentMode->intervalLightOff : 0);
    int i_suspend =  ((currentMode->canSuspend &&
                       currentMode->suspend) ? currentMode->intervalSuspend : 0);

#ifdef QTOPIA_PHONE
    showHomeScreen = d->display_home->isChecked();
    intervalShowHomeScreen = d->interval_home->value() * S_PER_MIN;

    QSettings config("Trolltech","qpe");
    config.beginGroup( "HomeScreen" );
    config.setValue( "ShowHomeScreen", showHomeScreen );
    config.setValue( "Interval_HomeScreen", intervalShowHomeScreen );

    if (!Qtopia::mousePreferred()) {
        if (d->autoPhoneLock->isChecked())
            keyLock = "Enabled";  //no tr
        else
            keyLock = "Disabled"; //no tr

        config.setValue( "AutoKeyLock", keyLock );
    }
    config.endGroup();
#endif

    set_fl(currentMode->initbright);

    QtopiaServiceRequest e("QtopiaPowerManager", "setIntervals(int,int,int)" );
#ifndef QTOPIA_PHONE
    e << i_dim << i_lightoff << i_suspend;
#else
    e << i_dim << i_lightoff << intervalShowHomeScreen;
    Q_UNUSED(i_suspend)
#endif
    e.send();
}

void LightSettings::writeMode(QSettings &config, PowerMode *mode)
{
    config.setValue( "Dim", mode->dim );
    config.setValue( "LightOff", mode->lightoff );
    config.setValue( "Interval_Dim", mode->intervalDim );
    config.setValue( "Interval_LightOff", mode->intervalLightOff );
    config.setValue( "Brightness", mode->initbright );
    if (mode->canSuspend) {
        config.setValue( "Interval", mode->intervalSuspend );
        config.setValue( "Suspend", mode->suspend );
    } else {
        config.setValue( "Interval", 0 );
        config.setValue( "Suspend", false );
    }
#ifndef QTOPIA_PHONE
    config.setValue( "NetworkedSuspend", mode->networkedsuspend );
#endif
}

void LightSettings::applyMode()
{
    b->interval_dim->setValue( currentMode->intervalDim );
    b->interval_lightoff->setValue( currentMode->intervalLightOff );
    b->interval_suspend->setValue( currentMode->intervalSuspend );
    int v = currentMode->initbright;
    b->brightness->setValue( (b->brightness->maximum()*v+128)/255 );

    b->screensaver_dim->setChecked( currentMode->dim );
    b->screensaver_lightoff->setChecked( currentMode->lightoff );
    b->screensaver_suspend->setEnabled( currentMode->canSuspend );
    b->screensaver_suspend->setChecked( currentMode->canSuspend && currentMode->suspend );
    b->notnetworkedsuspend->setChecked( !currentMode->networkedsuspend );
#ifdef QTOPIA_PHONE
    d->display_home->setChecked( showHomeScreen );
    backToHomeScreenClicked( d->display_home->checkState() );
    d->interval_home->setValue( intervalShowHomeScreen / S_PER_MIN );

    if (!Qtopia::mousePreferred()) {
        if (keyLock == "Disabled") //no tr
            d->autoPhoneLock->setChecked( false );
        else if (keyLock == "Enabled") //no tr
            d->autoPhoneLock->setChecked(true);
        else
            qWarning("Unknown locking type");
    }
#endif
}

void LightSettings::applyBrightness()
{
    // slot called, but we haven't changed the powerMode values yet
    int v = b->brightness->value();
    currentMode->initbright = (v*255+b->brightness->maximum()/2)/b->brightness->maximum();

    set_fl(currentMode->initbright);
}

void LightSettings::powerTypeClicked(QAbstractButton * selected)
{
    PowerMode *newMode = &batteryMode;

    if ( selected == b->externalButton )
        newMode = &externalMode;

    /*  store everytime (so we can store from accept)   */
    currentMode->intervalDim = b->interval_dim->value();
    currentMode->intervalLightOff = b->interval_lightoff->value();
    currentMode->intervalSuspend = b->interval_suspend->value();
    int v = b->brightness->value();
    currentMode->initbright = (v*255+b->brightness->maximum()/2)/b->brightness->maximum();
    currentMode->dim = b->screensaver_dim->isChecked();
    currentMode->lightoff = b->screensaver_lightoff->isChecked();
    currentMode->suspend = b->screensaver_suspend->isChecked();
    currentMode->networkedsuspend = !b->notnetworkedsuspend->isChecked();

    /*  Radio buttons toggled   */
    if ( newMode != currentMode ) {
        currentMode = newMode;
        applyMode();
    }
}

void LightSettings::sysMessage(const QString& msg, const QByteArray& data)
{
    QDataStream s((QByteArray *)&data, QIODevice::ReadOnly);
    if (msg == "brightnessChanged(int)" ) {
        int bright;
        s >> bright;
        currentMode->initbright = bright;

        b->brightness->disconnect();
        b->brightness->setValue( (b->brightness->maximum()*bright+128)/255 );
        connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
    }
}

#ifdef QTOPIA_PHONE
void LightSettings::backToHomeScreenClicked( int state )
{
    d->home_details->setEnabled( state == Qt::Checked );
}

void LightSettings::pushSettingStatus()
{
    // send QCop message to record its current status to a selected profile.
    QtopiaServiceRequest e( "SettingsManager", "pushSettingStatus(QString,QString,QString)" );
    e << QString( "light-and-power" ) << QString( windowTitle() ) << status();
    e.send();
}

void LightSettings::pullSettingStatus()
{
    QtopiaServiceRequest e( "SettingsManager", "pullSettingStatus(QString,QString,QString)" );
    e << QString( "light-and-power" ) << QString( windowTitle() ) << status();
    e.send();
}

QString LightSettings::status()
{
    // capture current status
    QString result;
    result = QString::number( b->screensaver_dim->isChecked() ) + ",";
    result += QString::number( b->interval_dim->value() ) + ",";
    result += QString::number( b->screensaver_lightoff->isChecked() ) + ",";
    result += QString::number( b->interval_lightoff->value() ) + ",";
    result += QString::number( b->brightness->value() ) + ",";
    result += QString::number( d->display_home->isChecked() ) + ",";
    result += QString::number( d->interval_home->value() ) + ",";
    result += QString::number( d->autoPhoneLock->isChecked() ) + ",";
    return result;
}

void LightSettings::setStatus( QString details )
{
    QStringList sl = details.split( ',' );
    currentMode = &batteryMode;
    currentMode->dim = sl.at( 0 ).toInt();
    currentMode->intervalDim = sl.at( 1 ).toInt();
    currentMode->lightoff = sl.at( 2 ).toInt();
    currentMode->intervalLightOff = sl.at( 3 ).toInt();
    currentMode->initbright = sl.at( 4 ).toInt();
    showHomeScreen = sl.at( 5 ).toInt();
    intervalShowHomeScreen = sl.at( 6 ).toInt();
    keyLock = ( sl.at( 7 ).toInt() ? "Enabled" : "Disabled" );
    applyMode();
}

void LightSettings::receive( const QString& msg, const QByteArray& data )
{
    QDataStream ds((QByteArray *)&data, QIODevice::ReadOnly);
    if (msg == "Settings::setStatus(bool,QString)") {
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
        setStatus( details );
    } else if ( msg == "Settings::activateSettings(QString)" ) {
        QString details;
        ds >> details;
        setStatus( details );
        saveConfig();
        hide();
    } else if ( msg == "Settings::pullSettingStatus()" ) {
        pullSettingStatus();
        hide();
    } else if ( msg == "Settings::activateDefault()" ) {
        PowerMode mode;
        batteryMode = mode;
        showHomeScreen = false;
        intervalShowHomeScreen = 10;
        keyLock = "Disabled";
        applyMode();
        saveConfig();
        hide();
    }
}
#endif
void LightSettings::updateEditBoxes()
{
    b->interval_dim->setEnabled( b->screensaver_dim->isChecked() );
    b->interval_lightoff->setEnabled( b->screensaver_lightoff->isChecked() );
    b->interval_suspend->setEnabled( b->screensaver_suspend->isChecked() );

    updateSuspendMinValue(10);
}

void LightSettings::updateLightOffMinValue( int dimValue )
{
    b->interval_lightoff->setMinimum( dimValue + 10 );
    if ( b->interval_lightoff->value() <= dimValue )
        b->interval_lightoff->setValue( dimValue + 10 );
}

void LightSettings::updateSuspendMinValue( int )
{
    int minValue = 10;

    if ( b->screensaver_dim->isChecked() )
        minValue = qMax(minValue, b->interval_dim->value());
    if ( b->screensaver_lightoff->isChecked() )
        minValue = qMax(minValue, b->interval_lightoff->value());
#ifdef QTOPIA_PHONE
    if ( d->display_home->isChecked() )
        minValue = qMax(minValue, d->interval_home->value() * S_PER_MIN);
#endif

    b->interval_suspend->setMinimum( minValue );
    if ( b->interval_suspend->value() <= minValue )
        b->interval_suspend->setValue( minValue );
}

