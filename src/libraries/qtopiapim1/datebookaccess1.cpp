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

#include <qtopia/pim/event.h>

#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qtopia/pim/datebookaccess.h>
#include "../qtopiapim/eventxmlio_p.h"

#ifndef QT_NO_COP
#include <qtopia/services.h>
#endif
/*!
  Adds an exception to the repeat partern of the \a event.  \a event will be suppressed from
  occuring on the \a date given, instead will occur as described by \a exception.  The date the
  \a exception occurs on need not be the same as the \a date given.

  \sa removeException() removeAllExceptions()
*/
void DateBookAccess::addException(const PimEvent &event, const QDate &date, const PimEvent &exception)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Calendar"),
	    "addException(PimEvent,QDate,PimEvent)");

    e << event;
    e << date;
    e << exception;
#else
    Q_UNUSED( event );
    Q_UNUSED( date );
    Q_UNUSED( exception );
#endif
}

/*!
  Adds an exception to the repeat partern of the \a event.  \a event will be suppressed from
  occuring on the \a date given.

  \sa removeException() removeAllExceptions()
*/
void DateBookAccess::addException(const PimEvent &event, const QDate &date )
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Calendar"),
	    "addException(PimEvent,QDate)");

    e << event;
    e << date;
#else
    Q_UNUSED( event );
    Q_UNUSED( date );
#endif
}

/*!
  Removes the \a exception for the repeat patern of \a event.  Does not compare the
  data for the \a exception passed, rather matches the uid for the \a exception passed
  and determines if it matches an event described as an exception to the repeat pattern
  of the \a event.  The \a exception event will be replaced by a normal occurance for the
  repeat pattern of \a event.

  If no exception can be matched, then this function will have no effect.

  \sa addException() removeAllExceptions()
 */ 
void DateBookAccess::removeException(const PimEvent &event, const PimEvent &exception)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Calendar"),
	    "removeException(PimEvent,PimEvent)");

    e << event;
    e << exception;
#else
    Q_UNUSED( event );
    Q_UNUSED( exception );
#endif
}

/*!
  Removes the exception given by \a date for the repeat pattern of \a event.
  The exception event will be replaced by a normal occurance for the
  repeat pattern of \a event.

  If there is no exception for the repeat pattern of \a event on the \date passed,
  then this function will have no effect.

  \sa addException() removeAllExceptions()
*/
void DateBookAccess::removeException(const PimEvent &event, const QDate &date )
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Calendar"),
	    "removeException(PimEvent,QDate)");

    e << event;
    e << date;
#else
    Q_UNUSED( event );
    Q_UNUSED( date );
#endif
}

/*!
  Removes all exceptions for the repeat pattern of \a event.  The exceptions
  will be replaced by the normal occurance for the repeat pattern of \a event.

  \sa removeException() addException()
*/
void DateBookAccess::removeAllExceptions(const PimEvent &event)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Calendar"),
	    "removeAllExceptions(PimEvent)");

    e << event;
#else
    Q_UNUSED( event );
#endif
}


