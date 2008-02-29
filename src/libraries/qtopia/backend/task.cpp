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
#include "recordfields.h"
#include "vobject_p.h"
#include "qfiledirect_p.h"

#include <qtopia/timeconversion.h>

#include <qregexp.h>
#include <qstring.h>

#include <stdio.h>

using namespace Qtopia;
UidGen Task::sUidGen( UidGen::Qtopia );

/*
  \class Task
  \internal
  \brief The Task class holds the data of a todo entry.

  This data includes the priority of the task, a description, an optional due
  date, and whether the task is completed or not.

  \ingroup qtopiaemb
  \ingroup qtopiadesktop
*/

/*
  Creates a new, empty task.
*/
Task::Task() : Record(), mDue( FALSE ),
mDueDate( QDate::currentDate() ),
mCompleted( FALSE ), mPriority( 3 ), mDesc()
{
}

/*
  \fn void Task::setPriority( int priority )

  Sets the priority of the task to \a priority.
*/

/*
  \fn int Task::priority() const

  Returns the priority of the task.
*/

/*
  \fn void Task::setDescription( const QString &description )

  Sets the description of the task to \a description.
 */

/*
  \fn const QString &Task::description() const

  Returns the description of the task.
 */

/*
  \fn void Task::setDueDate( const QDate &date, bool hasDue )

  \internal
  If \a hasDue is TRUE sets the due date of the task to \a date.
  Otherwise clears the due date of the task.
*/

/*
  \fn void Task::setDueDate( const QDate &date )

  Sets the due date of the task to \a date.
*/

/*
  \fn void Task::clearDueDate( )

  Clears the due date of the task.
*/

/*
  \fn void Task::setCompleted( bool b )

  If \a b is TRUE marks the task as completed.  Otherwise marks the task as
  uncompleted.
*/

/*
  \fn bool Task::isCompleted() const

  Returns TRUE if the task is completed.  Otherwise returns FALSE.
*/

/*
  \fn const QDate &Task::dueDate() const

  Returns the due date of the task.
 */

/*
  \fn bool Task::hasDueDate() const

  Returns TRUE if there is a due date set for the task.  Otherwise returns
  FALSE.
*/

/*
  \fn void Task::setHasDueDate( bool b )

  \internal
  Just don't ask.  I really can't justify the function.
*/


/*
  \internal
  Creates a new task.  The properties of the task are set from \a m.
*/

Task::Task( const QMap<int, QString> &m ) : Record(), mDue( FALSE ),
mDueDate( QDate::currentDate() ), mCompleted( FALSE ), mPriority( 3 ), mDesc()
{
    //qDebug("Task::Task fromMap");
    //dump( m );
    for ( QMap<int,QString>::ConstIterator it = m.begin(); it != m.end();++it )
	switch ( (TaskFields) it.key() ) {
	case HasDate: if ( *it == "1" ) mDue = TRUE; break;
	case Completed: setCompleted( *it == "1" ); break;
	case TaskCategory: setCategories( idsFromString( *it ) ); break;
	case TaskDescription: setDescription( *it ); break;
	case Priority: setPriority( (*it).toInt() ); break;
	case Date: mDueDate = TimeConversion::fromString( (*it) ); break;
	case TaskUid: setUid( (*it).toInt() ); break;
	case TaskRid:
	case TaskRinfo:
			break;
	}
}

/*
  Destroys a task.
*/
Task::~Task()
{
}

/*
  \internal
  Returns the task as a map of field ids to property values.
*/
QMap<int, QString> Task::toMap() const
{
    QMap<int, QString> m;
    m.insert( HasDate, hasDueDate() ? "1" : "0" );
    m.insert( Completed, isCompleted() ? "1" : "0" );
    if ( categories().count() )
	m.insert( TaskCategory, idsToString( categories() ) );
    if ( !description().isEmpty() )
	m.insert( TaskDescription, description() );
    m.insert( Priority, QString::number( priority() ) );
    if ( hasDueDate() )
	m.insert( Date, TimeConversion::toString( dueDate() ) );
    m.insert( TaskUid, QString::number(uid()) );

    //qDebug("Task::toMap");
    //dump( m );
    return m;
}

