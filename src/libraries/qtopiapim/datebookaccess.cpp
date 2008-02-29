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
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "event.h"
#include <qpe/qcopenvelope_qws.h>
#include "datebookaccess.h"
#include "eventxmlio_p.h"
#include <qtopia/services/services.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*! \class DateBookAccess
  \module qpepim
  \ingroup qpepim
  \brief The DateBookAccess class provides a safe API for accessing PimEvents.

  DateBookAccess provides a safe API for accessing PimEvents stored by
  Qtopia's Datebook application.  DateBookAccess tries to keep the Datebook 
  application in sync with modifications, and alerts the user of 
  the library when modifications ahve been made by other applications.
*/

/*!
 Constructor.
*/
DateBookAccess::DateBookAccess() 
: m_AccessPrivate(0L) {
  m_AccessPrivate = new EventXmlIO(EventIO::ReadOnly);

  connect( m_AccessPrivate, SIGNAL(eventsUpdated()),
	  this, SIGNAL(dateBookUpdated()) );
}

/*!
  Cleans up the the DataBookAccess.
*/
DateBookAccess::~DateBookAccess() {
  delete m_AccessPrivate;
}

/*!
  If supported will update \a event in the pim data.

  Updating events requires the EditEvents service to be available.
*/
void DateBookAccess::updateEvent(const PimEvent& event)
{
    QCopEnvelope e(Service::channel("EditEvents"),
	    "updateEvent(PimEvent)");

    e << event;
}


/*!
  If supported will remove event \a ev from the pim data.

  Removing events requires the EditEvents service to be available.
*/
void DateBookAccess::removeEvent(const PimEvent& ev) 
{
    QCopEnvelope e(Service::channel("EditEvents"),
	    "removeEvent(PimEvent)");

    e << ev;
}

/*!
 If supported will assign a new unique ID to \a ev and add the event to 
 the pim data.

 Adding event requires the EditEvents service to be available.
 */
void DateBookAccess::addEvent(const PimEvent& ev) 
{
    QCopEnvelope e(Service::channel("EditEvents"),
	    "addEvent(PimEvent)");

    e << ev;
}

/*!
  Returns TRUE if it is possible to add, remove and update events in
  the pim data.  Otherwise returns FALSE.

  Adding, removing and updating events requires the EditEvents service to be
  available
*/
bool DateBookAccess::editSupported() const
{
    return Service::list().contains("EditEvents");
}

/*! \fn void DateBookAccess::dateBookUpdated()
  Emitted whenever the datebook is updated.
*/

/*! \class DateBookIterator
  \module qpepim
  \ingroup qpepim
  \brief The DateBookIterator class provides iterators of DateBookAccess.

  The only way to traverse the data of an DateBookAccess is with an 
  DateBookIterator.
*/

/*!
  \internal
*/
DateBookIterator::DateBookIterator(EventIteratorMachine *m) : machine(m) {}

/*!
  Constructs a new DateBookIterator pointing to the start of the data in \a aba.
*/
DateBookIterator::DateBookIterator(const DateBookAccess &aba)
{
    machine = aba.m_AccessPrivate ? aba.m_AccessPrivate->begin() : 0;
}

/*!
  Constructs a copy of the iterator \a it.
*/
DateBookIterator::DateBookIterator(const DateBookIterator &it)
    : machine(it.machine)
{
    if (machine) machine->ref();
}

/*!
  \internal
*/
DateBookIterator::DateBookIterator(const EventIO &cio)
{
    machine = cio.begin();
}

/*!
  Sets the iterator to be a copy of \a other.
*/
DateBookIterator &DateBookIterator::operator=(const DateBookIterator &other)
{
    if (other.machine) other.machine->ref();
    if (machine && machine->deref() ) delete machine;
    machine = other.machine;
    return *this;
}

/*!
  Destroys the iterator
*/
DateBookIterator::~DateBookIterator() 
{
    if ( machine && machine->deref() ) delete machine;
}

/*!
  Returns TRUE if the iterator is at the first item of the data.
  Otherwise returns FALSE.
*/
bool DateBookIterator::atFirst() const 
{
    return machine ? machine->atFirst() : FALSE;
}

/*!
  Returns TRUE if the iterator is at the last item of the data.
  Otherwise returns FALSE.
*/
bool DateBookIterator::atLast() const
{
    return machine ? machine->atLast() : FALSE;
}

/*!
  Sets the iterator to the first item of the data.
  If a PimEvent exists in the data will return a const pointer to the 
  PimEvent.  Otherwise returns 0.
*/
const PimEvent *DateBookIterator::toFirst() 
{
    return machine ? machine->toFirst() : 0;
}

/*!
  Sets the iterator to the last item of the data.
  If a PimEvent exists in the data will return a const pointer to the 
  PimEvent.  Otherwise returns 0.
*/
const PimEvent *DateBookIterator::toLast() 
{
    return machine ? machine->toLast() : 0;
}

/*!
  Sets the iterator to the next item of the data.
  If the iterator is moved pass the end of the list returns 0.
  Otherwise returns a const pointer to the current PimEvent.
*/
const PimEvent *DateBookIterator::operator++()
{
    return machine ? machine->next() : 0;
}

/*!
  If the iterator is at a valid PimEvent returns a const pointer to 
  the current PimEvent.  Otherwise returns 0.
*/
const PimEvent* DateBookIterator::operator*() const
{
    return machine ? machine->current() : 0;
}

/*!
  If the iterator is at a valid PimEvent returns a const pointer to 
  the current PimEvent.  Otherwise returns 0.
*/
const PimEvent* DateBookIterator::current() const
{
    return machine ? machine->current() : 0;
}
