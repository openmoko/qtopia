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

#include "task.h"

#include <qtopia/private/recordfields.h>
#include <qtopia/private/vobject_p.h>
#include <qtopia/private/cp1252textcodec_p.h>
#include <qtopia/timeconversion.h>
#include <qtopia/private/qfiledirect_p.h>
#include <qtopia/pim/private/xmlio_p.h>
#include <qtopia/pim/private/todoxmlio_p.h>

#include <qregexp.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qstylesheet.h>
#include <qapplication.h>

#include <stdio.h>

/*!
  \class PimTask
  \module qpepim
  \ingroup qpepim
  \brief The PimTask class holds the data of a todo entry.

  This data includes the priority of the task, a description, an optional due
  date, and whether the task is completed or not.
*/

/*!
  \fn void PimTask::setPriority( PriorityValue priority )

  Sets the priority of the task to \a priority.

  \sa priority()
*/

/*!
  \fn int PimTask::priority() const

  Returns the priority of the task.

  \sa setPriority()
*/

/*!
  \fn QString PimTask::notes() const

  Returns any notes for the task.

  \sa setNotes()
*/

/*!
  \fn void PimTask::setNotes(const QString &s)

  Sets the notes of the task to \a s.

  \sa notes()
*/

/*!
  \fn void PimTask::setDescription( const QString &description )

  Sets the description of the task to \a description.

  \sa description()
 */

/*!
  \fn const QString &PimTask::description() const

  Returns the description of the task.

  \sa setDescription()
 */

/*!
  \fn void PimTask::setDueDate( const QDate &date )

  Sets the due date of the task to \a date.

  \sa clearDueDate(), dueDate()
*/

/*!
  \fn void PimTask::clearDueDate( )

  Clears the due date of the task.

  \sa setDueDate()
*/

/*!
  \fn void PimTask::setCompleted( bool b )

  If \a b is TRUE marks the task as completed.  Otherwise marks the task as
  uncompleted.

  \sa isCompleted()
*/

/*!
  \fn bool PimTask::isCompleted() const

  Returns TRUE if the task is completed.  Otherwise returns FALSE.

  \sa setCompleted()
*/

/*!
  \fn const QDate &PimTask::dueDate() const

  Returns the due date of the task.

  \sa setDueDate(), clearDueDate()
 */

/*!
  \fn bool PimTask::hasDueDate() const

  Returns TRUE if there is a due date set for the task.  Otherwise returns
  FALSE.

  \sa dueDate(), setDueDate(), clearDueDate()
*/

/*!
  \fn const QDate &PimTask::startedDate() const

  Returns the date the task was started.  If the task has not yet been started, the returned
  date is undefined.

  \sa hasStartedDate(), setStartedDate()
*/

/*!
  \fn void PimTask::setStartedDate(const QDate &date)

  Sets the tasks to have started on \a date.

  \sa hasStartedDate(), startedDate()
*/

/*!
  \fn const QDate &PimTask::completedDate() const

  Returns the date the task was completed.  If the task is not completed, the returned
  date is undefined.

  \sa isCompleted(), setCompletedDate()
*/

/*!
  \fn void PimTask::setCompletedDate(const QDate &date)

  Sets the tasks completed date to \a date.

  \sa isCompleted(), completedDate()
*/

/*!
  \enum PimTask::TaskStatus

  These enums describe the current \l status() of the Task.

  The values are:

  \value NotStarted
  \value InProgress
  \value Completed
  \value Waiting
  \value Deferred
*/

/*!
  \enum PimTask::PriorityValue

  These enums describe the current \l priority() of the Task.

  The values are:

  \value VeryHigh
  \value High
  \value Normal
  \value Low
  \value VeryLow
*/

/*!
  \enum PimTask::TaskFields
  \internal
*/

/*!
  \fn QMap<int,QString> PimTask::fields() const
  \internal
*/

/*!
  Creates a new, empty task.
*/
PimTask::PimTask()
    : PimRecord(), mDue( FALSE ), mDueDate( QDate::currentDate() ),
      mCompleted( FALSE ), mPriority( Normal ), mStatus(NotStarted), mPercentCompleted(0)
{
}

/*!
    \internal
*/
void PimTask::fromMap ( const QMap<int,QString> & m)
{
    setFields( m );
}

/*!
  Destroys the task
*/
PimTask::~PimTask()
{
}

