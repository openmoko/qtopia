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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
