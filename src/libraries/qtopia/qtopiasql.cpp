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

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QSettings>
#include <QTextStream>
#include <QTextCodec>
#include <QThread>
#include <QThreadStorage>
#include <QMutex>
#include <QTemporaryFile>
#include <QContentSet>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <QDSServiceInfo>
#include <QDSAction>
#endif
#include <qtopiasql.h>
#include <qtopianamespace.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qpluginmanager.h>
#include <qtopialog.h>
#include <qstorage.h>
#include <qtopia/private/qcontent_p.h>
#include <QtopiaIpcEnvelope>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

// Set this to control number of backups files rotated on upgrading
// database.  If set to 0 no backups are made - thus import of old data is
// not possible.
#define QTOPIA_SQL_MAX_BACKUPS 3

#define Q_USE_SQLITE
// #define Q_USE_MIMER
// #define Q_USE_MYSQL

// FIXME this should work on Windows too
#ifndef Q_OS_WIN32
#define USE_LOCALE_AWARE
#endif

#ifdef USE_LOCALE_AWARE
#ifdef Q_USE_SQLITE
#include <qsql_sqlite.h>
#include <sqlite3.h>
#endif //Q_USE_SQLITE
#endif

class QtopiaSqlPrivate : public QObject {
    Q_OBJECT
public:
    QtopiaSqlPrivate();
    ~QtopiaSqlPrivate() {}

    QString databaseFile( const QString &path );
private:
    QString type;
    QString name;
    QString user;
    QString password;
    QString hostname;
    static QSqlDatabase *defaultConn;
    static QHash<QtopiaDatabaseId, QSqlDatabase> masterAttachedConns;
    static QHash<QtopiaDatabaseId, QString> dbPaths;
    static QThreadStorage<QHash<QtopiaDatabaseId, QSqlDatabase> *> dbs;
    static QThreadStorage<QHash<QtopiaDatabaseId, QString> *> connectionNames;
    static QMutex guardMutex;
    QStorageMetaInfo smi;
    static quint32 conId;
    void installSorting( QSqlDatabase &db);
    friend class QtopiaSql;
private slots:
    void disksChanged ();
    void threadTerminated();
    void cardMessage(const QString &message,const QByteArray &data);
};

QtopiaSqlPrivate *QtopiaSql::d = 0;
QSqlDatabase *QtopiaSqlPrivate::defaultConn = 0;
QThreadStorage<QHash<QtopiaDatabaseId, QSqlDatabase> *> QtopiaSqlPrivate::dbs;
QHash<QtopiaDatabaseId, QSqlDatabase> QtopiaSqlPrivate::masterAttachedConns;
QHash<QtopiaDatabaseId, QString> QtopiaSqlPrivate::dbPaths;
QThreadStorage<QHash<QtopiaDatabaseId, QString> *> QtopiaSqlPrivate::connectionNames;
QMutex QtopiaSqlPrivate::guardMutex(QMutex::Recursive);
quint32 QtopiaSqlPrivate::conId = 0;

QtopiaSqlPrivate::QtopiaSqlPrivate ()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    if ( qApp->type() == QApplication::GuiServer ) {
        // The server handles mtab updates differently
        QtopiaChannel *channel = new QtopiaChannel( "QPE/Card", this );
        connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
                 this, SLOT(cardMessage(const QString&,const QByteArray&)) );
    } else {
        connect(&smi, SIGNAL(disksChanged()), this, SLOT(disksChanged()));
    }
#endif
}

void QtopiaSqlPrivate::cardMessage(const QString &message,const QByteArray &)
{
    // The mtab has changed. Manually update our filesystems and then handle this before
    // passing the message on to other processes.
    if ( message == "mtabChanged()" ) {
        smi.update();
        disksChanged();
        QtopiaIpcEnvelope("QPE/QStorage", "updateStorage()");
    }
}

