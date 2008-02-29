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
#include <fcntl.h>
#include "addressbookaccess_p.h"

static const uint smallestSize = 256;

AddressBookAccessPrivate::AddressBookAccessPrivate() : PIMAccessPrivate() {
  m_Contacts.setAutoDelete(TRUE);
  cFilter = -2;
  ensureDataCurrent();
}

SortedContacts::SortedContacts() 
    : QVector<PimContact>(smallestSize), so(Label), nextindex(0)
{}


SortedContacts::SortedContacts(uint s) 
    : QVector<PimContact>(QMIN(s, smallestSize)), so(Label), nextindex(0)
{}

SortedContacts::~SortedContacts() {}

int SortedContacts::compareItems(Item d1, Item d2) 
{
    PimContact *pc1 = (PimContact *)d1;
    PimContact *pc2 = (PimContact *)d2;

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
}

SortedContacts::SortOrder SortedContacts::sortOrder() const
{
    return so;
}

void SortedContacts::remove(PimContact *i) 
{
    uint pos = findRef(i);
    if ((int)pos != -1) {
	nextindex--;
	if (pos != nextindex) {
	    insert(pos, take(nextindex));
	} else {
	    take(pos);
	}
	if (nextindex > smallestSize && nextindex << 2 < size()) {
	    resize(size() >> 1);
	}
    }
}

void SortedContacts::append(PimContact *i) 
{
    qDebug("size is %d, nextindex %d", size(), nextindex);
    if (nextindex == size()) {
	resize(size() << 1);
    }
    insert(nextindex, i);
    nextindex++;
}

void SortedContacts::reset()
{
    clear();
    nextindex = 0;
    resize(smallestSize);
}

AddressBookAccessPrivate::~AddressBookAccessPrivate() 
{
    // one of the things to do on a quit is to save any data.
    saveData();
}

/**
 * Loads the contact data into the internal list
 */
bool AddressBookAccessPrivate::loadData() {
  // With the exception of the few lines at the bottom of this
  // method, this code was stolen from 1.5 addressbook.

    if (!loadFile(dataFilename()))
	return FALSE;;

    if ( QFile::exists( abJournalFilename() ) ) {
	if (!loadFile( abJournalFilename() ))
	    return FALSE;
	saveData();
    }
    return TRUE;
}

bool AddressBookAccessPrivate::loadFile(const QString &filename) 

{
    QFile f( filename );
    if ( !f.open(IO_ReadOnly) )
	return false;
    if (!lockDataFile(f)) {
      f.close();
      return false;
    }

    QByteArray ba = f.readAll();
    unlockDataFile(f);
    f.close();

    char *uc = ba.data();//(QChar *)data.unicode();
    int len = ba.size();//data.length();
    bool foundAction = false;
    journal_action action;
    bool foundKey = false;
    int journalKey = 0;

    const int JOURNALACTION = Qtopia::Notes + 1;
    const int JOURNALROW = JOURNALACTION + 1;

    // **********************************
    // CHANGE THE SIZE OF THE DICT IF YOU ADD ANY MORE FIELDS!!!!
    // **********************************
    QAsciiDict<int> dict( 47 );
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
    dict.insert( "action", new int(JOURNALACTION) ); // No tr
    dict.insert( "actionrow", new int(JOURNALROW) );

    int i = 0;
    char *point;
    while ( (point = strstr( uc+i, "<Contact " ) ) != NULL ) {
	i = point - uc;
	// if we are reading the standard file, we just need to
	// insert info, so just say we'll do an insert...
	action = ACTION_ADD;
	// new PimContact
	PimContact *cnt = new PimContact;
	i += 9;
	while ( 1 ) {
	    while ( i < len && (uc[i] == ' ' || uc[i] == '\n' || uc[i] == '\r') )
		i++;
	    if ( i >= len-2 || (uc[i] == '/' && uc[i+1] == '>') )
		break;
	    // we have another attribute read it.
	    int j = i;
	    while ( j < len && uc[j] != '=' )
		j++;
	    char *attr = uc+i;
	    uc[j] = '\0';
	    //qDebug("attr=%s", attr.latin1() );
	    i = ++j; // skip =
	    while ( i < len && uc[i] != '"' )
		i++;
	    j = ++i;
	    bool haveEnt = FALSE;
	    bool haveUtf = FALSE;
	    while ( j < len && uc[j] != '"' ) {
		if ( uc[j] == '&' )
		    haveEnt = TRUE;
		if ( ((unsigned char)uc[j]) > 0x7f )
		    haveUtf = TRUE;
		j++;
	    }

	    if ( j == i ) {
		// empty value
		i = j + 1;
		continue;
	    }

	    QString value = haveUtf ? QString::fromUtf8( uc+i, j-i )
			    : QString::fromLatin1( uc+i, j-i );
	    if ( haveEnt )
		value = Qtopia::plainString( value );
	    i = j + 1;

	    int *find = dict[ attr ];
	    if ( !find ) {
		cnt->setCustomField(attr, value);
		continue;
	    }
#if 1
	    switch( *find ) {
	    case Qtopia::AddressUid:
	      // We don't let PimContacts with a bogus UID through...
	      if (value.toInt() == 0) {
		assignNewUid(*cnt);
	      } else {
		setUid( *cnt, uuidFromInt(value.toInt()) );
	      }
	      break;
	    case Qtopia::AddressCategory:
		cnt->setCategories( idsFromString( value ));
		break;
	    case JOURNALACTION:
		action = (journal_action)value.toInt();
		break;
	    case JOURNALROW:
		journalKey = value.toInt();
		break;

	    default:
 		cnt->insert( *find, value );
		break;
	    }
#endif
	}

	// this will always be ACTION_ADD unless in a journal file that specs, otherwise.
	switch ( action ) {
	case ACTION_ADD:
	    m_Contacts.append( cnt );
	    if (select(*cnt))
		m_Filtered.append( cnt );
	    break;
	case ACTION_REMOVE:
	    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
		if (m_Contacts.current()->p_uid() == cnt->p_uid()) {
		    if ( select( *(m_Contacts.current()) ) ) {
			m_Filtered.remove(m_Contacts.current());
		    }
		    m_Contacts.remove();
		    break;
		}
	    }
	    break;
	case ACTION_REPLACE:
	    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
		PimContact *current = m_Contacts.current();
		if (current->p_uid() == cnt->p_uid()) {
		    if ( select(*current) ) {
			m_Filtered.remove(current);
		    }

		    m_Contacts.remove();
		    m_Contacts.append(cnt);

		    if (select(*cnt)) {
			m_Filtered.append(cnt);
		    }
		    break;
		}
	    }
	    break;
	default:
	    break;
	}
	foundAction = false;
	foundKey = false;
    }

    // Record when the file was last modified so we know if
    // someone messes with it while we are looking at it
    QFileInfo fileInfo(dataFilename());
    setLastDataReadTime(fileInfo.lastModified());
    qDebug("** Addresses last read: %s", lastDataReadTime().toString().latin1());
    setEditedExternally(false);
    m_Filtered.sort();
    return true;
}

