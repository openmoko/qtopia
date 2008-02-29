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

#ifndef ADDRESSBOOK_ACCESS_PRIVATE_H
#define ADDRESSBOOK_ACCESS_PRIVATE_H

#include <qvector.h>
#include <qasciidict.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/contact.h>
#include <qtopia/pim/private/contactio_p.h>
#include <qtopia/pim/private/xmlio_p.h>

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN
template class QTOPIAPIM_EXPORT QListIterator<class PrContact>;
//MOC_SKIP_END
#endif

class QTOPIAPIM_EXPORT SortedContacts : public SortedRecords<PimContact>
{
public:
    SortedContacts();
    SortedContacts(uint size);

    ~SortedContacts();

protected:
    int compareItems(QCollection::Item d1, QCollection::Item d2);
};

#ifdef QTOPIA_TEMPLATEDLL
//MOC_SKIP_BEGIN
QTOPIA_TEMPLATEDLL template class QTOPIAPIM_EXPORT QList<PrContact>;
//MOC_SKIP_END
#endif

class QTOPIAPIM_EXPORT ContactXmlIterator : public ContactIteratorMachine
{
public:
    ContactXmlIterator(const QList<PrContact>&list) : it(list) {}

    ~ContactXmlIterator() {}

    ContactXmlIterator &operator=(const ContactXmlIterator &o) {
	it = o.it;
	return *this;
    }

    bool atFirst() const { return it.atFirst(); }
    bool atLast() const { return it.atLast(); }
    const PrContact *toFirst() { return it.toFirst(); }
    const PrContact *toLast() { return it.toLast(); }

    const PrContact *next() { return ++it; }
    const PrContact *prev() { return --it; }
    const PrContact *current() const { return it.current(); }

private:
    QListIterator<PrContact>it;
};

class QTOPIAPIM_EXPORT ContactXmlIO : public ContactIO, public PimXmlIO {

    Q_OBJECT

public:
    ContactXmlIO(AccessMode m, 
		 const QString &file = QString::null,
		 const QString &journal = QString::null );

    ~ContactXmlIO();

    PrContact personal() const;
    bool hasPersonal() const;
    void setAsPersonal(const QUuid &);

    ContactIteratorMachine *begin() const
	{
	    return new ContactXmlIterator(m_Contacts);
	}

    /**
     * Returns the full contact list.  This is guaranteed
     * to be current against what is stored by other apps.
     */
    const QList<PrContact>& contacts();
    uint count() const { return m_Contacts.count(); }

    PrContact contactForId( const QUuid &, bool *ok ) const;
    const SortedContacts &sortedContacts();

    /**
     * Loads the contact data into the internal list
     */
    bool loadData();

    /**
     * Saves the current contact data.  Returns true if
     * successful.
     */
    bool saveData();

    /** Do a direct set w/o assigning new UIDs */
    void setContacts( const QValueList<PimContact> &l );
    void clear();

    // external methods.
    QUuid addContact(const PimContact &, bool assignNewUid = TRUE);
    void updateContact( const PimContact &);
    bool removeContact( const PimContact &);

    bool contains( const PimContact &) const;

    int sortKey() const;
    bool sortAcending() const;
    void setSorting(int key, bool ascending = FALSE);

    int filter() const;
    void setFilter(int);

    /**
     * Makes sure that the contact data is current.  Will reload
     * if necessary.  If the data is current, it will have no effect if
     * forceReload is false.  If forceReload is true, it will always refresh
     * the data.
     */
    void ensureDataCurrent(bool forceReload = false);

protected:

    const char *recordStart() const { return "<Contact "; } // No tr
    const char *listStart() const { return
					"<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook >"
					"<AddressBook>\n<Groups>\n</Groups>\n<Contacts>\n"; }
    const char *listEnd() const { return "</Contacts>\n</AddressBook>\n"; }

    PimRecord *createRecord() const { return new PrContact(); }


    bool internalAddRecord(PimRecord *);
    bool internalRemoveRecord(PimRecord *);
    bool internalUpdateRecord(PimRecord *);

    bool select(const PrContact &) const;

private slots:
    void pimMessage(const QCString &message, const QByteArray &data);

private:
    QList<PrContact> m_Contacts; // for the append and remove functions
    // more than anything else.
    SortedContacts m_Filtered;

    int cFilter;

    bool needsSave;
};

#endif
