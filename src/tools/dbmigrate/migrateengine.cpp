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

#include "migrateengine.h"

#include "qtopiapim/pimmigrate.h"
#include "qtopiaphone/phonemigrate.h"

#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QtopiaSql>
#include <qtopialog.h>
#include <QSqlRecord>
#include <QSqlDriver>
#include <QSqlField>
#include <QTextCodec>
#include <qtopiasql.h>
#include <QMimeType>
#include <qtopianamespace.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <QStorageMetaInfo>
#include <QDSData>
#include <QtopiaApplication>
#endif

Q_GLOBAL_STATIC(QDBMigrationEngine,migrationEngine);

QDBMigrationEngine *QDBMigrationEngine::instance()
{
    return migrationEngine();
}

bool QDBMigrationEngine::check(bool result, int line, const char *file, const char *message)
{
    if(result == false)
    {
        QString errString=QString("CHECK: %1:%2 check failed: %3\n").arg(file).arg(line).arg(message);
        db.rollback();
        db.close();
        qCritical(qPrintable(errString));
    }
    return result;
}

bool QDBMigrationEngine::exec(QSqlQuery &query, int line, const char *file)
{
    QtopiaSql::instance()->logQuery(query);
    if(query.exec() == false)
    {
        QString errString=QString("EXEC: %3:%2 Failed while executing query: %1\n").arg(query.executedQuery()).arg(line).arg(file);
        errString+=QString("SqlError: (%1, %2)\n").arg(query.lastError().number()).arg(query.lastError().text());
        db.rollback();
        db.close();
        qCritical(qPrintable(errString));
        return false;
    }
    else
        return true;
}

bool QDBMigrationEngine::exec(const QString &query, int line, const char *file)
{
    QSqlQuery qry(db);
    if(qry.exec(query) == false)
    {
        QtopiaSql::instance()->logQuery(qry);
        QString errString=QString("EXEC: %3:%2 Failed while executing query: %1\n").arg(query).arg(line).arg(file);
        errString+=QString("SqlError: (%1, %2, %3)\n").arg(qry.lastError().number()).arg(qry.lastError().databaseText()).arg(qry.lastError().driverText());
        db.rollback();
        db.close();
        qCritical(qPrintable(errString));
        return false;
    }
    else
    {
        QtopiaSql::instance()->logQuery(qry);
        return true;
    }
}


int QDBMigrationEngine::tableVersion(const QString &tableName)
{
    static bool versioninfoseen=false;
    if(versioninfoseen==false)
    {
        QStringList tables = db.tables();
        if(!tables.contains("versioninfo"))
            return 0;
        versioninfoseen=true;
    }

    QSqlQuery xsql(db);
    xsql.prepare("select coalesce(max(versionNum), 0) from versioninfo where tableName = :tableName");
    xsql.bindValue("tableName", tableName);
    int Result = 0;
    if(xsql.exec() && xsql.first())
        Result = xsql.value(0).toInt();
    return Result;
}

bool QDBMigrationEngine::setTableVersion(const QString &tableName, int versionNum)
{
    if (tableVersion(tableName) == versionNum)
        return true;

    QStringList tables = db.tables();
    if(!tables.contains("versioninfo"))
    {
        EXEC("CREATE TABLE versioninfo \
                ( \
                tableName NVARCHAR (255) NOT NULL, \
                versionNum INTEGER NOT NULL, \
                lastUpdated NVARCHAR(20) NOT NULL, \
                PRIMARY KEY(tableName, versionNum) \
                                    )");
    }
    QSqlQuery xsql(db);
    xsql.prepare("delete from versioninfo where tableName = ? and versionNum = ?");
    xsql.bindValue(0, tableName);
    xsql.bindValue(1, versionNum);
    EXEC(xsql);
    xsql.prepare("insert into versioninfo (tableName, versionNum, lastUpdated) VALUES (?, ?, ?)");
    xsql.bindValue(0, tableName);
    xsql.bindValue(1, versionNum);
    xsql.bindValue(2, QDateTime::currentDateTime().toString());
    EXEC(xsql);
    return true;
}

