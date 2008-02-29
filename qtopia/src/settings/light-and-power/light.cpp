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
#include "light.h"

#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/power.h>
#include <qtopia/vscrollview.h>

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qslider.h>
//#include <qtextstream.h>
#include <qdatastream.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qbuttongroup.h>
#if QT_VERSION >= 0x030000
#include <qstylefactory.h>
#endif

extern int qpe_sysBrightnessSteps();

LightSettings::LightSettings( QWidget* parent,  const char* name, WFlags fl )
    : QDialog( parent, name, TRUE, fl)
{
    setCaption(tr("Power Management"));
    QVBoxLayout * baseLayout = new QVBoxLayout( this );

#ifdef QTOPIA_PHONE
    QTabWidget *tabWidget = new QTabWidget( this, "tabwidget" );
    VScrollView *sView = new VScrollView(tabWidget);
    QWidget * container = sView->widget();
    QVBoxLayout *lightLayout = new QVBoxLayout(container);
    b = new LightSettingsBase(container);
    lightLayout->addWidget(b);
    tabWidget->insertTab( sView, tr("Screensaver"));

    QWidget *displayWidget = new QWidget(tabWidget);
    QVBoxLayout *displayLayout = new QVBoxLayout( displayWidget );
    d = new DisplaySettingsBase(displayWidget);
    displayLayout->addWidget(d);
    tabWidget->insertTab( displayWidget, tr("Display") );
    
    baseLayout->addWidget( tabWidget );
#else
    VScrollView *sView = new VScrollView( this );
    baseLayout->addWidget(sView);
    QWidget * container = sView->widget();
    QVBoxLayout *lightLayout = new QVBoxLayout(container);
    b = new LightSettingsBase(container);
    lightLayout->addWidget(b);
#endif
    
    // Not supported
    b->auto_brightness->hide();
    b->powerSource->setExclusive( TRUE );
    
    Config config( "qpe" );

#ifdef QTOPIA_PHONE
    b->powerSource->hide();
    b->notnetworkedsuspend->hide();
    b->screensaver_suspend->hide();
    b->interval_suspend->hide();
    b->biginfo->hide();
    b->TextLabel1->setText( tr("s", "seconds") );
    b->TextLabel2->setText( tr("s", "seconds") );
    b->TextLabel3->hide();
    if (Global::mousePreferred())
        d->autoPhoneLock->hide();
#else
    b->screensaver_suspend->setEnabled(TRUE);
    if (!PowerStatusManager::APMEnabled()) {
        b->powerSource->hide();
        b->notnetworkedsuspend->hide();
        b->screensaver_suspend->hide();
        b->interval_suspend->hide();
        b->TextLabel3->hide();
    }
#endif

    config.setGroup("BatteryPower");
    batteryMode.intervalDim = config.readNumEntry( "Interval_Dim", 20 );
    batteryMode.intervalLightOff = config.readNumEntry("Interval_LightOff", 30);
    batteryMode.intervalSuspend = config.readNumEntry("Interval", 60);
    batteryMode.initbright = config.readNumEntry("Brightness", 255);
    batteryMode.dim = config.readBoolEntry("Dim", TRUE);
    batteryMode.lightoff = config.readBoolEntry("LightOff", TRUE );
    batteryMode.suspend = config.readNumEntry("Suspend",1);
    batteryMode.networkedsuspend = config.readNumEntry("NetworkedSuspend",1);

    config.setGroup("ExternalPower");
    externalMode.intervalDim = config.readNumEntry( "Interval_Dim", 20 );
    externalMode.intervalLightOff = config.readNumEntry("Interval_LightOff", 30);
    externalMode.intervalSuspend = config.readNumEntry("Interval", 240);
    externalMode.initbright = config.readNumEntry("Brightness", 255);
    externalMode.dim = config.readBoolEntry("Dim", TRUE);	    //default to leave on
    externalMode.lightoff = config.readBoolEntry("LightOff", FALSE );    //default to leave on
    externalMode.suspend = config.readNumEntry("Suspend",1);
    externalMode.networkedsuspend = config.readNumEntry("NetworkedSuspend",0);
    
    int maxbright = qpe_sysBrightnessSteps();
    b->brightness->setMaxValue( maxbright );
    b->brightness->setTickInterval( QMAX(1,maxbright/16) );
    b->brightness->setLineStep( QMAX(1,maxbright/16) );
    b->brightness->setPageStep( QMAX(1,maxbright/16) );

    horizontalized = FALSE;
    currentMode = &batteryMode;
    b->screensaver_suspend->setEnabled(FALSE);

#ifdef QTOPIA_PHONE
    config.setGroup("HomeScreen");
    showHomeScreen = config.readBoolEntry("ShowHomeScreen", FALSE);
    intervalShowHomeScreen = config.readNumEntry("Interval_HomeScreen", 30 );
    if (!Global::mousePreferred())
        keyLock = config.readEntry("AutoKeyLock", "Disabled");

    if (!showHomeScreen) {
        d->home_details->setEnabled(FALSE);
    }
#endif

    applyMode();  

    powerStatus = PowerStatusManager::readStatus();
    connect(b->powerSource, SIGNAL(clicked(int)), this, SLOT(powerTypeClicked(int)) );
    if ( powerStatus.acStatus() == PowerStatus::Online ) {
	b->powerSource->setButton( b->powerSource->id(b->externalButton) );
	powerTypeClicked(b->powerSource->id(b->externalButton));
    } 
    
    initbright = currentMode->initbright;
    
    connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
    
#ifndef QT_NO_COP 
    QCopChannel *channel = new QCopChannel("QPE/System", this);
    connect(channel, SIGNAL(received(const QCString&, const QByteArray&)),
            this, SLOT(sysMessage(const QCString&, const QByteArray&)));
#endif
    
}

