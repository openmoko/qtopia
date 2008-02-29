/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef ADDRESSBOOK_ACCESS_H
#define ADDRESSBOOK_ACCESS_H

#include <qobject.h>
#include <qvaluelist.h>
#include "contact.h"

class ContactIO;
class AddressBookAccess;
class ContactIteratorMachine;

class AddressBookIterator 
{
    friend class AddressBookAccess;
public:
    AddressBookIterator(ContactIteratorMachine *);
    AddressBookIterator(const ContactIO &);
    AddressBookIterator(const AddressBookAccess &);
    AddressBookIterator(const AddressBookIterator &);
    AddressBookIterator &operator=(const AddressBookIterator &);
    ~AddressBookIterator();

    bool atFirst() const;
    bool atLast() const;
    const PimContact *toFirst();
    const PimContact *toLast();

    const PimContact *operator++();
    const PimContact *operator*() const;

    const PimContact *current() const;

private:
    ContactIteratorMachine *machine;
};

class AddressBookAccess : public QObject {
  Q_OBJECT

    friend class AddressBookIterator;
  public:

  AddressBookAccess();
  
  ~AddressBookAccess();

  bool editSupported() const;

  void updateContact(const PimContact& contact);

  void removeContact(const PimContact& contact);
  
  void addContact(const PimContact& contact);

 signals:
  void addressBookUpdated();

 private:
  ContactIO* m_AccessPrivate;
};


#endif

