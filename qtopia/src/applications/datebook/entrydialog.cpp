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

#define QTOPIA_INTERNAL_TZSELECT_INC_LOCAL

#include "entrydialog.h"
#include "repeatentry.h"
#include "nulldb.h"
#include "eventview.h"

#include <qtopia/vscrollview.h>
#include <qtopia/datetimeedit.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/categoryselect.h>
#include <qtopia/global.h>
#include <qtopia/timeconversion.h>
#include <qtopia/timestring.h>
#include <qtopia/tzselect.h>
#include <qtopia/pim/private/eventio_p.h>
#if !defined(QTOPIA_PHONE) && defined(QTOPIA_DATA_LINKING)
#include <qtopia/pixmapdisplay.h>
#endif

#include <qtopia/resource.h>
#ifdef QTOPIA_PHONE
#include <qtopia/iconselect.h>
#endif

#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif

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
#include <qtabwidget.h>
#include <qtimer.h>

#include <stdlib.h>


static bool onceAWeek(const PimEvent &e)
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
	if (i != orig && e.repeatOnWeekDay(i))
	    return FALSE;
    }
    return TRUE;
}

static void setOnceAWeek( PimEvent &e )
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
	if (i != orig)
	    e.setRepeatOnWeekDay(i, FALSE);
    }
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

//------------------------------------------------------------------------------

EntryDialog::EntryDialog( bool startOnMonday, const QDateTime &start, const QDateTime &end,
			  QWidget *parent, const char *name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f 
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    ),
    startWeekOnMonday( startOnMonday )
{
    Q_UNUSED(name);
    init();
    setDates(start,end);
}

EntryDialog::EntryDialog( bool startOnMonday, const QDate &/*occurrence*/, const PimEvent &event,
			  QWidget *parent, const char *name, bool modal, WFlags f )
    : QDialog( parent, name, modal, f
#ifdef QTOPIA_DESKTOP
    | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#endif
    ),
    mEvent(event), mOrigEvent( event ), startWeekOnMonday( startOnMonday )
{
    Q_UNUSED(name);
    init();
    if (mEvent.timeZone().isValid()) {
	entry->timezone->setCurrentZone(mEvent.timeZone().id());
    }
    setDates(mEvent.start(),mEvent.end());
    entry->comboCategory->setCategories( mEvent.categories(), "Calendar", // No tr
	    tr("Calendar") );
    addOrPick( entry->comboDescription, mEvent.description() );
    addOrPick( entry->comboLocation, mEvent.location() );
    entry->spinAlarm->setValue(mEvent.alarmDelay());
    entry->spinAlarm->setEnabled(mEvent.hasAlarm());
    if ( mEvent.hasAlarm() ) {
	if (mEvent.alarmSound() != PimEvent::Silent)
	    entry->comboSound->setCurrentItem(2);
	else
	    entry->comboSound->setCurrentItem(1);
    } else {
	entry->comboSound->setCurrentItem(0);
    }
    entry->checkAllDay->setChecked( mEvent.isAllDay() );
    if(!mEvent.notes().isEmpty())
	editNote->setText(mEvent.notes());
    if ( mEvent.hasRepeat() ) {
	if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Daily)
	    entry->repeatSelect->setCurrentItem(1);
	else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Weekly
		&& onceAWeek(mEvent) )
	    entry->repeatSelect->setCurrentItem(2);
	else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Yearly)
	    entry->repeatSelect->setCurrentItem(3);
	else
	    entry->repeatSelect->setCurrentItem(4);
	if (mEvent.repeatForever()) {
	    // so that when opens, will have view at or close to start date of event,
	    entry->endDateSelect->setDate(mEvent.start().date());
	    entry->endDateSelect->setDate(QDate());
	} else {
	    entry->endDateSelect->setDate(mEvent.repeatTill());
	}
    } else {
	entry->repeatSelect->setCurrentItem(0);
	entry->endDateSelect->setEnabled(FALSE);
	// so that when opens, will have view at or close to start date of event,
	entry->endDateSelect->setDate(mEvent.start().date());
	entry->endDateSelect->setDate(QDate());
    }
    setRepeatLabel();
    
#ifdef QTOPIA_DATA_LINKING
    QDL::loadLinks( event.customField( QDL::DATA_KEY ), QDL::clients( this ) );
#endif
}

