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

#ifndef ADDRESSBOOK_ACCESS_PRIVATE_H
#define ADDRESSBOOK_ACCESS_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/contact.h>
#include "pimaccess_p.h"

// need the SortOrder enum.
#include <qtopia/pim/addressbookaccess.h>

class SortedContacts : public QVector<PimContact>
{
public:
    SortedContacts();
    SortedContacts(uint size);

    ~SortedContacts();

    int compareItems(Item d1, Item d2);

    enum SortOrder {
	Label,
	LastName,
	FirstName
    };

    void setSortOrder(SortOrder);
    SortOrder sortOrder() const;

    // Warning.  Fast if not sorting, slow if wanting to
    // maintains sort.  Don't try to keep sorted if 
    // doing a lot of modications.
    void append(PimContact *);
    void remove(PimContact *);

    uint sCount() const { return nextindex; }

    void reset();
private:
    SortOrder so;
    uint nextindex;
};

class AddressBookAccessPrivate : public PIMAccessPrivate {
 public:
  AddressBookAccessPrivate();
  virtual ~AddressBookAccessPrivate();

  /**
   * Returns the full contact list.  This is guaranteed
   * to be current against what is stored by other apps.
   */
  QList<PimContact>& contacts();
  const SortedContacts &sortedContacts();

  /**
   * Loads the contact data into the internal list
   */
  virtual bool loadData();
  virtual bool loadFile(const QString &);

  /**
   * Saves the current contact data.  Returns true if
   * successful.
   */
  bool saveData();
  QString contactToXml(const PimContact &);
  void updateJournal(const PimContact &, journal_action);
 
  void addContact(PimContact &);
  void updateContact( const PimContact &);
  void removeContact( const PimContact &);  


  SortedContacts::SortOrder sortOrder() const;
  void setSortOrder(SortedContacts::SortOrder );

  int filter() const;
  void setFilter(int);

  /**
   * Makes sure that the contact data is current.  Will reload
   * if necessary.  If the data is current, it will have no effect if
   * forceReload is false.  If forceReload is true, it will always refresh
   * the data.
   */
  virtual void ensureDataCurrent(bool forceReload = false);

protected:
  /**
   * Returns the file name for the address book
   */
  virtual QString dataFilename();


  /**
   * Returns the file name for the address book's journal file
   */
  QString abJournalFilename();

  bool select(const PimContact &) const;

 private:  
  QList<PimContact> m_Contacts; // for the append and remove functions
  			     // more than anything else.
  SortedContacts m_Filtered;

  int cFilter;
};

#endif
