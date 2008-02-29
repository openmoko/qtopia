/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qtopiaapplication.h>
#include <qtopia/pim/qcontact.h>
#include "qcontactxmlio_p.h"
#include <qsettings.h>
#include <qtopianamespace.h>

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>

#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

class QContactCompare
{
public:
    QContactCompare() : mAscending(false) {}

    bool operator()(const QContact *, const QContact *) const;

    // TODO make set of columns based of sqlLabel
    QContactModel::Field mKey;
    bool mAscending;
};


bool QContactCompare::operator()(const QContact *pc1, const QContact *pc2) const
{
    if ( mKey == QContactModel::Invalid )
        return false; // all equal if can't sort.

    QString key1, key2;

    key1 = QContactModel::contactField(*pc1, mKey).toString();
    key2 = QContactModel::contactField(*pc2, mKey).toString();

    return QString::localeAwareCompare(key1, key2) < 0;
}

ContactXmlIO::ContactXmlIO(QObject *parent, const QString &file,
                           const QString &journal )
    : QContactIO(parent), QPimXmlIO(), cFilter(QCategoryFilter::All), cKey(QContactModel::Label), needsSave(false)
{
    if ( file != QString() )
        setDataFilename( file );
    else setDataFilename( Qtopia::applicationFileName( "addressbook", "addressbook.xml" ) );
    if ( journal != QString() )
        setJournalFilename( journal );
    else setJournalFilename( Qtopia::applicationFileName( "addressbook", "addressbook.journal" ) );

    connect(this, SIGNAL(labelFormatChanged()), this, SLOT(sort()));

    setScope("Contacts");

    loadData();
}

QUuid ContactXmlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("33a13e6c-cfc4-48c7-a2c3-c7eedacc2c44");
    return u;
}

void ContactXmlIO::checkAdded(const QContact &contact)
{
    internalAddRecord(new QContact(contact));
    emit recordsUpdated();
}

void ContactXmlIO::checkRemoved(const QUniqueId & id)
{
    QContact *cnt = new QContact;
    cnt->setUid(id);
    internalRemoveRecord(cnt);
    emit recordsUpdated();
}

void ContactXmlIO::checkUpdated(const QContact &contact)
{
    internalUpdateRecord(new QContact(contact));
    emit recordsUpdated();
}

void ContactXmlIO::checkRemoved(const QList<QUniqueId> &)
{
    ensureDataCurrent();
}

ContactXmlIO::~ContactXmlIO()
{
    saveData();
}

