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

#define QTOPIA_INTERNAL_FD
#define QTOPIA_INTERNAL_FILEOPERATIONS

#include "datebook.h"
#include "dayview.h"
#include "datebooksettings.h"
#include "datebookweek.h"
#include "entrydialog.h"
#include "monthview.h"
#include "finddialog.h"
#include "exceptiondialog.h"
#include "eventpicker.h"
#include "alarmdialog.h"
#include "eventview.h"

#ifdef QTOPIA_DESKTOP
#include <common/action.h>
#endif

#include <qtopia/vscrollview.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/qpedebug.h>
#include <qtopia/pim/event.h>
#include <qtopia/stringutil.h>
#ifdef Q_WS_QWS
#include <qtopia/ir.h>
#endif
#include <qtopia/qpemenubar.h>
#include <qtopia/qpemessagebox.h>
#include <qtopia/resource.h>
#include <qtopia/sound.h>
#include <qtopia/timestring.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/tzselect.h>
#include <qtopia/xmlreader.h>
#include <qtopia/applnk.h>
#include <qtopia/pim/private/eventio_p.h>
#include <qtopia/pim/private/eventxmlio_p.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#ifdef QTOPIA_DATA_LINKING
#include <qtopia/qdl.h>
#endif
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#endif
#ifdef QTOPIA_DESKTOP
#include <qtopia/categoryselect.h>
#endif
#include <qtopia/datetimeedit.h>

#include <qaction.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qdialog.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qtl.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qregexp.h>
//#ifdef QWS
////#include <qwindowsystem_qws.h>
////#include <qcopchannel_qws.h>
////#endif
#include <qarray.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

#include <stdlib.h>

#define DAY 1
#define WEEK 2
#define MONTH 3
#define EVENT 4


#if !defined(QTOPIA_DESKTOP)
DateBook::DateBook( QWidget *parent, const char *, WFlags f )
    : DateBookGui( parent, "datebook", f ),
      aPreset( FALSE ),
      presetTime( -1 ),
      startTime( 8 ), // an acceptable default
      compressDay( TRUE ),
      syncing(FALSE),
      inSearch(FALSE),
      exceptionMb(0),
      findDialog( 0 )
{
    init();
}
#else
DateBook::DateBook()
    : DateBookGui(),
      aPreset( FALSE ),
      presetTime( -1 ),
      startTime( 8 ), // an acceptable default
      compressDay( TRUE ),
      syncing(FALSE),
      inSearch(FALSE),
      exceptionMb(0),
      findDialog( 0 )
{
}
#endif

void DateBook::init()
{
    int timingLoad, timingOther;
#ifdef QTOPIA_DESKTOP
    Q_UNUSED(timingOther)
#endif

    dayView = 0;
    weekView = 0;
    monthView = 0;
    eventView = 0;

#ifdef Q_WS_QWS
    beamfile = Global::tempDir() + "obex";

    QDir d;
    d.mkdir(beamfile);
    beamfile += "/event.vcs";
#endif
    

    QTime t;
    t.start();
    db = new DateBookTable(this);
    timingLoad = t.elapsed();
    loadSettings();
    DateBookGui::init();

#if !defined(QTOPIA_DESKTOP)
    views = new QWidgetStack( parentWidget );
    setCentralWidget( views );

    timingOther = t.elapsed();
    viewToday();
    TimeString::connectChange(this, SLOT(changeClock()) );
    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)), 
	    this, SLOT(appMessage(const QCString&,const QByteArray&)) );
    connect( qApp, SIGNAL(weekChanged(bool)), this, SLOT(changeWeek(bool)) );
    connect( qApp, SIGNAL(timeChanged()), this, SLOT(checkToday() ));
    connect( qApp, SIGNAL(timeChanged()), this, SLOT(updateAlarms()) );
    connect( qApp, SIGNAL(flush()), this, SLOT(flush()) );
    connect( qApp, SIGNAL(reload()), this, SLOT(reload()) );
#endif // QTOPIA_DESKTOP

    // start Timer
    midnightTimer = new QTimer(this);
    connect(midnightTimer, SIGNAL(timeout()), this, SLOT(checkToday()));
    // 2 seconds after midnight
    midnightTimer->start( (QTime::currentTime().secsTo(QTime(23,59,59)) + 3) * 1000 );
}

void DateBook::checkToday()
{
    if ( lastToday != QDate::currentDate() ) {
	if ( lastToday == currentDate() )
	    slotToday();
	else
	    lastToday = QDate::currentDate();
    } else {
	refreshWidgets();
    }

    midnightTimer->start(
	    (QTime::currentTime().secsTo(QTime(23,59,59)) + 3)
	    * 1000 );
}

void DateBook::updateAlarms()
{
    /* 
       time may have gone backwards... but giving the same alarms 
       again won't hurt (server will ignore) so send them all again,
   */
    // no real way to tell if forward or backward, so need to do the lot.
    db->updateAlarms();
}

void DateBook::refreshWidgets()
{
    // update active view!
    if ( actionDay->isOn() )
	dayView->redraw();
#if !defined(QTOPIA_PHONE)
    else if ( actionWeek->isOn() )
	weekView->redraw();
#endif
    else if ( actionMonth->isOn() )
	monthView->updateOccurrences();
}

DateBook::~DateBook()
{
}

void DateBook::showSettings()
{
#if !defined(QTOPIA_DESKTOP)
    bool whichclock;
    Config config( "qpe" );
    config.setGroup("Time");
    whichclock = config.readBoolEntry("AMPM");
    
    DateBookSettings frmSettings(whichclock, parentWidget, "settings");
    frmSettings.setStartTime( startTime );
    frmSettings.setAlarmPreset( aPreset, presetTime );
    frmSettings.setCompressDay( compressDay );

    if ( QPEApplication::execDialog(&frmSettings) ) {
	aPreset = frmSettings.alarmPreset();
	presetTime = frmSettings.presetTime();
	startTime = frmSettings.startTime();
	compressDay = frmSettings.compressDay();
	if ( dayView ) {
	    dayView->setDayStarts( startTime );
	    dayView->setCompressDay( compressDay );
	}
#if !defined(QTOPIA_PHONE)
	if ( weekView )
	    weekView->setDayStarts( startTime );
#endif
	saveSettings();

	// make the change obvious
	if ( views->visibleWidget() ) {
	    if ( views->visibleWidget() == dayView )
		dayView->redraw();
#if !defined(QTOPIA_PHONE)
	    else if ( views->visibleWidget() == weekView )
		weekView->redraw();
#endif
	}
    }
#endif
}

void DateBook::fileNew()
{
    newEvent("");
}

