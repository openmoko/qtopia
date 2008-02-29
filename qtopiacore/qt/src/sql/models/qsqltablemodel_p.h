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

#ifndef QSQLTABLEMODEL_P_H
#define QSQLTABLEMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsql*model.h .  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qmap.h"
#include "private/qsqlquerymodel_p.h"

class QSqlTableModelPrivate: public QSqlQueryModelPrivate
{
    Q_DECLARE_PUBLIC(QSqlTableModel)

public:
    QSqlTableModelPrivate()
        : editIndex(-1), insertIndex(-1), sortColumn(-1),
          sortOrder(Qt::AscendingOrder),
          strategy(QSqlTableModel::OnRowChange)
    {}
    void clear();
    QSqlRecord primaryValues(int index);
    virtual void clearEditBuffer();
    QSqlRecord record(const QVector<QVariant> &values) const;

    bool exec(const QString &stmt, bool prepStatement,
              const QSqlRecord &rec, const QSqlRecord &whereValues = QSqlRecord());
    virtual void revertCachedRow(int row);
    void revertInsertedRow();
    bool setRecord(int row, const QSqlRecord &record);
    virtual int nameToIndex(const QString &name) const;
    void initRecordAndPrimaryIndex();

    QSqlDatabase db;
    int editIndex;
    int insertIndex;

    int sortColumn;
    Qt::SortOrder sortOrder;

    QSqlTableModel::EditStrategy strategy;

    QSqlQuery editQuery;
    QSqlIndex primaryIndex;
    QString tableName;
    QString filter;

    enum Op { None, Insert, Update, Delete };

    struct ModifiedRow
    {
        ModifiedRow(Op o = None, const QSqlRecord &r = QSqlRecord()): op(o), rec(r) {}
        ModifiedRow(const ModifiedRow &other): op(other.op), rec(other.rec) {}
        Op op;
        QSqlRecord rec;
    };

    QSqlRecord editBuffer;

    typedef QMap<int, ModifiedRow> CacheMap;
    CacheMap cache;
};

#endif // QSQLTABLEMODEL_P_H