LightSettings::~LightSettings()
{
}

static void set_fl(int bright)
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setBacklight(int)" );
    e << bright;
#endif
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

    // safe call, always one selected.
    powerTypeClicked( b->powerSource->id( b->powerSource->selected() ) );

    // Set settings for current power source
    currentMode = &batteryMode; 
    if (PowerStatusManager::APMEnabled()) {
        
        powerStatus = PowerStatusManager::readStatus();
        if ( powerStatus.acStatus() == PowerStatus::Online )
	    currentMode = &externalMode;
    }
    
    {
	Config config( "qpe" );
	config.setGroup("BatteryPower");
	writeMode(config, &batteryMode);  
	config.setGroup("ExternalPower");
	writeMode(config, &externalMode);
	
	/* This layer is written for compability with older versions    */
	config.setGroup("Screensaver");
	writeMode(config, currentMode);
        
	config.write();
    }

    initbright = currentMode->initbright;
    
    int i_dim =      (currentMode->dim ? currentMode->intervalDim : 0);
    int i_lightoff = (currentMode->lightoff ? currentMode->intervalLightOff : 0);
#ifndef QTOPIA_PHONE
    int i_suspend =  (currentMode->suspend ? currentMode->intervalSuspend : 0);
#endif

#ifdef QTOPIA_PHONE
    showHomeScreen = d->display_home->isChecked();
    intervalShowHomeScreen = d->interval_home->value();

    Config config( "qpe" );
    config.setGroup( "HomeScreen" );
    config.writeEntry( "ShowHomeScreen", showHomeScreen );
    config.writeEntry( "Interval_HomeScreen", intervalShowHomeScreen );

    if (!Global::mousePreferred()) {
        if (d->autoPhoneLock->isChecked())
            keyLock = "Enabled";  //no tr
        else
            keyLock = "Disabled"; //no tr
        config.writeEntry( "AutoKeyLock", keyLock );
    }
