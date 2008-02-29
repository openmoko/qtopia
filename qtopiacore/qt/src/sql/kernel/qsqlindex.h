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

#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#include <QtSql/qsqlrecord.h>
#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class Q_SQL_EXPORT QSqlIndex : public QSqlRecord
{
public:
    QSqlIndex(const QString &cursorName = QString(), const QString &name = QString());
    QSqlIndex(const QSqlIndex &other);
    ~QSqlIndex();
    QSqlIndex &operator=(const QSqlIndex &other);
    void setCursorName(const QString &cursorName);
    inline QString cursorName() const { return cursor; }
    void setName(const QString& name);
    inline QString name() const { return nm; }

    void append(const QSqlField &field);
    void append(const QSqlField &field, bool desc);

    bool isDescending(int i) const;
    void setDescending(int i, bool desc);

#ifdef QT3_SUPPORT
    QT3_SUPPORT QString toString(const QString &prefix = QString(),
                               const QString &sep = QLatin1String(","),
                               bool verbose = true) const;
    QT3_SUPPORT QStringList toStringList(const QString& prefix = QString(),
                                       bool verbose = true) const;
#endif

private:
    QString createField(int i, const QString& prefix, bool verbose) const;
    QString cursor;
    QString nm;
    QList<bool> sorts;
};

QT_END_HEADER

#endif // QSQLINDEX_H