bool QDBMigrationEngine::copyTable(const QString &from, const QString &to)
{
    QStringList tables = db.tables();
    if(!tables.contains(from))
        return false;
    QStringList createStatement;
    createStatement << ("create table \"" + to + "\" (");
    for (int i=0; i<db.driver()->record(from).count(); i++)
    {
        QSqlField field=db.driver()->record(from).field(i);
        QString type;
        switch(field.type())
        {
            case QVariant::Bool:
            case QVariant::Char:
                type = "CHAR";
                break;
            case QVariant::Date:
                type = "DATE";
                break;
            case QVariant::DateTime:
                type = "DATETIME";
                break;
            case QVariant::Double:
                type = "DOUBLE("+QString::number(field.length())+','+QString::number(field.precision())+')';
                break;
            case QVariant::Int:
                type = "INT";
                break;
            case QVariant::String:
                if(db.driverName() == "QSQLITE" && field.length() == -1)
                    type = "TEXT";
                else
                    type = "NVARCHAR("+QString::number(field.length())+')';
                break;
            case QVariant::Time:
                type = "TIME";
                break;
            case QVariant::LongLong:
                type = "BIGINT";
                break;
            case QVariant::ByteArray:
            default:
                type = "BLOB";
                break;
        }
        createStatement << (i != 0 ? "," : " ") << field.name() << " " << type;
    }

    createStatement << ")";
    if(tables.contains(to))
    {
        EXEC("DROP TABLE "+to);
    }
    EXEC(createStatement.join(" "));
    EXEC("insert into \""+to+"\" select * from \""+from+'"');
    return true;
}

bool QDBMigrationEngine::ensureSchema(const QStringList &list, bool transact)
{
    QStringList tables = db.tables();
    foreach(QString table, list) {
        if (tables.contains(table, Qt::CaseInsensitive))
            continue;
        // load schema.
        QFile data(QLatin1String(":/QtopiaSql/") + db.driverName() + QLatin1String("/") + table);
        CHECK(data.open(QIODevice::ReadOnly));
        QTextStream ts(&data);
        // read assuming utf8 encoding.
        ts.setCodec(QTextCodec::codecForName("utf8"));
        ts.setAutoDetectUnicode(true);

        CHECK(loadSchema(ts, transact));
    }
    return true;
}

bool QDBMigrationEngine::loadSchema(QTextStream &ts, bool transact)
{
    if (transact)
        db.transaction();
    QString qry = "";
    while (!ts.atEnd()) {
        /*
        Simplistic parsing.
        no comments in middle of line
        no starting another sql statment on same line one ends.

        For now, shouldn't be a problem.
        */

        QString line = ts.readLine();
        // comment, remove.
        if (line.contains (QLatin1String("--")))
            line.truncate (line.indexOf (QLatin1String("--")));
        if (line.trimmed ().length () == 0)
            continue;
        qry += line;

        // no ;, query spans to next line, keep going.
        if ( line.contains( ';' ) == false) {
            qry += QLatin1String(" ");
            continue;
        }

        EXEC( qry );
        qry = QLatin1String("");
    }
    if(!qry.isEmpty())
        EXEC( qry );
    if (transact)
    {
        if (db.commit())
            return true;
        else
            return false;
    }
    else
        return true;
}