void QtopiaSqlPrivate::disksChanged ()
{
#ifndef QTOPIA_CONTENT_INSTALLER
    // update the "databases" array after rescanning QStorageMetaInfo
    qLog(Sql) << "disks changed... update database info";
    QFileSystemFilter f;
    f.content = QFileSystemFilter::Set;
    foreach ( const QFileSystem *fs, smi.fileSystems(&f, false) ) {
        // Have I seen this database already?
        if ( fs->isConnected() && QtopiaSql::databaseIdForPath(fs->path()) == 0 ) {
            // qLog(Sql) << "fs->path() =" << fs->path() << "fs->isRemovable() =" << fs->isRemovable() << "fs->isWritable() =" << fs->isWritable();
            if ( !fs->isRemovable() && fs->isWritable() ) {
                // Simple case, just attach normally
                QtopiaSql::attachDB(fs->path());
            } else {
                QString dbPath = databaseFile(fs->path());
                bool dbExists = QFile::exists(dbPath);
                // If we're not writeable use a temporary file (sqlite can't open read-only files)
                bool useTemp = !fs->isWritable();
                // If we're removable, test that the file can be opened.
                // Read-only SD cards still show as writeable.
                if ( fs->isRemovable() && dbExists ) {
                    // try and write the database.
                    QString tmpfilename=fs->path()+QLatin1String("/.qtopia_write_test");
                    if(QFile::exists(tmpfilename))
                        QFile::remove(tmpfilename);
                    QTemporaryFile tmpfile(tmpfilename);
                    if( !tmpfile.open() ) {
                        qLog(Sql) << "test file open failed";
                        useTemp = true;
                    }
                    else if( !tmpfile.write("test", 4) == 4 || !tmpfile.flush() ) {
                        qLog(Sql) << "test file write failed";
                        useTemp = true;
                    }
                }
                // qLog(Sql) << "dbExists =" << dbExists << "useTemp =" << useTemp;
                if ( useTemp ) {
                    QString origPath = dbPath;

                    // Use a file in /tmp (we know it's writeable)
                    // This is "unique" by path and lets apps share read-only databases
                    QString path = QDir::cleanPath(Qtopia::tempDir()+fs->path());
                    QDir().mkpath(path);    // ensure the path exists
                    dbPath = databaseFile(path);

                    // Copy the database (if it exists at the source and not at the dest)
                    // The server does this before any other apps get a chance to notice this file
                    if ( dbExists && qApp->type() == QApplication::GuiServer ) {
                        // If it already exists, unlink it (just in case anyone has it open)
                        if ( QFileInfo(dbPath).exists() ) {
                            QFile::remove(dbPath);
                        }
                        if( !QFile::copy(origPath, dbPath) ) {
                            QFile::remove(dbPath);
                            continue;
                        }
                    }
                }

                // Finally, attach to the database
                qLog(Sql) << "Attaching database" << dbPath << "for path" << fs->path();
                QtopiaSql::attachDB(fs->path(), dbPath);
            }
            // Tell ContentServer to scan for documents (if this location contains them)
            if ( qApp->type() == QApplication::GuiServer && fs->documents() )
                QContentSet::scan(fs->path());
        } else if ( !fs->isConnected() && !fs->prevPath().isEmpty() ) {
            // Better late than never... detach this database now
            qLog(Sql) << "Detaching database for path" << fs->prevPath();
            QtopiaSql::detachDB(fs->prevPath());
        }
    }
#endif
}

void QtopiaSqlPrivate::threadTerminated()
{
    QThread *senderThread=qobject_cast<QThread *>(sender());
    if (senderThread != QThread::currentThread())
    {
        qWarning() << "Terminated not called from current thread!!";
        QObject::disconnect(senderThread, SIGNAL(finished()), this, SLOT(threadTerminated()));
        QObject::disconnect(senderThread, SIGNAL(terminated()), this, SLOT(threadTerminated()));
        return;
    }
    //qLog(Sql) << "thread Terminated call for thread:" << QThread::currentThreadId();
    if (QtopiaSqlPrivate::dbs.hasLocalData())
        QtopiaSqlPrivate::dbs.localData()->clear();
    if (QtopiaSqlPrivate::connectionNames.hasLocalData())
    {
        foreach(QString conname, *QtopiaSqlPrivate::connectionNames.localData())
            QSqlDatabase::removeDatabase(conname);
        QtopiaSqlPrivate::connectionNames.setLocalData(NULL);
    }
    QObject::disconnect(senderThread, SIGNAL(finished()), this, SLOT(threadTerminated()));
    QObject::disconnect(senderThread, SIGNAL(terminated()), this, SLOT(threadTerminated()));
}

