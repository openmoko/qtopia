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

#include <qfile.h>
#include <qasciidict.h>
#include "contact.h"
#include <qpe/config.h>
#include <qpe/global.h>
#include <qfileinfo.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/stringutil.h>
#include <qapplication.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "contactxmlio_p.h"

SortedContacts::SortedContacts() 
    : SortedRecords<PimContact>(), so(Label)
{}


SortedContacts::SortedContacts(uint s) 
    : SortedRecords<PimContact>(s), so(Label)
{}

SortedContacts::~SortedContacts() {}

int SortedContacts::compareItems(QCollection::Item d1, QCollection::Item d2) 
{
    PrContact *pc1 = (PrContact *)d1;
    PrContact *pc2 = (PrContact *)d2;

    QString key1, key2;
    switch (so) {
	case Label:
	    key1 = pc1->bestLabel().lower();
	    key2 = pc2->bestLabel().lower();
	    break;
	case LastName:
	    key1 = pc1->lastName().lower();
	    key2 = pc2->lastName().lower();
	    break;
	case FirstName:
	    key1 = pc1->firstName().lower();
	    key2 = pc2->firstName().lower();
	    break;
	default:
	    return 0;
    }

    return Qtopia::compare(key1, key2);
}

void SortedContacts::setSortOrder(SortOrder s)
{
    so = s;
    setDirty();
}

SortedContacts::SortOrder SortedContacts::sortOrder() const
{
    return so;
}

ContactXmlIO::ContactXmlIO(AccessMode m) : ContactIO(m), cFilter(-2) , dict(48), needsSave(FALSE)
{
    m_Contacts.setAutoDelete(TRUE);

    // **********************************
    // CHANGE THE SIZE OF THE DICT IF YOU ADD ANY MORE FIELDS!!!!
    // **********************************
    dict.setAutoDelete( TRUE );
    dict.insert( "Uid", new int(Qtopia::AddressUid) );
    dict.insert( "Title", new int(Qtopia::Title) ); // No tr
    dict.insert( "FirstName", new int(Qtopia::FirstName) );
    dict.insert( "MiddleName", new int(Qtopia::MiddleName) );
    dict.insert( "LastName", new int(Qtopia::LastName) );
    dict.insert( "Suffix", new int(Qtopia::Suffix) ); // No tr
    dict.insert( "FileAs", new int(Qtopia::FileAs) ); // No tr
    dict.insert( "Categories", new int(Qtopia::AddressCategory) ); // No tr
    dict.insert( "DefaultEmail", new int(Qtopia::DefaultEmail) );
    dict.insert( "Emails", new int(Qtopia::Emails) );
    dict.insert( "HomeStreet", new int(Qtopia::HomeStreet) );
    dict.insert( "HomeCity", new int(Qtopia::HomeCity) );
    dict.insert( "HomeState", new int(Qtopia::HomeState) );
    dict.insert( "HomeZip", new int(Qtopia::HomeZip) );
    dict.insert( "HomeCountry", new int(Qtopia::HomeCountry) );
    dict.insert( "HomePhone", new int(Qtopia::HomePhone) );
    dict.insert( "HomeFax", new int(Qtopia::HomeFax) );
    dict.insert( "HomeMobile", new int(Qtopia::HomeMobile) );
    dict.insert( "HomeWebPage", new int(Qtopia::HomeWebPage) );
    dict.insert( "Company", new int(Qtopia::Company) ); // No tr
    dict.insert( "BusinessStreet", new int(Qtopia::BusinessStreet) );
    dict.insert( "BusinessCity", new int(Qtopia::BusinessCity) );
    dict.insert( "BusinessState", new int(Qtopia::BusinessState) );
    dict.insert( "BusinessZip", new int(Qtopia::BusinessZip) );
    dict.insert( "BusinessCountry", new int(Qtopia::BusinessCountry) );
    dict.insert( "BusinessWebPage", new int(Qtopia::BusinessWebPage) );
    dict.insert( "JobTitle", new int(Qtopia::JobTitle) );
    dict.insert( "Department", new int(Qtopia::Department) ); // No tr
    dict.insert( "Office", new int(Qtopia::Office) ); // No tr
    dict.insert( "BusinessPhone", new int(Qtopia::BusinessPhone) );
    dict.insert( "BusinessFax", new int(Qtopia::BusinessFax) );
    dict.insert( "BusinessMobile", new int(Qtopia::BusinessMobile) );
    dict.insert( "BusinessPager", new int(Qtopia::BusinessPager) );
    dict.insert( "Profession", new int(Qtopia::Profession) ); // No tr
    dict.insert( "Assistant", new int(Qtopia::Assistant) ); // No tr
    dict.insert( "Manager", new int(Qtopia::Manager) ); // No tr
    dict.insert( "Spouse", new int(Qtopia::Spouse) ); // No tr
    dict.insert( "Children", new int(Qtopia::Children) ); // No tr
    dict.insert( "Gender", new int(Qtopia::Gender) ); // No tr
    dict.insert( "Birthday", new int(Qtopia::Birthday) ); // No tr
    dict.insert( "Anniversary", new int(Qtopia::Anniversary) ); // No tr
    dict.insert( "Nickname", new int(Qtopia::Nickname) ); // No tr
    dict.insert( "Notes", new int(Qtopia::Notes) ); // No tr
    dict.insert( "Pronunciation", new int(Qtopia::Pronunciation) ); // No tr

    ensureDataCurrent();

    if (m == ReadOnly) {
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
		this, SLOT(pimMessage(const QCString&, const QByteArray&)) );

    }
}