/*
  \internal
  Appends the task information to \a buf.
*/
void Task::save( QString& buf ) const
{
    buf += " Completed=\""; // No tr
    //    qDebug( "writing %d", complete );
    buf += QString::number( (int)mCompleted );
    buf += "\"";
    buf += " HasDate=\"";
    //    qDebug( "writing %d",  );
    buf += QString::number( (int)mDue );
    buf += "\"";
    buf += " Priority=\""; // No tr
    //    qDebug ("writing %d", prior );
    buf += QString::number( mPriority );
    buf += "\"";
    buf += " Categories=\""; // No tr
    buf += Qtopia::Record::idsToString( categories() );
    buf += "\"";
    buf += " Description=\""; // No tr
    //    qDebug( "writing note %s", note.latin1() );
    buf += Qtopia::escapeString( mDesc );
    buf += "\"";
    if ( mDue ) {
	// qDebug("saving ymd %d %d %d", mDueDate.year(), mDueDate.month(),
	// mDueDate.day() );
	buf += " DateYear=\"";
	buf += QString::number( mDueDate.year() );
	buf += "\"";
	buf += " DateMonth=\"";
	buf += QString::number( mDueDate.month() );
	buf += "\"";
	buf += " DateDay=\"";
	buf += QString::number( mDueDate.day() );
	buf += "\"";
    }
    buf += customToXml();
    //    qDebug ("writing uid %d", uid() );
    buf += " Uid=\"";
    buf += QString::number( uid() );
    // terminate it in the application...
    buf += "\"";
}

/*
  Returns TRUE if the task matches the regular expressions \a regexp.
  Otherwise returns FALSE.
*/
bool Task::match ( const QRegExp &regexp ) const
{
    // match on priority, description on due date...
    bool match;
    match = false;
    if ( QString::number( mPriority ).find( regexp ) > -1 )
	match = true;
    else if ( mDue && mDueDate.toString().find( regexp ) > -1 )
	match = true;
    else if ( mDesc.find( regexp ) > -1 )
	match = true;
    return match;
}

#ifdef Q_OS_WIN32
Qtopia::UidGen & Task::uidGen()
{
    return sUidGen;
}
#endif

/*
  \internal
*/
static inline VObject *safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() )
	ret = addPropValue( o, prop, value.latin1() );
    return ret;
}

/*
  \internal
*/
static inline VObject *safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
	ret = addProp( o, prop );
    return ret;
}


/*
  \internal
*/
static VObject *createVObject( const Task &t )
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

/*
  \internal
*/
static Task parseVObject( VObject *obj )
{
    Task t;

    VObjectIterator it;
    initPropIterator( &it, obj );
    while( moreIteration( &it ) ) {
	VObject *o = nextVObject( &it );
	QCString name = vObjectName( o );
	QCString value = vObjectStringZValue( o );
	if ( name == VCDueProp ) {
	    t.setDueDate( TimeConversion::fromISO8601( value ).date(), TRUE );
	}
	else if ( name == VCDescriptionProp ) {
	    t.setDescription( value );
	}
	else if ( name == VCStatusProp ) {
	    if ( value == "COMPLETED" )
		t.setCompleted( TRUE );
	}
	else if ( name == VCPriorityProp ) {
	    t.setPriority( value.toInt() );
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


/*
  Writes the list of \a tasks as a set of VCards to the file \a filename.
*/
void Task::writeVCalendar( const QString &filename, const QValueList<Task> &tasks)
{
  QFileDirect f( filename.utf8().data() );
  if ( !f.open( IO_WriteOnly ) ) {
    qWarning("Unable to open vcard write");
    return;
  }

    QValueList<Task>::ConstIterator it;
    for( it = tasks.begin(); it != tasks.end(); ++it ) {
	VObject *obj = createVObject( *it );
	writeVObject(f.directHandle() , obj );
	cleanVObject( obj );
    }

    cleanStrTbl();
}

/*
  Writes \a task as a VCard to the file \a filename.
*/
void Task::writeVCalendar( const QString &filename, const Task &task)
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

/*
  Returns the set of tasks read as VCards from the file \a filename.
*/
QValueList<Task> Task::readVCalendar( const QString &filename )
{
    VObject *obj = Parse_MIME_FromFileName( (char *)filename.utf8().data() );

    QValueList<Task> tasks;

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
