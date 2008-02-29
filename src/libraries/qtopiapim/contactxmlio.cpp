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

#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include "contact.h"
#include "contactxmlio_p.h"
#include <qtopia/config.h>
#include <qtopia/stringutil.h>

#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qasciidict.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif


void removeBusinessCard();
void updateBusinessCard(const PimContact &);

SortedContacts::SortedContacts()
    : SortedRecords<PimContact>()
{}


SortedContacts::SortedContacts(uint s)
    : SortedRecords<PimContact>(s)
{}

SortedContacts::~SortedContacts() {}

int SortedContacts::compareItems(QCollection::Item d1, QCollection::Item d2)
{
    PrContact *pc1 = (PrContact *)d1;
    PrContact *pc2 = (PrContact *)d2;

    if ( mKey == -1 )
	return 0;

    QString key1, key2;

    if ( mKey >= PimContact::ContactFieldsEnd ) {
	key1 = pc1->bestLabel().lower();
	key2 = pc2->bestLabel().lower();
    } else {
	key1 = pc1->field( mKey ).lower();
	key2 = pc2->field( mKey ).lower();
    }


    if ( mAscending )
	return -( Qtopia::compare(key1, key2) );

    return Qtopia::compare(key1, key2);
}

ContactXmlIO::ContactXmlIO(AccessMode m,
			   const QString &file,
			   const QString &journal )
    : ContactIO(m),
      PimXmlIO(PimContact::keyToIdentifierMap(), PimContact::identifierToKeyMap() ),
      cFilter(-2), needsSave(FALSE)
{
    if ( file != QString::null )
	setDataFilename( file );
    else setDataFilename( Global::applicationFileName( "addressbook", "addressbook.xml" ) );
    if ( journal != QString::null )
	setJournalFilename( journal );
    else setJournalFilename( Global::journalFileName( ".abjournal" ) );

    m_Contacts.setAutoDelete(TRUE);
    loadData();

#ifndef QT_NO_COP
    if (m == ReadOnly) {
	QCopChannel *channel = new QCopChannel( "QPE/PIM",  this );

	connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
		this, SLOT(pimMessage(const QCString&,const QByteArray&)) );

    }
