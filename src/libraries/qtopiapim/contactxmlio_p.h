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
     * Returns the full contact list in a very expensive
     * way. This is needed in a couple of places
     * but should be avoided unless a valuelist is
     * required
     */
    QValueList<PimContact> contactValueList() const;

    /**
     * Returns the full contact list.  This is guaranteed
     * to be current against what is stored by other apps.
     */
    QList<PrContact>& contacts();
    uint count() const { return m_Contacts.count(); }
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
    void addContact(const PimContact &, bool assignNewUid = TRUE);
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

    const char *recordStart() const { return "<Contact "; }
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
