/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef QPIMSYNCSTORAGE_H
#define QPIMSYNCSTORAGE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QUniqueId>
#include "qpimxml_p.h"
#include "qtopia4sync.h"

class QDateTime;
class QByteArray;
class QPimModel;
class QAppointmentModel;
class QContactModel;
class QTaskModel;

using namespace PIMXML_NAMESPACE;

class QPimSyncStorageFactory : public Qtopia4SyncPluginFactory {
public:
    QPimSyncStorageFactory( QObject *parent = 0 );
    ~QPimSyncStorageFactory();
    QStringList keys();
    Qtopia4SyncPlugin *plugin( const QString &key );
};

class QPimSyncStorage : public Qtopia4SyncPlugin
{
    Q_OBJECT
public:
    QPimSyncStorage(const QString &dataset, QObject *parent);
    virtual ~QPimSyncStorage();

    QString dataset() { return mDataset; }

    virtual void beginTransaction(const QDateTime &);
    virtual void abortTransaction();
    virtual void commitTransaction();

    virtual void createServerRecord(const QByteArray &) = 0;
    virtual void replaceServerRecord(const QByteArray &) = 0;
    virtual void removeServerRecord(const QString &) = 0;

    virtual void fetchChangesSince(const QDateTime &) = 0;

protected:
    QMap<QString, QUniqueId> idMap;

    void setModel(QPimModel *);

    void mergeUnmappedCategories(const QSet<QString> & categories)
    { mUnmappedCategories = mUnmappedCategories.unite(categories); }
    const QSet<QString> &unmappedCategories() const
    { return mUnmappedCategories; }
private:
    QPimModel *m;
    QSet<QString> mUnmappedCategories;
    QString mDataset;
};

class QAppointmentSyncStorage : public QPimSyncStorage
{
    Q_OBJECT
public:
    QAppointmentSyncStorage();
    ~QAppointmentSyncStorage();

    void commitTransaction();

    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);
    void fetchChangesSince(const QDateTime &);

private:
    QList<QPimXmlException> convertExceptions(const QList<QAppointment::Exception> origList) const;
    QAppointmentModel *model;
};

class QTaskSyncStorage : public QPimSyncStorage
{
    Q_OBJECT
public:
    QTaskSyncStorage();
    ~QTaskSyncStorage();

    void commitTransaction();
    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);
    void fetchChangesSince(const QDateTime &);

private:
    QTaskModel *model;
};

class QContactSyncStorage : public QPimSyncStorage
{
    Q_OBJECT
public:
    QContactSyncStorage();
    ~QContactSyncStorage();

    void commitTransaction();
    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);
    void fetchChangesSince(const QDateTime &);

private:
    QContactModel *model;
};

#endif// QPIMSYNCSTORAGE_H
