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
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QTextStream>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTimer>
#endif

class QDBMigrationEngine
{
public:
    int tableVersion(const QString &tableName);
    bool setTableVersion(const QString &tableName, int versionNum);
    bool doMigrate(const QStringList &args);
    bool copyTable(const QString &from, const QString &to);
    bool ensureSchema(const QString &table, bool transact=false) {return ensureSchema(QStringList() << table, transact);}
    bool ensureSchema(const QStringList &list, bool transact=false);
    void setDatabase(const QSqlDatabase& database) { db=database; }
    const QSqlDatabase& database() { return db; }
    bool loadSchema(QTextStream &ts, bool transact=false);
    bool check(bool result, int line, char *file);
    bool exec(QSqlQuery &query, int line, char *file);
    bool exec(const QString &query, int line, char *file);
    static QDBMigrationEngine *instance();
private:
    bool verifyLocale( const QSqlDatabase &database );
    QByteArray transformString( const QString &string ) const;

    QSqlDatabase db;
    bool failed;
};

#ifndef QTOPIA_CONTENT_INSTALLER
class MigrationEngineService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    MigrationEngineService( QObject *parent );
public slots:
    void doMigrate( const QDSActionRequest &request );
    void ensureTableExists( const QDSActionRequest &request );
private:
    QTimer unregistrationTimer;
private slots:
    void unregister();
};
#endif

#define CHECK(result) {if(QDBMigrationEngine::instance()->check((result), __LINE__, __FILE__) == false) return false; }
#define EXEC(query) {if(QDBMigrationEngine::instance()->exec((query), __LINE__, __FILE__) == false) return false; }

