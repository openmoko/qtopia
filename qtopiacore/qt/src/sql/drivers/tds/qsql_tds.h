/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_TDS_H
#define QSQL_TDS_H

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>
#include <QtSql/private/qsqlcachedresult_p.h>

#ifdef Q_OS_WIN32
#define DBNTWIN32 // indicates 32bit windows dblib
#include <QtCore/qt_windows.h>
#include <sqlfront.h>
#include <sqldb.h>
#define CS_PUBLIC
#else
#include <sybfront.h>
#include <sybdb.h>
#endif //Q_OS_WIN32

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_TDS
#else
#define Q_EXPORT_SQLDRIVER_TDS Q_SQL_EXPORT
#endif

QT_BEGIN_HEADER

class QTDSDriverPrivate;
class QTDSResultPrivate;
class QTDSDriver;

class QTDSResult : public QSqlCachedResult
{
public:
    explicit QTDSResult(const QTDSDriver* db);
    ~QTDSResult();
    QVariant handle() const;

protected:
    void cleanup();
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    bool gotoNext(QSqlCachedResult::ValueCache &values, int index);
    QSqlRecord record() const;

private:
    QTDSResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_TDS QTDSDriver : public QSqlDriver
{
    Q_OBJECT
    friend class QTDSResult;
public:
    explicit QTDSDriver(QObject* parent = 0);
    QTDSDriver(LOGINREC* rec, const QString& host, const QString &db, QObject* parent = 0);
    ~QTDSDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
               const QString & user,
               const QString & password,
               const QString & host,
               int port,
               const QString& connOpts);
    void close();
    QStringList tables(QSql::TableType) const;
    QSqlResult *createResult() const;
    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString& tablename) const;

    QString formatValue(const QSqlField &field,
                         bool trimStrings) const;
    QVariant handle() const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    void init();
    QTDSDriverPrivate *d;
};

QT_END_HEADER

#endif // QSQL_TDS_H
