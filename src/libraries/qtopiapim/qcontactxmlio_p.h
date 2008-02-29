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

#ifndef ADDRESSBOOK_ACCESS_PRIVATE_H
#define ADDRESSBOOK_ACCESS_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qcategorymanager.h>
#include <qtopia/pim/qcontact.h>
#include "qcontactio_p.h"
#include "qpimxmlio_p.h"

#ifdef QTOPIA_PHONE
#include <qphone.h>
#include <qphonebook.h>
#endif

#ifdef QTOPIAPIM_TEMPLATEDLL
//MOC_SKIP_BEGIN
template class QListIterator<class QContact>;
//MOC_SKIP_END
#endif

#ifdef QTOPIA_TEMPLATEDLL
//MOC_SKIP_BEGIN
QTOPIA_TEMPLATEDLL template class QList<QContact*>;
//MOC_SKIP_END
#endif

class ContactXmlIO : public QContactIO, public QPimXmlIO {

    Q_OBJECT

public:

    explicit ContactXmlIO(QObject *parent,
                 const QString &file = QString(),
                 const QString &journal = QString() );

    ~ContactXmlIO();

    QUuid contextId() const;

    int count() const { return m_Contacts.count(); }

    QContact contact( const QUniqueId & ) const;
    QContact contact(int row) const;
    QUniqueId id(int row) const;
    QVariant key(int row) const;
    int row(const QUniqueId & id) const;

    // external methods.
    QUniqueId addContact(const QContact &);
    bool updateContact( const QContact &);

    bool removeContact( const QContact &);
    bool removeContact(int row) { return removeContact(row); }
    bool removeContact(const QUniqueId &) { return false; }
    bool removeContacts(const QList<int> &) { return false; }
    bool removeContacts(const QList<QUniqueId> &) { return false; }

    bool exists(const QUniqueId &) const { return false; }
    bool contains(const QUniqueId &) const { return false; }

    void setFormat(const QString &);

    QContactModel::Field sortKey() const;
    void setSortKey(QContactModel::Field key);

    QCategoryFilter categoryFilter() const;
    void setCategoryFilter(const QCategoryFilter &);

    // Not implemented as should only be used for import in
    // Qtopia 4.
    int predictRow(const QContact &) const { return count(); }

#ifdef QTOPIA_PHONE
    void setSimCardContacts(const QList<QPhoneBookEntry> &);
    bool isSimCardContact(const QUniqueId &);
#endif

protected:
    // this is still needed, even if it does change
    // XXX should also not load Label Field.
    enum KeyType {
        Title,

        FirstName,
        MiddleName,
        LastName,
        Suffix,

        // email
        DefaultEmail,
        Emails,

        // home
        HomeStreet,
        HomeCity,
        HomeState,
        HomeZip,
        HomeCountry,
        HomePhone,
        HomeFax,
        HomeMobile,
        HomeWebPage,

        // business
        Company,
        BusinessStreet,
        BusinessCity,
        BusinessState,
        BusinessZip,
        BusinessCountry,
        BusinessWebPage,
        JobTitle,
        Department,
        Office,
        BusinessPhone,
        BusinessFax,
        BusinessMobile,
        BusinessPager,
        Profession,
        Assistant,
        Manager,

        //personal
        Spouse,
        Gender,
        Birthday,
        Anniversary,
        Nickname,
        Children,

        // other
        Notes,

        // Added in Qtopia 1.6
        LastNamePron,
        FirstNamePron,
        CompanyPron
    };

    static void initMap();
    static QMap<QString, KeyType> kLookup;

    virtual void setFields(QPimRecord *, const QMap<QString, QString> &) const;
    virtual QMap<QString, QString> fields(const QPimRecord *) const;

    void setField(QContact *rec, KeyType t, const QString &value) const;
    QString field(const QContact *, KeyType) const;

    const char *recordStart() const { return "<Contact "; } // No tr
    const char *listStart() const { return
                                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook >"
                                        "<AddressBook>\n<Groups>\n</Groups>\n<Contacts>\n"; }
    const char *listEnd() const { return "</Contacts>\n</AddressBook>\n"; }

    QPimRecord *createRecord() const { return new QContact(); }

    void checkAdded(const QContact &);
    void checkRemoved(const QUniqueId &);
    void checkRemoved(const QList<QUniqueId> &);
    void checkUpdated(const QContact &);

    bool internalAddRecord(QPimRecord *);
    bool internalRemoveRecord(QPimRecord *);
    bool internalUpdateRecord(QPimRecord *);

    bool select(const QContact &) const;

    // TODO
    void setFilter(const QString &, int) {}
    void clearFilter() {}
private slots:
    void sort();

private:
    void ensureDataCurrent(bool forceReload = false);
    QList<QContact*> contacts() const;
    QList<QContact*> sortedContacts() const;
    bool contains( const QContact &) const;
    void setContacts( const QList<QContact> &l );
    bool loadData();
    bool saveData();
    void clear();
    QList<QContact*> m_Contacts; // for the append and remove functions
    // more than anything else.
    QList<QContact*> m_Filtered;

    QCategoryFilter cFilter;
    QContactModel::Field cKey;
    bool needsSave;
};

#endif