QString QtopiaSqlPrivate::databaseFile(const QString &path)
{
    QString file = QDir::cleanPath(path+QLatin1String("/")+QFileInfo(name).fileName());
    return file;
}

void QtopiaSqlPrivate::installSorting( QSqlDatabase &db)
{
#if defined(Q_USE_SQLITE) && defined(USE_LOCALE_AWARE)
        int sqliteLocaleAwareCompare(void *, int ll, const void *l, int rl, const void *r);
        QVariant v = db.driver()->handle();
        if (v.isValid() && strcmp(v.typeName(), "sqlite3*") == 0) {
            // v.data() returns a pointer to the handle
            sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
            if (handle != 0) { // check that it is not NULL
                int result = sqlite3_create_collation(
                        handle,
                        "localeAwareCompare",
                        SQLITE_UTF16, // ANY would be nice, but we only encode in 16 anyway.
                        0,
                        sqliteLocaleAwareCompare);
                if (result != SQLITE_OK)
                    qLog(Sql) << "Could not add string collation function: " << result;
            } else {
                qLog(Sql) << "Could not get sqlite handle";
            }
        } else {
            qLog(Sql) << "handle variant returned typename " << v.typeName();
        }
#endif
}

void QtopiaSql::openDatabase()
{
    initPrivate();
    if (!QtopiaSqlPrivate::defaultConn) {
        QtopiaSqlPrivate::defaultConn = connectDatabase(QLatin1String(QSqlDatabase::defaultConnection));
        init(*QtopiaSqlPrivate::defaultConn);
        QtopiaSqlPrivate::masterAttachedConns[0] = *QtopiaSqlPrivate::defaultConn;
        d->disksChanged();
    }
}

void QtopiaSql::closeDatabase()
{
    delete QtopiaSqlPrivate::defaultConn;
    QtopiaSqlPrivate::defaultConn = 0;
}

#ifdef Q_USE_SQLITE
#ifdef USE_LOCALE_AWARE
// QString is ushort/utf16 internally.
// so save 2*malloc+memcpy if use
// textcodec->fromUnicode direct, along with strcoll.
int sqliteLocaleAwareCompare(void *, int ll, const void *l, int rl, const void *r)
{
    QString left = QString::fromUtf16((const ushort *)l, ll/2);
    QString right = QString::fromUtf16((const ushort *)r, rl/2);
    //qLog(Sql) << "comparing:" << left << "with" << right << "result" << QString::localeAwareCompare(left, right);
    return QString::localeAwareCompare(left, right);
}

int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    return QString::localeAwareCompare(l, r);
}
#else

/* copied straight from the sqlite3 source code */
const unsigned char sqlite3UpperToLower[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
     36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
     54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
    162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
    198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
    234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
    252,253,254,255
};
#define UpperToLower sqlite3UpperToLower

/*
** Some systems have stricmp().  Others have strcasecmp().  Because
** there is no consistency, we will define our own.
*/
int sqlite3StrICmp(const char *zLeft, const char *zRight){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return UpperToLower[*a] - UpperToLower[*b];
}
int sqlite3StrNICmp(const char *zLeft, const char *zRight, int N){
  register unsigned char *a, *b;
  a = (unsigned char *)zLeft;
  b = (unsigned char *)zRight;
  while( N-- > 0 && *a!=0 && UpperToLower[*a]==UpperToLower[*b]){ a++; b++; }
  return N<0 ? 0 : UpperToLower[*a] - UpperToLower[*b];
}

/* sqlite default text comparision operation, memcmp
    http://www.sqlite.org/datatype3.html
 */
