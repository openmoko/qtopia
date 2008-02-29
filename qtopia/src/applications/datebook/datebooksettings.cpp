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

#include "datebooksettings.h"

#include <qtopia/qpeapplication.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>

DateBookSettings::DateBookSettings( bool whichClock, QWidget *parent,
                                    const char *name, bool modal, WFlags fl )
    : DateBookSettingsBase( parent, name, modal, fl ),
      ampm( whichClock ), oldtime(0)
{
    // TODO make this check based on the screensize, not QTOPIA_PHONE
    // Need to change use of compressed day to do the same first
#ifdef QTOPIA_PHONE
    fraDayView->hide();
#endif
    init();
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( slotChangeClock(bool) ) );
}

DateBookSettings::~DateBookSettings()
{
}

void DateBookSettings::setStartTime( int newStartViewTime )
{
    if ( ampm ) {
	if ( newStartViewTime >= 12 ) {
	    newStartViewTime %= 12;
	    if ( newStartViewTime == 0 )
		newStartViewTime = 12;
	    spinStart->setSuffix( tr(":00 PM") );
	}
	else if ( newStartViewTime == 0 ) {
	    newStartViewTime = 12;
	    spinStart->setSuffix( tr(":00 AM") );
	}
	oldtime = newStartViewTime;
    }
    spinStart->setValue( newStartViewTime );
}

int DateBookSettings::startTime() const
{
    int returnMe = spinStart->value();
    if ( ampm ) {
	if ( returnMe != 12 && spinStart->suffix().contains(tr("PM"), FALSE) )
	    returnMe += 12;
	else if (returnMe == 12 && spinStart->suffix().contains(tr("AM"), TRUE))
	    returnMe = 0;
    }
    return returnMe;
}


void DateBookSettings::setAlarmPreset( bool bAlarm, int presetTime )
{
    chkAlarmPreset->setChecked( bAlarm );
    if ( presetTime >=0 )
	spinPreset->setValue( presetTime );
}

bool DateBookSettings::alarmPreset() const
{
    return chkAlarmPreset->isChecked();
}

int DateBookSettings::presetTime() const
{
    return spinPreset->value();
}

void DateBookSettings::setCompressDay( bool compress )
{
    chkCompressDayView->setChecked( compress );
}

bool DateBookSettings::compressDay() const
{
    return chkCompressDayView->isChecked();
}

void DateBookSettings::slot12Hour( int i )
{
    if ( ampm ) {
	if ( spinStart->suffix().contains( tr("AM"), FALSE ) ) {
	    if ( oldtime == 12 && i == 11 || oldtime == 11 && i == 12 )
		spinStart->setSuffix( tr(":00 PM") );
	} else {
	    if ( oldtime == 12 && i == 11 || oldtime == 11 && i == 12 )
		spinStart->setSuffix( tr(":00 AM") );
	}
	oldtime = i;
    }
}

void DateBookSettings::init()
{
    if ( ampm ) {
	spinStart->setMinValue( 1 );
	spinStart->setMaxValue( 12 );
	spinStart->setValue( 12 );
	spinStart->setSuffix( tr(":00 AM") );
	oldtime = 12;
    } else {
	spinStart->setMinValue( 0 );
	spinStart->setMaxValue( 23 );
	spinStart->setSuffix( tr(":00") );
    }
}

void DateBookSettings::slotChangeClock( bool whichClock )
{
    int saveMe;
    saveMe = spinStart->value();
    if ( ampm && spinStart->suffix().contains( tr("AM"), FALSE ) ) {
	if ( saveMe == 12 )
	    saveMe = 0;
    } else if ( ampm && spinStart->suffix().contains( tr("PM"), FALSE )  ) {
	if ( saveMe != 12 )
	    saveMe += 12;
    }
    ampm = whichClock;
    init();
    setStartTime( saveMe );
}
