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

#define QTOPIA_INTERNAL_TZSELECT_INC_LOCAL

#include "dateentryimpl.h"
#include "repeatentry.h"

#include "datetimeedit.h"
#include <qpe/qpeapplication.h>
#include <qpe/categoryselect.h>
#include <qpe/global.h>
#include <qpe/timeconversion.h>
#include <qpe/timestring.h>
#include <qpe/tzselect.h>


#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <stdlib.h>

/*
 *  Constructs a DateEntry which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

DateEntry::DateEntry( bool startOnMonday, const QDateTime &start,
		      const QDateTime &end, bool whichClock, QWidget* parent,
		      const char* name )
    : DateEntryBase( parent, name ),
      ampm( whichClock ),
      startWeekOnMonday( startOnMonday )
{
    init();
    setDates(start,end);
}

static void addOrPick( QComboBox* combo, const QString& t )
{
    for (int i=0; i<combo->count(); i++) {
	if ( combo->text(i) == t ) {
	    combo->setCurrentItem(i);
	    return;
	}
    }
    combo->setEditText(t);
}

DateEntry::DateEntry( bool startOnMonday, const PimEvent &event, bool whichClock,
                      QWidget* parent,  const char* name )
    : DateEntryBase( parent, name ), mEvent(event),
      ampm( whichClock ),
      startWeekOnMonday( startOnMonday )
{
    init();
    if (!mEvent.timeZone().isEmpty()) {
	timezone->setCurrentZone(mEvent.timeZone());
	qDebug("setting current zone");
    }
    setDates(mEvent.start(),mEvent.end());
    comboCategory->setCategories( mEvent.categories(), "Calendar", tr("Calendar") );
    if(!mEvent.description().isEmpty())
	addOrPick( comboDescription, mEvent.description() );
    if(!mEvent.location().isEmpty())
	addOrPick( comboLocation, mEvent.location() );
    if ( mEvent.hasAlarm() ) {
	if (mEvent.alarmSound() != PimEvent::Silent)
	    comboSound->setCurrentItem(2);
	else
	    comboSound->setCurrentItem(1);
	spinAlarm->setValue(mEvent.alarmDelay());
    } else {
	comboSound->setCurrentItem(0);
	spinAlarm->setValue(0);
    }
    checkAllDay->setChecked( mEvent.isAllDay() );
    if(!mEvent.notes().isEmpty())
	editNote->setText(mEvent.notes());
    if ( mEvent.hasRepeat() ) {
	cmdRepeat->setText( tr("Repeat...") );
    }
    setRepeatLabel();
}

void DateEntry::setDates( const QDateTime& s, const QDateTime& e )
{
    mEvent.setStart(s);
    mEvent.setEnd(e);
    startButton->setDateTime(s);
    endButton->setDateTime(e);
}

void DateEntry::init()
{
    if (QApplication::desktop()->width() < 200) {
	setMaximumWidth(QApplication::desktop()->width() 
		- style().scrollBarExtent().width());
	delete DateEntryBaseLayout;
	DateEntryBaseLayout = new QGridLayout(this);
	DateEntryBaseLayout->setSpacing(1);
	DateEntryBaseLayout->setMargin(3);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel1, 0,0, 0, 1);
	DateEntryBaseLayout->addMultiCellWidget(comboDescription, 1,1, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel2, 2,2, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(comboLocation, 3,3, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel2_2, 4,4, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(comboCategory, 5,5, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel3, 6,6, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(startButton, 7,7, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel3_2, 8,8, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(endButton, 9,9, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(checkAllDay, 10,10, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(TextLabel3_2_2, 11,11, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(timezone, 12,12, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(comboSound, 13,13, 0,0);
	DateEntryBaseLayout->addMultiCellWidget(spinAlarm, 13,13, 1,1);

	DateEntryBaseLayout->addMultiCellWidget(lblRepeat, 14,14, 0,1);
	DateEntryBaseLayout->addMultiCellWidget(cmdRepeat, 15,15, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(editNote, 16,16, 0,1);
    }

    // XXX enable these two lines to be able to specify local time events.
    //timezone->setLocalIncluded(TRUE);
    //timezone->setCurrentZone("None");

    comboDescription->setInsertionPolicy(QComboBox::AtCurrent);
    comboLocation->setInsertionPolicy(QComboBox::AtCurrent);

    startButton->setClock(ampm);
    endButton->setClock(ampm);

    connect( qApp, SIGNAL( clockChanged( bool ) ),
	     this, SLOT( slotChangeClock( bool ) ) );
    connect( qApp, SIGNAL(weekChanged(bool)),
	     this, SLOT(slotChangeStartOfWeek(bool)) );

    connect( checkAllDay, SIGNAL(toggled(bool)),
	     this, SLOT(updateDateEdits(bool)) );

    connect( spinAlarm, SIGNAL(valueChanged(int)),
	     this, SLOT(turnOnAlarm()) );
    connect( comboSound, SIGNAL(activated(int)),
	    this, SLOT(checkAlarmSpin(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DateEntry::~DateEntry()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void DateEntry::endDateTimeChanged( const QDateTime &e )
{
    mEvent.setEnd(e);
    if (e.addSecs(-300) < mEvent.start()) {
	mEvent.setStart(e.addSecs(-300));
	startButton->setDateTime(mEvent.start());
    }
}

/*
 * public slot
 */