int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    return sqlite3StrICmp(l.toUtf8().constData(), r.toUtf8().constData());
}
#endif
#endif
#ifdef Q_USE_MIMER
int QtopiaSql::stringCompare(const QString &l, const QString &r)
{
    // TODO MIMER
    // fix to use Mimer collation library
    return l < r;
}
#endif

QSqlDatabase *QtopiaSql::connectDatabase(const QString &connName)
{
    qLog(Sql) << "Trying to connect database with connection name:" << connName;
    loadConfig();

    QSqlDatabase *db = new QSqlDatabase(QSqlDatabase::addDatabase(d->type, connName));
    db->setDatabaseName(d->name);
    db->setUserName(d->user);
    db->setHostName(d->hostname);
    db->setPassword(d->password);
    if (!db->open())
    {
        qWarning() << "Could not connect" << d->name << db->lastError().text();
        delete db;
        return NULL;
    }

#if defined(Q_USE_SQLITE)
    // Execute these if for no other reason than to flush the SQLite schema
    QSqlQuery xsql( *db );
    xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
    xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif

    return db;
}

void QtopiaSql::init(QSqlDatabase &db, bool force)
{
    Q_UNUSED(db);   // used to be used, may be used again in the future.
    static bool doneInit = false;
    if (!doneInit || force) {
        d->installSorting(db);
        doneInit = true;

    }
}

void QtopiaSql::loadConfig(const QString &type, const QString &name, const QString &user)
{
    initPrivate();
    d->type = type;
    d->name = name;
    d->user = user;
    closeDatabase();
    openDatabase();
}

void QtopiaSql::loadConfig()
{
#ifdef Q_USE_SQLITE
    // Qtopia desktop top
    if (d && d->type.isEmpty())
    {
        d->type = QLatin1String("QSQLITE");
        d->name = Qtopia::applicationFileName(QLatin1String("Qtopia"), QLatin1String("qtopia_db.sqlite"));
    }
#endif
#ifdef Q_USE_MIMER
    d->type = QLatin1String("QODBC");
    d->name = QLatin1String("qtopia");
    d->user = QLatin1String("SYSADM");
    d->password = QLatin1String("lfer64"); // ok, not so good
    d->hostname = QLatin1String("10.1.1.22");
    //d->port = 1360;
#endif
#ifdef Q_USE_MYSQL
    d->type = QLatin1String("QMYSQL");
    d->name = QLatin1String("qtopia");
    d->user = QLatin1String("root");
#endif
}

void QtopiaSql::saveConfig()
{
}

void QtopiaSql::initPrivate()
{
    QMutexLocker guard(&QtopiaSqlPrivate::guardMutex);
    if (!d)
        d = new QtopiaSqlPrivate;
}

QSqlError QtopiaSql::exec(const QString &query, QSqlDatabase& db, bool inTransaction)
{
    QSqlError result = QSqlError();
    QSqlQuery qry(db);
    qry.prepare(query);
    if (inTransaction && db.driver()->hasFeature(QSqlDriver::Transactions))
        if(!db.driver()->beginTransaction())
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing beginTransaction:" << db.lastError();
    QtopiaSql::logQuery( qry );
    if(qry.exec() == false)
    {
#ifdef Q_USE_SQLITE
        int loopcount = 0;
        while (qry.lastError().number() == 5 && loopcount < 5)
        {
            Qtopia::usleep(5000);
            qry.exec(query);
            loopcount++;
        }
#endif
        result = qry.lastError();
    }
    else
        result = QSqlError();
    if (inTransaction && db.driver()->hasFeature(QSqlDriver::Transactions))
    {
        if (result.type() == QSqlError::NoError)
        {
            if(!db.driver()->commitTransaction())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing commitTransaction:" << result;
            }
        }
        else
        {
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing query (" << query << ") error:" << result << ", rolling back";
            if(!db.driver()->rollbackTransaction())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing rollbackTransaction:" << result;
            }
        }
    }
    return result;
}

