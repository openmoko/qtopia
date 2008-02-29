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

#include "datebooksettings.h"

#include <qtopiaapplication.h>

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>

DateBookSettings::DateBookSettings( bool whichClock, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
      ampm( whichClock ), oldtime(0)
{
    setupUi( this );
    // TODO make this check based on the screensize, not QTOPIA_PHONE
    // Need to change use of compressed day to do the same first
#ifdef QTOPIA_PHONE
    widCompress->hide();
#endif
    init();
    setObjectName("settings");
    QObject::connect( qApp, SIGNAL( clockChanged(bool) ),
                      this, SLOT( slotChangeClock(bool) ) );

    connect(chkAlarmPreset, SIGNAL(stateChanged(int)), this, SLOT(enablePresetDetails(int)));
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
        if ( returnMe != 12 && spinStart->suffix().contains(tr("PM"), Qt::CaseInsensitive) )
            returnMe += 12;
        else if (returnMe == 12 && spinStart->suffix().contains(tr("AM"),  Qt::CaseInsensitive))
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
        if ( spinStart->suffix().contains( tr("AM"), Qt::CaseInsensitive ) ) {
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
        spinStart->setMinimum( 1 );
        spinStart->setMaximum( 12 );
        spinStart->setValue( 12 );
        spinStart->setSuffix( tr(":00 AM") );
        oldtime = 12;
    } else {
        spinStart->setMinimum( 0 );
        spinStart->setMaximum( 23 );
        spinStart->setSuffix( tr(":00") );
    }
}

void DateBookSettings::slotChangeClock( bool whichClock )
{
    int saveMe;
    saveMe = spinStart->value();
    if ( ampm && spinStart->suffix().contains( tr("AM"), Qt::CaseInsensitive ) ) {
        if ( saveMe == 12 )
            saveMe = 0;
    } else if ( ampm && spinStart->suffix().contains( tr("PM"), Qt::CaseInsensitive )  ) {
        if ( saveMe != 12 )
            saveMe += 12;
    }
    ampm = whichClock;
    init();
    setStartTime( saveMe );
}

void DateBookSettings::enablePresetDetails(int state)
{
    spinPreset->setEnabled(state != Qt::Unchecked);
}

DateBookSettings::ViewType DateBookSettings::defaultView() const
{
    return (DateBookSettings::ViewType) cmbDefaultView->currentIndex();
}

void DateBookSettings::setDefaultView(DateBookSettings::ViewType type)
{
    // If we had holes in the range we'd need to translate
    if (type >= 0 && type < cmbDefaultView->count())
        cmbDefaultView->setCurrentIndex(type);
}