void DateEntry::startDateTimeChanged( const QDateTime &s )
{
    mEvent.setStart(s);
    // moving the start automatically moves the end.
    if (mEvent.end().addSecs(-300) < s) {
	mEvent.setEnd(s.addSecs(300));
    }
    // so even if we didn't need to set a new end, update the label.
    endButton->setDateTime(mEvent.end());
}

void DateEntry::slotRepeat()
{
    // Work around for compiler Bug..
    RepeatEntry *e;

    mEvent = event(); // update of the shown values;
    e = new RepeatEntry( startWeekOnMonday, mEvent, this);

    if ( QPEApplication::execDialog(e) ) {
	 mEvent = e->event();
	setRepeatLabel();
    }
}

void DateEntry::slotChangeStartOfWeek( bool onMonday )
{
    startWeekOnMonday = onMonday;
}

PimEvent DateEntry::event()
{
    mEvent.setDescription( comboDescription->currentText() );
    mEvent.setLocation( comboLocation->currentText() );
    mEvent.setCategories( comboCategory->currentCategories() );
    mEvent.setAllDay( checkAllDay->isChecked() );
    // don't set the time if theres no need too

    if (timezone->currentZone() != "None")
	mEvent.setTimeZone(timezone->currentZone());
    else 
	mEvent.setTimeZone(QString::null);

    // we only have one type of sound at the moment... LOUD!!!
    switch (comboSound->currentItem()) {
	case 0:
	    mEvent.clearAlarm();
	    break;
	case 1:
	    mEvent.setAlarm( spinAlarm->value(), PimEvent::Silent);
	    break;
	case 2:
	    mEvent.setAlarm( spinAlarm->value(), PimEvent::Loud);
	    break;
    }
    // don't need to do repeat, repeat dialog handles that.
    mEvent.setNotes( editNote->text() );
    return mEvent;
}

void DateEntry::setRepeatLabel()
{

    switch( mEvent.repeatType() ) {
	case PimEvent::Daily:
	    cmdRepeat->setText( tr("Daily...") );
	    break;
	case PimEvent::Weekly:
	    cmdRepeat->setText( tr("Weekly...") );
	    break;
	case PimEvent::MonthlyDay:
	case PimEvent::MonthlyEndDay:
	case PimEvent::MonthlyDate:
	    cmdRepeat->setText( tr("Monthly...") );
	    break;
	case PimEvent::Yearly:
	    cmdRepeat->setText( tr("Yearly...") );
	    break;
	default:
	    cmdRepeat->setText( tr("No Repeat...") );
    }
}

void DateEntry::setAlarmEnabled( bool alarmPreset, int presetTime, PimEvent::SoundTypeChoice sound )
{
    if ( alarmPreset ) {
	if (sound != PimEvent::Silent)
	    comboSound->setCurrentItem(2);
	else
	    comboSound->setCurrentItem(1);
	spinAlarm->setValue( presetTime );
	mEvent.setAlarm(presetTime, sound);
    } else {
	comboSound->setCurrentItem(0);
	spinAlarm->setValue(0);
	mEvent.clearAlarm();
    }
}

void DateEntry::slotChangeClock( bool whichClock )
{
    ampm = whichClock;
    startButton->setClock(ampm);
    endButton->setClock(ampm);
}

void DateEntry::updateDateEdits(bool b)
{
    startButton->setTimeEnabled(!b);
    endButton->setTimeEnabled(!b);
}

void DateEntry::turnOnAlarm()
{
    // if alarm spin spun, then user probably wants an alarm.  Make it load :)
    if (comboSound->currentItem() == 0 && spinAlarm->value() != 0)
	comboSound->setCurrentItem(2);
}

void DateEntry::checkAlarmSpin(int nAType) 
{
    if (nAType == 0 && spinAlarm->value() != 0)
	spinAlarm->setValue(0);
}