QSqlError QtopiaSql::exec(QSqlQuery &query, QSqlDatabase& db, bool inTransaction)
{
    QSqlError result = QSqlError();
    if (inTransaction && query.driver()->hasFeature(QSqlDriver::Transactions))
        if(!db.transaction())
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing beginTransaction:" << db.lastError();
    QtopiaSql::logQuery( query );
    if(query.exec() == false)
    {
#ifdef Q_USE_SQLITE
        int loopcount = 0;
        while (query.lastError().number() == 5 && loopcount < 5)
        {
            Qtopia::usleep(5000);
            query.exec();
            loopcount++;
        }
#endif
        result = query.lastError();
    }
    else
        result = QSqlError();
    if (inTransaction && query.driver()->hasFeature(QSqlDriver::Transactions))
    {
        if (result.type() == QSqlError::NoError)
        {
            if(!db.commit())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing commitTransaction:" << result;
            }
        }
        else
        {
            qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing query (" << query.lastQuery() << ") error:" << result << ", rolling back";
            if(!db.rollback())
            {
                result = db.lastError();
                qLog(Sql) << __PRETTY_FUNCTION__ << "Error executing rollbackTransaction:" << result;
            }
        }
    }
    return result;
}


/*!
  Returns a reference to the qtopia database object (used for
  settings storage, doc manager, etc).  There is a separate set of database objects
  created per thread.
  */
QSqlDatabase &QtopiaSql::database(const QtopiaDatabaseId& id)
{
    QMutexLocker guard(&QtopiaSqlPrivate::guardMutex);
    if (!QtopiaSqlPrivate::dbs.hasLocalData()) {
        QtopiaSql::openDatabase();
        QtopiaSqlPrivate::dbs.setLocalData(new QHash<QtopiaDatabaseId, QSqlDatabase>);
        QtopiaSqlPrivate::connectionNames.setLocalData(new QHash<QtopiaDatabaseId, QString>);
        QObject::connect(QThread::currentThread(), SIGNAL(finished()), d, SLOT(threadTerminated()), Qt::DirectConnection);
        QObject::connect(QThread::currentThread(), SIGNAL(terminated()), d, SLOT(threadTerminated()), Qt::DirectConnection);
    }
    QHash<QtopiaDatabaseId, QSqlDatabase> *dbs=QtopiaSqlPrivate::dbs.localData();
    if (!QtopiaSqlPrivate::masterAttachedConns.contains(id) && dbs->contains(id))
    {
        // remove the connection, it's stale.
        qWarning() << "Stale database handle, returning the system database instead";
        dbs->remove(id);
        return database(0);
    }
    else if(!QtopiaSqlPrivate::masterAttachedConns.contains(id))
    {
        qWarning() << "Database handle doesnt exist in the master list, returning the system database";
        if(id == 0)
            qFatal("Should not be requesting the system database from this location in the code");
        return database(0);
    }
    else if (!dbs->contains(id))
    {
        QString connName = QString("QtopiaSqlDB%1").arg(QtopiaSqlPrivate::conId++);
        // copy the masterAttachedCons version, and register the connname in the list
        QSqlDatabase &from=QtopiaSqlPrivate::masterAttachedConns[id];
        QSqlDatabase db=QSqlDatabase::addDatabase(from.driverName(), connName);
        db.setDatabaseName(from.databaseName());
        db.setUserName(from.userName());
        db.setPassword(from.password());
        db.setHostName(from.hostName());
        db.setPort(from.port());
        db.setConnectOptions(from.connectOptions());
        db.open();
        d->installSorting(db);
        QSqlQuery xsql( db );
#if defined(Q_USE_SQLITE)
        xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
        xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif
        dbs->insert(id, db);
        QtopiaSqlPrivate::connectionNames.localData()->insert(id, connName);
    }

    return (*dbs)[id];
}

QString QtopiaSql::escapeString(const QString &input)
{
    // add more changes as needed.
    static QLatin1String singleQuote("'"), doubleQuote("''");
    QString result=input;
    result.replace(singleQuote, doubleQuote);
    return result;
}

