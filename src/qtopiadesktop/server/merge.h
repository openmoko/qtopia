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
#ifndef __PIMSYNC_MERGE_H__
#define __PIMSYNC_MERGE_H__

#include <QMap>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QObject>

struct Change
{
    enum Type {
        Create,
        Replace,
        Remove,
    };
    Type type;
    QString id;
    QByteArray record;
};

struct Conflict
{
    enum Type {
        NoConflict,
        CreateCreate, // only relevant on slow-sync
        ReplaceReplace,
        RemoveRemove,
        ReplaceRemove,
        RemoveReplace
    };

    Conflict(const Change &c, const Change &s);
    virtual ~Conflict();

    Type type;
    Change client;
    Change server;
};

class QSyncMergeData;
class QXmlStreamAttributes;
class MergeItem;
class QSqlQuery;
class QSyncMerge : public QObject
{
    Q_OBJECT
public:
    QSyncMerge(QObject *parent);
    virtual ~QSyncMerge();


    void lastSync(const QString &id, const QString &source, QDateTime &clientLastSync, QDateTime &serverLastSync);
    void recordLastSync(const QString &id, const QString &source, const QDateTime &clientLastSync, const QDateTime &serverLastSync);

    void clearChanges();


    QList<Conflict> conflicts();

    bool resolveClient(const Conflict &);
    bool resolveServer(const Conflict &);

    bool resolveDuplicate(const Conflict &);
#if 0
    // not currently advised, id mapping still an issue.
    // better to have something that is more specific about which
    // way the idents are mapped for the byte array.
    bool resolveMerged(const Conflict &, const QByteArray &replacementrecord);
#endif

    // and of course the 'all' resolve choices
    bool resolveAllClient();
    bool resolveAllServer();

    QList<Change> serverDiff();
    QList<Change> clientDiff();


    enum ChangeSource {
        Server,
        Client
    };
    QString parseIdentifiers(QByteArray &, ChangeSource, bool revert=false);
    bool canMap(const QString &ident, ChangeSource source)
    { return !map(ident, source).isEmpty(); }
    QString map(const QString &, ChangeSource);

    void clearIdentifierMap();
    void setDatasource(const QString &datasource);

public slots:
    void setServerReferenceSchema(const QByteArray &);
    void setClientReferenceSchema(const QByteArray &);

    void mapIdentifier(const QString &serverId, const QString &clientId);

    void createServerRecord(const QByteArray &);
    void replaceServerRecord(const QByteArray &);
    void removeServerRecord(const QString &);

    void createClientRecord(const QByteArray &);
    void replaceClientRecord(const QByteArray &);
    void removeClientRecord(const QString &);


private:
    MergeItem *referenceItem();

    void readConflict(Change &client, Change &server, const QSqlQuery &q);
    bool resolveBiased(const Conflict &conflict, bool);

    void addServerChange(Change::Type, const MergeItem &);
    void addClientChange(Change::Type, const MergeItem &);

    void revertIdentifierMapping(const Conflict &);

    // convenience functions for QXmlStreamAttributes
    static bool contains(const QXmlStreamAttributes &, const QString &qName);
    static bool booleanValue(const QXmlStreamAttributes &, const QString &qName);
    static void setBooleanValue(QXmlStreamAttributes &, const QString &qName, bool);

    QSyncMergeData *d;
};

#include <desktopsettings.h>
#define SyncLog() if ( !DesktopSettings::debugMode() ); else _SyncLog()
QDebug _SyncLog();

#endif // __PIMSYNC_MERGE_H__