/**
 * Returns the full contact list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
QList<PimContact>& AddressBookAccessPrivate::contacts() {
  ensureDataCurrent();
  return m_Contacts;    
}

/**
 * Returns the filtered contact list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
const SortedContacts &AddressBookAccessPrivate::sortedContacts() 
{
  ensureDataCurrent();
  return m_Filtered;
}

/**
 * Saves the current contact data.  Returns true if
 * successful.
 */
bool AddressBookAccessPrivate::saveData() {
  // This code was stolen from the 1.5 addressbook implementation.

  // Check to make sure no one is locking the master xml file...
  // We'll close this right before we rename the new one.
  // We open it read/write so we don't plow over anything incase we crash.
  QFile masterFile(dataFilename());
  if (!masterFile.open(IO_ReadWrite))
    return false;
  if (!lockDataFile(masterFile)) {
    masterFile.close();
    return false;
  }
  
  
  QString strNewFile = dataFilename() + ".new";
  QFile f( strNewFile );
  if ( !f.open( IO_WriteOnly|IO_Raw ) ) {
    masterFile.close();
    return false;
  }
  if (!lockDataFile(f)) {
    masterFile.close();
    f.close();
    return false;
  }
  
  int total_written;
  QString out;
  out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook ><AddressBook>\n"
    " <Groups>\n"
    " </Groups>\n"
    " <Contacts>\n";
  QListIterator<PimContact> it(m_Contacts);
  for ( ; it.current(); ++it ) {
    out += "<Contact ";

    PimContact contact = *(it.current());
    out += contactToXml( contact );

    out += "/>\n";
    QCString cstr = out.utf8();
    total_written = f.writeBlock( cstr.data(), cstr.length() );
    if ( total_written != int(cstr.length()) ) {
      masterFile.close();
      f.close();
      QFile::remove( strNewFile );
      return false;
    }
    out = "";
  }
  out += " </Contacts>\n</AddressBook>\n";
  
  QCString cstr = out.utf8();
  total_written = f.writeBlock( cstr.data(), cstr.length() );
  if ( total_written != int(cstr.length()) ) {
    masterFile.close();
    f.close();
    QFile::remove( strNewFile );
    return false;
  }
  unlockDataFile(f);
  f.close();
  
  // Free up the master file briefly..
  unlockDataFile(masterFile);
  masterFile.close();

  //     qDebug("saving: %d", t.elapsed() );
  
  // move the file over, I'm just going to use the system call
  // because, I don't feel like using QDir.
  if ( ::rename( strNewFile.latin1(), dataFilename().latin1() ) < 0 ) {
    qWarning( "problem renaming file %s to %s, errno: %d",
	      strNewFile.latin1(), dataFilename().latin1(), errno );
    // remove the tmp file...
    QFile::remove( strNewFile );
  }
  // remove the journal...
  QFile::remove( abJournalFilename() );

  QFileInfo fileInfo(dataFilename());
  setLastDataReadTime(fileInfo.lastModified());
  
  return true;
}

