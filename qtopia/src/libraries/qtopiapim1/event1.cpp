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
#include <qtopia/timeconversion.h>
#include <qtopia/stringutil.h>
#include <qvaluelist.h>

/*! 
  Returns the list of exceptions to the repeat rule for this event.

  The exceptions are returned as a list of EventExceptions, with the form,

  <pre>
  struct EventException {
      QDate date;
      QUuid eventId;
  };
  </pre>

  The date field is the date for which a repeating event would otherwise occur on.
  For instance, if an event would normally occur on July 1st, every year, and date is
  July 1st, 2000, then the event exception would indicate the repeating event would
  not occur on July 1st, 2000.  The date for each EventException is unique for the
  list of exceptions returned.

  If eventId is not null, then eventId describes an alturnate event that would occur
  instead of the normal occurrance for the date specified by the date field.  The event
  it identifies does not have to occur on the same date as the date field.  To extend the
  above example, the event for eventId might occur on July 2nd, 2000.  If the EventId is null
  then it describes an exception where no alternate event occurred.

 */
QValueList<EventException> PimEvent::eventExceptions() const
{
    QValueList<EventException> result;
    QValueList<QDate>::ConstIterator eit = mExceptions.begin();
    QValueList<QUuid>::ConstIterator cit = mChildren.begin();

    for(; eit != mExceptions.end() && cit != mChildren.end(); ++eit, ++cit) {
	EventException r;
	r.date = *eit;
	r.eventId = *cit;
	result += r;
    }

    return result;
}
