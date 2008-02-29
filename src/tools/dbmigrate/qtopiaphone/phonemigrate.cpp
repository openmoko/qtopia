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

#include "phonemigrate.h"

#include <QStringList>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTimeZone>
#include <QDebug>
#include <QTextCodec>
#include <QFile>
#include <QSqlError>

#include "../migrateengine.h"

// migratoin tables
const QStringList &PhoneMigrate::tables() const
{
    static QStringList tables;

    if (tables.count() == 0) {
        // in order of when they should be processed.
        tables << "callhistorytimezone";
        tables << "callhistory";
    }

    return tables;
}

PhoneMigrate::~PhoneMigrate()
{}

PhoneMigrate::PhoneMigrate(QDBMigrationEngine *engine) : syncTime(QDateTime::currentDateTime().toUTC()), mi(engine)
{
}

bool PhoneMigrate::migrate()
{
    const QSqlDatabase &db  = mi->database();

    QStringList oldTables;
    oldTables << "callhistorytimezone";
    oldTables << "callhistory";

    // first we need to drop the old schema's in the right order.
    QStringList existingTables = db.tables();
    foreach(QString table, oldTables) {
        int v = mi->tableVersion(table);
        if (existingTables.contains(table) && v < 110) {
            // back it up and drop.
            CHECK(mi->copyTable(table, table+"_old"));
            QSqlQuery query(db);
            CHECK(query.exec("DROP TABLE "+table));
        }
    }

    foreach(QString table, tables()) {
        CHECK(migrate(db, table, mi->tableVersion(table)));
    }
    return true;
}


bool PhoneMigrate::migrate(const QSqlDatabase &db, const QString &table, int version)
{
    if (version >= 110)
        return true;
    else {
        CHECK(mi->ensureSchema(table));
        CHECK(mi->setTableVersion(table, 110));

        QStringList existingTables = db.tables();

        // if it was backed up
        if (existingTables.contains(table+"_old")) {
            QSqlQuery query(db);
            CHECK(query.prepare(copyText(table)));
            CHECK(query.exec());

            CHECK(query.exec("DROP TABLE "+table+"_old;"));
        }
    }
    return true;
}

QString PhoneMigrate::copyText(const QString &table)
{
    const QSqlDatabase &db  = mi->database();

    QFile data(QLatin1String(":/QtopiaSql/copy/") + db.driverName() + QLatin1String("/") + table);
    data.open(QIODevice::ReadOnly);
    QTextStream ts(&data);
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    // assumption, no comments or mult-statements.  those belong in rc file if anywhere.
    return ts.readAll();
}