#endif

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
    } else if (message == "reloadContacts()") {
        ensureDataCurrent();
    } else if (message == "reload(int)") {
	int force;
	ds >> force;
        ensureDataCurrent(force);
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
	emit contactsUpdated();
    	return TRUE;
    }
    return FALSE;
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
		//unlink the photo associated with this contact if it exists
		PimContact &curCon = *(m_Contacts.current());
		QString pFileName = curCon.customField( "photofile" );
		if( !pFileName.isEmpty() )
		{
		    QFile pFile( pFileName );
		    if( pFile.exists() )
			pFile.remove();
		    curCon.removeCustomField( "photofile" );
		}

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
	    
	    if ( current != cnt ) {
		*current = *cnt;
		delete cnt;
	    }

	    if (select(*current)) {
		m_Filtered.append(current);
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
const QList<PrContact>& ContactXmlIO::contacts() {
  ensureDataCurrent();
  return m_Contacts;
}

const SortedContacts& ContactXmlIO::sortedContacts() {
    ensureDataCurrent();
    return m_Filtered;
}

PrContact ContactXmlIO::contactForId( const QUuid &u, bool *ok ) const
{
    QListIterator<PrContact> it(m_Contacts);

    PrContact *p;
    for (; it.current(); ++it ) {
	p = *it;
	if (u == p->uid()) {
	    if (ok)
		*ok = TRUE;
	    return *p;
	}
    }

    if (ok)
	*ok = FALSE;
    return PrContact();
}

/**
 * Saves the current contact data.  Returns true if
 * successful.
 */
bool ContactXmlIO::saveData()
{
    if ( !QFile::exists( dataFilename() ) || QFile::exists( journalFilename() ) )
	needsSave = TRUE;

    if (!needsSave)
	return TRUE;
    if (accessMode() != ReadOnly ) {
	if (PimXmlIO::saveData((QList<PimRecord> &)m_Contacts)) {
	    needsSave = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

void ContactXmlIO::clear()
{
    cFilter = -2;
    m_Filtered.clear();
    m_Contacts.clear();
    needsSave = TRUE;
}

void ContactXmlIO::setContacts( const QValueList<PimContact> &l )
{
    clear();
    for ( QValueList<PimContact>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
	PrContact *cnt = new PrContact(*it);
	m_Contacts.append( cnt );
	m_Filtered.append( cnt );
    }
    m_Filtered.sort();
}

QUuid ContactXmlIO::addContact(const PimContact &contact, bool newUid )
{
    QUuid u;
    if (accessMode() == ReadOnly)
	return u;

    PrContact *cnt = new PrContact((const PrContact &)contact);

    if (cnt->customField("BusinessCard") == "TRUE") {
	// set the config.
	updateBusinessCard(*cnt);
    }

    if ( newUid || cnt->uid().isNull() )
	assignNewUid(cnt);

    u = cnt->uid();

    if (internalAddRecord(cnt )) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(*cnt, ACTION_ADD);
#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "addedContact(int,PimContact)");
	    e << getpid();
	    e << *cnt;
	}
#endif
    }
    return u;
}

bool ContactXmlIO::removeContact(const PimContact &contact)
{
    if (accessMode() == ReadOnly)
	return FALSE;

    PrContact *cnt = new PrContact((const PrContact &)contact);

    if (cnt->customField("BusinessCard") == "TRUE") {
	// set the config.
	removeBusinessCard();
    }

    if ( !internalRemoveRecord(cnt) )
	return FALSE;

    needsSave = TRUE;
    m_Filtered.sort();

    updateJournal(contact, ACTION_REMOVE);
#ifndef QT_NO_COP
    {
	QCopEnvelope e("QPE/PIM", "removedContact(int,PimContact)");
	e << getpid();
	e << contact;
    }
#endif
    return TRUE;
}

void ContactXmlIO::updateContact(const PimContact &contact)
{
    if (accessMode() == ReadOnly)
	return;

    PrContact *cnt = new PrContact((const PrContact &)contact);

    if (cnt->customField("BusinessCard") == "TRUE") {
	// set the config.
	updateBusinessCard(*cnt);
    }

    if (internalUpdateRecord(cnt)) {
	needsSave = TRUE;
	m_Filtered.sort();

	updateJournal(contact, ACTION_REPLACE);
#ifndef QT_NO_COP
	{
	    QCopEnvelope e("QPE/PIM", "updatedContact(int,PimContact)");
	    e << getpid();
	    e << contact;
	}
#endif
    }
}

// conservative on the yes vote.
bool ContactXmlIO::contains( const PimContact &c ) const
{
    //### FIXME: this should use the Qtopia Desktop match algorithm;
    // this probably will require some code reorganization
    QString regExp(c.fileAs());

    QListIterator<PrContact> it(m_Contacts);
    for (; it.current(); ++it) {
	if ( !c.uid().isNull() && it.current()->uid() == c.uid() )
	    return TRUE;
	//expensive compare; but rich text is only done if match passes
	else if ( it.current()->match(regExp) &&
		  (c.toRichText() == (*it)->toRichText() )  )
	    return TRUE;
    }
    return FALSE;
}


void ContactXmlIO::ensureDataCurrent(bool forceReload)
{
    if (accessMode() == WriteOnly || ( isDataCurrent() && !forceReload) )
	return;
    qDebug("ContactXmlIO: Reloading data...");

    m_Contacts.clear();
    m_Filtered.clear();
    loadData();
}

bool ContactXmlIO::select(const PrContact &c) const
{
    // -1 is unfiled
    // -2 is all
    QArray<int> cats = c.categories();
    if ( cFilter == -1 ) {
	if ( cats.count() > 0 )
	    return FALSE;
    }  else if ( cFilter != -2 ) {
	if (cats.find(cFilter) == -1)
	    return FALSE;
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

int ContactXmlIO::sortKey() const
{
    return m_Filtered.sortKey();
}

bool ContactXmlIO::sortAcending() const
{
    return m_Filtered.ascending();
}

void ContactXmlIO::setSorting(int key, bool ascending)
{
    if (key != m_Filtered.sortKey() || ascending != m_Filtered.ascending() ) {
	m_Filtered.setSorting( key, ascending );
	m_Filtered.sort();
    }
}

PrContact ContactXmlIO::personal() const
{
    QListIterator<PrContact> it(m_Contacts);
    for (; it.current(); ++it) {
	if (it.current()->customField("BusinessCard") == "TRUE")
	    return PrContact(*(it.current()));
    }
    return PrContact();
}

// ### slow, but safe for now
bool ContactXmlIO::hasPersonal() const
{
    return !personal().uid().isNull();
}

void ContactXmlIO::setAsPersonal(const QUuid &u)
{
    if (u.isNull())
	return;

    PrContact oldBC;
    PrContact newBC;
    bool foundOld = FALSE;
    bool foundNew = FALSE;
    QListIterator<PrContact> it(m_Contacts);
    for (; it.current(); ++it) {
	if (it.current()->customField("BusinessCard") == "TRUE") {
	    oldBC = *(it.current());
	    foundOld = TRUE;
	}
	if (it.current()->uid() == u) {
	    newBC = *(it.current());
	    foundNew = TRUE;
	}
    }
    if (newBC.uid() != oldBC.uid()) {
	if (foundNew) {
	    newBC.setCustomField("BusinessCard", "TRUE");
	    updateContact(newBC);
	}
	if (foundOld) {
	    oldBC.removeCustomField("BusinessCard");
	    updateContact(oldBC);
	}
    }
}

QString businessCardName() {
    return Global::applicationFileName("addressbook",
	    "businesscard.vcf");
}

void updateBusinessCard(const PimContact &cnt)
{
    Config cfg( "Security" );
    cfg.setGroup("Sync");
    cfg.writeEntry("ownername", cnt.fullName());

    PimContact::writeVCard( businessCardName(), cnt);
}

void removeBusinessCard()
{
    Config cfg( "Security" );
    cfg.setGroup("Sync");
    cfg.writeEntry("ownername", "");

    QFile::remove( businessCardName() );
}
