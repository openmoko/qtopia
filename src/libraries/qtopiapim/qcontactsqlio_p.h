/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef ADDRESSBOOK_SQLIO_PRIVATE_H
#define ADDRESSBOOK_SQLIO_PRIVATE_H

#include <qtopiasql.h>
#include <qsqlquery.h>
#include <qvariant.h>
#include <qmap.h>
#include <qlist.h>
#include <qcache.h>

#include <qtopia/pim/qcontact.h>

#include "qcontactio_p.h"
#include "qpimsqlio_p.h"
#include "qpimsource.h"

class ContactSqlIO;
class QContactDefaultContext : public QContactContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QContactDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;

    using QContactContext::title;
    QString title() const;

    // better to be flags ?
    using QContactContext::editable;
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QContactContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateContact(const QContact &);
    bool removeContact(const QUniqueId &);
    QUniqueId addContact(const QContact &, const QPimSource &);

    bool importContacts(const QPimSource &, const QList<QContact> &);

    QList<QContact> exportContacts(const QPimSource &, bool &) const;
    QContact exportContact(const QUniqueId &id, bool &) const;

private:
    ContactSqlIO *mAccess;
    QPreparedSqlQuery importQuery;
};

class QSqlPimTableModel;
class ContactSqlIO : public QContactIO, public QPimSqlIO {

    Q_OBJECT

public:
    explicit ContactSqlIO( QObject *parent = 0, const QString &name = QString());

    ~ContactSqlIO();

    QUuid contextId() const;

    int count() const { return QPimSqlIO::count(); }

    bool editableByRow() const { return true; }
    bool editableByField() const { return true; }

    QVariant key(const QUniqueId &) const;

    void setSortKey(QContactModel::Field k);
    QContactModel::Field sortKey() const;

    void setCategoryFilter(const QCategoryFilter &f);
    QCategoryFilter categoryFilter() const { return QPimSqlIO::categoryFilter(); }

    void setContextFilter(const QSet<int> &, ContextFilterType = ExcludeContexts );

    bool startSyncTransaction(const QSet<QPimSource> &sources, const QDateTime &syncTime) { return QPimSqlIO::startSync(sources, syncTime); }
    bool abortSyncTransaction() { return QPimSqlIO::abortSync(); }
    bool commitSyncTransaction() { return QPimSqlIO::commitSync(); }

    QList<QUniqueId> removed(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
    { return QPimSqlIO::removed(sources, timestamp); }

    QList<QUniqueId> added(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
    { return QPimSqlIO::added(sources, timestamp); }

    QList<QUniqueId> modified(const QSet<QPimSource> &sources, const QDateTime &timestamp) const
    { return QPimSqlIO::modified(sources, timestamp); }

    QVariant contactField(int row, QContactModel::Field k) const;
    bool setContactField(int row, QContactModel::Field k,  const QVariant &);

    QContact contact(const QUniqueId &) const;
    QContact contact(int row) const;
    QUniqueId id(int row) const { return QPimSqlIO::recordId(row); }
    QVariant key(int row) const;
    int row(const QUniqueId & id) const { return QPimSqlIO::row(id); }
    int predictedRow(const QVariant &k, const QUniqueId &i) const;

    bool removeContact(int row);
    bool removeContact(const QUniqueId & id);
    bool removeContact(const QContact &t);
    bool removeContacts(const QList<int> &rows);
    bool removeContacts(const QList<QUniqueId> &ids);

    bool updateContact(const QContact &t);
    QUniqueId addContact(const QContact &contact, const QPimSource &s)
    { return addContact(contact, s, true); }
    QUniqueId addContact(const QContact &contact, const QPimSource &, bool);

    bool exists(const QUniqueId & id) const { return !contact(id).uid().isNull(); }
    bool contains(const QUniqueId & id) const { return QPimSqlIO::contains(id); }

#ifdef SUPPORT_SYNCML
    bool canProvideDiff() const { return false; }
    void clearJournal() {}
    QList<QUniqueId> addedContacts() const { return QList<QUniqueId>(); }
    QList<QUniqueId> modifiedContacts() const { return QList<QUniqueId>(); }
    QList<QUniqueId> deletedContacts() const { return QList<QUniqueId>(); }
#endif

    void setFilter(const QString &, int);
    void clearFilter();

#ifdef QTOPIA_PHONE
    QUniqueId matchPhoneNumber(const QString &, int &) const;
#endif
    void invalidateCache();

    void checkAdded(const QUniqueId &) { invalidateCache(); }
    void checkRemoved(const QUniqueId &) { invalidateCache(); }
    void checkRemoved(const QList<QUniqueId> &) { invalidateCache(); }
    void checkUpdated(const QUniqueId &) { invalidateCache(); }
protected:
    void bindFields(const QPimRecord &, QSqlQuery &) const;
    QStringList sortColumns() const;
    QString sqlColumn(QContactModel::Field k) const;

    bool updateExtraTables(uint, const QPimRecord &);
    bool insertExtraTables(uint, const QPimRecord &);
    bool removeExtraTables(uint);

private slots:
    void updateSqlLabel();

private:
    QContactModel::Field orderKey;

    QString sqlLabel() const;
    QString sqlField(QContactModel::Field) const;
    bool canUpdate(QContactModel::Field) const;
    void initMaps();

    mutable bool contactByRowValid;
    mutable QContact lastContact;
    mutable QCache<QUniqueId,QContact> contactCache;
    QString sqlLabelCache;

    QString mSearchText;
    int mSearchIndex;
    QSqlPimTableModel *searchTable;

    bool tmptable;

    static QMap<QContactModel::Field, QString> mFields;
    static QMap<QContactModel::Field, bool> mUpdateable;

    // Saved queries
    mutable QPreparedSqlQuery contactQuery;
    mutable QPreparedSqlQuery categoryQuery;
    mutable QPreparedSqlQuery emailsQuery;
    mutable QPreparedSqlQuery addressesQuery;
    mutable QPreparedSqlQuery phoneQuery;
    mutable QPreparedSqlQuery customQuery;
    mutable QPreparedSqlQuery insertEmailsQuery;
    mutable QPreparedSqlQuery insertAddressesQuery;
    mutable QPreparedSqlQuery insertPhoneQuery;
    mutable QPreparedSqlQuery removeEmailsQuery;
    mutable QPreparedSqlQuery removeAddressesQuery;
    mutable QPreparedSqlQuery removePhoneQuery;
};

#endif