void ContactXmlIO::pimMessage(const QCString &message, const QByteArray &data)
{
    QDataStream ds(data, IO_ReadOnly);
    if (message == "addedContact(int,PimContact)") {
	int pid;
	PimContact contact;
	ds >> pid;
	ds >> contact;
	if (pid != getpid()) {
	    internalAddRecord(new PimContact(contact));
	    emit contactsUpdated();
	}
    } else if (message == "removedContact(int,PimContact)") {
	int pid;
	PimContact contact;
	ds >> pid;
	ds >> contact;
	if (pid != getpid()) {
	    internalRemoveRecord(new PimContact(contact));
	    emit contactsUpdated();
	}
    } else if (message == "updatedContact(int,PimContact)") {
	int pid;
	PimContact contact;
	ds >> pid;
	ds >> contact;
	if (pid != getpid()) {
	    internalUpdateRecord(new PimContact(contact));
	    emit contactsUpdated();
	}
    }
}

ContactXmlIO::~ContactXmlIO() 
{
    saveData();
}

bool ContactXmlIO::loadData() 
{
    if (PimXmlIO::loadData()) {
	m_Filtered.sort();
    	return TRUE;
    }
    return FALSE;
}

void ContactXmlIO::assignField(PimRecord *rec, const QCString &attr, const QString &value)
{
    PrContact *cnt = (PrContact *)rec;

    int *find = dict[ attr.data() ];

    if ( !find ) {
	cnt->setCustomField(attr, value);
	return;
    }
    switch( *find ) {
	case Qtopia::AddressUid:
	    // We don't let PrContacts with a bogus UID through...
	    if (value.toInt() == 0) {
		assignNewUid(cnt);
	    } else {
		setUid( *cnt, uuidFromInt(value.toInt()) );
	    }
	    break;
	case Qtopia::AddressCategory:
	    cnt->setCategories( idsFromString( value ));
	    break;
	default:
	    cnt->insertField( *find, value );
	    break;
    }
}

bool ContactXmlIO::internalAddRecord(PimRecord *r)
{
    PrContact *cnt = (PrContact *)r;
    m_Contacts.append( cnt );
    if (select(*cnt))
	m_Filtered.append( cnt );

    return TRUE;
}

bool ContactXmlIO::internalRemoveRecord(PimRecord *r)
{
    PrContact *cnt = (PrContact *)r;
    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	if (m_Contacts.current()->uid() == cnt->uid()) {
	    if ( select( *(m_Contacts.current()) ) ) {
		m_Filtered.remove(m_Contacts.current());
	    }
	    m_Contacts.remove();
	    delete cnt;
	    return TRUE;
	}
    }
    delete cnt;
    return FALSE;
}

bool ContactXmlIO::internalUpdateRecord(PimRecord *r)
{
    PrContact *cnt = (PrContact *)r;
    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	PrContact *current = m_Contacts.current();
	if (current->uid() == cnt->uid()) {
	    if ( select(*current) ) {
		m_Filtered.remove(current);
	    }

	    m_Contacts.remove();
	    m_Contacts.append(cnt);

	    if (select(*cnt)) {
		m_Filtered.append(cnt);
	    }
	    return TRUE;
	}
    }
    delete cnt;
    return FALSE;
}

/**
 * Returns the full contact list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
QList<PrContact>& ContactXmlIO::contacts() {
  ensureDataCurrent();
  return m_Contacts;    
}

/**
 * Returns the filtered contact list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
const QVector<PimContact> &ContactXmlIO::sortedContacts() 
{
  ensureDataCurrent();
  return (QVector<PimContact> &)m_Filtered;
}

/**
 * Saves the current contact data.  Returns true if
 * successful.
 */