/*!
  Returns the \l TaskStatus of the task.

  \sa setStatus()
*/
PimTask::TaskStatus PimTask::status() const
{
    if ( isCompleted() )
	return Completed;
    return mStatus;
}

/*!
  Sets the \l TaskStatus of the task to \a s.

  \sa status()
*/
void PimTask::setStatus(TaskStatus s)
{
    if ( s == Completed )
	setCompleted( TRUE );
    else
	mStatus = s;
}

void PimTask::setDueDate( const QDate &date )
{
    mDueDate = date;
    mDue = !(date.isNull());
}

/*!
  Returns an int indicating the percent completed of the task.
  For completed tasks, this function will always return 100.

  \sa setPercentCompleted(), status()
*/
uint PimTask::percentCompleted() const
{
    if ( isCompleted() )
	return 100;
    return mPercentCompleted;
}

/*!
  Sets the tasks percent completed field to \a percent.
  if \a percent is greater than 99 this function will also
  set the status to Completed.

  \sa percentCompleted(), status()
*/
void PimTask::setPercentCompleted( uint percent )
{
    if ( percent > 99 ) {
	setCompleted( TRUE );
    } else {
	mPercentCompleted = percent;
    }
}

/*!
  Returns TRUE if the task has a started date.

  \sa startedDate(), setStartedDate()
*/

bool PimTask::hasStartedDate() const
{
    return !mStartedDate.isNull() && ( mStatus != NotStarted || hasDueDate() );
}

static QString statusToTrString(PimTask::TaskStatus s)
{
    switch( s ) {
	case PimTask::NotStarted: return qApp->translate("QtopiaPim", "Not yet started"); break;
	case PimTask::InProgress: return qApp->translate("QtopiaPim", "In progress"); break;
	case PimTask::Waiting: return qApp->translate("QtopiaPim", "Waiting"); break;
	case PimTask::Deferred: return qApp->translate("QtopiaPim", "Deferred"); break;
	default: return qApp->translate("QtopiaPim", "Completed"); break;
    }
}

/*!
  Returns TRUE if the part of task matches \a r. Otherwise returns FALSE.
*/
bool PimTask::match ( const QRegExp &r ) const
{
    // match on priority, description on due date...
    bool match = FALSE;
    if ( QString::number( mPriority ).find( r ) > -1 )
	match = TRUE;
    else if ( mDue && mDueDate.toString().find( r ) > -1 )
	match = TRUE;
    else if ( mDesc.find( r ) > -1 )
	match = TRUE;
    else if ( mStatus != NotStarted && mStartedDate.toString().find(r) > -1 )
	match = TRUE;
    else if ( mStatus == Completed && mCompletedDate.toString().find(r) > -1 )
	match = TRUE;
    else if ( mStatus != NotStarted && mStatus != Completed && 
		QString::number(mPercentCompleted).find(r) > - 1 )
	match = TRUE;
    else if ( mNotes.find(r) > -1 )
	match = TRUE;
    else if ( statusToTrString( status() ).find(r) > -1 ) 
	match = TRUE;

    return match;
}

// In pimrecord.cpp
void qpe_startVObjectInput();
bool qpe_vobjectCompatibility(const char* misfeature);
void qpe_endVObjectInput();
void qpe_startVObjectOutput();
void qpe_endVObjectOutput(VObject *,const char* type,const PimRecord*);
void qpe_setVObjectProperty(const QString&, const QString&, const char* type, PimRecord*);
VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value );
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{ return qpe_safeAddPropValue(o,prop,value); }
VObject *qpe_safeAddProp( VObject *o, const char *prop);
static inline VObject *safeAddProp( VObject *o, const char *prop)
{ return qpe_safeAddProp(o,prop); }