void QtopiaSql::attachDB(const QString& path, const QString& dbPath)
{
    qLog(Sql) << "Attaching database for path" << path << "with the db located at" << dbPath;
    QMutexLocker guard(&QtopiaSqlPrivate::guardMutex);
    // add database to default connections list.
    if(path.isEmpty() || dbPath.isEmpty() || QtopiaSqlPrivate::defaultConn == NULL || dbPath == QtopiaSqlPrivate::defaultConn->databaseName())
        return;
    QtopiaDatabaseId dbid = databaseIdForDatabasePath(dbPath);
    if(!QtopiaSqlPrivate::masterAttachedConns.contains(dbid))
    {
        if (!QtopiaSqlPrivate::dbs.hasLocalData()) {
            QMutexLocker guard(&QtopiaSqlPrivate::guardMutex);
            QtopiaSqlPrivate::dbs.setLocalData(new QHash<QtopiaDatabaseId, QSqlDatabase>);
            QtopiaSqlPrivate::connectionNames.setLocalData(new QHash<QtopiaDatabaseId, QString>);
            QObject::connect(QThread::currentThread(), SIGNAL(finished()), d, SLOT(threadTerminated()), Qt::DirectConnection);
            QObject::connect(QThread::currentThread(), SIGNAL(terminated()), d, SLOT(threadTerminated()), Qt::DirectConnection);
        }

#ifndef QTOPIA_CONTENT_INSTALLER
        // ensure the database has the correct definition
        if ( qApp->type() == QApplication::GuiServer)
        {
            static QMutex mutex;
            QMutexLocker lock(&mutex);
            qLog(Sql) << QDateTime::currentDateTime () << "before doMigrate service call";
            QDSServiceInfo service( "doMigrate", "DBMigrationEngine" );
            QDSAction action( service );
            QDSData request(dbPath, QMimeType("text/x-dbm-qstring"));
            int rcode = action.exec(request);
            if ( /*rcode != QDSAction::Complete && */rcode != QDSAction::CompleteData) {
                qWarning() << QDateTime::currentDateTime () << "Couldn't use QDS service doMigrate from DBMigrationEngine, not attaching database"  << dbPath << "\nError:" << action.errorMessage();
                qLog(Sql) << "doMigrate: Error =" << action.errorMessage();
                return;
            }
            qLog(Sql) << QDateTime::currentDateTime () << "after service call";
            QString result=action.responseData().data();
            qLog(Sql) << "doMigrate: action.responseData().data() =" << result;
            if(result != "Y") {
                qWarning() << "QDS service doMigrate from DBMigrationEngine failed, not attaching database" << dbPath;
                return;
            }
        }
#endif

        QString connName = QString("QtopiaSqlDB%1").arg(QtopiaSqlPrivate::conId++);
        // copy the masterAttachedCons version, and register the connname in the list
        QHash<QtopiaDatabaseId, QSqlDatabase> *dbs=QtopiaSqlPrivate::dbs.localData();
        QSqlDatabase db=QSqlDatabase::addDatabase(d->type, connName);
        dbs->insert(dbid, db);
        db.setDatabaseName(dbPath);
        db.setUserName(d->user);
        db.setPassword(d->password);
        db.setHostName(d->hostname);
        if(!db.open())
        {
            qLog(Sql) << "!! Failed to open database:" << dbPath << db.lastError();
            return;
        }

        d->installSorting(db);

        QSqlQuery xsql( db );
        xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
        xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
        // Once temp_store is "memory" the temp_store_directory has no effect
        // xsql.exec(QLatin1String("PRAGMA temp_store_directory = '/tmp';"));
        QtopiaSqlPrivate::connectionNames.localData()->insert(dbid, connName);
        QtopiaSqlPrivate::dbPaths.insert(dbid, path);
        QtopiaSqlPrivate::masterAttachedConns.insert(dbid, db);
        QtopiaSql::init(db, true);
        if ( qApp->type() == QApplication::GuiServer )
            QtopiaIpcEnvelope se("QPE/System", "resetContent()");
#ifndef QTOPIA_CONTENT_INSTALLER
        else
            QContentUpdateManager::instance()->requestRefresh();
#endif
    }
    else
        qLog(Sql) << "tried to attach an already attached database:" << dbPath;
}

