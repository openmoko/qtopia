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
#include "nulldb.h"

#include <qtopia/datetimeedit.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/categoryselect.h>
#include <qtopia/global.h>
#include <qtopia/timeconversion.h>
#include <qtopia/timestring.h>
#include <qtopia/tzselect.h>
#ifdef QTOPIA_DESKTOP
#include <worldtimedialog.h>
#endif

#include <qcheckbox.h>
#include <qstyle.h>
#include <qregexp.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpopupmenu.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>

#include <stdlib.h>

bool onceAWeek(const PimEvent &e)
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
	if (i != orig && e.repeatOnWeekDay(i))
	    return FALSE;
    }
    return TRUE;
}

void setOnceAWeek( PimEvent &e )
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
	if (i != orig)
	    e.setRepeatOnWeekDay(i, FALSE);
    }
}

/*
 *  Constructs a DateEntry which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */

DateEntry::DateEntry( bool startOnMonday, const QDateTime &start,
		      const QDateTime &end, QWidget* parent,
		      const char* name )
    : DateEntryBase( parent, name ),
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

DateEntry::DateEntry( bool startOnMonday, const PimEvent &event,
                      QWidget* parent,  const char* name )
    : DateEntryBase( parent, name ), mEvent(event),
      startWeekOnMonday( startOnMonday )
{
#ifdef QTOPIA_DESKTOP
    tabLayout->setSpacing(6);
    startEndLayout->setSpacing(6);
#endif
    init();
    if (mEvent.timeZone().isValid()) {
	timezone->setCurrentZone(mEvent.timeZone().id());
    }
    setDates(mEvent.start(),mEvent.end());
    comboCategory->setCategories( mEvent.categories(), "Calendar", // No tr
	    tr("Calendar") );
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
	if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Daily)
	    repeatSelect->setCurrentItem(1);
	else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Weekly
		&& onceAWeek(mEvent) )
	    repeatSelect->setCurrentItem(2);
	else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Yearly)
	    repeatSelect->setCurrentItem(3);
	else
	    repeatSelect->setCurrentItem(4);
	if (mEvent.repeatForever()) {
	    // so that when opens, will have view at or close to start date of event,
	    endDateSelect->setDate(mEvent.start().date());
	    endDateSelect->setDate(QDate());
	} else {
	    endDateSelect->setDate(mEvent.repeatTill());
	}
    } else {
	repeatSelect->setCurrentItem(0);
	endDateSelect->setEnabled(FALSE);
	// so that when opens, will have view at or close to start date of event,
	endDateSelect->setDate(mEvent.start().date());
	endDateSelect->setDate(QDate());
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
    // Disabled, it gets out of date too easily.  Need a better solution
    // than breacking and rebuilding the layout.
#if 0
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
	DateEntryBaseLayout->addMultiCellWidget(repeatSelect, 15,15, 0,1);

	DateEntryBaseLayout->addMultiCellWidget(editNote, 16,16, 0,1);
    }
#endif

    // XXX should load/save this
    comboLocation->insertItem(tr("Office"));
    comboLocation->insertItem(tr("Home","ie. not work"));
    addOrPick( comboLocation, "" );

    // XXX enable these two lines to be able to specify local time events.
    timezone->setLocalIncluded(TRUE);
    timezone->setCurrentZone("None");

    comboDescription->setInsertionPolicy(QComboBox::AtCurrent);
    comboLocation->setInsertionPolicy(QComboBox::AtCurrent);

    setTabOrder(comboDescription, comboLocation);
    setTabOrder(comboLocation, (QWidget *)comboCategory->child("category combo")); // No tr
#ifndef QTOPIA_DESKTOP
    setTabOrder((QWidget *)comboCategory->child("category combo"), // No tr
	    (QWidget *)comboCategory->child("category button")); // No tr
    setTabOrder((QWidget *)comboCategory->child("category button"),  // No tr
	    (QWidget *)startButton->child("date")); // No tr
#else
    setTabOrder((QWidget *)comboCategory->child("category combo"), // No tr
	    (QWidget *)startButton->child("date")); // No tr
    connect((QObject *)timezone->d, SIGNAL(configureTimeZones()), this, SLOT(configureTimeZones()));
#endif
    setTabOrder((QWidget *)startButton->child("date"),  // No tr
	    (QWidget *)startButton->child("time")); // No tr
    setTabOrder((QWidget *)startButton->child("time"),  // No tr
	    (QWidget *)endButton->child("date")); // No tr
    setTabOrder((QWidget *)endButton->child("date"),  // No tr
	    (QWidget *)endButton->child("time")); // No tr
    setTabOrder((QWidget *)endButton->child("time"), checkAllDay); // No tr
    setTabOrder(checkAllDay, (QWidget *)timezone->child("timezone combo"));
    setTabOrder((QWidget *)timezone->child("timezone combo"),
	    (QWidget *)timezone->child("timezone button")); // No tr
    setTabOrder((QWidget *)timezone->child("timezone button"), comboSound); // No tr
    setTabOrder(comboSound, spinAlarm);
    setTabOrder(spinAlarm, repeatSelect);
    setTabOrder(repeatSelect, endDateSelect);

    comboDescription->setFocus();

    // Now for the lines to do with eneabling for exception events.
    if (mEvent.isException()) {
	repeatSelect->setEnabled(FALSE);
	repeatSelect->hide();
	endDateSelect->setEnabled(FALSE);
	endDateSelect->hide();
	lblEndDate->hide();
	lblRepeat->hide();
    }


    connect( checkAllDay, SIGNAL(toggled(bool)),
	     this, SLOT(allDayToggled(bool)) );

    connect( repeatSelect, SIGNAL(activated(int)),
	    this, SLOT(setRepeatType(int)));
    connect( endDateSelect, SIGNAL(valueChanged(const QDate &)),
	    this, SLOT(setEndDate(const QDate &)));

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
    startButton->blockSignals(TRUE);
    endButton->blockSignals(TRUE);

    QDateTime target = e;

    // since setting the start can change the end, do this first.
    if (target.addSecs(-300) < mEvent.start()) {
	mEvent.setStart(target.addSecs(-300));
    }

    mEvent.setEnd(target);
    startButton->setDateTime(mEvent.start());

    startButton->blockSignals(FALSE);
    endButton->blockSignals(FALSE);
}