void EntryDialog::setDates( const QDateTime& s, const QDateTime& e )
{
    mOccurrence = s.date();
    mEvent.setStart(s);
    mEvent.setEnd(e);
    entry->startButton->setDateTime(s);
    entry->endButton->setDateTime(e);
}

void EntryDialog::init()
{
    //setCaption?
    QGridLayout *gl = new QGridLayout( this );
    gl->setSpacing( 3 );
    gl->setMargin( 0 );

    tw = new QTabWidget( this, "tabwidget" );
    gl->addMultiCellWidget( tw, 0, 0, 0, 2 );

#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 6 );
    gl->setMargin( 6 );
    QPushButton *buttonCancel = new QPushButton( this, "buttoncancel" );
    buttonCancel->setText( tr("Cancel") );
    gl->addWidget( buttonCancel, 1, 2 );

    QPushButton *buttonOk = new QPushButton( this, "buttonok" );
    buttonOk->setText( tr( "OK" ) );
    gl->addWidget( buttonOk, 1, 1 );
    buttonOk->setDefault( TRUE );

    QSpacerItem *spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    gl->addItem( spacer, 1, 0 );
#endif

    QWidget *noteTab = new QWidget( this );
    QGridLayout *noteLayout = new QGridLayout( noteTab );

    editNote = new QMultiLineEdit( noteTab );
    editNote->setWordWrap( QMultiLineEdit::WidgetWidth );

    int rowCount = 0;
#if defined(QTOPIA_DATA_LINKING)
    QDLWidgetClient *client = new QDLWidgetClient( editNote, "editNote" );
#ifdef QTOPIA_PHONE
    client->setupStandardContextMenu();
#else
    PixmapDisplay *linkButton = new PixmapDisplay( noteTab );
    linkButton->setPixmap( Resource::loadIconSet( "qdllink" )
					    .pixmap( QIconSet::Small, TRUE ) );
    connect( linkButton, SIGNAL(clicked()), client, SLOT(requestLink()) );
    noteLayout->addWidget( linkButton, rowCount++, 0, Qt::AlignRight );
    linkButton->setFocusPolicy( NoFocus );
#endif
#endif
    noteLayout->addWidget( editNote, rowCount++, 0 );
#ifndef QTOPIA_DESKTOP
    VScrollView *sv = new VScrollView( this );
    entry = new EntryDetails( sv->viewport() );
    sv->addChild( entry );
    tw->addTab( sv, tr("Event") );
#else
    entry = new EntryDetails( tw );
    tw->addTab( entry, tr("Event") );
#endif
    entry->startButton->setOrientation(Vertical);
    entry->endButton->setOrientation(Vertical);

    tw->addTab( noteTab, tr("Notes") );

#if defined(QTOPIA_DESKTOP)
    eventView = new EventView( this );
    connect(tw, SIGNAL(currentChanged(QWidget*)),
	    this, SLOT(tabChanged(QWidget*)) );
    tw->addTab( eventView, tr("Summary") );
#else
    eventView = 0;
#endif

    // XXX should load/save this
    entry->comboLocation->insertItem(tr("Office"));
    entry->comboLocation->insertItem(tr("Home","ie. not work"));
    addOrPick( entry->comboLocation, "" );

    // XXX enable these two lines to be able to specify local time events.
    entry->timezone->setLocalIncluded(TRUE);
    entry->timezone->setCurrentZone("None");

    entry->comboDescription->setInsertionPolicy(QComboBox::AtCurrent);
    entry->comboLocation->setInsertionPolicy(QComboBox::AtCurrent);

#ifdef QTOPIA_PHONE
    // Setup the comboSound widget
    entry->comboSound->insertItem( QPixmap( Resource::loadPixmap( "noalarm" ) ), tr( "No Alarm" ) );
    entry->comboSound->insertItem( QPixmap( Resource::loadPixmap( "silent" ) ), tr( "Silent" ) );
    entry->comboSound->insertItem( QPixmap( Resource::loadPixmap( "audible" ) ), tr( "Audible" ) );
#endif

#ifdef QTOPIA_DESKTOP
    connect((QObject *)entry->timezone->d, SIGNAL(configureTimeZones()), this, SLOT(configureTimeZones()));
#endif

    setTabOrder(entry->comboDescription, entry->comboLocation);
    setTabOrder(entry->comboLocation, entry->checkAllDay);
    setTabOrder(entry->checkAllDay,  // No tr
	    (QWidget *)entry->startButton->child("date")); // No tr
    setTabOrder((QWidget *)entry->startButton->child("date"),  // No tr
	    (QWidget *)entry->startButton->child("time")); // No tr
    setTabOrder((QWidget *)entry->startButton->child("time"),  // No tr
	    (QWidget *)entry->endButton->child("date")); // No tr
    setTabOrder((QWidget *)entry->endButton->child("date"),  // No tr
	    (QWidget *)entry->endButton->child("time")); // No tr
    setTabOrder((QWidget *)entry->endButton->child("time"),
	    (QWidget *)entry->timezone->child("timezone combo"));
    setTabOrder((QWidget *)entry->timezone->child("timezone combo"), entry->comboSound); // No tr
    setTabOrder(entry->comboSound, entry->spinAlarm);
    setTabOrder(entry->spinAlarm, entry->repeatSelect);
    setTabOrder(entry->repeatSelect, entry->endDateSelect);
    setTabOrder(entry->endDateSelect, (QWidget *)entry->comboCategory->child("category combo")); // No tr

    QTimer::singleShot( 0, entry->comboDescription, SLOT( setFocus() ) );

    // Now for the lines to do with eneabling for exception events.
    if (mEvent.isException()) {
	entry->repeatSelect->setEnabled(FALSE);
	entry->repeatSelect->hide();
	entry->endDateSelect->setEnabled(FALSE);
	entry->endDateSelect->hide();
	entry->lblEndDate->hide();
	entry->lblRepeat->hide();
    }


    connect( entry->checkAllDay, SIGNAL(toggled(bool)),
	     this, SLOT(allDayToggled(bool)) );

    connect( entry->repeatSelect, SIGNAL(activated(int)),
	    this, SLOT(setRepeatType(int)));
    connect( entry->endDateSelect, SIGNAL(valueChanged(const QDate&)),
	    this, SLOT(setEndDate(const QDate&)));

    connect( entry->spinAlarm, SIGNAL(valueChanged(int)),
	     this, SLOT(turnOnAlarm()) );
    connect( entry->comboSound, SIGNAL(activated(int)),
	     this, SLOT(checkAlarmSpin(int)) );

    connect( entry->endButton, SIGNAL(valueChanged(const QDateTime&)),
	     this, SLOT(endDateTimeChanged(const QDateTime&)) );
    connect( entry->startButton, SIGNAL(valueChanged(const QDateTime&)),
	     this, SLOT(startDateTimeChanged(const QDateTime&)) );

#ifdef QTOPIA_DESKTOP
    connect( buttonOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( buttonCancel, SIGNAL(clicked()), this, SLOT(reject()) );
#endif

#ifdef QTOPIA_DESKTOP
    setMaximumSize( sizeHint()*2 );
#endif
}

/*
 *  Destroys the object and frees any allocated resources
 */
EntryDialog::~EntryDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void EntryDialog::endDateTimeChanged( const QDateTime &e )
{
    entry->startButton->blockSignals(TRUE);
    entry->endButton->blockSignals(TRUE);

    QDateTime target = e;

    // since setting the start can change the end, do this first.
    if (target.addSecs(-300) < mEvent.start()) {
	mEvent.setStart(target.addSecs(-300));
    }

    mEvent.setEnd(target);
    entry->startButton->setDateTime(mEvent.start());

    entry->startButton->blockSignals(FALSE);
    entry->endButton->blockSignals(FALSE);
}

/*
 * public slot
 */
void EntryDialog::startDateTimeChanged( const QDateTime &s )
{
    entry->startButton->blockSignals(TRUE);
    entry->endButton->blockSignals(TRUE);

    // start always works.
    mEvent.setStart(s);

    // modifying start modifies end, so no need check or modify anything.
    // just ensure we update the widget.
    entry->endButton->setDateTime(mEvent.end());

    entry->startButton->blockSignals(FALSE);
    entry->endButton->blockSignals(FALSE);
}

void EntryDialog::slotRepeat()
{
    // Work around for compiler Bug..
    RepeatEntry *e;

    event(); // update of the shown values;
    e = new RepeatEntry( startWeekOnMonday, mEvent, this );

    if ( QPEApplication::execDialog(e) ) {
	 mEvent = e->event();
	 entry->endDateSelect->setEnabled(TRUE);
    }
    setRepeatLabel();
}

void EntryDialog::setWeekStartsMonday( bool onMonday )
{
    startWeekOnMonday = onMonday;
}

PimEvent EntryDialog::event()
{
    mEvent.setDescription( entry->comboDescription->currentText() );
    mEvent.setLocation( entry->comboLocation->currentText() );
    mEvent.setCategories( entry->comboCategory->currentCategories() );
    mEvent.setAllDay( entry->checkAllDay->isChecked() );

    if (entry->endDateSelect->date().isValid()) {
	mEvent.setRepeatForever(FALSE);
	mEvent.setRepeatTill(entry->endDateSelect->date());
    } else {
	mEvent.setRepeatForever(TRUE);
    }
    // don't set the time if theres no need too

    if (entry->timezone->currentZone() == "None")
	mEvent.setTimeZone(TimeZone());
    else
	mEvent.setTimeZone(TimeZone(entry->timezone->currentZone()));

    // all day events don't have an alarm
    if ( entry->checkAllDay->isChecked() )
	entry->comboSound->setCurrentItem(0);
    // we only have one type of sound at the moment... LOUD!!!
    switch (entry->comboSound->currentItem()) {
	case 0:
	    mEvent.clearAlarm();
	    break;
	case 1:
	    mEvent.setAlarm( entry->spinAlarm->value(), PimEvent::Silent);
	    break;
	case 2:
	    mEvent.setAlarm( entry->spinAlarm->value(), PimEvent::Loud);
	    break;
    }
    // don't need to do repeat, repeat dialog handles that.
    mEvent.setNotes( editNote->text() );

#ifdef QTOPIA_DATA_LINKING
    QString links;
    QDL::saveLinks( links, QDL::clients( this ) );
    mEvent.setCustomField( QDL::DATA_KEY, links );
#endif

    return mEvent;
}

void EntryDialog::setRepeatLabel()
{

    if (!mEvent.isException()) {
	if ( mEvent.hasRepeat() ) {
	    //cmdRepeat->setText( tr("Repeat...") );
	    if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Daily)
		entry->repeatSelect->setCurrentItem(1);
	    else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Weekly
		    && onceAWeek(mEvent))
		entry->repeatSelect->setCurrentItem(2);
	    else if (mEvent.frequency() == 1 && mEvent.repeatType() == PimEvent::Yearly)
		entry->repeatSelect->setCurrentItem(3);
	    else
		entry->repeatSelect->setCurrentItem(4);
	} else {
	    entry->repeatSelect->setCurrentItem(0);
	}
    }
}

