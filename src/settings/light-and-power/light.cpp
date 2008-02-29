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
#include "settings.h"

#include <qpe/global.h>
#include <qpe/fontmanager.h>
#include <qpe/config.h>
#include <qpe/applnk.h>
#include <qpe/qpeapplication.h>
#include <qpe/power.h>

#if defined(Q_WS_QWS) && !defined(QT_NO_COP)
#include <qpe/qcopenvelope_qws.h>
#endif

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qslider.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qdir.h>
#include <qbuttongroup.h>
#if QT_VERSION >= 300
#include <qstylefactory.h>
#endif

extern int qpe_sysBrightnessSteps();

LightSettings::LightSettings( QWidget* parent,  const char* name, WFlags fl )
    : LightSettingsBase( parent, name, TRUE, fl )
{
    // Not supported
    auto_brightness->hide();
    powerSource->setExclusive( TRUE );
    
    Config config( "qpe" );

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
    externalMode.lightoff = config.readBoolEntry("LightOff", FALSE );    //default to leave one
    externalMode.suspend = config.readNumEntry("Suspend",1);
    externalMode.networkedsuspend = config.readNumEntry("NetworkedSuspend",0);
    
    int maxbright = qpe_sysBrightnessSteps();
    brightness->setMaxValue( maxbright );
    brightness->setTickInterval( QMAX(1,maxbright/16) );
    brightness->setLineStep( QMAX(1,maxbright/16) );
    brightness->setPageStep( QMAX(1,maxbright/16) );

    horizontalized = FALSE;

    currentMode = &batteryMode;
    applyMode();  

    powerStatus = PowerStatusManager::readStatus();
    connect(powerSource, SIGNAL(clicked(int)), this, SLOT(powerTypeClicked(int)) );
    if ( powerStatus.acStatus() == PowerStatus::Online ) {
	powerSource->setButton( powerSource->id(externalButton) );
	powerTypeClicked(powerSource->id(externalButton));
    } 
    
    initbright = currentMode->initbright;
    
    connect(brightness, SIGNAL(valueChanged(int)), this, SLOT(applyBrightness()));
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
}

void LightSettings::accept()
{
    if ( qApp->focusWidget() )
	qApp->focusWidget()->clearFocus();

    // safe call, always one selected.
    powerTypeClicked( powerSource->id( powerSource->selected() ) );
    
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

    // Set settings for current power source
    currentMode = &batteryMode;
    powerStatus = PowerStatusManager::readStatus();
    if ( powerStatus.acStatus() == PowerStatus::Online )
	currentMode = &externalMode;
    
    set_fl( currentMode->initbright );
    
    int i_dim =      (currentMode->dim ? currentMode->intervalDim : 0);
    int i_lightoff = (currentMode->lightoff ? currentMode->intervalLightOff : 0);
    int i_suspend =  currentMode->intervalSuspend;
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setScreenSaverIntervals(int,int,int)" );
    e << i_dim << i_lightoff << i_suspend;
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
    interval_dim->setValue( currentMode->intervalDim );
    interval_lightoff->setValue( currentMode->intervalLightOff );
    interval_suspend->setValue( currentMode->intervalSuspend );
    int v = currentMode->initbright;
    brightness->setValue( (brightness->maxValue()*v+128)/255 );
    
    screensaver_dim->setChecked( currentMode->dim );
    screensaver_lightoff->setChecked( currentMode->lightoff );
    screensaver_suspend->setChecked( currentMode->suspend );
    notnetworkedsuspend->setChecked( !currentMode->networkedsuspend );
}

void LightSettings::applyBrightness()
{
    // slot called, but we haven't changed the powerMode values yet
    int v = brightness->value();
    currentMode->initbright = (v*255+brightness->maxValue()/2)/brightness->maxValue();
    
    set_fl(currentMode->initbright);
}

void LightSettings::powerTypeClicked(int id)
{
    PowerMode *newMode = &batteryMode;
    
    QButton *selected = powerSource->find(id);
    if ( selected == externalButton )
	newMode = &externalMode;

    /*	store everytime (so we can store from accept)	*/
    currentMode->intervalDim = interval_dim->value();
    currentMode->intervalLightOff = interval_lightoff->value();
    currentMode->intervalSuspend = interval_suspend->value();
    int v = brightness->value();
    currentMode->initbright = (v*255+brightness->maxValue()/2)/brightness->maxValue();
    currentMode->dim = screensaver_dim->isChecked();
    currentMode->lightoff = screensaver_lightoff->isChecked();
    currentMode->suspend = screensaver_suspend->isChecked();
    currentMode->networkedsuspend = !notnetworkedsuspend->isChecked();

    QString tmp = screensaver_suspend->text();
    if ( tmp.isEmpty() ) tmp = suspend_phony->text();
    if ( selected == externalButton ) {
	suspend_phony->setText("");
	screensaver_suspend->setText(tmp);
    } else {
	screensaver_suspend->setText("");
	suspend_phony->setText(tmp);
    }
    
    /*	Radio buttons toggled	*/
    if ( newMode != currentMode ) {
	currentMode = newMode;
	applyMode();
    }
}

