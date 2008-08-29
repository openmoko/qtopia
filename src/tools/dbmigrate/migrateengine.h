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
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextStream>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <private/qtopiasqlmigrateplugin_p.h>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTimer>
#endif

#ifndef QTOPIA_CONTENT_INSTALLER
class QDBMigrationEngine : public QObject, public QtopiaSqlMigratePlugin
{
    Q_OBJECT
    Q_INTERFACES(QtopiaSqlMigratePlugin)
#else
class QDBMigrationEngine
{
#endif
public:
    QDBMigrationEngine();

    bool migrate(QSqlDatabase *database);

    int tableVersion(const QString &tableName);
    bool setTableVersion(const QString &tableName, int versionNum);
    bool copyTable(const QString &from, const QString &to);
    bool ensureSchema(const QString &table, bool transact=false) {return ensureSchema(QStringList() << table, transact);}
    bool ensureSchema(const QStringList &list, bool transact=false);
    void setDatabase(const QSqlDatabase& database) { db=database; }
    const QSqlDatabase& database() { return db; }
    bool loadSchema(QTextStream &ts, bool transact=false);
    bool check(bool result, int line, const char *file, const char *message);
    bool exec(QSqlQuery &query, int line, const char *file);
    bool exec(const QString &query, int line, const char *file);
    static QDBMigrationEngine *instance();
private:
    bool checkIntegrity( const QSqlDatabase &database, bool printErrors );
    bool verifyLocale( const QSqlDatabase &database );
    QByteArray transformString( const QString &string ) const;

    QDBMigrationEngine *mi;

    QSqlDatabase db;
    bool failed;
};


#define CHECK(result) {if(mi->check((result), __LINE__, __FILE__, #result) == false) return false; }
#define EXEC(query) {if(mi->exec((query), __LINE__, __FILE__) == false) return false; }