void EntryDialog::allDayToggled(bool b)
{
    entry->startButton->setTimeEnabled(!b);
    entry->endButton->setTimeEnabled(!b);
    entry->comboSound->setEnabled(!b);
    entry->timezone->setEnabled(!b);
}


void EntryDialog::configureTimeZones()
{
#ifdef QTOPIA_DESKTOP
    // Show the WorldTime as a dialog
    WorldTimeDialog dlg(0, entry->timezone);
    dlg.exec();
#endif
}

void EntryDialog::turnOnAlarm()
{
    // if alarm spin spun, then user probably wants an alarm.  Make it loud :)
    if (entry->comboSound->currentItem() == 0 && entry->spinAlarm->value() != 0)
	entry->comboSound->setCurrentItem(2);
}

void EntryDialog::checkAlarmSpin(int aType)
{
    entry->spinAlarm->setEnabled( aType != 0 );
}

void EntryDialog::setRepeatType(int i)
{
    switch (i) {
	case 0:
	    mEvent.setRepeatType(PimEvent::NoRepeat);
	    entry->endDateSelect->setEnabled(FALSE);
	    break;
	case 1:
	    mEvent.setRepeatType(PimEvent::Daily);
	    mEvent.setFrequency(1);
	    entry->endDateSelect->setEnabled(TRUE);
	    break;
	case 2:
	    mEvent.setRepeatType(PimEvent::Weekly);
	    mEvent.setFrequency(1);
	    setOnceAWeek(mEvent);
	    entry->endDateSelect->setEnabled(TRUE);
	    break;
	case 3:
	    mEvent.setRepeatType(PimEvent::Yearly);
	    mEvent.setFrequency(1);
	    entry->endDateSelect->setEnabled(TRUE);
	    break;
	case 4:
	default:
	    slotRepeat();
	    break;
    }
}

