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

#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#include <QtSql/qsql.h>
#include <QtSql/qsqldatabase.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResult;
class QSqlRecord;
template <class Key, class T> class QMap;
class QSqlQueryPrivate;

class Q_SQL_EXPORT QSqlQuery
{
public:
    QSqlQuery(QSqlResult *r);
    QSqlQuery(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
    explicit QSqlQuery(QSqlDatabase db);
    QSqlQuery(const QSqlQuery& other);
    QSqlQuery& operator=(const QSqlQuery& other);
    ~QSqlQuery();

    bool isValid() const;
    bool isActive() const;
    bool isNull(int field) const;
    int at() const;
    QString lastQuery() const;
    int numRowsAffected() const;
    QSqlError lastError() const;
    bool isSelect() const;
    int size() const;
    const QSqlDriver* driver() const;
    const QSqlResult* result() const;
    bool isForwardOnly() const;
    QSqlRecord record() const;

    void setForwardOnly(bool forward);
    bool exec(const QString& query);
    QVariant value(int i) const;

    bool seek(int i, bool relative = false);
    bool next();
    bool previous();
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT bool prev() { return previous(); }
#endif
    bool first();
    bool last();

    void clear();

    // prepared query support
    bool exec();
    enum BatchExecutionMode { ValuesAsRows, ValuesAsColumns };
    bool execBatch(BatchExecutionMode mode = ValuesAsRows);
    bool prepare(const QString& query);
    void bindValue(const QString& placeholder, const QVariant& val,
                   QSql::ParamType type = QSql::In);
    void bindValue(int pos, const QVariant& val, QSql::ParamType type = QSql::In);
    void addBindValue(const QVariant& val, QSql::ParamType type = QSql::In);
    QVariant boundValue(const QString& placeholder) const;
    QVariant boundValue(int pos) const;
    QMap<QString, QVariant> boundValues() const;
    QString executedQuery() const;
    QVariant lastInsertId() const;

private:
    QSqlQueryPrivate* d;
};

QT_END_HEADER

#endif // QSQLQUERY_H
