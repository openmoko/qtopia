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
#include "qpimsync_p.h"
#include "qpimxml_p.h"
#include "qpimsyncstorage.h"
#include <qtopialog.h>
QTOPIA_LOG_OPTION(QPimSync)
#include "common/qcopadaptor_qd.h"

#include <QIODevice>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

#include <QtopiaSql>
#include <QAppointmentModel>
#include <QContactModel>
#include <QTaskModel>

#include <stdio.h>

QTOPIA_EXPORT_PLUGIN(QSyncProtocol);

class QSyncProtocolController : public QCopAdaptor
{
    friend class QSyncProtocol;
    Q_OBJECT
public:
    QSyncProtocolController(QSyncProtocol *parent);
    ~QSyncProtocolController();

public slots:
    void serverIdentity(const QString &);
    void serverVersion(int, int, int);
    void serverSyncRequest(const QString &datasource); // e.g. contacts/notes/calendar
    void serverSyncAnchors(const QDateTime &, const QDateTime &);
    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);
    void requestTwoWaySync();
    void requestSlowSync();
    void serverError();
    void serverEnd();

signals:
    void clientSyncRequest(const QString &datasource);
    void clientIdentity(const QString &);
    void clientVersion(int, int, int);
    void clientSyncAnchors(const QDateTime &, const QDateTime &);
    void createClientRecord(const QByteArray &);
    void replaceClientRecord(const QByteArray &);
    void removeClientRecord(const QString &);
    void mapId(const QString &, const QString &);
    void clientError();
    void clientEnd();

protected:
    QStringList sendChannels(const QString &);

private:
    void sendAppointmentChanges(const QList<QUniqueId> &, const QList<QUniqueId> &, const QList<QUniqueId> &);
    void sendTaskChanges(const QList<QUniqueId> &, const QList<QUniqueId> &, const QList<QUniqueId> &);
    void sendContactChanges(const QList<QUniqueId> &, const QList<QUniqueId> &, const QList<QUniqueId> &);

    enum Expecting {
        None,
        Header,
        SyncType,
        DiffStart,
        Diff,
        Aborted
    };

    void abort(const QString &);
    bool startServerDiff();

    QSyncProtocol *q;
    Expecting state;

    QString serverid;
    QString datasource;

    QDateTime nextSync;
    QDateTime lastSync;

    QPimSyncStorage *mStorage;
    QTimer *mTimeout;
};

/*!
  \fn void QSyncProtocolController::syndSyncRequest(const QString &datasource)

  Requests that the server initializes sync for the specified \a datasource.
  The datasource refers to the data to be synced.  Qtopia supports;
  
  \ul
    \li calendar - sync the Calendar application data.
    \li contacts - sync the Contacts application data.
    \li tasks - sync the Tasks application data.
  \lu

  Only one data source can be synced at a time
*/

QSyncProtocolController::QSyncProtocolController(QSyncProtocol *parent)
    : QCopAdaptor("QPE/PimSync", parent), q(parent)
{
    publishAll(SignalsAndSlots);
    mTimeout = new QTimer;
    mTimeout->setInterval(2000);
    mTimeout->setSingleShot(true);
    mStorage = 0;
    state = None;
}

QSyncProtocolController::~QSyncProtocolController()
{
    if (state != None)
        abort(tr("Synchronization released while existing synchronization in progress"));
}

QStringList QSyncProtocolController::sendChannels(const QString &)
{
    QStringList sl;
    sl.append("QD/PimSync");
    return sl;
}

void QSyncProtocolController::abort(const QString &message)
{
    if (state == Diff)
        mStorage->abortSyncTransaction();
    emit clientError();
    emit q->syncError(message);
    state = None;
    if (mStorage)
        delete mStorage;
    mStorage = 0;
    datasource.clear();
    serverid.clear();
    mTimeout->stop();
}

void QSyncProtocolController::serverSyncRequest(const QString &source)
{
    qLog(QPimSync) << "received" << "serverSyncRequest()";
    if (state != None)
        abort(tr("Synchronization attempted while existing synchronization in progress"));
    state = Header;

    datasource = source;
}

void QSyncProtocolController::serverIdentity(const QString &server)
{
    qLog(QPimSync) << "received" << "serverIdentity()";
    // assert state == Header
    serverid = server;
}

void QSyncProtocolController::serverVersion(int major, int minor, int patch)
{
    qLog(QPimSync) << "received" << "serverVersion()";
    // assert state == Header
    if (major != 4 || minor != 3 || patch != 0)
        abort(tr("Invalid server version, server version 4.3.0 required"));
    emit clientIdentity("trolltech.qtopia"); // probably should be unique per-device
    emit clientVersion(4, 3, 0);
}

void QSyncProtocolController::serverSyncAnchors(const QDateTime &serverLastSync, const QDateTime &serverNextSync)
{
    qLog(QPimSync) << "received" << "serverSyncAnchors()";
    // If the last sync tags don't match up will need a full sync.
    // let the server catch this as it is expected to chose the sync
    // type already.
    Q_UNUSED(serverLastSync)
    Q_UNUSED(serverNextSync)

    // set model
    if (datasource == "calendar") {
        mStorage = new QAppointmentSyncStorage();
    } else if (datasource == "contacts") {
        mStorage = new QContactSyncStorage();
    } else if (datasource == "tasks") {
        mStorage = new QTaskSyncStorage();
    } else {
        abort(tr("Invalid synchronization source specified by server"));
        return;
    }

    connect(mStorage, SIGNAL(addClientRecord(QByteArray)),
            this, SIGNAL(createClientRecord(QByteArray)));
    connect(mStorage, SIGNAL(replaceClientRecord(QByteArray)),
            this, SIGNAL(replaceClientRecord(QByteArray)));
    connect(mStorage, SIGNAL(removeClientRecord(QString)),
            this, SIGNAL(removeClientRecord(QString)));

    connect(mStorage, SIGNAL(mappedId(QString,QString)),
            this, SIGNAL(mapId(QString,QString)));

    QSqlQuery q(QtopiaSql::instance()->systemDatabase());
    q.prepare("SELECT lastSyncAnchor FROM syncServers WHERE serverIdentity = :s AND datasource = :d");
    q.bindValue(":s", serverid);
    q.bindValue(":d", datasource);
    !q.exec();
    if (q.next())
        lastSync = q.value(0).toDateTime();
     else
        lastSync = QDateTime();

    nextSync = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    // send client header
    emit clientSyncAnchors(lastSync, nextSync);
    state = SyncType;
}