QString DateBook::checkEvent(const PimEvent &e)
{
    /* check if overlaps with itself */
    bool checkFailed = FALSE;

    /* check the next 12 repeats. should catch most problems */
    QDate current_date = e.end().date();
    int duration = e.start().date().daysTo(e.end().date());
    PimEvent previous = e;
    for(int i = 0; i < 12; i++)
    {
	bool ok;
	QDateTime next;
	next = e.nextOccurrence(current_date.addDays(1), &ok);
	next.setTime(e.start().time());
	if (!ok)
	    break;  // no more repeats
	if(next < previous.end()) {
	    checkFailed = TRUE;
	    break;
	}
	current_date = next.date().addDays(duration);
    }


    if(checkFailed)
	return tr("<qt>Event duration is potentially longer "
		  "than interval between repeats.</qt>");

    return QString::null;
}

QDate DateBook::currentDate()
{
    if ( dayView && views->visibleWidget() == dayView ) {
	return dayView->currentDate();
#if !defined(QTOPIA_PHONE)
    } else if ( weekView && views->visibleWidget() == weekView ) {
        return weekView->currentDate();
#endif
    } else if ( monthView && views->visibleWidget() == monthView ) {
	return monthView->selectedDate();
    } else {
	return QDate(); // invalid;
    }
}

void DateBook::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay( lastToday );
}

void DateBook::viewDay(const QDate& dt)
{
    initDay();
    actionDay->setOn( TRUE );
    dayView->selectDate( dt );
    raiseWidget( dayView );
    dayView->setFocus();
    dayView->redraw();

    updateIcons();
}

void DateBook::viewDay()
{
    viewDay( currentDate() );
}

void DateBook::viewWeek()
{
    viewWeek( currentDate() );
}

void DateBook::viewWeek( const QDate& dt )
{
    initWeek();
    actionWeek->setOn( TRUE );
    weekView->selectDate( dt );
    raiseWidget( weekView );
    weekView->redraw();

    updateIcons();
}

void DateBook::viewMonth()
{
    viewMonth( currentDate() );
}

void DateBook::viewMonth( const QDate& dt)
{
    initMonth();
    actionMonth->setOn( TRUE );
    monthView->setDate( dt );
    raiseWidget( monthView );
    monthView->updateOccurrences();

    updateIcons();
}

void DateBook::updateIcons()
{
    bool view = (eventView && views->visibleWidget() == eventView);

#ifdef QTOPIA_DESKTOP
    bool s = eventSelected();
    actionDelete->setEnabled( s );
    actionEdit->setEnabled( s );
#else
    if ( view ) {
	if ( sub_bar )     sub_bar->hide();
	if ( details_bar ) details_bar->show();
    } else {
	if ( details_bar ) details_bar->hide();
	if ( sub_bar )     sub_bar->show();
    }
    actionNew->setEnabled( !view );
    actionEdit->setEnabled( view  );
    actionDelete->setEnabled( view );
#ifdef Q_WS_QWS
    if (Ir::supported())
	actionBeam->setEnabled( view );
#endif
    actionToday->setEnabled( !view );
    actionDay->setEnabled( !view );
    actionMonth->setEnabled( !view );
    actionSettings->setEnabled( !view );
#ifndef QTOPIA_PHONE
    actionBack->setEnabled( view );
    actionWeek->setEnabled( !view );
#endif
#endif

#ifdef QTOPIA_PHONE
    if ( eventSelected() ) // dayView definitely exists
	ContextBar::setLabel( dayView, Qt::Key_Select, ContextBar::View );
    else if ( dayView ) // make sure it does in this case
	ContextBar::setLabel( dayView, Qt::Key_Select, ContextBar::NoLabel );

    if ( !view ) {
	if ( views->visibleWidget() == monthView )
	    actionMonth->setEnabled( FALSE );
	else
	    actionMonth->setEnabled( TRUE );
    }
#endif
}

void DateBook::initExceptionMb() {
    exceptionMb = new ExceptionDialog( parentWidget, "exceptionDlg", TRUE );
}

// a bit to extreme.
// should be possible to check each exception for if it still fits
// on a valid date.
bool affectsExceptions(const PimEvent &ne, const PimEvent &oe)
{
    if (oe.hasExceptions()) {
	if (ne.start() != oe.start())
	    return TRUE;
	if (ne.frequency() != oe.frequency())
	    return TRUE;
	if (ne.repeatType() != oe.repeatType())
	    return TRUE;
	if ( ((PrEvent &)ne).p_weekMask() != ((PrEvent &)oe).p_weekMask())
	    return TRUE;
    }
    return FALSE;
}