void EntryDialog::setEndDate(const QDate &date)
{
    // chekc the date...
    if (date.isValid() && date < entry->startButton->date()) {
	entry->endDateSelect->blockSignals(TRUE);
	entry->endDateSelect->setDate(entry->startButton->date());
	entry->endDateSelect->blockSignals(FALSE);
    }
}

void EntryDialog::updateCategories()
{
#ifdef QTOPIA_DESKTOP
    connect( this, SIGNAL( categoriesChanged() ),
	     entry->comboCategory, SLOT( categoriesChanged() ) );
    emit categoriesChanged();
    disconnect( this, SIGNAL( categoriesChanged() ),
		entry->comboCategory, SLOT( categoriesChanged() ) );
#endif
}

//#define ACCEPT_DEBUG
void EntryDialog::accept()
{
    hide();
    event();
    PrEvent &oe = (PrEvent&)mOrigEvent;
    PrEvent &ne = (PrEvent&)mEvent;

    QValueList<int> fields;
    fields += PimEvent::Description;
    fields += PimEvent::Location;
    fields += PimEvent::StartTimeZone;
    fields += PimEvent::Notes;
    fields += PimEvent::StartDateTime;
    fields += PimEvent::EndDateTime;
    fields += PimEvent::DatebookType;
    fields += PimEvent::HasAlarm;
    fields += PimEvent::AlarmDelay;
    fields += PimEvent::SoundType;
    fields += PimEvent::RepeatPattern;
    fields += PimEvent::RepeatFrequency;
    fields += PimEvent::RepeatWeekdays;
    fields += PimEvent::RepeatHasEndDate;
    fields += PimEvent::RepeatEndDate;
    fields += PimEvent::RecordParent;
    fields += PimEvent::RecordChildren;
    fields += PimEvent::Exceptions;
    fields += PimRecord::Categories;

#ifdef ACCEPT_DEBUG
    QMap<int, QString> fieldNames;
    fieldNames.insert( PimEvent::Description, "Description" );
    fieldNames.insert( PimEvent::Location, "Location" );
    fieldNames.insert( PimEvent::StartTimeZone, "StartTimeZone" );
    fieldNames.insert( PimEvent::Notes, "Notes" );
    fieldNames.insert( PimEvent::StartDateTime, "StartDateTime" );
    fieldNames.insert( PimEvent::EndDateTime, "EndDateTime" );
    fieldNames.insert( PimEvent::DatebookType, "DatebookType" );
    fieldNames.insert( PimEvent::HasAlarm, "HasAlarm" );
    fieldNames.insert( PimEvent::AlarmDelay, "AlarmDelay" );
    fieldNames.insert( PimEvent::SoundType, "SoundType" );
    fieldNames.insert( PimEvent::RepeatPattern, "RepeatPattern" );
    fieldNames.insert( PimEvent::RepeatFrequency, "RepeatFrequency" );
    fieldNames.insert( PimEvent::RepeatWeekdays, "RepeatWeekdays" );
    fieldNames.insert( PimEvent::RepeatHasEndDate, "RepeatHasEndDate" );
    fieldNames.insert( PimEvent::RepeatEndDate, "RepeatEndDate" );
    fieldNames.insert( PimEvent::RecordParent, "RecordParent" );
    fieldNames.insert( PimEvent::RecordChildren, "RecordChildren" );
    fieldNames.insert( PimEvent::Exceptions, "Exceptions" );
    fieldNames.insert( PimRecord::Categories, "Categories" );
#endif

    bool modified = FALSE;
    for( QValueList<int>::ConstIterator it = fields.begin(); it != fields.end(); ++it )
    {
	QString of = oe.field( *it );
	QString nf = ne.field( *it );
#ifdef ACCEPT_DEBUG
	qDebug( "%s %s %s", fieldNames[*it].latin1(), of.latin1(), nf.latin1() );
#endif
	// if both are empty, we don't care that one is QString::null while the other is QString("")
	if ( !(of.isEmpty() && nf.isEmpty()) && of != nf ) {
	    modified = TRUE;
	    break;
	}
    }

    // Accepting the dialog is pointless if the contents haven't changed
    if ( modified ) {
	QDialog::accept();
    } else {
	qDebug( "Event not modified" );
        QDialog::reject();
    }
}

void EntryDialog::tabChanged( QWidget *tab )
{
    if ( eventView && tab == eventView ) {
	eventView->init( Occurrence( mOccurrence, event() ) );
    }
}

void EntryDialog::showSummary()
{
    tw->showPage( eventView );
}

