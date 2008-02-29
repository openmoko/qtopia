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

#ifndef _QPESQL_H_
#define _QPESQL_H_

#include <qtopiaglobal.h>

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <qcontent.h>

class QStringList;
class QtopiaSqlPrivate;

// create table support required?
// named db support required (qtopia name of default db?)
class QTOPIA_EXPORT QtopiaSql {
public:
    static void openDatabase();

    static bool ensureSchema(const QStringList &tables, QSqlDatabase &db,  bool transact = true);
    static bool ensureSchema(const QString &table, QSqlDatabase &db, bool transact = true);

    static int stringCompare(const QString &, const QString &);


    static QSqlDatabase &systemDatabase();
    static void loadConfig(const QString &type, const QString &name, const QString &user);

    /*static QSqlError exec(const QString &query);*/
    static QSqlError exec(const QString &query, QSqlDatabase& db, bool inTransaction=true );
    static QSqlError exec(QSqlQuery &query, QSqlDatabase& db, bool inTransaction=true );
    static void attachDB(const QString& path);
    static void attachDB(const QString& path, const QString &dbPath);
    static void detachDB(const QString& path);
    static QtopiaDatabaseId databaseIdForPath(const QString& path);
    static QSqlDatabase &database(const QtopiaDatabaseId& id);
    static const QList<QtopiaDatabaseId> databaseIds();
    static const QList<QSqlDatabase> databases();
    static bool isDatabase(const QString &path);
    static QString databasePathForId(const QtopiaDatabaseId& id);

    static QString escapeString(const QString &input);

    static void logQuery(const QSqlQuery &q);
    static bool loadSchema(QSqlDatabase &db, const QString &, bool transact=true);
private:
    static void closeDatabase();
    static void setupPreloadDatabase();
    static QSqlDatabase *connectDatabase(const QString &connName);
    static void init(QSqlDatabase &db, bool force=false);
    static void loadConfig();
    static void saveConfig();

    static void initPrivate();
    static void ensureDatabaseSchema(QSqlDatabase &db);

    static QtopiaSqlPrivate *d;
};

#endif