void QtopiaSql::attachDB(const QString& path)
{
    QtopiaSql::attachDB(path, d->databaseFile(path));
}

void QtopiaSql::detachDB(const QString& path)
{
    qLog(Sql) << "Detaching database at" << path;
    QMutexLocker guard(&QtopiaSqlPrivate::guardMutex);
    QtopiaDatabaseId dbid = databaseIdForPath(path);
    if(QtopiaSqlPrivate::dbPaths.contains(dbid) && dbid != 0)
    {
        QString dbPath=QtopiaSqlPrivate::masterAttachedConns[dbid].databaseName();
        QtopiaSqlPrivate::masterAttachedConns.remove(dbid);
        QtopiaSqlPrivate::dbPaths.remove(dbid);
        if(QtopiaSqlPrivate::dbs.hasLocalData())
            QtopiaSqlPrivate::dbs.localData()->remove(dbid);
        if(QtopiaSqlPrivate::connectionNames.hasLocalData())
        {
            QSqlDatabase::removeDatabase(QtopiaSqlPrivate::connectionNames.localData()->value(dbid));
            QtopiaSqlPrivate::connectionNames.localData()->remove(dbid);
        }
        // todo: if database itself is located in the temp directory, then delete it.
        if(dbPath.startsWith(Qtopia::tempDir())) {
            QFile::remove(dbPath);
#ifdef Q_OS_UNIX
            sync();
#endif
            
        }
        if ( qApp->type() == QApplication::GuiServer )
            QtopiaIpcEnvelope se("QPE/System", "resetContent()");
#ifndef QTOPIA_CONTENT_INSTALLER
        else
            QContentUpdateManager::instance()->requestRefresh();
#endif
        // todo: tell the scanner which dbs to invalidate...
    }
    else
        qLog(Sql) << "tried to detach an invalid database path mapping:" << path;
}

const QList<QSqlDatabase> QtopiaSql::databases()
{
    openDatabase();

    database(0);    // to force at least one entry to be loaded (0 being the system database of course).
    foreach(QtopiaDatabaseId id, QtopiaSql::databaseIds())
        database(id);
    return QtopiaSqlPrivate::dbs.localData()->values();
}

QtopiaDatabaseId QtopiaSql::databaseIdForPath(const QString& path)
{
    openDatabase();

    for(QHash<QtopiaDatabaseId, QString>::iterator i=QtopiaSqlPrivate::dbPaths.begin(); i != QtopiaSqlPrivate::dbPaths.end(); i++)
    {
        if(i.value() != QLatin1String("/"))
            if (i.key() != 0 && path.startsWith(i.value()))
                return i.key();
    }
    // fall through. if it's not under a registered path, then use the system database id(0)
    return 0;
}

QtopiaDatabaseId QtopiaSql::databaseIdForDatabasePath(const QString& dbPath)
{
    openDatabase();
    if(database(0).databaseName() == QDir::cleanPath(dbPath))
        return 0;
    else
        return qHash(QDir::cleanPath(dbPath));
}

const QList<QtopiaDatabaseId> QtopiaSql::databaseIds()
{
    openDatabase();

    return QtopiaSqlPrivate::masterAttachedConns.keys().toSet().toList();
}

QSqlDatabase &QtopiaSql::systemDatabase()
{
    return database(0);
}

bool QtopiaSql::isDatabase(const QString &path)
{
    openDatabase();

    qLog(Sql) << "QtopiaSql::isDatabase("<< path << ")";
    foreach(const QSqlDatabase &db, QtopiaSqlPrivate::masterAttachedConns)
    {
        qLog(Sql) << "db.databaseName:" << db.databaseName();
        if(path == db.databaseName() || path == db.databaseName() + QLatin1String("-journal"))
           return true;
    }
    return false;
}