static VObject *createVObject( const PimTask &t )
{
    qpe_startVObjectOutput();

    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *task = safeAddProp( vcal, VCTodoProp );

    if ( t.hasDueDate() )
	safeAddPropValue( task, VCDueProp, TimeConversion::toISO8601( t.dueDate(), FALSE ) );
    if ( t.isCompleted() ) {
	// if we say its completed, then we have a completed date.
	safeAddPropValue( task, VCStatusProp, "COMPLETED" );
	safeAddPropValue( task, VCCompletedProp, 
		TimeConversion::toISO8601( t.completedDate(), FALSE ) );
    }
    safeAddPropValue( task, VCPriorityProp, QString::number( t.priority() ) );

    // status *2 (enum && %)
    // ^^ We don't match VCStatusProp and vCal doesn't support ::percent.

    safeAddPropValue( task, "X-Qtopia-STATUS", QString::number( t.status() ));
    safeAddPropValue( task, "X-Qtopia-PERCOMP", 
	    QString::number( t.percentCompleted() ) );

    if (t.hasStartedDate())
    {
	// ok, need to set this one too.
	safeAddPropValue( task, "X-Qtopia-STARTED", 
		TimeConversion::toISO8601( t.startedDate(), FALSE ) );
    }

    // vCal spec: VCSummaryProp is required
    // Palm m100:     No (violates spec)
    // Ericsson T39m: Yes
    if ( qpe_vobjectCompatibility("Palm-Task-DN") ) {
	safeAddPropValue( task, VCSummaryProp, t.description() );
	safeAddPropValue( task, VCDescriptionProp, t.description() );
	safeAddPropValue( task, VCAttachProp, t.notes() );
    } else {
	safeAddPropValue( task, VCSummaryProp, t.description() );
	safeAddPropValue( task, VCDescriptionProp, t.notes() );
    }

    qpe_endVObjectOutput(task,"Todo List",&t); // No tr

    return vcal;
}


static PimTask parseVObject( VObject *obj )
{
    PimTask t;

    VObjectIterator it;
    initPropIterator( &it, obj );
    QString summary, description, attach; // vCal properties, not Qtopias
    
    static CP1252TextCodec *palmCodec = 0;
    if( !palmCodec )
        palmCodec = new CP1252TextCodec();
    
    while( moreIteration( &it ) ) {
	VObject *o = nextVObject( &it );
	QCString name = vObjectName( o );

	// check this key/value for a CHARSET field.
	VObjectIterator tnit;
	initPropIterator( &tnit, o );
	QTextCodec *tc = 0;
	while( moreIteration( &tnit ) ) {
	    VObject *otc = nextVObject( &tnit );
	    if ( qstrcmp(vObjectName(otc), VCCharSetProp ) == 0) {
		tc = QTextCodec::codecForName(vObjectStringZValue(otc));
		break;
	    }
	}
	QString value;
	if (tc)
	    value = tc->toUnicode( vObjectStringZValue( o ) );
	else
	    value = vObjectStringZValue( o );

	if ( name == VCDueProp ) {
	    t.setDueDate( TimeConversion::fromISO8601( QCString(value) ).date() );
	}
	else if ( name == VCSummaryProp ) {
	    summary = value;
	}
	else if ( name == VCDescriptionProp ) {
	    description = value;
	}
	else if (name == VCAttachProp ) { 
	    attach = value;
	}
	else if ( name == VCStatusProp ) {
	    if ( value == "COMPLETED" )
		t.setCompleted( TRUE );
	}
	else if ( name == VCCompletedProp ) {
	    t.setCompletedDate(
		    TimeConversion::fromISO8601( QCString(value) ).date() );
	}
	else if ( name == VCPriorityProp ) {
	    t.setPriority( (PimTask::PriorityValue) value.toInt() );
	}
	else if (name == "X-Qtopia-STATUS" ) { 
	    t.setStatus( (PimTask::TaskStatus) value.toInt() );
	}
	else if (name == "X-Qtopia-PERCOMP" ) { 
	    t.setPercentCompleted( value.toInt() );
	}
	else if (name == "X-Qtopia-STARTED" ) { 
	    t.setStartedDate(
		    TimeConversion::fromISO8601( QCString(value) ).date() );
	} else {
	    qpe_setVObjectProperty(name,value,"Todo List",&t); // No tr
	}
    }

    // Find best mapping from (Summary,Description,Attach) to our (Description,Notes)
    // Similar code in event.cpp
    if ( !!summary && !!description && summary != description ) {
	t.setDescription( summary );
	t.setNotes( description );
	// all 3 - drop attach
    } else if ( !!summary ) {
	t.setDescription( summary );
	t.setNotes( attach );
    } else {
	t.setDescription( description );
	t.setNotes( attach );
    }

    return t;
}

