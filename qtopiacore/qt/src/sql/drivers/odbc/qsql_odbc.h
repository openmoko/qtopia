/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_ODBC_H
#define QSQL_ODBC_H

#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif

#if defined (Q_OS_MAC) && (MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_3)
// assume we use iodbc on MACX
// comment next line out if you use a
// unicode compatible manager
# define Q_ODBC_VERSION_2
#endif

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_ODBC
#else
#define Q_EXPORT_SQLDRIVER_ODBC Q_SQL_EXPORT
#endif

#ifdef Q_OS_UNIX
#define HAVE_LONG_LONG 1 // force UnixODBC NOT to fall back to a struct for BIGINTs
#endif

#if defined(Q_CC_BOR)
// workaround for Borland to make sure that SQLBIGINT is defined
#  define _MSC_VER 900
#endif
#include <sql.h>
#if defined(Q_CC_BOR)
#  undef _MSC_VER
#endif

#ifndef Q_ODBC_VERSION_2
#include <sqlucode.h>
#endif

#include <sqlext.h>

QT_BEGIN_HEADER

class QODBCPrivate;
class QODBCDriverPrivate;
class QODBCDriver;
class QSqlRecordInfo;

class QODBCResult : public QSqlResult
{
public:
    QODBCResult(const QODBCDriver * db, QODBCDriverPrivate* p);
    virtual ~QODBCResult();

    bool prepare(const QString& query);
    bool exec();

    QVariant handle() const;

protected:
    bool fetchNext();
    bool fetchFirst();
    bool fetchLast();
    bool fetchPrior();
    bool fetch(int i);
    bool reset (const QString& query);
    QVariant data(int field);
    bool isNull(int field);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QODBCPrivate *d;
};

class Q_EXPORT_SQLDRIVER_ODBC QODBCDriver : public QSqlDriver
{
    Q_OBJECT
public:
    explicit QODBCDriver(QObject *parent=0);
    QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent=0);
    virtual ~QODBCDriver();
    bool hasFeature(DriverFeature f) const;
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QVariant handle() const;
    QString formatValue(const QSqlField &field,
                        bool trimStrings) const;
    bool open(const QString& db,
              const QString& user,
              const QString& password,
              const QString& host,
              int port,
              const QString& connOpts);

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    void init();
    bool endTrans();
    void cleanup();
    QODBCDriverPrivate* d;
};

QT_END_HEADER

#endif // QSQL_ODBC_H
