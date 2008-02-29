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

#include "pimmigrate.h"

#include <QStringList>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTimeZone>
#include <QDebug>
#include <QTextCodec>
#include <QFile>
#include <QSqlError>

#include "../migrateengine.h"

#include <qsql_sqlite.h>
#include <sqlite3.h>
#include <netinet/in.h>

// migratoin tables
const QStringList &PimMigrate::tables() const
{
    static QStringList tables;

    if (tables.count() == 0) {
        // in order of when they should be processed.
        tables << "changelog";
        tables << "sqlsources";

        tables << "appointments";
        tables << "appointmentcategories";
        tables << "appointmentcustom";
        tables << "appointmentexceptions";
        tables << "contacts";
        tables << "contactaddresses";
        tables << "contactcategories";
        tables << "contactcustom";
        tables << "contactphonenumbers";
        tables << "emailaddresses";
        tables << "tasks";
        tables << "taskcategories";
        tables << "taskcustom";

        tables << "simcardidmap";
        tables << "googleid";
        tables << "gcal_appointments";
        tables << "gcal_appointmentcategories";
        tables << "gcal_appointmentcustom";
        tables << "gcal_appointmentexceptions";
    }

    return tables;
}


void convertRecIdFunc(sqlite3_context *context, int, sqlite3_value**values)
{
    int size = sqlite3_value_bytes(values[0]);
    if (size != 8)
        return;

    struct PairedUint {
        quint32 left;
        quint32 right;
    };
    // network byte order
    PairedUint *id = (PairedUint *)sqlite3_value_blob(values[0]);

    quint32 result = (ntohl(id->left) << 24) | (ntohl(id->right) & 0x00ffffff);

    sqlite3_result_int(context, result);
}


PimMigrate::~PimMigrate()
{}

//PimMigrate::PimMigrate() : syncTime(QTimeZone::current().toUtc(QDateTime::currentDateTime()))
PimMigrate::PimMigrate(QDBMigrationEngine *engine) : syncTime(QDateTime::currentDateTime().toUTC()), mi(engine)
{
}

bool PimMigrate::migrate()
{
    // 4.2.0 brings in the changelog table
    // 4.2.2 brings in a change to rec id's.
    // but because migrate only handles from 4.1.x, 
    // the only relavant table version number is 110, the
    // number that maps to 4.2.2

    // first ensure changelog exists.
    CHECK(mi->ensureSchema("changelog"));
    CHECK(mi->setTableVersion("changelog", 110));

    const QSqlDatabase &db  = mi->database();

    // add function to migrate recid.  Since only db this affects is sqlite, use sqlite func
    QVariant v = db.driver()->handle();
    if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
        // v.data() returns a pointer to the handle
        sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
        CHECK(handle);
        int result = sqlite3_create_function( handle, "convertRecId", 1, SQLITE_ANY, 0, convertRecIdFunc, 0, 0);
        CHECK(result == SQLITE_OK);
    }

    QStringList oldTables;
    oldTables << "taskcustom";
    oldTables << "taskcategories";
    oldTables << "tasks";
    oldTables << "emailaddresses";
    oldTables << "contactphonenumbers";
    oldTables << "contactcustom";
    oldTables << "contactcategories";
    oldTables << "contactaddresses";
    oldTables << "contacts";
    oldTables << "appointmentexceptions";
    oldTables << "appointmentcustom";
    oldTables << "appointmentcategories";
    oldTables << "appointments";

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


bool PimMigrate::migrate(const QSqlDatabase &db, const QString &table, int version)
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

            if (table == "tasks" || table == "contacts" || table == "appointments") {
                QSqlQuery changelog(db);
                CHECK(changelog.prepare("INSERT INTO changelog (recid, created, modified, removed) SELECT recid, :dt1, :dt2, NULL FROM " + table + ";"));
                changelog.bindValue("dt1", syncTime);
                changelog.bindValue("dt2", syncTime);
                CHECK(changelog.exec());
            }

            //mi->dropTable(table+"_old");
            CHECK(query.exec("DROP TABLE "+table+"_old;"));
        }
    }
    return true;
}

QString PimMigrate::copyText(const QString &table)
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