/*!
   Write the list of \a tasks as vCalendar objects to the file
   specified by \a filename.

   \sa readVCalendar()
*/
void PimTask::writeVCalendar( const QString &filename, const QValueList<PimTask> &tasks)
{
  QFileDirect f( filename.utf8().data() );
  if ( !f.open( IO_WriteOnly ) ) {
    qWarning("Unable to open vcard write");
    return;
  }

    QValueList<PimTask>::ConstIterator it;
    for( it = tasks.begin(); it != tasks.end(); ++it ) {
	VObject *obj = createVObject( *it );
	writeVObject(f.directHandle() , obj );
	cleanVObject( obj );
    }

    cleanStrTbl();
}

/*!
   Write the \a task as a vCalendar to the file specified by \a filename.

   \sa readVCalendar()
*/
void PimTask::writeVCalendar( const QString &filename, const PimTask &task)
{
  QFileDirect f( filename.utf8().data() );
  if ( !f.open( IO_WriteOnly ) ) {
    qWarning("Unable to open vcard write");
    return;
  }

  VObject *obj = createVObject( task );
  writeVObject(f.directHandle() , obj );
  cleanVObject( obj );

  cleanStrTbl();
}

/*!
  Reads the file specified by \a filename as a list of vCalendar objects
  and returns the list of near equivalent tasks.

  \sa writeVCalendar()
*/
QValueList<PimTask> PimTask::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (char *)filename.utf8().data() );

    QValueList<PimTask> tasks;

    qpe_startVObjectInput();
    while ( obj ) {
	QCString name = vObjectName( obj );
	if ( name == VCCalProp ) {
	    VObjectIterator nit;
	    initPropIterator( &nit, obj );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		if ( name == VCTodoProp )
		    tasks.append( parseVObject( o ) );
	    }
	} else if ( name == VCTodoProp ) {
	    // shouldn't happen, but just to be sure
	    tasks.append( parseVObject( obj ) );
	}
	VObject *t = obj;
	obj = nextVObjectInList(obj);
	cleanVObject( t );
    }
    qpe_endVObjectInput();

    return tasks;
}

/*!
  Returns a rich text formatted QString of the PimTask.
*/
QString PimTask::toRichText() const
{
    QString text;

    text = "<center><b>" + Qtopia::escapeString(mDesc) + "</b></center><br>"; // No tr
    if ( mDue )
	text += "<b>" + qApp->translate("QtopiaPim", "Due:") + "</b> " + mDueDate.toString() + "<br>";
    if ( !mStartedDate.isNull() && mStatus != NotStarted)
	text += "<b>" + qApp->translate("QtopiaPim", "Started:") + "</b> " + mStartedDate.toString() + "<br>";
    if ( !mCompletedDate.isNull() && isCompleted() )
	text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " + mCompletedDate.toString() + "<br>";

    //if ( !isCompleted() ) { //We remember old status and treat completed separately
    // such remembering is already done by, status();
    
    QString statusString = statusToTrString( status() );
    text += "<b>" + qApp->translate("QtopiaPim", "Status:") + "</b> " + statusString + "<br>";
    text +="<b>" + qApp->translate("QtopiaPim", "Priority:") + "</b> " + QString::number( mPriority ) + "<br>";

    if ( (mStatus != NotStarted && !isCompleted() ) )
	text += "<b>" + qApp->translate("QtopiaPim", "Completed:") + "</b> " + QString::number(mPercentCompleted) + " " +
	    qApp->translate("QtopiaPim", "percent", "Word or symbol after numbers for percentage") + "<br>";

    if ( !mNotes.isEmpty() )
	text += "<br> <b> " + qApp->translate("QtopiaPim", "Notes:") + "</b> "
	    + Qtopia::escapeMultiLineString(mNotes);

    return text;
}

static int dueDateYear = -1;
static int dueDateMonth = -1;
static int dueDateDay = -1;

/*!
  \internal

    // Reimplemenmted to handle post processing of reading a task
*/
void PimTask::setFields(const QMap<int,QString> &m)
{
    dueDateYear = -1;
    dueDateMonth = -1;
    dueDateDay = -1;

    PimRecord::setFields( m );

    if ( dueDateYear != -1 && dueDateMonth != -1 && dueDateDay != -1 ) {
	mDueDate.setYMD(dueDateYear, dueDateMonth, dueDateDay);
    }
}