QString QtopiaSql::databasePathForId(const QtopiaDatabaseId& id)
{
    openDatabase();

    QString result;
    if(id != 0 && QtopiaSqlPrivate::dbPaths.contains(id))
        result = QtopiaSqlPrivate::dbPaths.value(id);
    return result;
}

void QtopiaSql::logQuery(const QSqlQuery &q)
{
    if (qLogEnabled(Sql))
    {
        qLog(Sql) << "executing:" << q.lastQuery();
        if (!q.boundValues().empty())
            qLog(Sql) << "   params:" << q.boundValues();
    }
}

QSqlDatabase QtopiaSql::applicationSpecificDatabase(const QString &appname)
{
    QSqlDatabase db;
#ifdef Q_USE_SQLITE
    openDatabase();
    QString connName = QString("QtopiaSqlDB%1").arg(QtopiaSqlPrivate::conId++);
    // copy the masterAttachedCons version, and register the connname in the list
    db=QSqlDatabase::addDatabase(d->type, connName);
    db.setDatabaseName(Qtopia::applicationFileName(appname, QLatin1String("qtopia_db.sqlite")));
    db.setUserName(d->user);
    db.setPassword(d->password);
    db.setHostName(d->hostname);
    db.open();
    d->installSorting(db);
    QSqlQuery xsql( db );
    xsql.exec(QLatin1String("PRAGMA synchronous = OFF"));   // full/normal sync is safer, but by god slower.
    xsql.exec(QLatin1String("PRAGMA temp_store = memory"));
#endif

    return db;
}

bool QtopiaSql::ensureTableExists(const QString &tableName, QSqlDatabase &db )
{
#ifndef QTOPIA_CONTENT_INSTALLER
    loadConfig();
    static QMutex mutex;
    QMutexLocker lock(&mutex);
    if(db.tables().contains(tableName))
        return true;
    qLog(Sql) << QDateTime::currentDateTime () << "before ensureTableExists service call";
    QDSServiceInfo service( "ensureTableExists", "DBMigrationEngine" );
    QDSAction action( service );
    QDSData request(tableName, QMimeType("text/x-dbm-qstring"));
    QFile data(QLatin1String(":/QtopiaSql/") + db.driverName() + QLatin1String("/") + tableName);
    if (!data.open(QIODevice::ReadOnly))
    {
        qLog(Sql) << "QtopiaSql::ensureTableExists: resource" << tableName << "not found";
        return false;
    }

    QTextStream ts(&data);
    // read assuming utf8 encoding.
    ts.setCodec(QTextCodec::codecForName("utf8"));
    ts.setAutoDetectUnicode(true);

    QStringList script;
    script << db.driverName();
    script << db.databaseName();
    script << db.userName();
    script << db.password();
    script << db.hostName();
    script << ts.readAll().split("\n");

    db.close();
    int rcode = action.exec(request, script.join("\n").toUtf8());
    if ( rcode != QDSAction::CompleteData) {
        qWarning() << QDateTime::currentDateTime () << "Couldn't use QDS service ensureTableExists from DBMigrationEngine\nError:" << action.errorMessage();
        qLog(Sql) << "ensureTableExists: Error =" << action.errorMessage();
        db.open();
        return false;
    }
    qLog(Sql) << QDateTime::currentDateTime () << "after service call";
    QString result=action.responseData().data();
    qLog(Sql) << "ensureTableExists: action.responseData().data() =" << result;
    if(result != "Y") {
        qWarning() << "QDS service ensureTableExists from DBMigrationEngine failed";
        db.open();
        return false;
    }
    db.open();
    return true;
#else
    Q_UNUSED(tableName);
    Q_UNUSED(db);
    return false;
#endif
}

bool QtopiaSql::ensureTableExists(const QStringList &tableNames, QSqlDatabase&db )
{

    foreach(const QString table, tableNames)
    {
        if(ensureTableExists(table, db) == false)
            return false;
    }
    return true;
}

#include "qtopiasql.moc"
