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
  \fn void PimTask::setPriority( int priority )

  Sets the priority of the task to \a priority.
  
  \sa priority()
*/

/*!
  \fn int PimTask::priority() const

  Returns the priority of the task.
  
  \sa setPriority()
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
  Creates a new, empty task.
*/
PimTask::PimTask() : PimRecord(), mDue( FALSE ),
mDueDate( QDate::currentDate() ),
mCompleted( FALSE ), mPriority( 3 ), mDesc()
{
}

/*!
  Destroys the task
*/
PimTask::~PimTask()
{
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
    s >> c.mPriority;
    s >> c.mDesc;
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
    return s;
}
#endif
