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

#ifndef _QPESQL_H_
#define _QPESQL_H_

#include <qtopiaglobal.h>

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <qcontent.h>

class QStringList;
class QtopiaSqlPrivate;
class QtopiaSqlMigratePlugin;

// create table support required?
// named db support required (qtopia name of default db?)
class QTOPIA_EXPORT QtopiaSql {
public:
    static QtopiaSql *instance();

    void openDatabase();

    static int stringCompare(const QString &, const QString &);

    QSqlDatabase &systemDatabase();
    void loadConfig(const QString &type, const QString &name, const QString &user);

    QSqlError exec(const QString &query, QSqlDatabase& db, bool inTransaction=true );
    QSqlError exec(QSqlQuery &query, QSqlDatabase& db, bool inTransaction=true );
    void attachDB(const QString& path);
    void attachDB(const QString& path, const QString &dbPath);
    void detachDB(const QString& path);
    QtopiaDatabaseId databaseIdForPath(const QString& path);
    QtopiaDatabaseId databaseIdForDatabasePath(const QString& dbPath);
    QSqlDatabase &database(const QtopiaDatabaseId& id);
    const QList<QtopiaDatabaseId> databaseIds();
    const QList<QSqlDatabase> databases();
    bool isDatabase(const QString &path);
    QString databasePathForId(const QtopiaDatabaseId& id);
    bool isValidDatabaseId(const QtopiaDatabaseId& id);

    QString escapeString(const QString &input);
    QSqlDatabase applicationSpecificDatabase(const QString &appname);
    bool ensureTableExists(const QString &table, QSqlDatabase &db );
    bool ensureTableExists(const QStringList &, QSqlDatabase& );

    void logQuery(const QSqlQuery &q);
private:
    void attachDB(const QString &path, const QString &dbPath, QtopiaSqlMigratePlugin *plugin);
    QtopiaSql();
    void closeDatabase();
    QSqlDatabase *connectDatabase(const QString &connName);
    void init(QSqlDatabase &db, bool force=false);
    void loadConfig();
    void saveConfig();
    void connectDiskChannel();

    QtopiaSqlPrivate *d();

    friend class QtopiaApplication;
    friend class QtopiaSqlPrivate;
};

#endif
