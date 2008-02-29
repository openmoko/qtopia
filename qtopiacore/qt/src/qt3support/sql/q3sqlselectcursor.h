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

#ifndef Q3SQLSELECTCURSOR_H
#define Q3SQLSELECTCURSOR_H

#include <Qt3Support/q3sqlcursor.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL

class Q3SqlSelectCursorPrivate;

class Q_COMPAT_EXPORT Q3SqlSelectCursor : public Q3SqlCursor
{
public:
    Q3SqlSelectCursor(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
    Q3SqlSelectCursor(const Q3SqlSelectCursor& other);
    ~Q3SqlSelectCursor();
    bool exec(const QString& query);
    bool select() { return Q3SqlCursor::select(); }

protected:
    QSqlIndex primaryIndex(bool = true) const { return QSqlIndex(); }
    QSqlIndex index(const QStringList&) const { return QSqlIndex(); }
    QSqlIndex index(const QString&) const { return QSqlIndex(); }
    QSqlIndex index(const char*) const { return QSqlIndex(); }
    void setPrimaryIndex(const QSqlIndex&) {}
    void append(const Q3SqlFieldInfo&) {}
    void insert(int, const Q3SqlFieldInfo&) {}
    void remove(int) {}
    void clear() {}
    void setGenerated(const QString&, bool) {}
    void setGenerated(int, bool) {}
    QSqlRecord*        editBuffer(bool = false) { return 0; }
    QSqlRecord*        primeInsert() { return 0; }
    QSqlRecord*        primeUpdate() { return 0; }
    QSqlRecord*        primeDelete() { return 0; }
    int        insert(bool = true) { return 0; }
    int        update(bool = true) { return 0; }
    int        del(bool = true) { return 0; }
    void setMode(int) {}

    void setSort(const QSqlIndex&) {}
    QSqlIndex sort() const { return QSqlIndex(); }
    void setFilter(const QString&) {}
    QString filter() const { return QString(); }
    void setName(const QString&, bool = true) {}
    QString name() const { return QString(); }
    QString toString(const QString& = QString(), const QString& = ",") const { return QString(); }
    bool select(const QString &, const QSqlIndex& = QSqlIndex());

private:
    void populateCursor();

    Q3SqlSelectCursorPrivate * d;

protected:
#if !defined(Q_NO_USING_KEYWORD)
    using Q3SqlCursor::update;
#else
    virtual int update(const QString & filter, bool invalidate = true) { return Q3SqlCursor::update(filter, invalidate); }
#endif
};

#endif // QT_NO_SQL

QT_END_HEADER

#endif // Q3SQLSELECTCURSOR_H
