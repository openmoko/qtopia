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

#ifndef QSQLQUERYMODEL_H
#define QSQLQUERYMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtSql/qsqldatabase.h>

QT_BEGIN_HEADER

QT_MODULE(Sql)

class QSqlQueryModelPrivate;
class QSqlError;
class QSqlRecord;
class QSqlQuery;

class Q_SQL_EXPORT QSqlQueryModel: public QAbstractTableModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlQueryModel)

public:
    explicit QSqlQueryModel(QObject *parent = 0);
    virtual ~QSqlQueryModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QSqlRecord record(int row) const;
    QSqlRecord record() const;

    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole);

    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());

    void setQuery(const QSqlQuery &query);
    void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
    QSqlQuery query() const;

    virtual void clear();

    QSqlError lastError() const;

    void fetchMore(const QModelIndex &parent = QModelIndex());
    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;

protected:
    virtual void queryChange();

    QModelIndex indexInQuery(const QModelIndex &item) const;
    void setLastError(const QSqlError &error);
    QSqlQueryModel(QSqlQueryModelPrivate &dd, QObject *parent = 0);
};

QT_END_HEADER

#endif // QSQLQUERYMODEL_H