bool ContactXmlIO::loadData()
{
    if (QPimXmlIO::loadData()) {
        sort();
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool ContactXmlIO::internalAddRecord(QPimRecord *r)
{
    QContact *cnt = (QContact *)r;
    m_Contacts.append( cnt );
    if (select(*cnt))
        m_Filtered.append( cnt );

    return true;
}

bool ContactXmlIO::internalRemoveRecord(QPimRecord *r)
{
    QContact *cnt = (QContact *)r;
    QMutableListIterator<QContact*> it(m_Contacts);
    while(it.hasNext()) {
        QContact *current = it.next();
        if (current->uid() == cnt->uid()) {
            if ( select( *current ) ) {
                //unlink the photo associated with this contact if it exists
                QString pFileName = current->customField( "photofile" );
                if( !pFileName.isEmpty() )
                {
                    QFile pFile( pFileName );
                    if( pFile.exists() )
                        pFile.remove();
                    current->removeCustomField( "photofile" );
                }

                m_Filtered.removeAll(current);
            }
            it.remove();
            delete cnt;
            return true;
        }
    }
    delete cnt;
    return false;
}

bool ContactXmlIO::internalUpdateRecord(QPimRecord *r)
{
    QContact *cnt = (QContact *)r;
    QMutableListIterator<QContact*> it(m_Contacts);
    while(it.hasNext()) {
        QContact *current = it.next();
        if (current->uid() == cnt->uid()) {
            if ( select(*current) ) {
                m_Filtered.removeAll(current);
            }

            if ( current != cnt ) {
                *current = *cnt;
            }

            if (select(*current)) {
                m_Filtered.append(current);
            }
            delete cnt;
            return true;
        }
    }
    delete cnt;
    return false;
}

/**
 * Returns the full contact list.  This is guaranteed
 * to be current against what is stored by other apps.
 */
QList<QContact*> ContactXmlIO::contacts() const {
  //ensureDataCurrent();
  return m_Contacts;
}

QList<QContact*> ContactXmlIO::sortedContacts() const {
    //ensureDataCurrent();
    return m_Filtered;
}

int ContactXmlIO::row(const QUniqueId & id) const
{
    for (int index = 0 ; index < m_Filtered.count(); ++index) {
        if (m_Filtered.at(index)->uid() == id) {
            return index;
        }
    }
    return -1;
}

void ContactXmlIO::sort()
{
    QContactCompare c;
    c.mKey = cKey;
    qSort(m_Filtered.begin(), m_Filtered.end(), c);
}

QContact ContactXmlIO::contact( const QUniqueId & u ) const
{
    QListIterator<QContact*> it(m_Contacts);

    foreach(QContact *p, m_Contacts) {
        if (u == p->uid())
            return *p;
    }

    return QContact();
}

QContact ContactXmlIO::contact(int row) const
{
    if (row < 0 || row >= count())
        return QContact();
    return *m_Filtered.at(row);
}

QUniqueId ContactXmlIO::id(int row) const
{
    return m_Filtered.at(row)->uid();
}

QVariant ContactXmlIO::key(int row) const
{
    if (cKey != QContactModel::Invalid)
        return QContactModel::contactField(*(m_Filtered.at(row)), cKey);
    return QVariant();
}

/**
 * Saves the current contact data.  Returns true if
 * successful.
 */
bool ContactXmlIO::saveData()
{
    if ( !QFile::exists( dataFilename() ) || QFile::exists( journalFilename() ) )
        needsSave = true;

    if (!needsSave)
        return true;
    if (QPimXmlIO::saveData((QList<QPimRecord*> &)m_Contacts)) {
        needsSave = false;
        return true;
    }
    return false;
}

void ContactXmlIO::clear()
{
    cFilter = QCategoryFilter(QCategoryFilter::All);
    m_Filtered.clear();
    m_Contacts.clear();
    needsSave = true;
}

void ContactXmlIO::setContacts( const QList<QContact> &l )
{
    clear();
    for ( QList<QContact>::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        QContact *cnt = new QContact(*it);
        m_Contacts.append( cnt );
        m_Filtered.append( cnt );
    }
    sort();
}

QUniqueId ContactXmlIO::addContact(const QContact &contact)
{
    QUniqueId u;

    QContact *cnt = new QContact((const QContact &)contact);

    assignNewUid(cnt);

    u = cnt->uid();

    if (internalAddRecord(cnt )) {
        needsSave = true;
        sort();

        updateJournal(*cnt, ACTION_ADD);
        notifyAdded(*cnt);
    }
    return u;
}

bool ContactXmlIO::removeContact(const QContact &contact)
{
    QContact *cnt = new QContact((const QContact &)contact);

    if ( !internalRemoveRecord(cnt) )
        return false;

    needsSave = true;
    sort();

    updateJournal(contact, ACTION_REMOVE);
    notifyRemoved(contact.uid());
    return true;
}

bool ContactXmlIO::updateContact(const QContact &contact)
{
    QContact *cnt = new QContact((const QContact &)contact);

    if (internalUpdateRecord(cnt)) {
        needsSave = true;
        sort();

        updateJournal(contact, ACTION_REPLACE);
        notifyUpdated(contact);
        return true;
    }
    return false;
}

// conservative on the yes vote.
bool ContactXmlIO::contains( const QContact &c ) const
{
    //### FIXME: this should use the Qtopia Desktop match algorithm;
    // this probably will require some code reorganization
    QString regExp(c.label());

    foreach( QContact *current, m_Contacts) {
        if ( !c.uid().isNull() && current->uid() == c.uid() )
            return true;
        //expensive compare; but rich text is only done if match passes
        else if ( current->match(regExp) &&
                (c.toRichText() == current->toRichText() )  )
            return true;
    }
    return false;
}


void ContactXmlIO::ensureDataCurrent(bool forceReload)
{
    if (isDataCurrent() && !forceReload)
        return;

    m_Contacts.clear();
    m_Filtered.clear();
    loadData();
}

QMap<QString, ContactXmlIO::KeyType> ContactXmlIO::kLookup;

void ContactXmlIO::initMap()
{
    if (kLookup.count() != 0)
        return;

    struct KeyText {
        ContactXmlIO::KeyType k;
        const char *text;
    };
    static const KeyText staticLookup[] = {
        { ContactXmlIO::Title, "Title" },

        { ContactXmlIO::FirstName, "FirstName" },
        { ContactXmlIO::MiddleName, "MiddleName" },
        { ContactXmlIO::LastName, "LastName" },
        { ContactXmlIO::Suffix, "Suffix" },

        // email
        { ContactXmlIO::DefaultEmail, "DefaultEmail" },
        { ContactXmlIO::Emails, "Emails" },

        // home
        { ContactXmlIO::HomeStreet, "HomeStreet" },
        { ContactXmlIO::HomeCity, "HomeCity" },
        { ContactXmlIO::HomeState, "HomeState" },
        { ContactXmlIO::HomeZip, "HomeZip" },
        { ContactXmlIO::HomeCountry, "HomeCountry" },
        { ContactXmlIO::HomePhone, "HomePhone" },
        { ContactXmlIO::HomeFax, "HomeFax" },
        { ContactXmlIO::HomeMobile, "HomeMobile" },
        { ContactXmlIO::HomeWebPage, "HomeWebPage" },

        // business
        { ContactXmlIO::Company, "Company" },
        { ContactXmlIO::BusinessStreet, "BusinessStreet" },
        { ContactXmlIO::BusinessCity, "BusinessCity" },
        { ContactXmlIO::BusinessState, "BusinessState" },
        { ContactXmlIO::BusinessZip, "BusinessZip" },
        { ContactXmlIO::BusinessCountry, "BusinessCountry" },
        { ContactXmlIO::BusinessWebPage, "BusinessWebPage" },
        { ContactXmlIO::JobTitle, "JobTitle" },
        { ContactXmlIO::Department, "Department" },
        { ContactXmlIO::Office, "Office" },
        { ContactXmlIO::BusinessPhone, "BusinessPhone" },
        { ContactXmlIO::BusinessFax, "BusinessFax" },
        { ContactXmlIO::BusinessMobile, "BusinessMobile" },
        { ContactXmlIO::BusinessPager, "BusinessPager" },
        { ContactXmlIO::Profession, "Profession" },
        { ContactXmlIO::Assistant, "Assistant" },
        { ContactXmlIO::Manager, "Manager" },

        //personal
        { ContactXmlIO::Spouse, "Spouse" },
        { ContactXmlIO::Gender, "Gender" },
        { ContactXmlIO::Birthday, "Birthday" },
        { ContactXmlIO::Anniversary, "Anniversary" },
        { ContactXmlIO::Nickname, "Nickname" },
        { ContactXmlIO::Children, "Children" },

        // other
        { ContactXmlIO::Notes, "Notes" },

        // Added in Qtopia 1.6
        { ContactXmlIO::LastNamePron, "LastNamePronunciation" },
        { ContactXmlIO::FirstNamePron, "FirstNamePronunciation" },
        { ContactXmlIO::CompanyPron, "CompanyPronunciation" },
        { ContactXmlIO::CompanyPron, 0},
    };
    const KeyText *item = staticLookup;
    while(item->text) {
        kLookup.insert(item->text, item->k);
        ++item;
    }
};

void ContactXmlIO::setFields(QPimRecord *rec, const QMap<QString, QString> &values) const
{
    initMap();
    QMap<QString, QString> tmpvalues(values);
    QMutableMapIterator<QString, QString> it(tmpvalues);
    while(it.hasNext()) {
        it.next();
        QString key = it.key();
        if (kLookup.contains(key)) {
            setField((QContact *)rec, kLookup[key], it.value());
            it.remove();
        }
    }
    QPimXmlIO::setFields(rec, tmpvalues);
}

void ContactXmlIO::setField(QContact *contact, KeyType t, const QString &value) const
{
    switch(t) {
        case ContactXmlIO::Title:
            contact->setNameTitle(value);
            break;
        case ContactXmlIO::FirstName:
            contact->setFirstName(value);
            break;
        case ContactXmlIO::MiddleName:
            contact->setMiddleName(value);
            break;
        case ContactXmlIO::LastName:
            contact->setLastName(value);
            break;
        case ContactXmlIO::Suffix:
            contact->setSuffix(value);
            break;

            // email
        case ContactXmlIO::DefaultEmail:
            contact->setDefaultEmail(value);
            break;
        case ContactXmlIO::Emails:
            contact->setEmailList(value.split(" "));
            break;

            // home
        case ContactXmlIO::HomeStreet:
            contact->setHomeStreet(value);
            break;
        case ContactXmlIO::HomeCity:
            contact->setHomeCity(value);
            break;
        case ContactXmlIO::HomeState:
            contact->setHomeState(value);
            break;
        case ContactXmlIO::HomeZip:
            contact->setHomeZip(value);
            break;
        case ContactXmlIO::HomeCountry:
            contact->setHomeCountry(value);
            break;
        case ContactXmlIO::HomePhone:
            contact->setHomePhone(value);
            break;
        case ContactXmlIO::HomeFax:
            contact->setHomeFax(value);
            break;
        case ContactXmlIO::HomeMobile:
            contact->setHomeMobile(value);
            break;
        case ContactXmlIO::HomeWebPage:
            contact->setHomeWebpage(value);
            break;

            // business
        case ContactXmlIO::Company:
            contact->setCompany(value);
            break;
        case ContactXmlIO::BusinessStreet:
            contact->setBusinessStreet(value);
            break;
        case ContactXmlIO::BusinessCity:
            contact->setBusinessCity(value);
            break;
        case ContactXmlIO::BusinessState:
            contact->setBusinessState(value);
            break;
        case ContactXmlIO::BusinessZip:
            contact->setBusinessZip(value);
            break;
        case ContactXmlIO::BusinessCountry:
            contact->setBusinessCountry(value);
            break;
        case ContactXmlIO::BusinessWebPage:
            contact->setBusinessWebpage(value);
            break;
        case ContactXmlIO::JobTitle:
            contact->setJobTitle(value);
            break;
        case ContactXmlIO::Department:
            contact->setDepartment(value);
            break;
        case ContactXmlIO::Office:
            contact->setOffice(value);
            break;
        case ContactXmlIO::BusinessPhone:
            contact->setBusinessPhone(value);
            break;
        case ContactXmlIO::BusinessFax:
            contact->setBusinessFax(value);
            break;
        case ContactXmlIO::BusinessMobile:
            contact->setBusinessMobile(value);
            break;
        case ContactXmlIO::BusinessPager:
            contact->setBusinessPager(value);
            break;
        case ContactXmlIO::Profession:
            contact->setProfession(value);
            break;
        case ContactXmlIO::Assistant:
            contact->setAssistant(value);
            break;
        case ContactXmlIO::Manager:
            contact->setManager(value);
            break;

            //personal
        case ContactXmlIO::Spouse:
            contact->setSpouse(value);
            break;
        case ContactXmlIO::Gender:
            switch(value.toInt()) {
                default:
                case 0:
                    contact->setGender(QContact::UnspecifiedGender);
                    break;
                case 1:
                    contact->setGender(QContact::Male);
                    break;
                case 2:
                    contact->setGender(QContact::Female);
                    break;
            }
            break;
        case ContactXmlIO::Birthday:
            contact->setBirthday(QPimXmlIO::xmlToDate(value));
            break;
        case ContactXmlIO::Anniversary:
            contact->setAnniversary(QPimXmlIO::xmlToDate(value));
            break;
        case ContactXmlIO::Nickname:
            contact->setNickname(value);
            break;
        case ContactXmlIO::Children:
            contact->setChildren(value);
            break;

            // other
        case ContactXmlIO::Notes:
            contact->setNotes(value);
            break;

            // Added in Qtopia 1.6
        case ContactXmlIO::LastNamePron:
            contact->setLastNamePronunciation(value);
            break;
        case ContactXmlIO::FirstNamePron:
            contact->setFirstNamePronunciation(value);
            break;
        case ContactXmlIO::CompanyPron:
            contact->setCompanyPronunciation(value);
            break;
    }
}

QMap<QString, QString> ContactXmlIO::fields(const QPimRecord *rec) const
{
    initMap();
    QMap<QString, QString> map = QPimXmlIO::fields(rec);
    QMapIterator<QString, KeyType> it(kLookup);
    while(it.hasNext()) {
        it.next();
        QString v = field((QContact *)rec, it.value());
        if (!v.isEmpty()) {
            map.insert(it.key(), v);
        }
    }
    return map;
}

QString ContactXmlIO::field(const QContact *contact, KeyType t) const
{
    switch(t) {
        case ContactXmlIO::Title:
            return contact->nameTitle();
        case ContactXmlIO::FirstName:
            return contact->firstName();
        case ContactXmlIO::MiddleName:
            return contact->middleName();
        case ContactXmlIO::LastName:
            return contact->lastName();
        case ContactXmlIO::Suffix:
            return contact->suffix();

            // email
        case ContactXmlIO::DefaultEmail:
            return contact->defaultEmail();
        case ContactXmlIO::Emails:
            if (contact->emailList().count() == 0)
                return QString();
            return contact->emailList().join(" ");

            // home
        case ContactXmlIO::HomeStreet:
            return contact->homeStreet();
        case ContactXmlIO::HomeCity:
            return contact->homeCity();
        case ContactXmlIO::HomeState:
            return contact->homeState();
        case ContactXmlIO::HomeZip:
            return contact->homeZip();
        case ContactXmlIO::HomeCountry:
            return contact->homeCountry();
        case ContactXmlIO::HomePhone:
            return contact->homePhone();
        case ContactXmlIO::HomeFax:
            return contact->homeFax();
        case ContactXmlIO::HomeMobile:
            return contact->homeMobile();
        case ContactXmlIO::HomeWebPage:
            return contact->homeWebpage();

            // business
        case ContactXmlIO::Company:
            return contact->company();
        case ContactXmlIO::BusinessStreet:
            return contact->businessStreet();
        case ContactXmlIO::BusinessCity:
            return contact->businessCity();
        case ContactXmlIO::BusinessState:
            return contact->businessState();
        case ContactXmlIO::BusinessZip:
            return contact->businessZip();
        case ContactXmlIO::BusinessCountry:
            return contact->businessCountry();
        case ContactXmlIO::BusinessWebPage:
            return contact->businessWebpage();
        case ContactXmlIO::JobTitle:
            return contact->jobTitle();
        case ContactXmlIO::Department:
            return contact->department();
        case ContactXmlIO::Office:
            return contact->office();
        case ContactXmlIO::BusinessPhone:
            return contact->businessPhone();
        case ContactXmlIO::BusinessFax:
            return contact->businessFax();
        case ContactXmlIO::BusinessMobile:
            return contact->businessMobile();
        case ContactXmlIO::BusinessPager:
            return contact->businessPager();
        case ContactXmlIO::Profession:
            return contact->profession();
        case ContactXmlIO::Assistant:
            return contact->assistant();
        case ContactXmlIO::Manager:
            return contact->manager();

            //personal
        case ContactXmlIO::Spouse:
            return contact->spouse();
        case ContactXmlIO::Gender:
            switch(contact->gender()) {
                case QContact::UnspecifiedGender:
                    return QString();
                case QContact::Male:
                    return "1";
                case QContact::Female:
                    return "2";
            }
            break;
        case ContactXmlIO::Birthday:
            if (contact->birthday().isNull())
                return QString();
            return QPimXmlIO::dateToXml(contact->birthday());
        case ContactXmlIO::Anniversary:
            if (contact->anniversary().isNull())
                return QString();
            return QPimXmlIO::dateToXml(contact->anniversary());
        case ContactXmlIO::Nickname:
            return contact->nickname();
        case ContactXmlIO::Children:
            return contact->children();

            // other
        case ContactXmlIO::Notes:
            return contact->notes();

            // Added in Qtopia 1.6
        case ContactXmlIO::LastNamePron:
            return contact->lastNamePronunciation();
        case ContactXmlIO::FirstNamePron:
            return contact->firstNamePronunciation();
        case ContactXmlIO::CompanyPron:
            return contact->companyPronunciation();
    }
    return QString();
}

bool ContactXmlIO::select(const QContact &c) const
{
    return cFilter.accepted(c.categories());
}

QCategoryFilter ContactXmlIO::categoryFilter() const
{
    return cFilter;
}

void ContactXmlIO::setCategoryFilter(const QCategoryFilter &f) {
    if (f != cFilter) {
        cFilter = f;
        m_Filtered.clear();
        foreach (QContact *cnt, m_Contacts)
            if (select(*cnt))
                m_Filtered.append(cnt);
        sort();
    }
}

QContactModel::Field ContactXmlIO::sortKey() const
{
    return cKey;
}

void ContactXmlIO::setSortKey(QContactModel::Field key)
{
    if (key != cKey) {
        cKey = key;
        sort();
    }
}
