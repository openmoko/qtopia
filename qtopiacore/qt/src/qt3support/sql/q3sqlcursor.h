/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SQLCURSOR_H
#define Q3SQLCURSOR_H

#include <QtCore/qvariant.h>
#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlrecord.h>
#include <QtCore/qstringlist.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlindex.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL

class Q3SqlCursorPrivate;
class Q3SqlFieldInfo;

class Q_COMPAT_EXPORT Q3SqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    Q3SqlCursor(const QString & name = QString(), bool autopopulate = true,
               QSqlDatabase db = QSqlDatabase());
    Q3SqlCursor(const Q3SqlCursor & other);
    Q3SqlCursor& operator=(const Q3SqlCursor& other);
    virtual ~Q3SqlCursor();

    enum Mode {
        ReadOnly = 0,
        Insert = 1,
        Update = 2,
        Delete = 4,
        Writable = 7
    };

    QVariant value(int i) const;
    inline QVariant value(const QString &name) const { return value(indexOf(name)); }
    virtual void setValue(int i, const QVariant &val);
    inline void setValue(const QString &name, const QVariant &val) { setValue(indexOf(name), val); }
    virtual QSqlIndex primaryIndex(bool prime = true) const;
    virtual QSqlIndex index(const QStringList& fieldNames) const;
    QSqlIndex index(const QString& fieldName) const;
    virtual void setPrimaryIndex(const QSqlIndex& idx);

    virtual void append(const Q3SqlFieldInfo& fieldInfo);
    virtual void insert(int pos, const Q3SqlFieldInfo &fieldInfo);
    virtual void remove(int pos);
    virtual void clear();
    virtual void setGenerated(const QString& name, bool generated);
    virtual void setGenerated(int i, bool generated);

    virtual QSqlRecord*        editBuffer(bool copy = false);
    virtual QSqlRecord*        primeInsert();
    virtual QSqlRecord*        primeUpdate();
    virtual QSqlRecord*        primeDelete();
    virtual int                insert(bool invalidate = true);
    virtual int                update(bool invalidate = true);
    virtual int                del(bool invalidate = true);

    virtual void        setMode(int flags);
    int                        mode() const;
    virtual void        setCalculated(const QString& name, bool calculated);
    bool                isCalculated(const QString& name) const;
    virtual void        setTrimmed(const QString& name, bool trim);
    bool                isTrimmed(const QString& name) const;

    bool                isReadOnly() const;
    bool                canInsert() const;
    bool                canUpdate() const;
    bool                canDelete() const;

    bool                select();
    bool                select(const QSqlIndex& sort);
    bool                select(const QSqlIndex & filter, const QSqlIndex & sort);
    virtual bool        select(const QString & filter, const QSqlIndex & sort = QSqlIndex());

    virtual void        setSort(const QSqlIndex& sort);
    QSqlIndex                sort() const;
    virtual void        setFilter(const QString& filter);
    QString                filter() const;
    virtual void        setName(const QString& name, bool autopopulate = true);
    QString                name() const;
    QString                toString(const QString& prefix = QString(),
                                const QString& sep = ",") const;
    bool                 isNull(int i) const;
    bool                 isNull(const QString& name) const;
    virtual bool seek(int i, bool relative = false);
    virtual bool next();
    inline bool previous() { return prev(); }
    virtual bool prev();
    virtual bool first();
    virtual bool last();

protected:
    virtual bool        exec(const QString & sql);

    virtual QVariant calculateField(const QString& name);
    virtual int                update(const QString & filter, bool invalidate = true);
    virtual int                del(const QString & filter, bool invalidate = true);

    virtual QString        toString(const QString& prefix, QSqlField* field, const QString& fieldSep) const;
    virtual QString        toString(QSqlRecord* rec, const QString& prefix, const QString& fieldSep,
                                const QString& sep) const;
    virtual QString        toString(const QSqlIndex& i, QSqlRecord* rec, const QString& prefix,
                                const QString& fieldSep, const QString& sep) const;

private:
    void                sync();
    int                        apply(const QString& q, bool invalidate);
    int                        applyPrepared(const QString& q, bool invalidate);
    QSqlRecord&                operator=(const QSqlRecord & list);
    void                 append(const QSqlField& field);

    Q3SqlCursorPrivate*        d;
};

#endif // QT_NO_SQL

QT_END_HEADER

#endif // Q3SQLCURSOR_H