/*!
  \internal
*/
void PimTask::setField(int key,const QString &s)
{
    switch( key ) {
	case HasDate: if ( s == "1" ) mDue = TRUE; break;
	case Description: setDescription( s ); break;
	case Priority: setPriority( (PriorityValue) (s).toInt() ); break;
	case CompletedField: setCompleted( s == "1" ); break;
	case PercentCompleted: setPercentCompleted( s.toInt() ); break;
	case DueDateYear: dueDateYear = s.toInt(); break;
	case DueDateMonth: dueDateMonth = s.toInt(); break;
	case DueDateDay: dueDateDay = s.toInt(); break;
	case Status: setStatus( xmlToStatus(s) ); break;
	case StartedDate: setStartedDate( PimXmlIO::xmlToDate( s ) ); break;
	case CompletedDate: setCompletedDate( PimXmlIO::xmlToDate(s) ); break;
	case Notes: setNotes( s ); break;
	default: PimRecord::setField(key, s);
    }
}

/*!
  \internal
*/
QString PimTask::field(int key) const
{
    switch( key ) {
	case HasDate: return QString::number( mDue );
	case Description: return mDesc;
	case Priority:  return QString::number( mPriority );
	case PercentCompleted: return QString::number( mPercentCompleted );
	case CompletedField: return QString::number( mCompleted );
	case DueDateYear: return QString::number( mDueDate.year() );
	case DueDateMonth: return QString::number( mDueDate.month() );
	case DueDateDay: return QString::number( mDueDate.day() );
	case Status: return statusToXml( mStatus );
	case StartedDate: return PimXmlIO::dateToXml( mStartedDate );
	case CompletedDate:  return PimXmlIO::dateToXml( mCompletedDate );
	case Notes: return mNotes;
	default: return PimRecord::field(key);
    }
}

static QMap<int, int> *uniquenessMapPtr = 0;
static QMap<QCString, int> *identifierToKeyMapPtr = 0;
static QMap<int, QCString> *keyToIdentifierMapPtr = 0;
static QMap<int, QString> * trFieldsMapPtr = 0;

QMap<int, QString> PimTask::fields() const
{
    QMap<int, QString> m = PimRecord::fields();

    if (!keyToIdentifierMapPtr)
	initMaps();
    QMap<int, QCString>::Iterator it;
    for (it = keyToIdentifierMapPtr->begin(); 
	    it != keyToIdentifierMapPtr->end(); ++it) {
	int i = it.key();
	QString str = field(i);
	if (!str.isEmpty())
	    m.insert(i, str);
    }

    return m;
}

PimTask::TaskStatus PimTask::xmlToStatus(const QString &s)
{
    if ( s == "InProgress" )
	return InProgress;
    else if ( s == "Completed" ) // No tr
	return Completed;
    else if ( s == "Waiting" ) // No tr
	return Waiting;
    else if ( s == "Deferred" ) // No tr
	return Deferred;
    else
	return NotStarted;
}

