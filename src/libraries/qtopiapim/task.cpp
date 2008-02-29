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

#include "task.h"
#include <qregexp.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtopia/private/recordfields.h>
#include <qtopia/private/vobject_p.h>
#include <qtopia/timeconversion.h>
#include <qtopia/private/qfiledirect_p.h>

#include <qtopia/pim/private/xmlio_p.h>
#include <qtopia/pim/private/todoxmlio_p.h>

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

  Sets the tasks started date.

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

  Sets the tasks completed date.

  \sa isCompleted(), completedDate()
*/

/*!
  \fn virtual int PimTask::endFieldMarker() const

  \internal
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
  Sets the \l TaskStatus of the task.

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
  Sets the tasks percent completed field.  Note that for values above 99 this
  function will set the status to \l Completed.

  \sa percentCompleted(), status()
*/
void PimTask::setPercentCompleted( uint u )
{
    if ( u > 99 ) {
	setCompleted( TRUE );
    } else {
	mPercentCompleted = u;
    }
}

/*!
  Returns TRUE if the task has a started date.

  \sa startedDate(), setStartedDate()
*/

bool PimTask::hasStartedDate() const
{
    return mStatus != NotStarted || hasDueDate();
}

/*!
  Returns TRUE if the part of task matches \a r. Otherwise returns FALSE.
*/
bool PimTask::match ( const QRegExp &r ) const
{
    // match on priority, description on due date...
    bool match;
    match = false;
    if ( QString::number( mPriority ).find( r ) > -1 )
	match = true;
    else if ( mDue && mDueDate.toString().find( r ) > -1 )
	match = true;
    else if ( mDesc.find( r ) > -1 )
	match = true;
    return match;
}

static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() )
	ret = addPropValue( o, prop, value.latin1() );
    return ret;
}

static inline VObject *safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
	ret = addProp( o, prop );
    return ret;
}


static VObject *createVObject( const PimTask &t )
{
    VObject *vcal = newVObject( VCCalProp );
    safeAddPropValue( vcal, VCVersionProp, "1.0" );
    VObject *task = safeAddProp( vcal, VCTodoProp );

    if ( t.hasDueDate() )
	safeAddPropValue( task, VCDueProp, TimeConversion::toISO8601( t.dueDate() ) );
    safeAddPropValue( task, VCDescriptionProp, t.description() );
    if ( t.isCompleted() )
	safeAddPropValue( task, VCStatusProp, "COMPLETED" );
    safeAddPropValue( task, VCPriorityProp, QString::number( t.priority() ) );

    return vcal;
}


static PimTask parseVObject( VObject *obj )
{
    PimTask t;

    VObjectIterator it;
    initPropIterator( &it, obj );
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
	else if ( name == VCDescriptionProp ) {
	    t.setDescription( value );
	}
	else if ( name == VCStatusProp ) {
	    if ( value == "COMPLETED" )
		t.setCompleted( TRUE );
	}
	else if ( name == VCPriorityProp ) {
	    t.setPriority( (PimTask::PriorityValue) value.toInt() );
	}
#if 0
	else {
	    printf("Name: %s, value=%s\n", name.data(), vObjectStringZValue( o ) );
	    VObjectIterator nit;
	    initPropIterator( &nit, o );
	    while( moreIteration( &nit ) ) {
		VObject *o = nextVObject( &nit );
		QCString name = vObjectName( o );
		QString value = vObjectStringZValue( o );
		printf(" subprop: %s = %s\n", name.data(), value.latin1() );
	    }
	}
#endif
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

    return tasks;
}

/*!
  Returns a rich text formatted QString of the PimTask.
*/
QString PimTask::toRichText() const
{
    QString text;

    text = "<b><center>" + Qtopia::escapeString(mDesc) + "</b></center><br>";
    if ( mDue )
	text += "<b> Due: </b>" + mDueDate.toString() + "<br>";
    if ( !mStartedDate.isNull() && mStatus != NotStarted)
	text += "<b> Started: </b>" + mStartedDate.toString() + "<br>";
    if ( !mCompletedDate.isNull() && isCompleted() )
	text += "<b> Completed: </b>" + mCompletedDate.toString() + "<br>";

    QString status;
    if ( !isCompleted() ) { //We remember old status and treat completed separately
	switch( mStatus ) {
	    case NotStarted: status = "Not yet started"; break;
	    case InProgress: status = "In progress"; break;
	    case Waiting: status = "Waiting"; break;
	    case Deferred: status = "Deferred"; break;
	    default: status = "Completed"; break;
	}
    } else {
	status = "Completed";
    }

    text += "<b>Status: </b> " + status + "<br>";
    text +="<b>Priority: </b>" + QString::number( mPriority ) + "<br>";

    if ( mPercentCompleted && (mStatus != NotStarted && !isCompleted() ) )
	text += "<b>Completed: </b>" + QString::number(mPercentCompleted) + " percent <br>";

    if ( !mNotes.isEmpty() )
	text += "<br> <b> Notes </b> <br> " + Qtopia::escapeString(mNotes) + "<br>";

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
    else if ( s == "Completed" )
	return Completed;
    else if ( s == "Waiting" )
	return Waiting;
    else if ( s == "Deferred" )
	return Deferred;
    else
	return NotStarted;
}

QString PimTask::statusToXml(TaskStatus status)
{
    switch( status ) {
	default: return "NotStarted";
	case InProgress: return "InProgress";
    	case Completed: return "Completed";
	case Waiting: return "Waiting";
	case Deferred: return "Deferred";
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
    { "Description", QT_TRANSLATE_NOOP("PimTask", "Description"), PimTask::Description, 50 },
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

void PimTask::initMaps()
{
    delete keyToIdentifierMapPtr;
    keyToIdentifierMapPtr = new QMap<int, QCString>;

    delete identifierToKeyMapPtr;
    identifierToKeyMapPtr = new QMap<QCString, int>;

    delete trFieldsMapPtr;
    trFieldsMapPtr = new QMap<int,QString>;

    delete uniquenessMapPtr;
    uniquenessMapPtr = new QMap<int, int>;

    PimRecord::initMaps(todolistentries, *uniquenessMapPtr, *identifierToKeyMapPtr, *keyToIdentifierMapPtr,
			*trFieldsMapPtr );
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