/*
 * public slot
 */
void DateEntry::startDateTimeChanged( const QDateTime &s )
{
    startButton->blockSignals(TRUE);
    endButton->blockSignals(TRUE);

    // start always works.
    mEvent.setStart(s);

    // modifying start modifies end, so no need check or modify anything.
    // just ensure we update the widget.
    endButton->setDateTime(mEvent.end());

    startButton->blockSignals(FALSE);
    endButton->blockSignals(FALSE);
}

void DateEntry::slotRepeat()
{
    // Work around for compiler Bug..
    RepeatEntry *e;

    mEvent = event(); // update of the shown values;
    e = new RepeatEntry( startWeekOnMonday, mEvent, this);

    if ( QPEApplication::execDialog(e) ) {
	 mEvent = e->event();
	 endDateSelect->setEnabled(TRUE);
    }
    setRepeatLabel();
}

void DateEntry::setWeekStartsMonday( bool onMonday )
{
    startWeekOnMonday = onMonday;
}

PimEvent DateEntry::event()
{
    mEvent.setDescription( comboDescription->currentText() );
    mEvent.setLocation( comboLocation->currentText() );
    mEvent.setCategories( comboCategory->currentCategories() );
    mEvent.setAllDay( checkAllDay->isChecked() );

    if (endDateSelect->date().isValid()) {
	mEvent.setRepeatForever(FALSE);
	mEvent.setRepeatTill(endDateSelect->date());
    } else {
	mEvent.setRepeatForever(TRUE);
    }
    // don't set the time if theres no need too

    if (timezone->currentZone() != "None")
	mEvent.setTimeZone(TimeZone(timezone->currentZone()));
    else
	mEvent.setTimeZone(TimeZone());

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

    if (!mEvent.isException()) {
	if ( mEvent.hasRepeat() ) {
	    //cmdRepeat->setText( tr("Repeat...") );
	    if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Daily)
		repeatSelect->setCurrentItem(1);
	    else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Weekly
		    && onceAWeek(mEvent))
		repeatSelect->setCurrentItem(2);
	    else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Yearly)
		repeatSelect->setCurrentItem(3);
	    else
		repeatSelect->setCurrentItem(4);
	} else {
	    repeatSelect->setCurrentItem(0);
	}
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

void DateEntry::allDayToggled(bool b)
{
    startButton->setTimeEnabled(!b);
    endButton->setTimeEnabled(!b);
    comboSound->setEnabled(!b);
    comboSound->setCurrentItem(0);
    spinAlarm->setEnabled(!b);
}


void DateEntry::configureTimeZones()
{
#ifdef QTOPIA_DESKTOP
    // Show the WorldTime as a dialog
    WorldTimeDialog *dlg = new  WorldTimeDialog(0, timezone);
    dlg->exec();
    delete dlg;
#endif
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

void DateEntry::setRepeatType(int i)
{
    switch (i) {
	case 0:
	    mEvent.setRepeatType(PimEvent::NoRepeat);
	    endDateSelect->setEnabled(FALSE);
	    break;
	case 1:
	    mEvent.setRepeatType(PimEvent::Daily);
	    mEvent.setFrequency(1);
	    endDateSelect->setEnabled(TRUE);
	    break;
	case 2:
	    mEvent.setRepeatType(PimEvent::Weekly);
	    mEvent.setFrequency(1);
	    setOnceAWeek(mEvent);
	    endDateSelect->setEnabled(TRUE);
	    break;
	case 3:
	    mEvent.setRepeatType(PimEvent::Yearly);
	    mEvent.setFrequency(1);
	    endDateSelect->setEnabled(TRUE);
	    break;
	case 4:
	default:
	    slotRepeat();
	    break;
    }
}

void DateEntry::setEndDate(const QDate &date)
{
    // chekc the date...
    if (date.isValid() && date < startButton->date()) {
	endDateSelect->blockSignals(TRUE);
	endDateSelect->setDate(startButton->date());
	endDateSelect->blockSignals(FALSE);
    }
}

#ifdef QTOPIA_DESKTOP
void DateEntry::updateCategories()
{
    connect( this, SIGNAL( categoriesChanged() ),
	     comboCategory, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
		comboCategory, SLOT( categoriesChanged() ) );
}
#endif