QString PimTask::statusToXml(TaskStatus status)
{
    switch( status ) {
	default: return "NotStarted";
	case InProgress: return "InProgress";
    	case Completed: return "Completed"; // No tr
	case Waiting: return "Waiting"; // No tr
	case Deferred: return "Deferred"; // No tr
    }
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator>>( QDataStream &s, PimTask &c )
{
    s >> (PimRecord&)c;
    uchar val;
    s >> val;
    c.mDue = val == 0 ? FALSE : TRUE;
    s >> c.mDueDate;
    s >> val;
    c.mCompleted = val == 0 ? FALSE : TRUE;
    int p;
    s >> p;
    c.mPriority = (PimTask::PriorityValue) p;
    s >> c.mDesc;
    s >> c.mStartedDate;
    s >> c.mCompletedDate;

    int i;
    s >> i;
    c.mStatus = (PimTask::TaskStatus)i;

    s >> c.mNotes;
    return s;
}

QDataStream &operator<<( QDataStream &s, const PimTask &c )
{
    s << (const PimRecord&)c;
    s << (c.mDue ? (uchar)1 : (uchar)0);
    s << c.mDueDate;
    s << (c.mCompleted ? (uchar)1 : (uchar)0);
    s << c.mPriority;
    s << c.mDesc;
    s << c.mStartedDate;
    s << c.mCompletedDate;
    int i = (int) c.mStatus;
    s << i;
    s << c.mNotes;

    return s;
}

static const QtopiaPimMapEntry todolistentries[] = {
    { "HasDate", NULL, PimTask::HasDate, 0 },
    { "Description", // No tr
	    QT_TRANSLATE_NOOP("PimTask", "Description"), PimTask::Description, 50 },
    { "Priority", QT_TRANSLATE_NOOP("PimTask", "Priority"), PimTask::Priority, 0 },
    { "Completed", QT_TRANSLATE_NOOP("PimTask", "Completed"), PimTask::CompletedField, 0},
    { "PercentCompleted", QT_TRANSLATE_NOOP("PimTask", "Percent Completed"), PimTask::PercentCompleted, 0},
    { "DateYear", NULL, PimTask::DueDateYear, 0 },
    { "DateMonth", NULL, PimTask::DueDateMonth, 0 },
    { "DateDay", NULL, PimTask::DueDateDay, 0 },
    { "Status", QT_TRANSLATE_NOOP("PimTask", "Status"), PimTask::Status, 0 },
    { "StartedDate", QT_TRANSLATE_NOOP("PimTask", "Started Date"), PimTask::StartedDate, 50 },
    { "CompletedDate", QT_TRANSLATE_NOOP("PimTask", "Completed Date"), PimTask::CompletedDate, 0 },
    { "Notes", QT_TRANSLATE_NOOP("PimTask", "Notes"), PimTask::Notes, 0 },
    { 0, 0, 0, 0 }
};

static const QtopiaPimMapEntry todolistentries_opie[] = {
    { "HasDate", NULL, PimTask::HasDate, 0 },
    { "Summary", // No tr
	    QT_TRANSLATE_NOOP("PimTask", "Description"), PimTask::Description, 50 },
    { "Priority", QT_TRANSLATE_NOOP("PimTask", "Priority"), PimTask::Priority, 0 },
    { "Completed", QT_TRANSLATE_NOOP("PimTask", "Completed"), PimTask::CompletedField, 0},
    { "PercentCompleted", QT_TRANSLATE_NOOP("PimTask", "Percent Completed"), PimTask::PercentCompleted, 0},
    { "DateYear", NULL, PimTask::DueDateYear, 0 },
    { "DateMonth", NULL, PimTask::DueDateMonth, 0 },
    { "DateDay", NULL, PimTask::DueDateDay, 0 },
    { "Status", QT_TRANSLATE_NOOP("PimTask", "Status"), PimTask::Status, 0 },
    { "StartedDate", QT_TRANSLATE_NOOP("PimTask", "Started Date"), PimTask::StartedDate, 50 },
    { "CompletedDate", QT_TRANSLATE_NOOP("PimTask", "Completed Date"), PimTask::CompletedDate, 0 },
    { "Description", QT_TRANSLATE_NOOP("PimTask", "Notes"), PimTask::Notes, 0 },
    { 0, 0, 0, 0 }
};

void PimTask::initMaps()
{
    PimTask::initMaps( FALSE );
}

void PimTask::initMaps( bool isOpieDevice )
{
    delete keyToIdentifierMapPtr;
    keyToIdentifierMapPtr = new QMap<int, QCString>;

    delete identifierToKeyMapPtr;
    identifierToKeyMapPtr = new QMap<QCString, int>;

    delete trFieldsMapPtr;
    trFieldsMapPtr = new QMap<int,QString>;

    delete uniquenessMapPtr;
    uniquenessMapPtr = new QMap<int, int>;

    PimRecord::initMaps("PimTask", (isOpieDevice?todolistentries_opie:todolistentries),
	    *uniquenessMapPtr, *identifierToKeyMapPtr, *keyToIdentifierMapPtr, *trFieldsMapPtr );
}

/*!
  \internal
*/
const QMap<int, QCString> &PimTask::keyToIdentifierMap()
{
    if ( !keyToIdentifierMapPtr )
	initMaps();
    return *keyToIdentifierMapPtr;
}

/*!
  \internal
*/
const QMap<QCString,int> &PimTask::identifierToKeyMap()
{
    if ( !identifierToKeyMapPtr )
	initMaps();
    return *identifierToKeyMapPtr;
}

/*!
  \internal
*/
const QMap<int, QString> & PimTask::trFieldsMap()
{
    if ( !trFieldsMapPtr )
	initMaps();
    return *trFieldsMapPtr;
}

/*!
  \internal
*/
const QMap<int,int> & PimTask::uniquenessMap()
{
    if ( !uniquenessMapPtr )
	initMaps();
    return *uniquenessMapPtr;
}

#endif
