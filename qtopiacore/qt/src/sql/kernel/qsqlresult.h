/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QSQLRESULT_H
#define QSQLRESULT_H

#include <QtCore/qvariant.h>
#include <QtCore/qvector.h>
#include <QtSql/qsql.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

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
    friend class QSqlTableModelPrivate;
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

    enum VirtualHookOperation { BatchOperation, DetachFromResultSet, SetNumericalPrecision, NextResult };
    virtual void virtual_hook(int id, void *data);
    bool execBatch(bool arrayBind = false);
    void detachFromResultSet();
    void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy policy);
    bool nextResult();

private:
    QSqlResultPrivate* d;
    void resetBindCount(); // HACK

private:
    Q_DISABLE_COPY(QSqlResult)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSQLRESULT_H