#endif

#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setScreenSaverIntervals(int,int,int)" );
#ifndef QTOPIA_PHONE
    e << i_dim << i_lightoff << i_suspend;
#else
    // phone edition doesn't use 3rd screensaver level
    e << i_dim << i_lightoff << 0;
#endif
#endif
    
    QDialog::accept();
    close();
}

void LightSettings::writeMode(Config &config, PowerMode *mode)
{
    config.writeEntry( "Dim", mode->dim );
    config.writeEntry( "LightOff", mode->lightoff );
    config.writeEntry( "Interval_Dim", mode->intervalDim );
    config.writeEntry( "Interval_LightOff", mode->intervalLightOff );
    config.writeEntry( "Interval", mode->intervalSuspend );
    config.writeEntry( "Brightness", mode->initbright );
    config.writeEntry( "Suspend", mode->suspend );
    config.writeEntry( "NetworkedSuspend", mode->networkedsuspend );
}

void LightSettings::applyMode()
{
    b->interval_dim->setValue( currentMode->intervalDim );
    b->interval_lightoff->setValue( currentMode->intervalLightOff );
    b->interval_suspend->setValue( currentMode->intervalSuspend );
    int v = currentMode->initbright;
    b->brightness->setValue( (b->brightness->maxValue()*v+128)/255 );
    
    b->screensaver_dim->setChecked( currentMode->dim );
    b->screensaver_lightoff->setChecked( currentMode->lightoff );
    b->screensaver_suspend->setChecked( currentMode->suspend );
    b->notnetworkedsuspend->setChecked( !currentMode->networkedsuspend );
#ifdef QTOPIA_PHONE
    d->display_home->setChecked( showHomeScreen );
    d->interval_home->setValue( intervalShowHomeScreen );

    if (!Global::mousePreferred()) {
        if (keyLock == "Disabled") //no tr
            d->autoPhoneLock->setChecked(FALSE);
        else if (keyLock == "Enabled") //no tr
            d->autoPhoneLock->setChecked(TRUE);
        else 
            qWarning("Unknown locking type");
    }
#endif
}

void LightSettings::applyBrightness()
{
    // slot called, but we haven't changed the powerMode values yet
    int v = b->brightness->value();
    currentMode->initbright = (v*255+b->brightness->maxValue()/2)/b->brightness->maxValue();
    
    set_fl(currentMode->initbright);
}

void LightSettings::powerTypeClicked(int id)
{
    PowerMode *newMode = &batteryMode;
    
    QButton *selected = b->powerSource->find(id);
    if ( selected == b->externalButton )
	newMode = &externalMode;

    /*	store everytime (so we can store from accept)	*/
    currentMode->intervalDim = b->interval_dim->value();
    currentMode->intervalLightOff = b->interval_lightoff->value();
    currentMode->intervalSuspend = b->interval_suspend->value();
    int v = b->brightness->value();
    currentMode->initbright = (v*255+b->brightness->maxValue()/2)/b->brightness->maxValue();
    currentMode->dim = b->screensaver_dim->isChecked();
    currentMode->lightoff = b->screensaver_lightoff->isChecked();
    currentMode->suspend = b->screensaver_suspend->isChecked();
    currentMode->networkedsuspend = !b->notnetworkedsuspend->isChecked();

    /*	Radio buttons toggled	*/
    if ( newMode != currentMode ) {
	currentMode = newMode;
	applyMode();
    }
}

void LightSettings::sysMessage(const QCString& msg, const QByteArray& data)
{
#ifndef QT_NO_COP
    QDataStream s(data, IO_ReadOnly);
    if (msg == "setBacklight(int)" ) {
        int bright;
        s >> bright;
        initbright = currentMode->initbright = bright;

        b->brightness->disconnect();
        b->brightness->setValue( (b->brightness->maxValue()*bright+128)/255 );
        connect(b->brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
    }
#endif
}