bool DateBook::checkSyncing()
{
    if (syncing) {
	if ( QMessageBox::warning(parentWidget, tr("Calendar"),
		    tr("<qt>Can not edit data, currently syncing</qt>"),
		    QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort )
	{
	    // Okay, if you say so (eg. Qtopia Desktop may have crashed)....
	    syncing = FALSE;
	} else
	    return TRUE;
    }
    return FALSE;
}

void DateBook::editOccurrence( const Occurrence &ev )
{
    editOccurrence( ev, FALSE );
}

void DateBook::editOccurrence( const Occurrence &ev, bool preview )
{
#if !defined(QTOPIA_DESKTOP)
    if ( checkSyncing() )
	return;
#endif

    if ( views->visibleWidget() == eventView )
	hideEventDetails();

    // if this event is an exception, or has exceptions we may need to do somethign different.
    bool asException;
    PimEvent e;

    // stuff for "following" events
    bool following = FALSE;
    PimEvent orig;
    QValueList<QDate> pastExceptions;
    QValueList<QDate> futureExceptions;
    QValueList<QUuid> pastChildren;
    QValueList<QUuid> futureChildren;

    if (ev.event().hasRepeat()) {
	// ask if just this one or is series?
	if (!exceptionMb)
	    initExceptionMb();
	exceptionMb->setCaption( tr("Edit Event") );
	switch (exceptionMb->exec()) {
	    default:
		return;
	    case ExceptionDialog::Current:
		e = ev.event();
		// modify e to be an exceptional event.
		// with no uid (yet).
		e.setSeriesUid(ev.event().uid());
		e.setRepeatType(PimEvent::NoRepeat);
		e.setStart(ev.start());
		e.clearExceptions();
		asException = TRUE;
		break;
	    case ExceptionDialog::All:
		e = ev.event();
		asException = FALSE;
		break;
	    case ExceptionDialog::Following:
		// If we select Following but we're looking at the first instance,
		// just do the same as All.
		if ( ev.date() == e.start().date() ) {
		    e = ev.event();
		    asException = FALSE;
		    break;
		}

		e = ev.event();
		orig = ev.event();
		// modify e to be a new series with no uid (yet).
		e.setSeriesUid(QUuid());
		e.setStart(ev.start());
		asException = TRUE;
		following = TRUE;

		pastExceptions = ((PrEvent&)e).exceptions();
		futureExceptions = ((PrEvent&)e).exceptions();
		futureChildren = ((PrEvent&)e).childUids();
		pastChildren = ((PrEvent&)e).childUids();
		QValueList<QDate> processedDates;
		// we can only remove everything
		// put it back in while processing the lists
		e.clearExceptions();

		QValueListIterator<QUuid> uiter;
		QValueListIterator<QDate> diter;
		// disassociate all children that are in the "past"
		for ( uiter = futureChildren.begin(); uiter != futureChildren.end(); ) {
		    QUuid u = *uiter;
		    PimEvent child = db->find( u );
		    QDate dateOfChild = child.start().date();
		    if ( dateOfChild < ev.date() ) {
			uiter = futureChildren.remove( uiter );
		    } else {
			uiter++;
			pastChildren.remove( u );
			e.addException( dateOfChild, u );
			processedDates.append( dateOfChild );
		    }
		}

		// remove all exceptions from the "past"
		for ( diter = futureExceptions.begin(); diter != futureExceptions.end(); ) {
		    QDate d = *diter;
		    if ( d < ev.date() ) {
			diter = futureExceptions.remove( diter );
		    } else {
			diter++;
			pastExceptions.remove( d );
			// some of these would have been added by processing the children
			if ( processedDates.contains( d ) == 0 )
			    ((PrEvent&)e).addException( d );
		    }
		}

		break;
	}
    } else {
	e = ev.event();
	asException = FALSE;
    }

    EntryDialog editDlg( onMonday, ev.date(), e, parentWidget, "edit-event", TRUE );
    if ( preview )
	editDlg.showSummary();
    // workaround added for text input.
    if (e.isException())
#ifdef QTOPIA_PHONE
	editDlg.setCaption( tr("Edit Exception") );
#else
	editDlg.setCaption( tr("Edit Event Exception") );
#endif
    else
	editDlg.setCaption( tr("Edit Event") );

    // connect the qApp stuff.
    connect( qApp, SIGNAL(weekChanged(bool)),
	     &editDlg, SLOT(setWeekStartsMonday(bool)) );

#ifdef QTOPIA_DESKTOP
    connect( editDlg.entryDetails()->comboCategory,
	     SIGNAL( editCategoriesClicked(QWidget*) ),
	     SLOT( editCategories(QWidget*) ) );
    editDlg.resize(500, 300);
#endif

    //entry->timezone->setEnabled(FALSE);

    while (QPEApplication::execDialog(&editDlg) ) {
	PimEvent newEv = editDlg.event();
	QString error = checkEvent(newEv);
	if (!error.isNull()) {
	    if (QMessageBox::warning(parentWidget, "Error",
			error, "Fix it", "Continue", 0, 0, 1) == 0)
		continue;
	}
	QUuid u;
	if ( following ) {
	    // We've got all the forward events and exceptions already
	    // Now remove then from the "old" series. Of course, just
	    // to be difficult, we can't do that so remove then all and
	    // then add back the appropriate ones
	    orig.clearExceptions();
	    QValueList<QDate> processedDates;
	    QValueListIterator<QUuid> uiter;
	    QValueListIterator<QDate> diter;
	    // Children first
	    for ( uiter = pastChildren.begin(); uiter != pastChildren.end(); uiter++ ) {
		QUuid u = *uiter;
		PimEvent child = db->find( u );
		QDate dateOfChild = child.start().date();
		orig.addException( dateOfChild, u );
		processedDates.append( dateOfChild );
	    }
	    // Now the extra exceptions
	    for ( diter = pastExceptions.begin(); diter != pastExceptions.end(); diter++ ) {
		QDate d = *diter;
		if ( processedDates.contains( d ) == 0 )
		    ((PrEvent&)orig).addException( d );
	    }

	    // Change the repeat end for the "old" series to be the current event's date
	    orig.setRepeatTill( newEv.start().date().addDays( -1 ) );
	    db->updateEvent( orig );
	    u = db->addEvent( newEv );

	    // Now change all the "future" children to belong to the correct series
	    for ( uiter = futureChildren.begin(); uiter != futureChildren.end(); uiter++ ) {
		QUuid uid = *uiter;
		PimEvent child = db->find( uid );
		((PrEvent&)child).setParentUid( u );
		db->updateEvent( child );
	    }
	} else {
	    if (asException) {
		u = db->addException(ev.date(),ev.event(), newEv);
	    } else {
		if (affectsExceptions(newEv, e)) {
		    if (QMessageBox::warning(parentWidget, tr("Calendar"),
				tr( "<p>Changes to the start time or recurrence pattern"
				    " of this event will cause all exceptions to this event"
				    " to be lost.  Continue?"),
				QMessageBox::Ok, QMessageBox::Cancel|QMessageBox::Default
				) != QMessageBox::Ok)
			return;
		    /// need to clear exceptions
		    db->removeExceptions(newEv);
		    newEv.clearExceptions();
		}
		db->updateEvent(newEv);
		u = newEv.uid();
	    }
	}
	emit eventsChanged();
	if ( views->visibleWidget() == dayView ) {
	    bool ok;
	    PimEvent e = db->find( u, &ok );
	    if ( ok )
		dayView->setCurrentEvent( e );
	}
	break;
    }
}

void DateBook::removeOccurrence( const Occurrence &o )
{
#if !defined(QTOPIA_DESKTOP)
    if ( checkSyncing() )
	return;
#endif

    PimEvent e = o.event();

    QString strName = e.description();
    if (e.hasRepeat()) {
	// ask if just this one or is series?
	if (!exceptionMb)
	    initExceptionMb();
	exceptionMb->setCaption( tr("Delete Event") );
	QValueListIterator<QUuid> uiter;
	QValueListIterator<QDate> diter;
	switch (exceptionMb->exec()) {
	    default:
		return;
	    case ExceptionDialog::Current:
		db->addException( o.start().date(), e );
		break;
	    case ExceptionDialog::All:
		db->removeEvent( e );
		break;
	    case ExceptionDialog::Following:
		// If we select Following but we're looking at the first instance,
		// just do the same as All.
		if ( o.date() == e.start().date() ) {
 		    db->removeEvent( e );
		    break;
		}

		PimEvent orig = o.event();
		QValueList<QDate> pastExceptions = ((PrEvent&)e).exceptions();
		QValueList<QDate> futureExceptions = ((PrEvent&)e).exceptions();
		QValueList<QUuid> futureChildren = ((PrEvent&)e).childUids();
		QValueList<QUuid> pastChildren = ((PrEvent&)e).childUids();
		// we can only remove everything
		// put it back in while processing the lists
		e.clearExceptions();

		// disassociate all children that are in the "past"
		for ( uiter = futureChildren.begin(); uiter != futureChildren.end(); ) {
		    QUuid u = *uiter;
		    PimEvent child = db->find( u );
		    QDate dateOfChild = child.start().date();
		    if ( dateOfChild < o.date() ) {
			uiter = futureChildren.remove( uiter );
		    } else {
			uiter++;
			pastChildren.remove( u );
		    }
		}

		// remove all exceptions from the "past"
		for ( diter = futureExceptions.begin(); diter != futureExceptions.end(); ) {
		    QDate d = *diter;
		    if ( d < o.date() ) {
			diter = futureExceptions.remove( diter );
		    } else {
			diter++;
			pastExceptions.remove( d );
		    }
 		}

		// We've got all the forward events and exceptions already
		// Now remove them from the "old" series. Of course, just
		// to be difficult, we can't do that so remove then all and
		// then add back the appropriate ones
		orig.clearExceptions();
		QValueList<QDate> processedDates;
		// Children first
		for ( uiter = pastChildren.begin(); uiter != pastChildren.end(); uiter++ ) {
		    QUuid u = *uiter;
		    PimEvent child = db->find( u );
		    QDate dateOfChild = child.start().date();
		    orig.addException( dateOfChild, u );
		    processedDates.append( dateOfChild );
		}
		// Now the extra exceptions
		for ( diter = pastExceptions.begin(); diter != pastExceptions.end(); diter++ ) {
		    QDate d = *diter;
		    if ( processedDates.contains( d ) == 0 )
			((PrEvent&)orig).addException( d );
		}

		// Change the repeat end for the "old" series to be the current event's date
		orig.setRepeatTill( o.date().addDays( -1 ) );
		db->updateEvent( orig );

		// Now delete all the "future" children
		for ( uiter = futureChildren.begin(); uiter != futureChildren.end(); uiter++ ) {
		    QUuid uid = *uiter;
		    PimEvent child = db->find( uid );
		    db->removeEvent( child );
		}

		break;
	}
    } else  {
	if ( !QPEMessageBox::confirmDelete( parentWidget, tr( "Calendar" ),strName ) )
	    return;
	db->removeEvent( e );
    }

    if ( views->visibleWidget() == dayView && dayView )
        dayView->redraw();
}

void DateBook::addEvent( const PimEvent &e )
{
    QDate d = e.start().date();
    initDay();
    dayView->selectDate( d );
}

void DateBook::editCurrentEvent()
{
    if ( views->visibleWidget() == eventView )
	hideEventDetails();
    if (eventSelected())
	editOccurrence(currentOccurrence());
}

void DateBook::removeCurrentEvent()
{
    if ( views->visibleWidget() == eventView )
	hideEventDetails();
    if (eventSelected())
	removeOccurrence(currentOccurrence());
}

void DateBook::beamCurrentEvent()
{
#ifdef Q_WS_QWS
    if ( views->visibleWidget() == eventView )
	hideEventDetails();
    if (eventSelected())
	beamEvent(currentEvent());
#endif
}


bool DateBook::eventSelected() const
{
    if (views->visibleWidget() && views->visibleWidget() == dayView) {
	return dayView->hasSelection();
    }
    return FALSE;
}

PimEvent DateBook::currentEvent() const
{
    return dayView->currentEvent();
}

Occurrence DateBook::currentOccurrence() const
{
    return dayView->currentItem();
}

void DateBook::initDay()
{
    if ( !dayView ) {
	dayView = new DayView( db, onMonday, views, "dayview" ); // No tr
	dayView->setCompressDay( compressDay );
	views->addWidget( dayView, DAY );
	dayView->setDayStarts( startTime );
	connect( this, SIGNAL( eventsChanged() ),
		 dayView, SLOT( redraw() ) );
	connect( dayView, SIGNAL( newEvent() ),
		 this, SLOT( fileNew() ) );
	connect( dayView, SIGNAL( removeOccurrence(const Occurrence&) ),
		 this, SLOT( removeOccurrence(const Occurrence&) ) );
	connect( dayView, SIGNAL( editOccurrence(const Occurrence&) ),
		 this, SLOT( editOccurrence(const Occurrence&) ) );
	connect( dayView, SIGNAL( beamEvent(const PimEvent&) ),
		 this, SLOT( beamEvent(const PimEvent&) ) );
	connect( dayView, SIGNAL(newEvent(const QString&)),
		 this, SLOT(newEvent(const QString&)) );
	connect( dayView, SIGNAL(selectionChanged()),
		 this, SLOT(updateIcons()) );
	connect( dayView, SIGNAL(showDetails()),
		this, SLOT(showEventDetails()) );

	// qApp connections
	connect( qApp, SIGNAL(weekChanged(bool)),
		dayView, SLOT(setStartOnMonday(bool)) );
    }
}

void DateBook::initWeek()
{
    if ( !weekView ) {
	weekView = new WeekView( db, onMonday, views, "weekview" ); // No tr
	weekView->setDayStarts( startTime );
	views->addWidget( weekView, WEEK );
	connect( weekView, SIGNAL( dateActivated(const QDate&) ),
             this, SLOT( viewDay(const QDate&) ) );
	connect( this, SIGNAL( eventsChanged() ),
		 weekView, SLOT( redraw() ) );

	// qApp connections
	connect( qApp, SIGNAL(weekChanged(bool)),
		weekView, SLOT(setStartOnMonday(bool)) );
    }
}

void DateBook::initMonth()
{
    if ( !monthView ) {
	monthView = new MonthView( db, views, "monthview" ); // No tr
#if !defined(QTOPIA_DESKTOP)
	monthView->setMargin(0);
#endif
	views->addWidget( monthView, MONTH );
	connect( monthView, SIGNAL( dateClicked(const QDate&) ),
             this, SLOT( viewDay(const QDate&) ) );
	connect( this, SIGNAL( eventsChanged() ),
		 monthView, SLOT( updateOccurrences() ) );
    }
}

void DateBook::initEvent()
{
    if ( ! eventView ) {
	eventView = new EventView( views, "eventview" );
	views->addWidget( eventView, EVENT );
#ifdef QTOPIA_PHONE
	eventView->setMargin(0);
#endif

	connect( eventView, SIGNAL(done()), this, SLOT(hideEventDetails()) );
    }
}

void DateBook::loadSettings()
{
    {
	Config config( "qpe" );
	config.setGroup("Time");
	onMonday = config.readBoolEntry( "MONDAY" );
    }

    {
	Config config("DateBook");
	config.setGroup("Main");
	startTime = config.readNumEntry("startviewtime", 8);
	aPreset = config.readBoolEntry("alarmpreset");
	presetTime = config.readNumEntry("presettime");
#ifdef QTOPIA_PHONE
	compressDay = TRUE;
#else
	compressDay = config.readBoolEntry("compressday", TRUE);
#endif
    }
}

void DateBook::saveSettings()
{
    Config config( "qpe" );
    Config configDB( "DateBook" );
    configDB.setGroup( "Main" );
    configDB.writeEntry("startviewtime",startTime);
    configDB.writeEntry("alarmpreset",aPreset);
    configDB.writeEntry("presettime",presetTime);
    configDB.writeEntry("compressday", compressDay);
}

void DateBook::appMessage(const QCString& msg, const QByteArray& data)
{
    bool needShow = FALSE;
    QDataStream stream( data, IO_ReadOnly );

    // There's a problem somewhere. A parent that is not visible has a
    // screwy geometry so if we use the parent the dialogs will be
    // misaligned and incorrectly sized. Better to just use a parent of
    // 0 because the dialogs show correctly in this case.
    QWidget *parent = 0;
    if ( parentWidget && !parentWidget->isVisible() ) {
        parent = parentWidget;
        parentWidget = 0;
    }

    /*@ \service TimeMonitor */
    if ( msg == "alarm(QDateTime,int)" ) {
        /*@ \message
            \arguments when warn
          Register or respond to a preset alarm.
          The event is at \e when and the alarm goes off \e warn minutes before.
        */
	QDateTime when; int warn;
	stream >> when >> warn;

	// may be more than one item.
	QValueList<Occurrence> items = db->getNextAlarm(when, warn);
	QValueListIterator<Occurrence> it;
	bool skip_dialogs = FALSE;

	for (it = items.begin(); it != items.end(); ++it) {
	    Occurrence item = *it;
	    // First Update the alarm for the event.
	    db->updateAlarm(item.event());

	    QDateTime current = QDateTime::currentDateTime();
	    // if we are told to skip and still getting a flood of messages,
	    // continue.
	    if (lastcall.addSecs(1) >= current 
		    && lastcall.addSecs(-1) <= current
		    && skip_dialogs)
		continue;


	    // if alarm in past, (or nearly in the past) go off.
	    if (current.addSecs(60) >= when) {
		bool bSound = FALSE;
		int stopTimer = 0;

		if ( item.event().alarmSound() != PimEvent::Silent ) {
		    bSound = TRUE;
		    Sound::soundAlarm();
		    stopTimer = startTimer( 5000 );
		}

		AlarmDialog dlg( parentWidget, 0, TRUE );
		switch ( dlg.exec(item) ) {
		    case AlarmDialog::Details:
			needShow = TRUE;
			break;
		    default:
                        //close();
			needShow = FALSE;
			break;
		}
		skip_dialogs = dlg.getSkipDialogs();

		if ( bSound )
		    killTimer( stopTimer );

		lastcall = QDateTime::currentDateTime();

		if ( needShow ) {
                    parentWidget = parent;
		    viewDay( item.startInCurrentTZ().date() );
		    dayView->setCurrentItem(item);
		    showEventDetails();
		}
	    }
	}
    /*@ \service Receive */
    } else if ( msg == "receiveData(QString,QString)" ) {
        /*@ \message
            \arguments file mimetype
          Handle incoming IR data.
          The data is in \e file and the \e mimetype must match text/x-vcalendar.
        */
	QString f,t;
	stream >> f >> t;
	if ( t.lower() == "text/x-vcalendar" )
	    if ( receiveFile(f) )
		needShow = TRUE;
	QFile::remove(f);
    /*@ \service Calendar */
    } else if ( msg == "newEvent()" ) {
        /*@ \message
          Open a dialog so the user can create a new event. Default values are used.
        */
	if ( newEvent("") )
	    needShow = TRUE;
    } else if ( msg == "newEvent(QDateTime,QDateTime,QString,QString)" ) {
        /*@ \message
            \arguments start end description notes
          Open a dialog so the user can create a new event.
          The new event will use the \e start and \e end times and will set the
          \e description and \e notes.
        */
	QDateTime s,e;
	QString d,n;
	stream >> s >> e >> d >> n;
	if ( newEvent(s,e,d,n) )
	    needShow = TRUE;
    } else if ( msg == "updateEvent(PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event
          Update the \e event (if not syncing).
        */
	PimEvent e;
	stream >> e;
	db->updateEvent(e);
	refreshWidgets();
    } else if ( msg == "addEvent(PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event
          Add the \e event (if not syncing).
        */
	PimEvent e;
	stream >> e;
	db->addEvent(e);
	refreshWidgets();
    } else if ( msg == "removeEvent(PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event
          Remove the \e event (if not syncing).
        */
	PimEvent e;
	stream >> e;
	db->removeEvent(e);
	refreshWidgets();
    } else if ( msg == "addException(PimEvent,QDate)" && !syncing ) {
        /*@ \message
            \arguments event, date
          Add an exception to \e event on \e date (if not syncing).
          This removes an occurrence from a repeating event.
        */
	PimEvent e, x;
	QDate date;
	stream >> e;
	stream >> date;
	db->addException(date, e);
	refreshWidgets();
    } else if ( msg == "addException(PimEvent,QDate,PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event date event2
          Add an exception to \e event on \e date (if not syncing).
          This links \e event2 as the modified occurrence.
        */
	PimEvent e, x;
	QDate date;
	stream >> e;
	stream >> date;
	stream >> x;
	db->addException(date, e, x);
	refreshWidgets();
    } else if ( msg == "removeException(PimEvent,QDate)" && !syncing ) {
        /*@ \message
            \arguments event date
          Remove an exception from \e event on \e date (if not syncing).
        */
	PimEvent e;
	QDate date;
	stream >> e;
	stream >> date;
	e.removeException(date);
	db->updateEvent(e);
	refreshWidgets();
    } else if ( msg == "removeException(PimEvent,PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event date
          Remove exception \e event2 from \e event (if not syncing).
        */
	PimEvent e, x;
	stream >> e;
	stream >> x;
	if (x.seriesUid() == e.uid()) {
	    db->removeEvent(x);
	    refreshWidgets();
	}
    } else if ( msg == "removeAllExceptions(PimEvent)" && !syncing ) {
        /*@ \message
            \arguments event
          Remove all exceptions from \e event (if not syncing).
        */
	PimEvent e;
	stream >> e;
	db->removeExceptions(e);
	e.clearExceptions();
	db->updateEvent(e);
	refreshWidgets();
    } else if ( msg == "raiseToday()" ) {
        /*@ \message
          Open the datebook showing today.
        */
	bool visible=FALSE;
	if ( data.size() ) {
	    int i;
	    stream >> i; // backdoor kludge
	    visible = i;
	}
	if ( visible )
	    nextView();
	else
	    viewToday();
	needShow = TRUE;
    } else if ( msg == "nextView()" ) {
        /*@ \message
          Switch to the next view.
        */
	nextView();
	needShow = !!views->visibleWidget();
    } else if ( msg == "showEvent(QUuid)" ) {
        /*@ \message
            \arguments uid
          Show the event indicated by \e uid.
        */
	QUuid u;

	stream >> u;

	bool ok;
	Occurrence o = db->find(u, QDate::currentDate(), &ok);

	if (ok) {
	    viewDay( o.startInCurrentTZ().date() );
	    //dayView->selectDate( o.startInCurrentTZ().date() );
	    dayView->setCurrentItem(o);
	    showEventDetails();
	    needShow = TRUE;
	}
    } else if ( msg == "showEvent(QUuid,QDate)" ) {
        /*@ \message
            \arguments uid, date
          Show the occurence of the event indicated by \e uid on \e date.
        */
	QUuid u;
	QDate date;

	stream >> u >> date;

	bool ok;
	Occurrence o = db->find(u, date, &ok);

	if (ok) {
	    viewDay( o.startInCurrentTZ().date() );
	    //dayView->selectDate( o.startInCurrentTZ().date() );
	    dayView->setCurrentItem(o);
	    showEventDetails();
	    needShow = TRUE;
	}
    }
    else if ( msg == "cleanByDate(QDate)" ) {
        /*@ \message
            \arguments date
          Allow the system cleanup wizard to recover some space.
          The \e date to clean from is a hint only. Datebook will only remove
          events and occurrences where doing so will save space.
        */
	QDate d;
	stream >> d;
	purgeEvents( d, FALSE );
    }
#ifdef QTOPIA_DATA_LINKING
    /*@ \service qdl */
    else if ( msg == "QDLRequestLink(QString,QString)" ) {
        /*@ \message
            \arguments client, hint
          Allow another app to embed a link to an event. Popup a dialog so that user can select the event to link.
          The client \e QString sends a hint \e QString (that the datebook currently ignores).
        */
	QString clientID, hint;
	stream >> clientID >> hint;
	QDLHeartBeat hb( clientID );

	EventPicker evtPick( this, parentWidget, "evtPick", TRUE );
	evtPick.showMaximized();
	if ( evtPick.exec() ) {
	    if ( !evtPick.eventSelected() ) {
		qDebug( "No Event Selected!" );
	    } else {
		QByteArray dataref;
		QDataStream ds( dataref, IO_WriteOnly );
		PimEvent event = evtPick.currentEvent();
		ds << event.uid() << evtPick.currentDate();

#ifndef QT_NO_COP
		QCopEnvelope e( QDL::CLIENT_CHANNEL, "QDLProvideLink(QString,int,...)" );
		e << clientID;

		e << (int)1;

		e << QDLLink( QCString( "datebook" ), dataref, event.description(),
							    QCString( "datebook/DateBook" ) );
#endif
	    }
	} else {
#ifndef QT_NO_COP
	    QCopEnvelope e( QDL::CLIENT_CHANNEL, "QDLProvideLink(QString,int,...)" );

	    e << clientID;
	    e << (int)0;
#endif
	}
    }
    else if ( msg == "QDLActivateLink(QByteArray)" ) {
        /*@ \message
            \arguments data
          Another app is activating an embedded link.
          The \e data was set by datebook in response to QDLRequestLink(QString,QString).
        */
	QByteArray dataref;
	stream >> dataref;
	QDataStream ds( dataref, IO_ReadOnly );

	QUuid u;
	QDate date;
	ds >> u >> date;

	bool ok;
	Occurrence o = db->find( u, date, &ok );
	if ( ok ) {
	    viewDay( o.startInCurrentTZ().date() );
	    dayView->setCurrentItem( o );
            showEventDetails();
	    needShow = TRUE;
	}
    }
#endif

    if ( parent )
        parentWidget = parent;

    if ( needShow ) {
#if defined(Q_WS_QWS) || defined(_WS_QWS_)
	//showMaximized();
#else
	//show();
#endif
	//raise();
	QPEApplication::setKeepRunning();
	//setActiveWindow();
    }
}

void DateBook::nextView()
{
    QWidget* cur = views->visibleWidget();
    if ( cur ) {
	if ( cur == dayView )
#if !defined(QTOPIA_PHONE)
	    viewWeek();
	else if ( cur == weekView )
#endif
	    viewMonth();
	else if ( cur == monthView )
	    viewDay();
    }
}

void DateBook::reload()
{
    // reload isn't as meaningful anymore.
    //db->reload();} else
    db->reload();
    if ( actionDay->isOn() )
	viewDay();
#if !defined(QTOPIA_PHONE)
    else if ( actionWeek->isOn() )
	viewWeek();
#endif
    else if ( actionMonth->isOn() )
	viewMonth();
    syncing = FALSE;
}

void DateBook::flush()
{
    db->flush();
    syncing = TRUE;
   // db->save();
    // neither is saving.
}

void DateBook::timerEvent( QTimerEvent *e )
{
    static int stop = 0;
    if ( stop < 10 ) {
	Sound::soundAlarm();
	stop++;
    } else {
	stop = 0;
	killTimer( e->timerId() );
    }
}

void DateBook::changeClock()
{
    // repaint the affected objects...
    if (dayView) dayView->redraw();
#if !defined(QTOPIA_PHONE)
    if (weekView) weekView->redraw();
#endif
}

void DateBook::changeWeek( bool m )
{
    /* no need to redraw, each widget catches.  Do need to
       store though for widgets we haven't made yet */
    onMonday = m;
}

void DateBook::slotToday()
{
    // we need to view today
    lastToday = QDate::currentDate();
    if ( views->visibleWidget() == dayView ) {
	viewDay( lastToday );
#if !defined(QTOPIA_PHONE)
    } else if (views->visibleWidget() == weekView) {
	weekView->selectDate( lastToday );
#endif
    } else if (views->visibleWidget() == monthView){
	monthView->setDate( lastToday );
    }
}

void DateBook::closeEvent( QCloseEvent *e )
{
    if ( views->visibleWidget() == eventView ) {
        hideEventDetails();
#ifdef QTOPIA_PHONE
        e->ignore();
        return;
#endif
    }

    slotToday();
    if(syncing) {
	/* no need to save, did that at flush */
	e->accept();
	return;
    }

    // save settings will generate it's own error messages, no
    // need to do checking ourselves.
    saveSettings();
    //if ( db->save() )
    e->accept();
#if 0
    else {
	if ( QMessageBox::critical( parentWidget, tr( "Out of space" ),
				    tr("<qt>Calendar was unable to save "
				       "your changes. "
				       "Free up some space and try again."
				       "<br>Quit anyway?</qt>"),
				    QMessageBox::Yes|QMessageBox::Escape,
				    QMessageBox::No|QMessageBox::Default )
	     != QMessageBox::No )
	    e->accept();
	else
	    e->ignore();
    }
#endif
}

bool DateBook::newEvent( const QString &description )
{
    QDateTime current = QDateTime::currentDateTime();
    current.setDate( currentDate() );
    QDateTime start=current, end=current;

    int mod = QTime(0,0,0).secsTo(current.time()) % 900;
    if (mod != 0)  {
	mod = 900 - mod;
        current = current.addSecs( mod );
    }

    start.setTime(current.time());
    start.setDate(current.date());
    end = current.addSecs( 3600 );

    return newEvent(start,end,description,QString::null);
}

bool DateBook::newEvent(const QDateTime& dstart,const QDateTime& dend,const QString& description,const QString& notes)
{
#ifndef QTOPIA_DESKTOP
    if ( checkSyncing() )
	return FALSE;
#endif

    QDateTime start=dstart, end=dend;
    QDateTime current = QDateTime::currentDateTime();
    current.setDate( currentDate() );
    bool snull = start.date().isNull();
    bool enull = end.date().isNull();

    if ( views->visibleWidget() ) {
	if ( views->visibleWidget() == dayView ) {
	    dayView->selectedDates( start, end );
	} else if ( views->visibleWidget() == monthView ) {
	    start.setDate( monthView->selectedDate() );
	}
#if !defined(QTOPIA_PHONE)
	else if ( views->visibleWidget() == weekView ) {
	    start.setDate( weekView->currentDate() );
	}
#endif
    }
    if ( start.date().isNull() )
	start.setDate( current.date() );
    if ( end.date().isNull() )
	end.setDate( start.date() );
    if ( (snull && end.time().isNull()) || !start.time().isValid() ) {
	// We get to here from a key pressed in the Day View
	// So we can assume some things.  We want the string
	// passed in to be part of the description.
	// move current to the next fifteen minutes
	int mod = QTime(0,0,0).secsTo(current.time()) % 900;
	if (mod != 0)  {
	    mod = 900 - mod;
            current = current.addSecs( mod );
	}

	// default start
	start.setTime(current.time());
    }
    if ( (enull && end.time().isNull()) || !end.time().isValid() ) {
	// default end
	end = start.addSecs(3600);
    }

    PimEvent ev;
    ev.setDescription( description );
    // When the new gui comes in, change this...
    ev.setLocation( "" );
    ev.setStart( start );
    ev.setEnd( end );
    ev.setNotes( notes );
    if ( aPreset )
	ev.setAlarm( presetTime, PimEvent::Loud );

    EntryDialog e( onMonday, start.date(), ev, parentWidget, "new-event", TRUE );
    e.setCaption( EntryDetails::tr("New Event") );

#ifdef QTOPIA_DESKTOP
    connect( e.entryDetails()->comboCategory,
	     SIGNAL(editCategoriesClicked(QWidget*)),
	     SLOT(editCategories(QWidget*)) );
    connect( this, SIGNAL(categoriesChanged()),
	     &e, SLOT(updateCategories()) );
    e.resize( 500, 300 );
#endif

    while (QPEApplication::execDialog(&e)) {
	ev = e.event();
	//ev.assignUid(); // um, don't know if we can drop this or not
	QString error = checkEvent( ev );
	if ( !error.isNull() ) {
	    if ( QMessageBox::warning( parentWidget, tr("Error!"),
				       error, tr("Fix it"), tr("Continue"), 0, 0, 1 ) == 0 )
		continue;
	}
	QUuid id = db->addEvent( ev );
	emit eventsChanged();
	if ( views->visibleWidget() == dayView ) {
	    dayView->clearSelectedDates();
	    bool ok;
	    PimEvent e = db->find( id, &ok );
	    if ( ok )
		dayView->setCurrentEvent( e );
	}
	return TRUE;
    }
    if ( views->visibleWidget() == dayView )
	dayView->clearSelectedDates();
    return FALSE;
}

void DateBook::setDocument( const QString &filename )
{
    DocLnk doc(filename);
    if ( doc.isValid() )
	receiveFile(doc.file());
    else
	receiveFile(filename);
}

bool DateBook::receiveFile( const QString &filename )
{
    QValueList<PimEvent> tl = PimEvent::readVCalendar( filename );

    QString msg = tr("<P>%1 new events.<p>Do you want to add them to your Calendar?").
	arg(tl.count());

    if ( QMessageBox::information(parentWidget, tr("New Events"),
	    msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
	QDateTime from,to;
	for( QValueList<PimEvent>::Iterator it = tl.begin(); it != tl.end(); ++it ) {
	    if ( from.isNull() || (*it).start() < from )
		from = (*it).start();
	    if ( to.isNull() || (*it).end() < to )
		to = (*it).end();
	    db->addEvent( *it );
	}

	// Change view to a sensible one...
#ifdef QTOPIA_PHONE
	if ( from.date() == to.date() )
	    viewDay( from.date() );
	else
	    viewMonth( from.date() );
#else
	if ( from.date() == to.date() ) {
	    viewDay(from.date());
	} else {
	    initWeek();
	    int fw,fy,tw,ty;
	    weekView->calcWeek(from.date(), fw, fy);
	    weekView->calcWeek(to.date(), tw, ty);
	    if ( fw == tw && fy == ty ) {
		viewWeek(from.date());
	    } else if ( from.date().month() == to.date().month()
		    && from.date().year() == to.date().year() ) {
		viewMonth(from.date());
	    } else {
		viewDay(from.date());
	    }
	}
#endif

	emit eventsChanged();
	return TRUE;
    }
    return FALSE;
}

void DateBook::beamEvent( const PimEvent &e )
{
#ifdef Q_WS_QWS
    ::unlink( beamfile.local8Bit().data() ); // delete if exists

    PimEvent::writeVCalendar( beamfile, e );
    Ir *ir = new Ir( this );
    connect( ir, SIGNAL( done(Ir*) ), this, SLOT( beamDone(Ir*) ) );
    QString description = e.description();
    ir->send( beamfile, description, "text/x-vCalendar" );
#else
    Q_UNUSED(e)
#endif
}

void DateBook::beamDone( Ir *ir )
{
#ifdef Q_WS_QWS
    delete ir;
    ::unlink( beamfile.local8Bit().data() );
#else
    Q_UNUSED(ir)
#endif
}


void DateBook::slotFind()
{
    // move it to the day view...
    viewDay();
    if ( ! findDialog ) {
        findDialog = new FindDialog( "Calendar", parentWidget ); // no tr
    }
    findDialog->setUseDate( true );
    findDialog->setDate( currentDate() );
    connect( findDialog, SIGNAL(signalFindClicked(const QString&,const QDate&,bool,bool,int)),
	     this, SLOT(slotDoFind(const QString&,const QDate&,bool,bool,int)) );
    QPEApplication::execDialog(findDialog);
    inSearch = false;
}

bool catComp( QArray<int> cats, int category )
{
    bool returnMe;
    int i,
	count;

    count = int(cats.count());
    returnMe = false;
    if ( (category == -1 && count == 0) || category == -2 )
	returnMe = true;
    else {
	for ( i = 0; i < count; i++ ) {
	    if ( category == cats[i] ) {
		returnMe = true;
		break;
	    }
	}
    }
    return returnMe;
}

void DateBook::slotDoFind( const QString& txt, const QDate &dt,
			   bool caseSensitive, bool searchForward,
			   int category )
{
    bool ok;
    QRegExp r(txt);
    r.setCaseSensitive(caseSensitive);

    Occurrence o = db->find(r, category, dt, searchForward, &ok);

    if ( ok ) {
	dayView->selectDate( o.startInCurrentTZ().date() );
	dayView->setCurrentItem(o);
        emit findDone( 0 );
    } else {
        emit findDone( 1 );
    }
}

void DateBook::showEventDetails()
{
#ifdef QTOPIA_DESKTOP
    if (eventSelected())
	editOccurrence(currentOccurrence(), TRUE);
#else
    initEvent();

    if ( views->visibleWidget() != eventView ) {
	eventView->previousView = views->visibleWidget();

	raiseWidget( eventView );
	eventView->setFocus();

	updateIcons();
	if (actionFind)
	    actionFind->setEnabled( FALSE );
    }

    eventView->init( currentOccurrence() );
#endif
}

void DateBook::hideEventDetails()
{
    raiseWidget( eventView->previousView );
    eventView->previousView->setFocus();

    updateIcons();
    if (actionFind)
	actionFind->setEnabled( TRUE );
}

void DateBook::raiseWidget( QWidget *widget )
{
    if ( !widget )
	return;

#ifndef QTOPIA_DESKTOP
    setName( widget->name() );
#endif
    views->raiseWidget( widget );
}

void DateBook::slotPurge()
{
    QDialog dlg( parentWidget, "purge", TRUE );
#ifdef QTOPIA_PHONE
    dlg.setCaption( tr("Purge") );
#else
    dlg.setCaption( tr("Purge Events") );
#endif

    QVBoxLayout *vb = new QVBoxLayout( &dlg );
    QLabel *lbl = new QLabel( tr("<qt>Please select a date. Everything on and before this date will be removed.</qt>"), &dlg );
    lbl->setAlignment( AlignCenter|AlignVCenter );
    vb->addWidget( lbl );
    QPEDateEdit *dp = new QPEDateEdit( &dlg, 0, FALSE, TRUE );
    vb->addWidget( dp );
#ifdef QTOPIA_DESKTOP
    QWidget *buttons = new QWidget( &dlg );
    vb->addWidget( buttons );
    QHBoxLayout *hb = new QHBoxLayout( buttons );
    hb->addStretch( 1 );
    QPushButton *ok = new QPushButton( tr("Ok"), buttons );
    hb->addWidget( ok );
    connect( ok, SIGNAL(clicked()), &dlg, SLOT(accept()) );
    QPushButton *cancel = new QPushButton( tr("Cancel"), buttons );
    hb->addWidget( cancel );
    connect( cancel, SIGNAL(clicked()), &dlg, SLOT(reject()) );
    dlg.resize( 100, 50 );
#endif

    if ( QPEApplication::execDialog( &dlg ) ) {
	purgeEvents( dp->date() );
    }
}

void DateBook::purgeEvents( const QDate &date, bool prompt )
{
    if (date.isNull())
        return;
#ifndef QTOPIA_DESKTOP
    QDialog *wait = new QDialog( parentWidget, 0, TRUE, WStyle_Customize | WStyle_NoBorder );
    QVBox *vb = new QVBox( wait );
    QLabel *l = new QLabel( tr("<b>Please Wait</b>"), vb );
    l->setAlignment( AlignCenter|AlignVCenter );
    wait->show();
    qApp->processEvents();
#endif

    QDateTime from( date );
    {
	const QList<PrEvent> &pevents = db->eventsIO().events();
	QListIterator<PrEvent> it( pevents );
	for ( PrEvent *e = it.current(); (e = it.current()); ++it ) {
	    QDateTime s = e->startInCurrentTZ();
	    if ( s < from )
		from = s;
	}
    }
    
    QValueList<PimEvent> events = purge_getEvents( from, date );

#ifndef QTOPIA_DESKTOP
    wait->close();
#endif

    // nothing to do
    if ( events.count() == 0 )
	return;

    // give the user a chance to back out
    if ( prompt && QMessageBox::warning( parentWidget, tr("WARNING"),
		tr( "<qt>You are about to delete %1 events. "
		    "Are you sure you want to do this?</qt>" ).arg( events.count() ),
		tr("Delete"), tr("Abort"), 0, 1, 1 ) )
	return;

#ifndef QTOPIA_DESKTOP
    wait->show();
    qApp->processEvents();
#endif

    // delete everything (multiple passes because of interdependant events
    do {
	for ( QValueList<PimEvent>::Iterator it = events.begin(); it != events.end(); ++it ) {
	    db->removeEvent( *it );
	}
	events = purge_getEvents( from, date );
    } while ( events.count() > 0 );

#ifndef QTOPIA_DESKTOP
    delete wait;
#endif

}

QValueList<PimEvent> DateBook::purge_getEvents( const QDateTime &from, const QDate &date )
{
    QValueList<Occurrence> occs = db->getOccurrences( from.date().addDays( -2 ), date );
    QValueList<PimEvent> events;
    for ( QValueList<Occurrence>::Iterator it = occs.begin(); it != occs.end(); ++it ) {
	Occurrence o = *it;
	PimEvent e = o.event();
	if ( e.hasRepeat() && (e.repeatForever() || e.repeatTill() >= date) )
	    continue;
	if ( !events.contains( e ) )
	    events.append( e );
    }
    return events;
}