bool QSyncProtocolController::startServerDiff()
{
    switch(state) {
        default:
            abort(tr("Protocol error - unexpected response from server"));
            return false;
        case DiffStart:
            mStorage->startSyncTransaction(nextSync);
            state = Diff;
            break;
        case Diff:
            break;
    }
    return true;
}

void QSyncProtocolController::createServerRecord(const QByteArray &record)
{
    qLog(QPimSync) << "received" << "createServerRecord()";
    if (!startServerDiff())
        return;
    mStorage->addServerRecord(record);
}

void QSyncProtocolController::replaceServerRecord(const QByteArray &record)
{
    qLog(QPimSync) << "received" << "replaceServerRecord()";
    if (!startServerDiff())
        return;
    mStorage->replaceServerRecord(record);
}

void QSyncProtocolController::removeServerRecord(const QString &localId)
{
    qLog(QPimSync) << "received" << "removeServerRecord()";
    if (!startServerDiff())
        return;
    mStorage->removeServerRecord(localId);
}

void QSyncProtocolController::requestTwoWaySync()
{
    qLog(QPimSync) << "received" << "requestTwoWaySync()";
    if (state != SyncType)
        abort(tr("Protocol error - unexpected response from server"));
    state = DiffStart;

    // add a second, don't want to catch items resulting from previous
    // sync, timestamps in Qtopia are only to second accuracy
    mStorage->performSync(lastSync.addSecs(1));

    emit clientEnd();
}

void QSyncProtocolController::requestSlowSync()
{
    qLog(QPimSync) << "received" << "requestSlowSync()";
    if (state != SyncType)
        abort(tr("Protocol error - unexpected response from server"));
    state = DiffStart;

    mStorage->performSync(QDateTime());

    emit clientEnd();
}

void QSyncProtocolController::serverError()
{
    qLog(QPimSync) << "received" << "serverError()";
    if (state == Diff)
        mStorage->abortSyncTransaction();
    emit q->syncError(tr("Server indicated synchronization failure"));
    state = None;
}

void QSyncProtocolController::serverEnd()
{
    qLog(QPimSync) << "received" << "serverEnd()";
    if (state == Diff || state == DiffStart) {
        QSqlQuery q(QtopiaSql::instance()->systemDatabase());
        if (lastSync.isNull())
            q.prepare("INSERT INTO syncServers (serverIdentity, datasource, lastSyncAnchor) VALUES (:id, :ds, :ls)");
        else
            q.prepare("UPDATE syncServers SET lastSyncAnchor = :ls WHERE serverIdentity = :id AND datasource = :ds");
        q.bindValue(":id", serverid);
        q.bindValue(":ds", datasource);
        q.bindValue(":ls", nextSync);

        if (!q.exec()) {
            abort(tr("Failed to update last sync information"));
            return;
        }
    }

    switch (state) {
       case Diff:
           mStorage->commitSyncTransaction();
           // fallthrough
       case DiffStart:
           emit clientEnd();
           // server will disconect..
           mTimeout->stop();
           q->syncComplete();
           state = None;
           break; // no server changes.
       default:
           abort("Protocol error - unexpected response from server");
           break;
    }
}


QSyncProtocol::QSyncProtocol(QObject *parent)
    : QObject(parent)
{
    d = new QSyncProtocolController(this);
    connect(d->mTimeout, SIGNAL(timeout()), this, SIGNAL(timeout()));
}

QSyncProtocol::~QSyncProtocol()
{
}

void QSyncProtocol::startSync(const QString &source)
{
    d->clientSyncRequest(source);
}

void QSyncProtocol::abortSync()
{
    d->abort(tr("Requested abort of synchronization"));
}

/*!
  \fn void QSyncProtocol::timeout()

  Emitted when the QSyncProtocol has had no response from the server for a period longer
  than the specified timeout duration.

  \sa timeoutDuration(), setTimeoutDuration()
*/

/*!
  If the timeout is active restarts the timeout.

  \sa timeoutDuration, setTimeoutDuration(), timeout()
*/
void QSyncProtocol::resetTimeout()
{
    if (d->mTimeout->isActive())
        d->mTimeout->start();
}

/*!
  Returns the timeout duration.

  \sa setTimeoutDuration(), timeout()
*/
int QSyncProtocol::timeoutDuration() const
{
    return d->mTimeout->interval();
}

/*!
  Sets the timeout duration to \a duration.

  \sa timeoutDuration(), timeout()
*/
void QSyncProtocol::setTimeoutDuration(int duration)
{
    if (d->mTimeout->isActive()) {
        d->mTimeout->setInterval(duration);
        d->mTimeout->start();
    } else {
        d->mTimeout->setInterval(duration);
    }
}

#include "qpimsync.moc"