bool QDBMigrationEngine::doMigrate(const QStringList &args)
{
#ifndef QTOPIA_CONTENT_INSTALLER
    // Ensure QStorageMetaInfo and QtopiaSql are up to date.
    QStorageMetaInfo::instance()->update();
#endif

    // this should be passed a list of database paths
    QList<QtopiaDatabaseId> databaseIds;
    if(args.count() > 0)
    {
        foreach(const QString &arg, args) {
            const QtopiaDatabaseId &id( QtopiaSql::instance()->databaseIdForDatabasePath(arg) );
            if(!databaseIds.contains(id))
                databaseIds.append(id);
        }
    }
    else
        databaseIds = QtopiaSql::instance()->databaseIds();
    foreach(QtopiaDatabaseId id, databaseIds)
    {
#ifndef QTOPIA_CONTENT_INSTALLER
        bool removable=false;
        if(id != 0 && !QtopiaSql::instance()->databasePathForId(id).isEmpty() && QStorageMetaInfo::instance()->fileSystemOf(QtopiaSql::instance()->databasePathForId(id)) != NULL)
            removable = QStorageMetaInfo::instance()->fileSystemOf(QtopiaSql::instance()->databasePathForId(id))->isRemovable();
#endif

        db=QtopiaSql::instance()->database(id);
        if(db.isOpen()==false)
        {
            if(!db.open())
            {
                QString errString=QString("OPEN DATABASE: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
                qCritical(qPrintable(errString));
                return false;
            }
        }
        bool hasChanges=false;
        if(db.driverName() == QLatin1String("QSQLITE"))
        {
#ifndef QTOPIA_CONTENT_INSTALLER
            if( !checkIntegrity( db, false ) )
            {   // Attempt to recover by doing a vacuum.
                db.exec( "VACUUM" );

                if( !checkIntegrity( db, true ) )
                {   // Still no good, backup the old database to documents and create a new one.
                    db.close();

                    QFileSystem fs = QFileSystem::fromFileName( db.databaseName() );
                    QDir backupDir( fs.documents() ? fs.documentsPath() : QFileSystem::documentsFileSystem().documentsPath() );
                    QString backup = QLatin1String( "qtopia_db.sqlite.corrupt000" );
                    for( int i = 0; backupDir.exists( backup ); backup = QString( "qtopia_db.sqlite.corrupt%1" ).arg( ++i, 3, 10, QLatin1Char( '0' ) ) );

                    QFile dbFile( db.databaseName() );

                    dbFile.copy( backupDir.absoluteFilePath( backup ) );

                    dbFile.remove();

                    // Should notify the user at this point that the database is corrupted and unrecoverable.
                    // Deferred to a later release due to string freeze: Task 193949.

                    if(!db.open())
                    {
                        QString errString=QString("OPEN DATABASE: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
                        qCritical(qPrintable(errString));
                        return false;
                    }
                }
            }
#endif

            EXEC("PRAGMA synchronous = OFF");   // full/normal sync is safer, but by god slower.
            EXEC("PRAGMA temp_store = memory");
            EXEC("PRAGMA cache_size = 1000");
            //EXEC("PRAGMA default_cache_size = 1000");
        }
        if(!db.transaction())
        {
            QString errString=QString("BEGIN TRANSACTION: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
            db.close();
            qCritical(qPrintable(errString));
            return false;
        }

        QStringList tables = db.tables();   // grr. db.tables() holds open a lock on the database
//         db.close();
//         db.open();
        
//
// Note that content_installer always starts from scratch so it should
// not attempt to upgrade the database
//

#ifndef QTOPIA_CONTENT_INSTALLER
        if(tables.contains("content")) // pre-existing database
        {
            QSqlQuery xsql(db);
            if(!tables.contains("versioninfo"))
            {
                EXEC("CREATE TABLE \"versioninfo\" \
                        ( \
                        tableName NVARCHAR (255) NOT NULL, \
                        versionNum INTEGER NOT NULL, \
                        lastUpdated NVARCHAR(20) NOT NULL, \
                        PRIMARY KEY(tableName, versionNum) \
                                            )");
                hasChanges=true;
            }
            int user_version=0;
            if(db.driverName() == QLatin1String("QSQLITE") && tableVersion("content") < 111)
            {
                CHECK(xsql.exec(QLatin1String("PRAGMA user_version")) && xsql.first());
                user_version=xsql.value(0).toInt();
                if(user_version < 105)
                {
                    qLog(Sql) << "Performing version 105 updates";
                    EXEC("DROP TABLE content");
                    EXEC("DROP TABLE contentProps");
                    EXEC("DROP TABLE mapCategoryToContent");
                    EXEC("DROP TABLE locationLookup");
                    EXEC("DROP TABLE mimeTypeLookup");
                    EXEC("DROP TRIGGER fkdc_mapCategoryToContent_content");
                    EXEC("DROP TRIGGER fkdc_mapCategoryToContent_category");
                    EXEC("DROP TRIGGER fkuc_mapCategoryToContent_category");
                    EXEC("DROP TRIGGER fkdc_contentProps");
                    EXEC("DROP TRIGGER fkuc_contentProps");
                    QStringList tableslist;
                    tableslist << "content" << "contentProps" << "mapCategoryToContent" << "locationLookup" << "mimeTypeLookup";
                    CHECK(ensureSchema(tableslist));
                    CHECK(setTableVersion("content", 105));
                    CHECK(setTableVersion("contentProps", 105));
                    CHECK(setTableVersion("mapCategoryToContent", 105));
                    CHECK(setTableVersion("locationLookup", 105));
                    CHECK(setTableVersion("mimeTypeLookup", 105));
                    
                    user_version = 105;
                    hasChanges=true;
                }
                if(user_version < 106)
                {
                    qLog(Sql) << "Performing version 106 updates";
                    if(!db.record(QLatin1String("categories")).contains(QLatin1String("categoryicon")))
                    {
                        EXEC("ALTER TABLE categories ADD categoryicon VARCHAR(255)");
                    }
                    EXEC("CREATE UNIQUE INDEX mCidCatIndex ON mapCategoryToContent ( cid, categoryid )");
                    CHECK(setTableVersion("categories", 106));
                    
                    user_version = 106;
                    hasChanges=true;
                }
                if(user_version < 107)
                {
                    qLog(Sql) << "Performing version 107 updates";
                    EXEC("delete from content");
                    EXEC("delete from contentProps");
                    EXEC("delete from mapCategoryToContent");
                    EXEC("delete from mimeTypeLookup");
                    EXEC("delete from locationLookup");
                    CHECK(setTableVersion("content", 107));
                    CHECK(setTableVersion("contentProps", 107));
                    CHECK(setTableVersion("mapCategoryToContent", 107));
                    CHECK(setTableVersion("mimeTypeLookup", 107));
                    CHECK(setTableVersion("locationLookup", 107));
                    user_version = 107;
                    hasChanges=true;
                }
                if(user_version < 108)
                {
                    qLog(Sql) << "Performing version 108 updates";
                    if(!db.record(QLatin1String("categories")).contains("flags"))
                        EXEC("ALTER TABLE categories ADD flags INTEGER");
                    CHECK(setTableVersion("categories", 108));
                    user_version = 108;
                    hasChanges=true;
                }
                if(user_version < 109)
                {
                    xsql.prepare(QLatin1String("update categories set flags=1 where categoryid=:id"));
                    xsql.bindValue(QLatin1String(":id"), QLatin1String("Business"));
                    EXEC(xsql);
                    xsql.bindValue(QLatin1String(":id"), QLatin1String("Personal"));
                    EXEC(xsql);
                    xsql.bindValue(QLatin1String(":id"), QLatin1String("SystemRingtones"));
                    EXEC(xsql);
                    CHECK(setTableVersion("categories", 109));
                    user_version = 109;
                    hasChanges=true;
                }
                if(user_version < 110)
                {
                    EXEC("ALTER TABLE content ADD uiNameSortOrder VARCHAR(100)");
                    EXEC("CREATE INDEX cNameSortOrderIndex ON content ( uiNameSortOrder )");

                    EXEC("CREATE TABLE databaseProperties("
                            "key NVARCHAR (255) NOT NULL, "
                            "property NVARCHAR (255) NOT NULL)");
                }
                CHECK(setTableVersion("content", 111));
                CHECK(setTableVersion("contentProps", 111));
                CHECK(setTableVersion("mapCategoryToContent", 111));
                CHECK(setTableVersion("mimeTypeLookup", 111));
                CHECK(setTableVersion("locationLookup", 111));
                CHECK(setTableVersion("categories", 111));
                CHECK(setTableVersion("databaseProperties",100));
                user_version = 110;
                hasChanges=true;

            }
            if (!db.tables().contains("categoryringtone"))
            {
                CHECK(ensureSchema("categoryringtone"));
                CHECK(setTableVersion("categoryringtone", 110));
            }
            if(!db.tables().contains("mimeTypeMapping"))
            {
                CHECK(ensureSchema("mimeTypeMapping"));
                CHECK(setTableVersion("mimeTypeMapping", 100));
            }
        }
        else
#endif // QTOPIA_CONTENT_INSTALLER
        {
            QStringList tableList;
            tableList << "categories" << "content" << "mapCategoryToContent" << "locationLookup" << "contentProps";
            CHECK(ensureSchema(tableList));
            foreach(const QString &tableName, tableList)
                CHECK(setTableVersion(tableName, 111));
            CHECK(setTableVersion("mimeTypeLookup", 111));
            CHECK(ensureSchema("categoryringtone"));
            CHECK(setTableVersion("categoryringtone", 110));
            CHECK(ensureSchema("mimeTypeMapping"));
            CHECK(setTableVersion("mimeTypeMapping", 100));

            if( !tables.contains("databaseProperties") )
            {
                EXEC("CREATE TABLE databaseProperties("
                        "key NVARCHAR (255) NOT NULL, "
                        "property NVARCHAR (255) NOT NULL)");
                CHECK(setTableVersion("databaseProperties",100));
            }

            hasChanges=true;
        }
        if(hasChanges)
        {
            if(!db.commit())
            {
                QString errString=QString("COMMIT: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
                db.rollback();
                db.close();
                qCritical(qPrintable(errString));
                return false;
            }
        }
        else
            db.rollback();

        CHECK(verifyLocale( db ));

    }

#ifndef QTOPIA_CONTENT_INSTALLER
    // do system-database only migrations
    if(databaseIds.contains(0))
    {
        db=QtopiaSql::instance()->systemDatabase();
        if(db.isOpen()==false)
        CHECK(db.open());
        if(!db.transaction())
        {
            QString errString=QString("BEGIN TRANSACTION: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
            db.close();
            qCritical(qPrintable(errString));
            return false;
        }

        PimMigrate pimmigrate(this);
        CHECK(pimmigrate.migrate());

        PhoneMigrate phonemigrate(this);
        CHECK(phonemigrate.migrate());

        if(!db.commit())
        {
            QString errString=QString("COMMIT: failed (%1, %2, %3)\n").arg(db.lastError().number()).arg(db.lastError().databaseText()).arg(db.lastError().driverText());
            db.rollback();
            db.close();
            qCritical(qPrintable(errString));
            return false;
        }
    }
#endif

    return true;
}

bool QDBMigrationEngine::verifyLocale( const QSqlDatabase &database )
{
    QString dbLanguage;
    QString dbCollation;
    {
        QSqlQuery localeQuery( database );

        CHECK(localeQuery.exec(QLatin1String("SELECT property FROM databaseProperties WHERE key='LANG'")));
        if(localeQuery.first())
            dbLanguage = localeQuery.value(0).toString();

        CHECK(localeQuery.exec(QLatin1String("SELECT property FROM databaseProperties WHERE key='LC_COLLATE'")));
        if(localeQuery.first())
            dbCollation = localeQuery.value(0).toString();

        QString envLanguage = QLatin1String( getenv("LANG") );
        QString envCollation = QLatin1String( getenv("LC_COLLATE") );

        if( envCollation.isEmpty() )
            envCollation = envLanguage;

        if( !dbLanguage.isEmpty() && envLanguage == dbLanguage && envCollation == dbCollation )
            return true;

        dbLanguage = envLanguage;
        dbCollation = envCollation;
    }

    QSqlQuery selectQuery( database );

    for( int offset = 0; ; offset += 100 )
    {
        selectQuery.prepare( QString( QLatin1String(
            "select content.cid, content.uiName, transFile.value, transContext.value "
            "from content "
                "left join contentProps as transFile on content.cid = transFile.cid and transFile.grp = 'Translation' and transFile.name = 'File' "
                "left join contentProps as transContext on content.cid = transContext.cid and transContext.grp = 'Translation' and transContext.name = 'Context' "
                "order by content.cid "
                "limit 100 offset %1" ) ).arg( offset ) );

        QVariantList ids;
        QVariantList names;

        EXEC(selectQuery);

        while( selectQuery.next() )
        {
            ids.append( selectQuery.value( 0 ) );

            QString name = selectQuery.value( 1 ).toString();
            QVariant translationFile = selectQuery.value( 2 );
            QVariant translationContext = selectQuery.value( 3 );

            if( !translationFile.isNull() && !translationContext.isNull() )
                name = Qtopia::dehyphenate( Qtopia::translate( translationFile.toString(), translationContext.toString(), name ) );

            names.append( transformString( name ) );
        }

        if( names.isEmpty() )
            return true;

        QSqlQuery updateQuery( database );
        updateQuery.prepare( QLatin1String(
            "update content "
            "set uiNameSortOrder = ? "
            "where cid = ?" ) );

        updateQuery.addBindValue( names );
        updateQuery.addBindValue( ids );

        CHECK(updateQuery.execBatch());

        EXEC(QLatin1String("DELETE FROM databaseProperties WHERE key = 'LANG'"));
        EXEC(QLatin1String("DELETE FROM databaseProperties WHERE key = 'LC_COLLATE'"));
        EXEC(QString(QLatin1String("INSERT INTO databaseProperties (key,property) VALUES( 'LANG', '%1')")).arg(dbLanguage));
        EXEC(QString(QLatin1String("INSERT INTO databaseProperties (key,property) VALUES( 'LC_COLLATE', '%1')")).arg(dbCollation));
    }
}

QByteArray QDBMigrationEngine::transformString( const QString &string ) const
{
    char buffer[ 1024 ];

    QByteArray localString = string.toLocal8Bit();

    if( strxfrm( buffer, localString.constData(), 1024 ) > 0 )
        return QByteArray( buffer );
    else
        return localString;
}

bool QDBMigrationEngine::checkIntegrity( const QSqlDatabase &database, bool printErrors )
{
    QSqlQuery query;

    query.prepare( "PRAGMA integrity_check" );

    if( query.exec() && query.next() )
    {
        if( query.value( 0 ).toString() == QLatin1String( "ok" ) )
        {
            return true;
        }
        else if( printErrors )
        {
            qWarning() << "Integrity check of" << database.databaseName() << "failed";
            do
            {
                qWarning() << query.value( 0 ).toString().toLatin1().constData();
            }
            while( query.next() );
        }
    }
    return false;
}

#ifndef QTOPIA_CONTENT_INSTALLER
MigrationEngineService::MigrationEngineService( QObject *parent )
        : QtopiaAbstractService( "DBMigrationEngine", parent )
{
    publishAll();
    QtopiaApplication::instance()->registerRunningTask("dbmigrate");
    unregistrationTimer.setSingleShot(true);
    unregistrationTimer.setInterval(60000);
    connect(&unregistrationTimer, SIGNAL(timeout()), this, SLOT(unregister()));
}

void MigrationEngineService::doMigrate( const QDSActionRequest &request )
{
    if(unregistrationTimer.isActive())
        unregistrationTimer.stop();
    unregistrationTimer.start();
    QDSActionRequest requestCopy( request );
    QString data=requestCopy.requestData().data();
    if(QDBMigrationEngine::instance()->doMigrate(data.split("\n"))==true)
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
    else
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
}

void MigrationEngineService::unregister()
{
    QtopiaApplication::instance()->unregisterRunningTask("dbmigrate");
}

void MigrationEngineService::ensureTableExists( const QDSActionRequest &request )
{
    if(unregistrationTimer.isActive())
        unregistrationTimer.stop();
    unregistrationTimer.start();
    QDSActionRequest requestCopy( request );
    QString table=requestCopy.requestData().data();
    QStringList script=QString(requestCopy.auxiliaryData().data()).split("\n");
    if(script.count() < 5)
    {
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
        return;
    }

    // set up the database connection...
    QSqlDatabase db=QSqlDatabase::addDatabase(script.first(), QLatin1String("dbmigrateconnection"));
    script.erase(script.begin());
    db.setDatabaseName(script.first());
    script.erase(script.begin());
    db.setUserName(script.first());
    script.erase(script.begin());
    db.setPassword(script.first());
    script.erase(script.begin());
    db.setHostName(script.first());
    script.erase(script.begin());
    if(!db.open())
    {
        qLog(Sql) << "failed to open database";
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
        return;
    }
    db.open();
    QDBMigrationEngine::instance()->setDatabase(db);

    QStringList tables = QDBMigrationEngine::instance()->database().tables();
    if (tables.contains(table, Qt::CaseInsensitive))
    {
        QDBMigrationEngine::instance()->setDatabase(QSqlDatabase());
        db=QSqlDatabase();
        QSqlDatabase::removeDatabase(QLatin1String("dbmigrateconnection"));
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
        return;
    }
    
    QTextStream ts(script.join("\n").toUtf8());
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    bool result = QDBMigrationEngine::instance()->loadSchema(ts, true);
    QDBMigrationEngine::instance()->setDatabase(QSqlDatabase());
    db.close();
    db=QSqlDatabase();
    QSqlDatabase::removeDatabase(QLatin1String("dbmigrateconnection"));
    if (result)
        requestCopy.respond(QDSData(QByteArray("Y"), QMimeType("text/x-dbm-qstring")));
    else
        requestCopy.respond(QDSData(QByteArray("N"), QMimeType("text/x-dbm-qstring")));
}


#endif
