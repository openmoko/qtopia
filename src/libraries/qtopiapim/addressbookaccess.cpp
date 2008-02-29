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

#include "contact.h"
#include "addressbookaccess.h"
#include "contactxmlio_p.h"

#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services.h>
#endif

#include <qmessagebox.h>
#include <qlist.h>

#include <sys/types.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif
#include <stdlib.h>

/*! \class AddressBookAccess
  \module qpepim
  \ingroup qpepim
  \brief The AddressBookAccess class provides a safe API for accessing address book data.

  AddressBookAccess provides a safe API for accessing PimContacts stored by
  Qtopia's contacts application.  AddressBookAccess tries to keep in sync with
  modifications, and alerts the user of the library when modifications have been made
  to the data.
*/

/*! \fn AddressBookAccess::addressBookUpdated()

  This signal is emitted when a contact in the address book is added, removed or modified.
*/

/*!
 Constructs a new AddressBookAccess.
*/
AddressBookAccess::AddressBookAccess()
: m_AccessPrivate(0L) {
  m_AccessPrivate = new ContactXmlIO(ContactIO::ReadOnly);

  connect( m_AccessPrivate, SIGNAL(contactsUpdated()),
	  this, SIGNAL(addressBookUpdated()) );
}

/*!
  Cleans up the the Addressbook access.
*/
AddressBookAccess::~AddressBookAccess()
{
  delete m_AccessPrivate;
}

/*!
  If there exists an contact with the uid \a u, return 
  that contact.  Returns a null contact if otherwise.

  If \a ok is non-null, *ok is set to TRUE if the contact is
  found, and FALSE if the contact is not found.
*/
PimContact AddressBookAccess::contactForId(const QUuid &u, bool *ok) const
{
    return m_AccessPrivate->contactForId(u, ok);
}

/*!
  If supported will update \a contact in pim data.

 Updating contacts requires the Contacts service to be available.
*/
void AddressBookAccess::updateContact(const PimContact& contact)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Contacts"),
	    "updateContact(PimContact)");

    e << contact;
#endif
}


/*!
  If supported will remove \a contact from pim data.

 Removing contacts requires the Contacts service to be available.
 */
void AddressBookAccess::removeContact(const PimContact& contact)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Contacts"),
	    "removeContact(PimContact)");

    e << contact;
#endif
}

/*!
 If supported will assign a new unique ID to \a contact and add the contact
 to the pim data.

 Adding contacts requires the Contacts service to be available.
 */
void AddressBookAccess::addContact(const PimContact& contact)
{
#ifndef QT_NO_COP
    QCopEnvelope e(Service::channel("Contacts"),
	    "addContact(PimContact)");

    e << contact;
#endif
}

/*!
  Returns TRUE if it is possible to add, remove and update contacts in
  the pim data.  Otherwise returns FALSE.

  Adding, removing and updating contacts requires the Contacts service to be
  available
*/
bool AddressBookAccess::editSupported() const
{
#ifndef QT_NO_COP
    return Service::list().contains("Contacts"); // No tr
#else
    return FALSE;
#endif
}

/*!
  Returns the PimContact marked as Personal Details.
*/
PimContact AddressBookAccess::personalDetails() const
{
    return PimContact((const PimContact &)m_AccessPrivate->personal());
}

/*! \class AddressBookIterator
  \module qpepim
  \ingroup qpepim
  \brief The AddressBookIterator class provides iterators of AddressBookAccess.

  The only way to traverse the data of an AddressBookAccess is with an
  AddressBookIterator.
*/

/*!
  \internal
*/
AddressBookIterator::AddressBookIterator(ContactIteratorMachine *m) : machine(m) {}

/*!
  Constructs a new AddressBookIterator pointing to the start of the data in \a aba.
*/
AddressBookIterator::AddressBookIterator(const AddressBookAccess &aba)
{
    machine = aba.m_AccessPrivate ? aba.m_AccessPrivate->begin() : 0;
}

/*!
  Constructs a copy of the iterator \a it.
*/
AddressBookIterator::AddressBookIterator(const AddressBookIterator &it)
    : machine(it.machine)
{
    if (machine) machine->ref();
}

/*!
  \internal
*/
AddressBookIterator::AddressBookIterator(const ContactIO &cio)
{
    machine = cio.begin();
}

/*!
  Sets the iterator to be a copy of \a other.
*/
AddressBookIterator &AddressBookIterator::operator=(const AddressBookIterator &other)
{
    if (other.machine) other.machine->ref();
    if (machine && machine->deref() ) delete machine;
    machine = other.machine;
    return *this;
}

/*!
  Destroys the iterator.
*/
AddressBookIterator::~AddressBookIterator()
{
    if ( machine && machine->deref() ) delete machine;
}

/*!
  Returns TRUE if the iterator is at the first item of the data.
  Otherwise returns FALSE.
*/
bool AddressBookIterator::atFirst() const
{
    return machine ? machine->atFirst() : FALSE;
}

/*!
  Returns TRUE if the iterator is at the last item of the data.
  Otherwise returns FALSE.
*/
bool AddressBookIterator::atLast() const
{
    return machine ? machine->atLast() : FALSE;
}

/*!
  Sets the iterator to the first item of the data.
  If a PimContact exists in the data will return a const pointer to the
  PimContact.  Otherwise returns 0.
*/
const PimContact *AddressBookIterator::toFirst()
{
    return machine ? machine->toFirst() : 0;
}

/*!
  Sets the iterator to the last item of the data.
  If a PimContact exists in the data will return a const pointer to the
  PimContact.  Otherwise returns 0.
*/
const PimContact *AddressBookIterator::toLast()
{
    return machine ? machine->toLast() : 0;
}

/*!
  Sets the iterator to the next item of the data.
  If the iterator is moved pass the end of the list returns 0.
  Otherwise returns a const pointer to the current PimContact.
*/
const PimContact *AddressBookIterator::operator++()
{
    return machine ? machine->next() : 0;
}

/*!
  If the iterator is at a valid PimContact returns a const pointer to
  the current PimContact.  Otherwise returns 0.
*/
const PimContact* AddressBookIterator::operator*() const
{
    return machine ? machine->current() : 0;
}

/*!
  If the iterator is at a valid PimContact returns a const pointer to
  the current PimContact.  Otherwise returns 0.
*/
const PimContact* AddressBookIterator::current() const
{
    return machine ? machine->current() : 0;
}