bool ContactXmlIO::saveData() 
{
    if (!needsSave)
	return TRUE;
    if (accessMode() == ReadWrite) {
	if (PimXmlIO::saveData((QList<PimRecord> &)m_Contacts)) {
	    needsSave = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

QString ContactXmlIO::recordToXml( const PimRecord *rec )
{
    const PrContact *contact = (const PrContact *)rec;

    QString out;
    static const QStringList SLFIELDS = contact->fields();

    const QMap<int, QString> mMap = contact->mapRef();
    for ( QMap<int, QString>::ConstIterator fit = mMap.begin();
	    fit != mMap.end(); ++fit ) {
	const QString &value = fit.data();
	int key = fit.key();
	if ( !value.isEmpty() ) {
	    if ( key == Qtopia::AddressCategory || key == Qtopia::AddressUid)
		continue;

	    key -= Qtopia::AddressCategory+1;
	    out += SLFIELDS[key];
	    out += "=\"" + Qtopia::escapeString(value) + "\" ";
	}
    }

    if ( contact->categories().count() > 0 )
	out += "Categories=\"" + idsToString( contact->categories() ) + "\" "; // No tr
    out += "Uid=\"" + QString::number( uuidToInt(contact->uid()) ) + "\" ";

    out += customToXml( contact );

    return out;
}

void ContactXmlIO::addContact(const PimContact &contact)
{
    if (accessMode() != ReadWrite)
	return;

    PrContact *cnt = new PrContact((const PrContact &)contact);
    assignNewUid(cnt);
    if (internalAddRecord(cnt)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(*cnt, ACTION_ADD);

	{
	    QCopEnvelope e("QPE/PIM", "addedContact(int,PimContact)"); 
	    e << getpid();
	    e << *cnt;
	}
    }
}

void ContactXmlIO::removeContact(const PimContact &contact)
{
    if (accessMode() != ReadWrite)
	return;

    PrContact *cnt = new PrContact((const PrContact &)contact);
    if (internalRemoveRecord(cnt)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(contact, ACTION_REMOVE);
	{
	    QCopEnvelope e("QPE/PIM", "removedContact(int,PimContact)"); 
	    e << getpid();
	    e << contact;
	}
    }
}

void ContactXmlIO::updateContact(const PimContact &contact)
{
    if (accessMode() != ReadWrite)
	return;

    PrContact *cnt = new PrContact((const PrContact &)contact);

    if (internalUpdateRecord(cnt)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(contact, ACTION_REPLACE);
	{
	    QCopEnvelope e("QPE/PIM", "updatedContact(int,PimContact)"); 
	    e << getpid();
	    e << contact;
	}
    }
}

void ContactXmlIO::ensureDataCurrent(bool forceReload) {
  if (isDataCurrent() && !forceReload)
    return;
  qDebug("ContactXmlIO: Reloading data...");

  m_Contacts.clear();
  m_Filtered.clear();
  loadData();
}

const QString ContactXmlIO::dataFilename() const {
  QString filename = Global::applicationFileName("addressbook",
						 "addressbook.xml");
  return filename;
}

const QString ContactXmlIO::journalFilename() const {
  QString str = getenv("HOME");
  str +="/.abjournal";
  return str;
}

bool ContactXmlIO::select(const PrContact &c) const 
{
    QArray<int> cats = c.categories();
    if ( cFilter == -1 ) {
	if ( cats.count() > 0 )
	    return FALSE;
    }  else if ( cFilter != -2 ) {
	if (cats.find(cFilter) == -1)
	    return FALSE;
	/*
	for (int i = 0; i < int(cats.count()); i++) {
	    if (cats[i] == cFilter) {
		return TRUE;
	    }
	}
	return FALSE;
	*/
    }
    return TRUE;
}

int ContactXmlIO::filter() const
{
    return cFilter;
}

void ContactXmlIO::setFilter(int f) {
    if (f != cFilter) {
	cFilter = f;
	m_Filtered.clear();
	for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	    PrContact *cnt = m_Contacts.current();
	    if (select(*cnt)) {
		m_Filtered.append(cnt);
	    }
	}
	m_Filtered.sort();
    }
}

SortedContacts::SortOrder ContactXmlIO::sortOrder() const
{
    return m_Filtered.sortOrder();
}

void ContactXmlIO::setSortOrder( SortedContacts::SortOrder so )
{
    if (so != m_Filtered.sortOrder() ) {
	m_Filtered.setSortOrder(so);
	m_Filtered.sort();
    }
}
