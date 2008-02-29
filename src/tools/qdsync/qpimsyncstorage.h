/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

class QDateTime;
class QByteArray;
class QPimModel;
class QAppointmentModel;
class QContactModel;
class QTaskModel;

using namespace PIMXML_NAMESPACE;

class QPimSyncStorage : public QObject
{
    Q_OBJECT
public:
    QPimSyncStorage(QObject *parent);
    ~QPimSyncStorage();

public slots:
    virtual bool startSyncTransaction(const QDateTime &);
    virtual bool abortSyncTransaction();
    virtual bool commitSyncTransaction();

    virtual void addServerRecord(const QByteArray &) = 0;
    virtual void replaceServerRecord(const QByteArray &) = 0;
    virtual void removeServerRecord(const QString &) = 0;

    virtual void performSync(const QDateTime &) = 0;

signals:
    void mappedId(const QString &, const QString &);

    void addClientRecord(const QByteArray &);
    void replaceClientRecord(const QByteArray &);
    void removeClientRecord(const QString &);

    void clientChangesCompleted();

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
};

class QAppointmentSyncStorage : public QPimSyncStorage
{
    Q_OBJECT
public:
    QAppointmentSyncStorage();
    ~QAppointmentSyncStorage();

    virtual bool commitSyncTransaction();

    virtual void addServerRecord(const QByteArray &);
    virtual void replaceServerRecord(const QByteArray &);
    virtual void removeServerRecord(const QString &);

    virtual void performSync(const QDateTime &);

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

    virtual bool commitSyncTransaction();
    virtual void addServerRecord(const QByteArray &);
    virtual void replaceServerRecord(const QByteArray &);
    virtual void removeServerRecord(const QString &);

    virtual void performSync(const QDateTime &);

private:
    QTaskModel *model;
};

class QContactSyncStorage : public QPimSyncStorage
{
    Q_OBJECT
public:
    QContactSyncStorage();
    ~QContactSyncStorage();

    virtual bool commitSyncTransaction();
    virtual void addServerRecord(const QByteArray &);
    virtual void replaceServerRecord(const QByteArray &);
    virtual void removeServerRecord(const QString &);

    virtual void performSync(const QDateTime &);

private:
    QContactModel *model;
};

#endif// QPIMSYNCSTORAGE_H
