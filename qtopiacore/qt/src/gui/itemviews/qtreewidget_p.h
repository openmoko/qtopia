/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QTREEWIDGET_P_H
#define QTREEWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qabstractitemmodel.h>
#include <QtCore/qpair.h>
#include <QtGui/qtreewidget.h>

#ifndef QT_NO_TREEWIDGET

class QTreeWidgetItem;
class QTreeWidgetItemIterator;
class QTreeModelPrivate;

class QTreeModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class QTreeWidget;
    friend class QTreeWidgetPrivate;
    friend class QTreeWidgetItem;
    friend class QTreeWidgetItemIterator;
    friend class QTreeWidgetItemIteratorPrivate;

public:
    QTreeModel(int columns = 0, QTreeWidget *parent = 0);
    ~QTreeModel();

    inline QTreeWidget *view() const
        { return qobject_cast<QTreeWidget*>(QObject::parent()); }

    void clear();
    void setColumnCount(int columns);

    QTreeWidgetItem *item(const QModelIndex &index) const;
    void itemChanged(QTreeWidgetItem *item);

    QModelIndex index(const QTreeWidgetItem *item, int column) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QMap<int, QVariant> itemData(const QModelIndex &index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role);

    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order);
    void ensureSorted(int column, Qt::SortOrder order,
                      int start, int end, const QModelIndex &parent);
    static bool itemLessThan(const QPair<QTreeWidgetItem*,int> &left,
                             const QPair<QTreeWidgetItem*,int> &right);
    static bool itemGreaterThan(const QPair<QTreeWidgetItem*,int> &left,
                                const QPair<QTreeWidgetItem*,int> &right);
    static QList<QTreeWidgetItem*>::iterator sortedInsertionIterator(
        const QList<QTreeWidgetItem*>::iterator &begin,
        const QList<QTreeWidgetItem*>::iterator &end,
        Qt::SortOrder order, QTreeWidgetItem *item);

    bool insertRows(int row, int count, const QModelIndex &);
    bool insertColumns(int column, int count, const QModelIndex &);

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    // dnd
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    QMimeData *internalMimeData()  const;

protected:
    QTreeModel(QTreeModelPrivate &, QTreeWidget *parent = 0);
    void emitDataChanged(QTreeWidgetItem *item, int column);
    void beginInsertItems(QTreeWidgetItem *parent, int row, int count);
    void endInsertItems();
    void beginRemoveItems(QTreeWidgetItem *parent, int row, int count);
    void endRemoveItems();
    void sortItems(QList<QTreeWidgetItem*> *items, int column, Qt::SortOrder order);

private:
    QTreeWidgetItem *rootItem;
    QTreeWidgetItem *headerItem;

    mutable QModelIndexList cachedIndexes;
    QList<QTreeWidgetItemIterator*> iterators;

    mutable bool sortPending;
    bool executePendingSort() const;

    Q_DECLARE_PRIVATE(QTreeModel)
};

#include "private/qabstractitemmodel_p.h"

class QTreeModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QTreeModel)
};

#endif // QT_NO_TREEWIDGET
#endif // QTREEWIDGET_P_H
