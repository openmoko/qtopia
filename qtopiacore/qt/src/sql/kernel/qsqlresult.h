/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtSql/qsql.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class QString;
class QSqlRecord;
template <typename T> class QVector;
class QVariant;
class QSqlDriver;
class QSqlError;
class QSqlResultPrivate;

class Q_SQL_EXPORT QSqlResult
{
friend class QSqlQuery;
friend class QSqlResultPrivate;
public:
    virtual ~QSqlResult();
    virtual QVariant handle() const;

protected:
    enum BindingSyntax {
        PositionalBinding,
        NamedBinding
#ifdef QT3_SUPPORT
        , BindByPosition = PositionalBinding,
        BindByName = NamedBinding
#endif
    };

    explicit QSqlResult(const QSqlDriver * db);
    int at() const;
    QString lastQuery() const;
    QSqlError lastError() const;
    bool isValid() const;
    bool isActive() const;
    bool isSelect() const;
    bool isForwardOnly() const;
    const QSqlDriver* driver() const;
    virtual void setAt(int at);
    virtual void setActive(bool a);
    virtual void setLastError(const QSqlError& e);
    virtual void setQuery(const QString& query);
    virtual void setSelect(bool s);
    virtual void setForwardOnly(bool forward);

    // prepared query support
    virtual bool exec();
    virtual bool prepare(const QString& query);
    virtual bool savePrepare(const QString& sqlquery);
    virtual void bindValue(int pos, const QVariant& val, QSql::ParamType type);
    virtual void bindValue(const QString& placeholder, const QVariant& val,
                           QSql::ParamType type);
    void addBindValue(const QVariant& val, QSql::ParamType type);
    QVariant boundValue(const QString& placeholder) const;
    QVariant boundValue(int pos) const;
    QSql::ParamType bindValueType(const QString& placeholder) const;
    QSql::ParamType bindValueType(int pos) const;
    int boundValueCount() const;
    QVector<QVariant>& boundValues() const;
    QString executedQuery() const;
    QString boundValueName(int pos) const;
    void clear();
    bool hasOutValues() const;

    BindingSyntax bindingSyntax() const;

    virtual QVariant data(int i) = 0;
    virtual bool isNull(int i) = 0;
    virtual bool reset(const QString& sqlquery) = 0;
    virtual bool fetch(int i) = 0;
    virtual bool fetchNext();
    virtual bool fetchPrevious();
    virtual bool fetchFirst() = 0;
    virtual bool fetchLast() = 0;
    virtual int size() = 0;
    virtual int numRowsAffected() = 0;
    virtual QSqlRecord record() const;
    virtual QVariant lastInsertId() const;

    enum VirtualHookOperation { BatchOperation };
    virtual void virtual_hook(int id, void *data);

    bool execBatch(bool arrayBind = false);

private:
    QSqlResultPrivate* d;
    void resetBindCount(); // HACK

private:
    Q_DISABLE_COPY(QSqlResult)
};

QT_END_HEADER

#endif // QSQLRESULT_H