void AddressBookAccessPrivate::addContact(PimContact &contact)
{
    assignNewUid(contact);
    PimContact *cnt = new PimContact(contact);
    m_Contacts.append(cnt);
    if (select(contact)) {
	m_Filtered.append(cnt);
	m_Filtered.sort();
    }

    updateJournal(contact, ACTION_ADD);

    {
	QCopEnvelope e("QPE/PIM", "addedContact(int,PimContact)"); 
	e << getpid();
	e << contact;
    }
}

QString AddressBookAccessPrivate::contactToXml( const PimContact &contact )
{
    QString out;
    static const QStringList SLFIELDS = contact.fields();
    for ( QMap<int, QString>::ConstIterator fit = contact.mMap.begin();
	    fit != contact.mMap.end(); ++fit ) {
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

    if ( contact.categories().count() > 0 )
	out += "Categories=\"" + idsToString( contact.categories() ) + "\" "; // No tr
    out += "Uid=\"" + QString::number( uuidToInt(uid(contact)) ) + "\" ";

    out += customToXml( contact );

    return out;
}

void AddressBookAccessPrivate::updateJournal(const PimContact &cnt, journal_action action) 
{
    QFile f( abJournalFilename() );
    if ( !f.open(IO_WriteOnly|IO_Append) )
	return;
    QString buf;
    QCString str;
    buf = "<Contact ";
    buf += contactToXml(cnt);
    buf += " action=\"" + QString::number( (int)action ) + "\" ";
    buf += "/>\n";
    QCString cstr = buf.utf8();
    f.writeBlock( cstr.data(), cstr.length() );
    f.close();
}

void AddressBookAccessPrivate::removeContact(const PimContact &contact)
{
    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	if (m_Contacts.current()->p_uid() == contact.p_uid()) {
	    if ( select( *(m_Contacts.current()) ) ) {
		m_Filtered.remove(m_Contacts.current());
		m_Filtered.sort();
	    }
	    m_Contacts.remove();
	    updateJournal(contact, ACTION_REMOVE);
	    {
		QCopEnvelope e("QPE/PIM", "removedContact(int,PimContact)"); 
		e << getpid();
		e << contact;
	    }
	    break;
	}
    }
}

void AddressBookAccessPrivate::updateContact(const PimContact &contact)
{
    // don't use iterator.. need to remove when found and that means keeping
    // a track of the current node.
    // this also means DON'T call other functions that might act on
    // m_Contacts while in this loop!
    for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	PimContact *current = m_Contacts.current();
	if (current->p_uid() == contact.p_uid()) {
	    bool needSort = FALSE;
	    if ( select(*current) ) {
		needSort = TRUE;
		m_Filtered.remove(current);
	    }

	    m_Contacts.remove();

	    PimContact *cptr = new PimContact(contact);
	    m_Contacts.append(cptr);
	    updateJournal(contact, ACTION_REPLACE);
	    {
		QCopEnvelope e("QPE/PIM", "updatedContact(int,PimContact)"); 
		e << getpid();
		e << contact;
	    }

	    if (select(contact)) {
		needSort = TRUE;
		m_Filtered.append(cptr);
	    }

	    if (needSort == TRUE) {
		m_Filtered.sort();
	    }

	    break;
	}
    }
}

void AddressBookAccessPrivate::ensureDataCurrent(bool forceReload) {
  if (isDataCurrent() && !forceReload)
    return;
  qDebug("AddressBookAccessPrivate: Reloading data...");

  m_Contacts.clear();
  m_Filtered.reset();
  loadData();
}

QString AddressBookAccessPrivate::dataFilename() {
  QString filename = Global::applicationFileName("addressbook",
						 "addressbook.xml");
  return filename;
}

QString AddressBookAccessPrivate::abJournalFilename() {
  QString str = getenv("HOME");
  str +="/.abjournal";
  return str;
}

bool AddressBookAccessPrivate::select(const PimContact &c) const 
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

int AddressBookAccessPrivate::filter() const
{
    return cFilter;
}

void AddressBookAccessPrivate::setFilter(int f) {
    if (f != cFilter) {
	cFilter = f;
	m_Filtered.reset();
	for (m_Contacts.first(); m_Contacts.current(); m_Contacts.next()) {
	    PimContact *cnt = m_Contacts.current();
	    if (select(*cnt)) {
		m_Filtered.append(cnt);
	    }
	}
	m_Filtered.sort();
    }
}

SortedContacts::SortOrder AddressBookAccessPrivate::sortOrder() const
{
    return m_Filtered.sortOrder();
}

void AddressBookAccessPrivate::setSortOrder( SortedContacts::SortOrder so )
{
    if (so != m_Filtered.sortOrder() ) {
	m_Filtered.setSortOrder(so);
	m_Filtered.sort();
    }
}
