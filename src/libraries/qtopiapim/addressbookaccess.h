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

#ifndef ADDRESSBOOK_ACCESS_H
#define ADDRESSBOOK_ACCESS_H

#include <qobject.h>
#include <qvaluelist.h>
#include "contact.h"

class ContactIO;
class AddressBookAccess;
class ContactIteratorMachine;

class QTOPIAPIM_EXPORT AddressBookIterator 
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

class QTOPIAPIM_EXPORT AddressBookAccess : public QObject {
  Q_OBJECT

    friend class AddressBookIterator;
  public:

  AddressBookAccess();
  
  ~AddressBookAccess();

  bool editSupported() const;

  void updateContact(const PimContact& contact);

  void removeContact(const PimContact& contact);
  
  void addContact(const PimContact& contact);

  PimContact personalDetails() const;

 signals:
  void addressBookUpdated();

 private:
  ContactIO* m_AccessPrivate;
};


#endif

