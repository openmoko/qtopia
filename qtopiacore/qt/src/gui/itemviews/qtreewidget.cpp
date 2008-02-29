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

#include "qtreewidget.h"

#ifndef QT_NO_TREEWIDGET
#include <qheaderview.h>
#include <qpainter.h>
#include <qitemdelegate.h>
#include <qstack.h>
#include <qdebug.h>
#include <private/qtreeview_p.h>
#include <private/qwidgetitemdata_p.h>
#include <private/qtreewidget_p.h>
#include <private/qtreewidgetitemiterator_p.h>

// workaround for VC++ 6.0 linker bug (?)
typedef bool(*LessThan)(const QPair<QTreeWidgetItem*,int>&,const QPair<QTreeWidgetItem*,int>&);

class QTreeWidgetMimeData : public QMimeData
{
    Q_OBJECT
public:
    QList<QTreeWidgetItem*> items;
};

class QTreeModelLessThan
{
public:
    inline bool operator()(QTreeWidgetItem *i1, QTreeWidgetItem *i2) const
        { return *i1 < *i2; }
};

class QTreeModelGreaterThan
{
public:
    inline bool operator()(QTreeWidgetItem *i1, QTreeWidgetItem *i2) const
        { return *i2 < *i1; }
};

#include "qtreewidget.moc"

/*
    \class QTreeModel
    \brief The QTreeModel class manages the items stored in a tree view.

    \ingroup model-view
    \mainclass
*/

/*!
  \internal

  Constructs a tree model with a \a parent object and the given
  number of \a columns.
*/

QTreeModel::QTreeModel(int columns, QTreeWidget *parent)
    : QAbstractItemModel(parent), rootItem(new QTreeWidgetItem),
      headerItem(new QTreeWidgetItem), sortPending(false)
{
    rootItem->view = parent;
    headerItem->view = parent;
    setColumnCount(columns);
}

/*!
  \internal

*/

QTreeModel::QTreeModel(QTreeModelPrivate &dd, QTreeWidget *parent)
    : QAbstractItemModel(dd, parent), rootItem(new QTreeWidgetItem),
      headerItem(new QTreeWidgetItem), sortPending(false)
{
    rootItem->view = parent;
    headerItem->view = parent;
}

/*!
  \internal

  Destroys this tree model.
*/

QTreeModel::~QTreeModel()
{
    clear();
    delete headerItem;
    rootItem->view = 0;
    delete rootItem;
}

/*!
  \internal

  Removes all items in the model.
*/

void QTreeModel::clear()
{
    for (int i = 0; i < rootItem->childCount(); ++i) {
        QTreeWidgetItem *item = rootItem->children.at(i);
        item->par = 0;
        item->view = 0;
        delete item;
    }
    rootItem->children.clear();
    sortPending = false;
    reset();
}

/*!
  \internal

  Sets the number of \a columns in the tree model.
*/

void QTreeModel::setColumnCount(int columns)
{
    if (columns < 0)
        return;
    if (!headerItem) {
        headerItem = new QTreeWidgetItem();
        headerItem->view = view();
    }
    int count = columnCount();
    if (count == columns)
        return;

    if (columns < count) {
        beginRemoveColumns(QModelIndex(), columns, count - 1);
        headerItem->values.resize(columns);
        endRemoveColumns();
    } else {
        beginInsertColumns(QModelIndex(), count, columns - 1);
        headerItem->values.resize(columns);
        for (int i = count; i < columns; ++i) {// insert data without emitting the dataChanged signal
            headerItem->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i + 1)));
            headerItem->display.append(QString::number(i + 1));
        }
        endInsertColumns();
    }
}

/*!
  \internal

  Returns the tree view item corresponding to the \a index given.

  \sa QModelIndex
*/

QTreeWidgetItem *QTreeModel::item(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return static_cast<QTreeWidgetItem*>(index.internalPointer());
}

/*!
  \internal

  Returns the model index that refers to the
  tree view \a item and \a column.
*/

QModelIndex QTreeModel::index(const QTreeWidgetItem *item, int column) const
{
    if (!item || (item == rootItem))
        return QModelIndex();
    const QTreeWidgetItem *par = item->parent();
    QTreeWidgetItem *itm = const_cast<QTreeWidgetItem*>(item);
    if (!par)
        par = rootItem;
    return createIndex(par->children.lastIndexOf(itm), column, itm);
}

/*!
  \internal
  \reimp

  Returns the model index with the given \a row,
  \a column and \a parent.
*/

QModelIndex QTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    int c = columnCount(parent);

    if (row < 0 || column < 0 || column >= c)
        return QModelIndex();

    QTreeWidgetItem *parentItem = parent.isValid() ? item(parent) : rootItem;
    if (parentItem && row < parentItem->childCount()) {
        QTreeWidgetItem *itm = parentItem->child(row);
        if (itm)
            return createIndex(row, column, itm);
        return QModelIndex();
    }

    return QModelIndex();
}

/*!
  \internal
  \reimp

  Returns the parent model index of the index given as
  the \a child.
*/

QModelIndex QTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();
    QTreeWidgetItem *itm = static_cast<QTreeWidgetItem *>(child.internalPointer());
    if (!itm)
        return QModelIndex();
    QTreeWidgetItem *parent = itm->parent();
    return index(parent, 0);
}

/*!
  \internal
  \reimp

  Returns the number of rows in the \a parent model index.
*/

int QTreeModel::rowCount(const QModelIndex &parent) const
{
    executePendingSort();

    if (!parent.isValid())
        return rootItem->childCount();

    QTreeWidgetItem *parentItem = item(parent);
    if (parentItem)
        return parentItem->childCount();
    return 0;
}

/*!
  \internal
  \reimp

  Returns the number of columns in the item referred to by
  the given \a index.
*/

int QTreeModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (!headerItem)
        return 0;
    return headerItem->columnCount();
}

bool QTreeModel::hasChildren(const QModelIndex &parent) const
{
    executePendingSort();

    if (!parent.isValid())
        return (rootItem->childCount() > 0);

    QTreeWidgetItem *itm = item(parent);
    if (!itm)
        return false;
    return (itm->childCount() > 0);
}

/*!
  \internal
  \reimp

  Returns the data corresponding to the given model \a index
  and \a role.
*/

QVariant QTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    QTreeWidgetItem *itm = item(index);
    if (itm)
        return itm->data(index.column(), role);
    return QVariant();
}

/*!
  \internal
  \reimp

  Sets the data for the item specified by the \a index and \a role
  to that referred to by the \a value.

  Returns true if successful; otherwise returns false.
*/

bool QTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    QTreeWidgetItem *itm = item(index);
    if (itm) {
        itm->setData(index.column(), role, value);
        return true;
    }
    return false;
}

QMap<int, QVariant> QTreeModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> roles;
    QTreeWidgetItem *itm = item(index);
    if (itm) {
        int column = index.column();
        for (int i = 0; i < itm->values.at(column).count(); ++i) {
            roles.insert(itm->values.at(column).at(i).role,
                         itm->values.at(column).at(i).value);
        }

        // the two special cases
        QVariant displayValue = itm->data(column, Qt::DisplayRole);
        if (displayValue.isValid())
            roles.insert(Qt::DisplayRole, displayValue);

        QVariant checkValue = itm->data(column, Qt::CheckStateRole);
        if (checkValue.isValid())
            roles.insert(Qt::CheckStateRole, checkValue);
    }
    return roles;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent) || parent.column() > 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    QTreeWidgetItem *par = item(parent);
    while (count > 0) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->view = view();
        item->par = par;
        if (par)
            par->children.insert(row++, item);
        else
            rootItem->children.insert(row++, item);
        --count;
    }
    endInsertRows();
    return true;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (count < 1 || column < 0 || column > columnCount(parent) || parent.column() > 0 || !headerItem)
        return false;

    beginInsertColumns(parent, column, column + count - 1);

    int oldCount = columnCount(parent);
    column = qBound(0, column, oldCount);
    headerItem->values.resize(oldCount + count);
    for (int i = oldCount; i < oldCount + count; ++i) {
        headerItem->values[i].append(QWidgetItemData(Qt::DisplayRole, QString::number(i + 1)));
        headerItem->display.append(QString::number(i + 1));
    }

    QStack<QTreeWidgetItem*> itemstack;
    itemstack.push(0);
    while (!itemstack.isEmpty()) {
        QTreeWidgetItem *par = itemstack.pop();
        QList<QTreeWidgetItem*> children = par ? par->children : rootItem->children;
        for (int row = 0; row < children.count(); ++row) {
            QTreeWidgetItem *child = children.at(row);
            if (child->children.count())
                itemstack.push(child);
            child->values.insert(column, count, QVector<QWidgetItemData>());
        }
    }

    endInsertColumns();
    return true;
}

/*!
  \internal
  \reimp
*/
bool QTreeModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (count < 1 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(parent, row, row + count - 1);

    bool blockSignal = signalsBlocked();
    blockSignals(true);

    QTreeWidgetItem *itm = item(parent);
    for (int i = row + count - 1; i >= row; --i) {
        QTreeWidgetItem *child = itm ? itm->takeChild(i) : rootItem->children.takeAt(i);
        Q_ASSERT(child);
        child->view = 0;
        delete child;
        child = 0;
    }
    blockSignals(blockSignal);

    endRemoveRows();
    return true;
}

/*!
  \internal
  \reimp

  Returns the header data corresponding to the given header \a section,
  \a orientation and data \a role.
*/

QVariant QTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (headerItem)
        return headerItem->data(section, role);
    if (role == Qt::DisplayRole)
        return QString::number(section + 1);
    return QVariant();
}

/*!
  \internal
  \reimp

  Sets the header data for the item specified by the header \a section,
  \a orientation and data \a role to the given \a value.

  Returns true if successful; otherwise returns false.
*/

bool QTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant &value, int role)
{
    if (section < 0 || orientation != Qt::Horizontal || !headerItem)
        return false;

    headerItem->setData(section, role, value);
    return true;
}

/*!
  \reimp

  Returns the flags for the item referred to the given \a index.

*/

Qt::ItemFlags QTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled; // can drop on the viewport
    QTreeWidgetItem *itm = item(index);
    Q_ASSERT(itm);
    return itm->flags();
}

/*!
  \internal

  Sorts the entire tree in the model in the given \a order,
  by the values in the given \a column.
*/

void QTreeModel::sort(int column, Qt::SortOrder order)
{
    sortPending = false;

    if (column < 0 || column >= columnCount())
        return;

    emit layoutAboutToBeChanged();
    rootItem->sortChildren(column, order, true);
    emit layoutChanged();
}

/*!
  \internal
*/
void QTreeModel::ensureSorted(int column, Qt::SortOrder order,
                              int start, int end, const QModelIndex &parent)
{
    sortPending = false;

    if (column < 0 || column >= columnCount())
        return;

    QTreeWidgetItem *itm = item(parent);
    if (!itm)
        itm = rootItem;
    QList<QTreeWidgetItem*> lst = itm->children;

    int count = end - start + 1;
    QVector < QPair<QTreeWidgetItem*,int> > sorting(count);
    for (int i = 0; i < count; ++i) {
        sorting[i].first = lst.at(start + i);
        sorting[i].second = start + i;
    }

    LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
    qStableSort(sorting.begin(), sorting.end(), compare);

    QModelIndexList oldPersistentIndexes = persistentIndexList();
    QModelIndexList newPersistentIndexes = oldPersistentIndexes;
    QList<QTreeWidgetItem*>::iterator lit = lst.begin();
    bool changed = false;
    for (int i = 0; i < count; ++i) {
        int oldRow = sorting.at(i).second;
        QTreeWidgetItem *item = lst.takeAt(oldRow);
        lit = sortedInsertionIterator(lit, lst.end(), order, item);
        int newRow = qMax(lit - lst.begin(), 0);
        lit = lst.insert(lit, item);
        if (newRow != oldRow) {
            changed = true;
            for (int j = i + 1; j < count; ++j) {
                int otherRow = sorting.at(j).second;
                if (oldRow < otherRow && newRow >= otherRow)
                    --sorting[j].second;
                else if (oldRow > otherRow && newRow <= otherRow)
                    ++sorting[j].second;
            }
            for (int k = 0; k < newPersistentIndexes.count(); ++k) {
                QModelIndex pi = newPersistentIndexes.at(k);
                if (pi.parent() != parent)
                    continue;
                int oldPersistentRow = pi.row();
                int newPersistentRow = oldPersistentRow;
                if (oldPersistentRow == oldRow)
                    newPersistentRow = newRow;
                else if (oldRow < oldPersistentRow && newRow >= oldPersistentRow)
                    newPersistentRow = oldPersistentRow - 1;
                else if (oldRow > oldPersistentRow && newRow <= oldPersistentRow)
                    newPersistentRow = oldPersistentRow + 1;
                if (newPersistentRow != oldPersistentRow)
                    newPersistentIndexes[k] = createIndex(newPersistentRow,
                                                          pi.column(), pi.internalPointer());
            }
        }
    }

    if (changed) {
        emit layoutAboutToBeChanged();
        itm->children = lst;
        changePersistentIndexList(oldPersistentIndexes, newPersistentIndexes);
        emit layoutChanged();
    }
}

/*!
  \internal

  Returns true if the value of the \a left item is
  less than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::itemLessThan(const QPair<QTreeWidgetItem*,int> &left,
                              const QPair<QTreeWidgetItem*,int> &right)
{
    return *(left.first) < *(right.first);
}

/*!
  \internal

  Returns true if the value of the \a left item is
  greater than the value of the \a right item.

  Used by the sorting functions.
*/

bool QTreeModel::itemGreaterThan(const QPair<QTreeWidgetItem*,int> &left,
                                 const QPair<QTreeWidgetItem*,int> &right)
{
    return !(*(left .first) < *(right.first));
}

/*!
  \internal
*/
QList<QTreeWidgetItem*>::iterator QTreeModel::sortedInsertionIterator(
    const QList<QTreeWidgetItem*>::iterator &begin,
    const QList<QTreeWidgetItem*>::iterator &end,
    Qt::SortOrder order, QTreeWidgetItem *item)
{
    if (order == Qt::AscendingOrder)
        return qLowerBound(begin, end, item, QTreeModelLessThan());
    return qLowerBound(begin, end, item, QTreeModelGreaterThan());
}

QStringList QTreeModel::mimeTypes() const
{
    return view()->mimeTypes();
}

QMimeData *QTreeModel::internalMimeData()  const
{
    return QAbstractItemModel::mimeData(cachedIndexes);
}

QMimeData *QTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i) {
        if (indexes.at(i).column() == 0) // only one item per row
            items << item(indexes.at(i));
    }

    // cachedIndexes is a little hack to avoid copying from QModelIndexList to
    // QList<QTreeWidgetItem*> and back again in the view
    cachedIndexes = indexes;
    QMimeData *mimeData = view()->mimeData(items);
    cachedIndexes.clear();
    return mimeData;
}

bool QTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent)
{
    if (row == -1 && column == -1)
        row = rowCount(parent); // append
    return view()->dropMimeData(item(parent), row, data, action);
}

Qt::DropActions QTreeModel::supportedDropActions() const
{
    return view()->supportedDropActions();
}

void QTreeModel::itemChanged(QTreeWidgetItem *item)
{
    QModelIndex left = index(item, 0);
    QModelIndex right = index(item, item->columnCount() - 1);
    emit dataChanged(left, right);
}

bool QTreeModel::executePendingSort() const
{
    if (sortPending) {
        sortPending = false;
        int column = view()->header()->sortIndicatorSection();
        Qt::SortOrder order = view()->header()->sortIndicatorOrder();
        QTreeModel *that = const_cast<QTreeModel*>(this);
        const bool blocked = that->signalsBlocked();
        that->blockSignals(true);
        that->sort(column, order);
        that->blockSignals(blocked);
        return true;
    }
    return false;
}

/*!
  \internal
    Emits the dataChanged() signal for the given \a item.
    if column is -1 then all columns have changed
*/

void QTreeModel::emitDataChanged(QTreeWidgetItem *item, int column)
{
    if (signalsBlocked())
        return;

    if (headerItem == item && column < item->columnCount()) {
        if (column == -1)
            emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
        else
            emit headerDataChanged(Qt::Horizontal, column, column);
        return;
    }

    QModelIndex bottomRight, topLeft;
    if (column == -1) {
        topLeft = index(item, 0);
        bottomRight = createIndex(topLeft.row(), columnCount() - 1, item);
    } else {
        topLeft = index(item, column);
        bottomRight = topLeft;
    }
    emit dataChanged(topLeft, bottomRight);
}

void QTreeModel::beginInsertItems(QTreeWidgetItem *parent, int row, int count)
{
    beginInsertRows(index(parent, 0), row, row + count - 1);
}

void QTreeModel::endInsertItems()
{
    endInsertRows();
}

void QTreeModel::beginRemoveItems(QTreeWidgetItem *parent, int row, int count)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(count > 0);
    beginRemoveRows(index(parent, 0), row, row + count - 1);
    if (!parent)
        parent = rootItem;
    // now update the iterators
    for (int i = 0; i < iterators.count(); ++i) {
        for (int j = 0; j < count; j++) {
            QTreeWidgetItem *c = parent->child(row + j);
            iterators[i]->d_func()->ensureValidIterator(c);
        }
    }
}

void QTreeModel::endRemoveItems()
{
    endRemoveRows();
}

void QTreeModel::sortItems(QList<QTreeWidgetItem*> *items, int column, Qt::SortOrder order)
{
    Q_UNUSED(column);
    // store the original order of indexes
    QVector< QPair<QTreeWidgetItem*,int> > sorting(items->count());
    for (int i = 0; i < sorting.count(); ++i) {
        sorting[i].first = items->at(i);
        sorting[i].second = i;
    }

    // do the sorting
    LessThan compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
    qStableSort(sorting.begin(), sorting.end(), compare);

    int colCount = columnCount();
    for (int r = 0; r < sorting.count(); ++r) {
        QTreeWidgetItem *item = sorting.at(r).first;
        items->replace(r, item);
        int oldRow = sorting.at(r).second;
        for (int c = 0; c < colCount; ++c) {
            QModelIndex from = createIndex(oldRow, c, item);
            QModelIndex to = createIndex(r, c, item);
            changePersistentIndex(from, to);
        }
    }
}

/*!
  \class QTreeWidgetItem

  \brief The QTreeWidgetItem class provides an item for use with the
  QTreeWidget convenience class.

  \ingroup model-view

  Tree widget items are used to hold rows of information for tree widgets.
  Rows usually contain several columns of data, each of which can contain
  a text label and an icon.

  The QTreeWidgetItem class is a convenience class that replaces the
  QListViewItem class in Qt 3. It provides an item for use with
  the QTreeWidget class.

  Items are usually constructed with a parent that is either a QTreeWidget
  (for top-level items) or a QTreeWidgetItem (for items on lower levels of
  the tree). For example, the following code constructs a top-level item
  to represent cities of the world, and adds a entry for Oslo as a child
  item:

  \quotefile snippets/qtreewidget-using/mainwindow.cpp
  \skipto QTreeWidgetItem *cities
  \printuntil osloItem->setText(1, tr("Yes"));

  Items can be added in a particular order by specifying the item they
  follow when they are constructed:

  \skipto QTreeWidgetItem *planets
  \printuntil planets->setText(0

  Each column in an item can have its own background brush which is set with
  the setBackground() function. The current background brush can be
  found with background().
  The text label for each column can be rendered with its own font and brush.
  These are specified with the setFont() and setForeground() functions,
  and read with font() and foreground().

  The main difference between top-level items and those in lower levels of
  the tree is that a top-level item has no parent(). This information
  can be used to tell the difference between items, and is useful to know
  when inserting and removing items from the tree.
  Children of an item can be removed with takeChild() and inserted at a
  given index in the list of children with the insertChild() function.

  By default, items are enabled, selectable, checkable, and can be the source
  of a drag and drop operation.
  Each item's flags can be changed by calling setFlags() with the appropriate
  value (see \l{Qt::ItemFlags}). Checkable items can be checked and unchecked
  with the setCheckState() function. The corresponding checkState() function
  indicates whether the item is currently checked.

  \section1 Subclassing

  When subclassing QTreeWidgetItem to provide custom items, it is possible to
  define new types for them so that they can be distinguished from standard
  items. The constructors for subclasses that require this feature need to
  call the base class constructor with a new type value equal to or greater
  than \l UserType.

  \sa QTreeWidget, {Model/View Programming}, QListWidgetItem, QTableWidgetItem
*/

/*!
    \enum QTreeWidgetItem::ItemType

    This enum describes the types that are used to describe tree widget items.

    \value Type     The default type for tree widget items.
    \value UserType The minimum value for custom types. Values below UserType are
                    reserved by Qt.

    You can define new user types in QTreeWidgetItem subclasses to ensure that
    custom items are treated specially; for example, when items are sorted.

    \sa type()
*/

/*!
    \fn int QTreeWidgetItem::type() const

    Returns the type passed to the QTreeWidgetItem constructor.
*/

/*!
  \fn void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order)
  \since 4.2

  Sorts the children of the item using the given \a order,
  by the values in the given \a column.
*/

/*!
    \fn QTreeWidget *QTreeWidgetItem::treeWidget() const

    Returns the tree widget that contains the item.
*/

/*!
  \fn void QTreeWidgetItem::setSelected(bool select)
  \since 4.2

  Sets the selected state of the item to \a select.

  \sa isSelected()

*/

/*!
  \fn bool QTreeWidgetItem::isSelected() const
  \since 4.2

  Returns true if the item is selected, otherwise returns false.

  \sa setSelected()
*/

/*!
  \fn void QTreeWidgetItem::setHidden(bool hide)
  \since 4.2

  Hides the item if \a hide is true, otherwise shows the item.

  \sa isHidden()
*/

/*!
  \fn bool QTreeWidgetItem::isHidden() const
  \since 4.2

  Returns true if the item is hidden, otherwise returns false.

  \sa setHidden()
*/
/*!
  \fn void QTreeWidgetItem::setExpanded(bool expand)
  \since 4.2

  Expands the item if \a expand is true, otherwise collapses the item.

  \sa isExpanded()
*/

/*!
  \fn bool QTreeWidgetItem::isExpanded() const
  \since 4.2

  Returns true if the item is expanded, otherwise returns false.

  \sa setExpanded()
*/

/*!
    \fn Qt::ItemFlags QTreeWidgetItem::flags() const

    Returns the flags used to describe the item. These determine whether
    the item can be checked, edited, and selected.

    The default value for flags is
    Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled.
    If the item was constructed with a parent, flags will in addition contain Qt::ItemIsDropEnabled.

    \sa setFlags()
*/

/*!
    \fn void QTreeWidgetItem::setFlags(Qt::ItemFlags flags)

    Sets the flags for the item to the given \a flags. These determine whether
    the item can be selected or modified.  This is often used to disable an item.

    \sa flags()
*/

/*!
    \fn QString QTreeWidgetItem::text(int column) const

    Returns the text in the specified \a column.

    \sa setText()
*/

/*!
    \fn void QTreeWidgetItem::setText(int column, const QString &text)

    Sets the text to be displayed in the given \a column to the given \a text.

    \sa text() setFont() setForeground()
*/

/*!
    \fn QIcon QTreeWidgetItem::icon(int column) const

    Returns the icon that is displayed in the specified \a column.

    \sa setIcon(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn void QTreeWidgetItem::setIcon(int column, const QIcon &icon)

    Sets the icon to be displayed in the given \a column to \a icon.

    \sa icon(), setText(), {QAbstractItemView::iconSize}{iconSize}
*/

/*!
    \fn QString QTreeWidgetItem::statusTip(int column) const

    Returns the status tip for the contents of the given \a column.

    \sa setStatusTip()
*/

/*!
    \fn void QTreeWidgetItem::setStatusTip(int column, const QString &statusTip)

    Sets the status tip for the given \a column to the given \a statusTip.
    QTreeWidget mouse tracking needs to be enabled for this feature to work.

    \sa statusTip() setToolTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::toolTip(int column) const

    Returns the tool tip for the given \a column.

    \sa setToolTip()
*/

/*!
    \fn void QTreeWidgetItem::setToolTip(int column, const QString &toolTip)

    Sets the tooltip for the given \a column to \a toolTip.

    \sa toolTip() setStatusTip() setWhatsThis()
*/

/*!
    \fn QString QTreeWidgetItem::whatsThis(int column) const

    Returns the "What's This?" help for the contents of the given \a column.

    \sa setWhatsThis()
*/

/*!
    \fn void QTreeWidgetItem::setWhatsThis(int column, const QString &whatsThis)

    Sets the "What's This?" help for the given \a column to \a whatsThis.

    \sa whatsThis() setStatusTip() setToolTip()
*/

/*!
    \fn QFont QTreeWidgetItem::font(int column) const

    Returns the font used to render the text in the specified \a column.

    \sa setFont()
*/

/*!
    \fn void QTreeWidgetItem::setFont(int column, const QFont &font)

    Sets the font used to display the text in the given \a column to the given
    \a font.

    \sa font() setText() setForeground()
*/

/*!
    \fn QColor QTreeWidgetItem::backgroundColor(int column) const
    \obsolete

    This function is deprecated. Use background() instead.
*/

/*!
    \fn void QTreeWidgetItem::setBackgroundColor(int column, const QColor &color)
    \obsolete

    This function is deprecated. Use setBackground() instead.
*/

/*!
    \fn QBrush QTreeWidgetItem::background(int column) const
    \since 4.2

    Returns the brush used to render the background of the specified \a column.

    \sa foreground()
*/

/*!
    \fn void QTreeWidgetItem::setBackground(int column, const QBrush &brush)
    \since 4.2

    Sets the background brush of the label in the given \a column to the
    specified \a brush.

    \sa setForeground()
*/

/*!
    \fn QColor QTreeWidgetItem::textColor(int column) const
    \obsolete

    This function is deprecated. Use foreground() instead.
*/

/*!
    \fn void QTreeWidgetItem::setTextColor(int column, const QColor &color)
    \obsolete

    This function is deprecated. Use setForeground() instead.
*/

/*!
    \fn QBrush QTreeWidgetItem::foreground(int column) const
    \since 4.2

    Returns the brush used to render the foreground (e.g. text) of the
    specified \a column.

    \sa background()
*/

/*!
    \fn void QTreeWidgetItem::setForeground(int column, const QBrush &brush)
    \since 4.2

    Sets the foreground brush of the label in the given \a column to the
    specified \a brush.

    \sa setBackground()
*/

/*!
    \fn Qt::CheckState QTreeWidgetItem::checkState(int column) const

    Returns the check state of the label in the given \a column.

    \sa Qt::CheckState
*/

/*!
    \fn void QTreeWidgetItem::setCheckState(int column, Qt::CheckState state)

    Sets the item in the given \a column check state to be \a state.

    \sa checkState()
*/

/*!
  \fn QSize QTreeWidgetItem::sizeHint(int column) const
  \since 4.1

  Returns the size hint set for the tree item in the given
  \a column (see \l{QSize}).
*/

/*!
  \fn void QTreeWidgetItem::setSizeHint(int column, const QSize &size)
  \since 4.1

  Sets the size hint for the tree item in the given \a column to be \a size.
  If no size hint is set, the item delegate will compute the size hint based
  on the item data.
*/

/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::parent() const

    Returns the item's parent.

    \sa child()
*/

/*!
    \fn QTreeWidgetItem *QTreeWidgetItem::child(int index) const

    Returns the item at the given \a index in the list of the item's children.

    \sa parent()
*/

/*!
    \fn int QTreeWidgetItem::childCount() const

    Returns the number of child items.
*/

/*!
    \fn int QTreeWidgetItem::columnCount() const

    Returns the number of columns in the item.
*/

/*!
  \fn int QTreeWidgetItem::textAlignment(int column) const

  Returns the text alignment for the label in the given \a column
  (see \l{Qt::AlignmentFlag}).
*/

/*!
  \fn void QTreeWidgetItem::setTextAlignment(int column, int alignment)

  Sets the text alignment for the label in the given \a column to
  the \a alignment specified (see \l{Qt::AlignmentFlag}).
*/

/*!
    \fn int QTreeWidgetItem::indexOfChild(QTreeWidgetItem *child) const

    Returns the index of the given \a child in the item's list of children.
*/

/*!
    Constructs a tree widget item of the specified \a type. The item
    must be inserted into a tree widget.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
}


/*!
    Constructs a tree widget item of the specified \a type. The item
    must be inserted into a tree widget.
    The given list of \a strings will be set as the item text for each
    column in the item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(const QStringList &strings, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, int type)

    Constructs a tree widget item of the specified \a type and appends it
    to the items in the given \a parent.

    \sa type()
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (view && view->model()) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        model->rootItem->addChild(this);
        values.reserve(model->headerItem->columnCount());
    }
}

/*!
  \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, const QStringList &strings, int type)

  Constructs a tree widget item of the specified \a type and appends it
  to the items in the given \a parent. The given list of \a strings will be set as
  the item text for each column in the item.

  \sa type()
*/

QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, const QStringList &strings, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
    if (view && view->model()) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        model->rootItem->addChild(this);
        values.reserve(model->headerItem->columnCount());
    }
}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *parent, QTreeWidgetItem *preceding, int type)

    Constructs a tree widget item of the specified \a type and inserts it into
    the given \a parent after the \a preceding item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidget *view, QTreeWidgetItem *after, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (view) {
        QTreeModel *model = ::qobject_cast<QTreeModel*>(view->model());
        if (model) {
            int i = model->rootItem->indexOfChild(after) + 1;
            model->rootItem->insertChild(i, this);
            values.reserve(model->headerItem->columnCount());
        }
    }
}

/*!
    Constructs a tree widget item and append it to the given \a parent.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (parent)
        parent->addChild(this);
}

/*!
    Constructs a tree widget item and append it to the given \a parent.
    The given list of \a strings will be set as the item text for each column in the item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &strings, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    for (int i = 0; i < strings.count(); ++i)
        setText(i, strings.at(i));
    if (parent)
        parent->addChild(this);

}

/*!
    \fn QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *preceding, int type)

    Constructs a tree widget item of the specified \a type that is inserted
    into the \a parent after the \a preceding child item.

    \sa type()
*/
QTreeWidgetItem::QTreeWidgetItem(QTreeWidgetItem *parent, QTreeWidgetItem *after, int type)
    : rtti(type), view(0), par(0),
      itemFlags(Qt::ItemIsSelectable
                |Qt::ItemIsUserCheckable
                |Qt::ItemIsEnabled
                |Qt::ItemIsDragEnabled
                |Qt::ItemIsDropEnabled)
{
    if (parent) {
        int i = parent->indexOfChild(after) + 1;
        parent->insertChild(i, this);
    }
}

/*!
  Destroys this tree widget item.
*/

QTreeWidgetItem::~QTreeWidgetItem()
{
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (par) {
        int i = par->children.indexOf(this);
        if (model) model->beginRemoveItems(par, i, 1);
        par->children.takeAt(i);
        if (model) model->endRemoveItems();
    } else if (model) {
        if (this == model->headerItem) {
            model->headerItem = 0;
        } else {
            int i = model->rootItem->children.indexOf(this);
            model->beginRemoveItems(0, i, 1);
            model->rootItem->children.takeAt(i);
            model->endRemoveItems();
        }
    }
    // at this point the persistent indexes for the children should also be invalidated
    // since we invalidated the parent
    for (int i = 0; i < children.count(); ++i) {
        QTreeWidgetItem *child = children.at(i);
        // make sure the child does not try to remove itself from our children list
        child->par = 0;
        // make sure the child does not try to remove itself from the top level list
        child->view = 0;
        delete child;
    }

    children.clear();
}

/*!
    Creates a deep copy of the item and of its children.
*/
QTreeWidgetItem *QTreeWidgetItem::clone() const
{
    QTreeWidgetItem *copy = 0;

    QStack<const QTreeWidgetItem*> stack;
    QStack<QTreeWidgetItem*> parentStack;
    stack.push(this);
    parentStack.push(0);

    QTreeWidgetItem *root = 0;
    const QTreeWidgetItem *item = 0;
    QTreeWidgetItem *parent = 0;
    while (!stack.isEmpty()) {
        // get current item, and copied parent
        item = stack.pop();
        parent = parentStack.pop();

        // copy item
        copy = new QTreeWidgetItem(*item);
        if (!root)
            root = copy;

        // set parent and add to parents children list
        if (parent) {
            copy->par = parent;
            parent->children.append(copy);
        }

        for (int i=0; i<item->childCount(); ++i) {
            stack.push(item->child(i));
            parentStack.push(copy);
        }
    }
    return root;
}

/*!
    Sets the value for the item's \a column and \a role to the given
    \a value.

    The \a role describes the type of data specified by \a value, and is defined by
    the Qt::ItemDataRole enum.
*/
void QTreeWidgetItem::setData(int column, int role, const QVariant &value)
{
    if (column < 0)
        return;

    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole: {
        if (values.count() <= column) {
            if (model && this == model->headerItem)
                model->setColumnCount(column + 1);
            else
                values.resize(column + 1);
        }
        if (display.count() <= column) {
            for (int i = display.count() - 1; i < column - 1; ++i)
                display.append(QVariant());
            display.append(value);
        } else if (display[column] != value) {
            display[column] = value;
        } else {
            return; // value is unchanged
        }
    } break;
    case Qt::CheckStateRole:
        if (itemFlags & Qt::ItemIsTristate) {
            for (int i = 0; i < children.count(); ++i) {
                QTreeWidgetItem *child = children.at(i);
                if (child->data(column, role).isValid()) {// has a CheckState
                    Qt::ItemFlags f = itemFlags; // a little hack to avoid multiple dataChanged signals
                    itemFlags &= ~Qt::ItemIsTristate;
                    child->setData(column, role, value);
                    itemFlags = f;
                }
            }
        }
        // Don't break, but fall through
    default:
        if (column < values.count()) {
            bool found = false;
            QVector<QWidgetItemData> column_values = values.at(column);
            for (int i = 0; i < column_values.count(); ++i) {
                if (column_values.at(i).role == role) {
                    if (column_values.at(i).value == value)
                        return; // value is unchanged
                    values[column][i].value = value;
                    found = true;
                    break;
                }
            }
            if (!found)
                values[column].append(QWidgetItemData(role, value));
        } else {
            if (model && this == model->headerItem)
                model->setColumnCount(column + 1);
            else
                values.resize(column + 1);
            values[column].append(QWidgetItemData(role, value));
        }
    }

    if (model) {
        model->emitDataChanged(this, column);
        if (role == Qt::CheckStateRole) {
            QTreeWidgetItem *p;
            for (p = par; p && (p->itemFlags & Qt::ItemIsTristate); p = p->par)
                model->emitDataChanged(p, column);
        }
    }
}

/*!
    Returns the value for the item's \a column and \a role.
*/
QVariant QTreeWidgetItem::data(int column, int role) const
{
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (column >= 0 && column < display.count())
            return display.at(column);
        break;
    case Qt::CheckStateRole:
        // special case for check state in tristate
        if (children.count() && (itemFlags & Qt::ItemIsTristate))
            return childrenCheckState(column);
   default:
        if (column >= 0 && column < values.size()) {
            const QVector<QWidgetItemData> &column_values = values.at(column);
            for (int i = 0; i < column_values.count(); ++i)
                if (column_values.at(i).role == role)
                    return column_values.at(i).value;
        }
    }
    return QVariant();
}

/*!
  Returns true if the text in the item is less than the text in the
  \a other item, otherwise returns false.
*/

bool QTreeWidgetItem::operator<(const QTreeWidgetItem &other) const
{
    int column = view ? view->sortColumn() : 0;
    return text(column) < other.text(column);
}

#ifndef QT_NO_DATASTREAM

/*!
    Reads the item from stream \a in. This only reads data into a single item.

    \sa write()
*/
void QTreeWidgetItem::read(QDataStream &in)
{
    // convert from streams written before we introduced display (4.2.0)
    if (in.version() < QDataStream::Qt_4_2) {
        display.clear();
        in >> values;
        // move the display value over to the display string list
        for (int column = 0; column < values.count(); ++column) {
            display << QVariant();
            for (int i = 0; i < values.at(column).count(); ++i) {
                if (values.at(column).at(i).role == Qt::DisplayRole) {
                    display[column] = values.at(column).at(i).value;
                    values[column].remove(i--);
                }
            }
        }
    } else {
        in >> values >> display;
    }
}

/*!
    Writes the item to stream \a out. This only writes data from one single item.

    \sa read()
*/
void QTreeWidgetItem::write(QDataStream &out) const
{
    out << values << display;
}

/*!
    \since 4.1

    Constructs a copy of \a other. Note that type() and treeWidget()
    are not copied.

    This function is useful when reimplementing clone().

    \sa data(), flags()
*/
QTreeWidgetItem::QTreeWidgetItem(const QTreeWidgetItem &other)
    : rtti(Type), values(other.values), view(0), display(other.display),
      par(0), itemFlags(other.itemFlags)
{
}

/*!
    Assigns \a other's data and flags to this item. Note that type()
    and treeWidget() are not copied.

    This function is useful when reimplementing clone().

    \sa data(), flags()
*/
QTreeWidgetItem &QTreeWidgetItem::operator=(const QTreeWidgetItem &other)
{
    values = other.values;
    display = other.display;
    itemFlags = other.itemFlags;
    return *this;
}

#endif // QT_NO_DATASTREAM

/*!
  Appends the \a child item to the list of children.

  \sa insertChild() takeChild()
*/
void QTreeWidgetItem::addChild(QTreeWidgetItem *child)
{
    insertChild(children.count(), child);
}

/*!
  Inserts the \a child item at \a index in the list of children.

  If the child has already been inserted somewhere else it wont be inserted again.
*/
void QTreeWidgetItem::insertChild(int index, QTreeWidgetItem *child)
{
    if (index < 0 || index > children.count() || child == 0 || child->view != 0 || child->par != 0)
        return;

    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0)) {
        if (model->rootItem == this)
            child->par = 0;
        else
            child->par = this;
        if (view->isSortingEnabled()) {
#if 0
            Qt::SortOrder order = view->header()->sortIndicatorOrder();
            QList<QTreeWidgetItem*>::iterator it;
            it = model->sortedInsertionIterator(children.begin(), children.end(),
                                                order, child);
            index = qMax(it - children.begin(), 0);
#else
            // do a delayed sort instead
            index = children.count(); // append
            model->sortPending = true;
#endif
        }
        model->beginInsertItems(this, index, 1);
        int cols = model->columnCount();
        QStack<QTreeWidgetItem*> stack;
        stack.push(child);
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = view;
            i->values.reserve(cols);
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        children.insert(index, child);
        model->endInsertItems();
    } else {
        child->par = this;
        children.insert(index, child);
    }
}

/*!
  Removes the item at \a index and returns it, otherwise return 0.
*/
QTreeWidgetItem *QTreeWidgetItem::takeChild(int index)
{
    // we move this outside the check of the index to allow executing
    // pending sorts from inline functions, using this function (hack)
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (model && model->executePendingSort())
        model = 0; // no need to emit signals
    if (index >= 0 && index < children.count()) {
        if (model) model->beginRemoveItems(this, index, 1);
        QTreeWidgetItem *item = children.takeAt(index);
        item->par = 0;
        QStack<QTreeWidgetItem*> stack;
        stack.push(item);
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = 0;
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        if (model) model->endRemoveRows();
        return item;
    }
    return 0;
}

/*!
  \since 4.1

  Appends the given list of \a children to the item.

  \sa insertChildren() takeChildren()
*/
void QTreeWidgetItem::addChildren(const QList<QTreeWidgetItem*> &children)
{
    insertChildren(this->children.count(), children);
}

/*!
  \since 4.1

  Inserts the given list of \a children into the list of the item children at \a index .

  Children that have already been inserted somewhere else wont be inserted.
*/
void QTreeWidgetItem::insertChildren(int index, const QList<QTreeWidgetItem*> &children)
{
    if (view && view->isSortingEnabled()) {
        for (int n = 0; n < children.count(); ++n)
            insertChild(index, children.at(n));
        return;
    }
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    QStack<QTreeWidgetItem*> stack;
    QList<QTreeWidgetItem*> itemsToInsert;
    for (int n = 0; n < children.count(); ++n) {
        QTreeWidgetItem *child = children.at(n);
        if (child->view || child->par)
            continue;
        itemsToInsert.append(child);
        if (view && model) {
            if (child->childCount() == 0)
                child->view = view;
            else
                stack.push(child);
        }
        if (model && (model->rootItem == this))
            child->par = 0;
        else
            child->par = this;
    }
    if (!itemsToInsert.isEmpty()) {
        while (!stack.isEmpty()) {
            QTreeWidgetItem *i = stack.pop();
            i->view = view;
            for (int c = 0; c < i->children.count(); ++c)
                stack.push(i->children.at(c));
        }
        if (model) model->beginInsertItems(this, index, itemsToInsert.count());
        for (int n = 0; n < itemsToInsert.count(); ++n)
            this->children.insert(index + n, itemsToInsert.at(n));
        if (model) model->endInsertItems();
    }
}

/*!
  \since 4.1

  Removes the list of children and returns it, otherwise return an empty list.
*/
QList<QTreeWidgetItem*> QTreeWidgetItem::takeChildren()
{
    QList<QTreeWidgetItem*> removed;
    if (children.count() > 0) {
        QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
        if (model && model->executePendingSort())
            model = 0; // no need to emit signals
        if (model) model->beginRemoveItems(this, 0, children.count());
        for (int n = 0; n < children.count(); ++n) {
            QTreeWidgetItem *item = children.at(n);
            item->par = 0;
            QStack<QTreeWidgetItem*> stack;
            stack.push(item);
            while (!stack.isEmpty()) {
                QTreeWidgetItem *i = stack.pop();
                i->view = 0;
                for (int c = 0; c < i->children.count(); ++c)
                    stack.push(i->children.at(c));
            }
        }
        removed = children;
        children.clear(); // detach
        if (model) model->endRemoveItems();
    }
    return removed;
}

/*!
  \internal

  Sorts the children by the value in the given \a column, in the \a order
  specified. If \a climb is true, the items below each of the children will
  also be sorted.
*/
void QTreeWidgetItem::sortChildren(int column, Qt::SortOrder order, bool climb)
{
    QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0);
    if (!model)
        return;
    model->sortItems(&children, column, order);
    if (climb) {
        QList<QTreeWidgetItem*>::iterator it = children.begin();
        for (; it != children.end(); ++it)
            (*it)->sortChildren(column, order, climb);
    }
}

/*!
  \internal

  Calculates the checked state of the item based on the checked state
  of its children. E.g. if all children checked => this item is also
  checked; if some children checked => this item is partially checked;
  if no children checked => this item is unchecked.
*/
QVariant QTreeWidgetItem::childrenCheckState(int column) const
{
    if (column < 0)
        return QVariant();
    int checkedChildrenCount = 0;
    int uncheckedChildrenCount = 0;
    int validChildrenCount = 0;
    for (int i = 0; i < children.count(); ++i) {
        QVariant value = children.at(i)->data(column, Qt::CheckStateRole);
        if (!value.isValid())
            continue;
        Qt::CheckState checkState = static_cast<Qt::CheckState>(value.toInt());
        if (checkState == Qt::Unchecked)
            ++uncheckedChildrenCount;
        else
            ++checkedChildrenCount; // includes partially checked items
        ++validChildrenCount;
    }
    if (checkedChildrenCount + uncheckedChildrenCount == 0)
        return QVariant(); // value was not defined
    if (checkedChildrenCount == validChildrenCount)
        return Qt::Checked;
    if (uncheckedChildrenCount == validChildrenCount)
        return Qt::Unchecked;
    return Qt::PartiallyChecked;
}

/*!
  \internal
*/
void QTreeWidgetItem::itemChanged()
{
    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0))
        model->itemChanged(this);
}

/*!
  \internal
*/
void QTreeWidgetItem::executePendingSort() const
{
    if (QTreeModel *model = (view ? ::qobject_cast<QTreeModel*>(view->model()) : 0))
        model->executePendingSort();
}


#ifndef QT_NO_DATASTREAM
/*!
    \relates QTreeWidgetItem

    Writes the tree widget item \a item to stream \a out.

    This operator uses QTreeWidgetItem::write().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator<<(QDataStream &out, const QTreeWidgetItem &item)
{
    item.write(out);
    return out;
}

/*!
    \relates QTreeWidgetItem

    Reads a tree widget item from stream \a in into \a item.

    This operator uses QTreeWidgetItem::read().

    \sa {Format of the QDataStream Operators}
*/
QDataStream &operator>>(QDataStream &in, QTreeWidgetItem &item)
{
    item.read(in);
    return in;
}
#endif // QT_NO_DATASTREAM

class QTreeWidgetPrivate : public QTreeViewPrivate
{
    Q_DECLARE_PUBLIC(QTreeWidget)
public:
    QTreeWidgetPrivate() : QTreeViewPrivate() {}
    inline QTreeModel *model() const
        { return ::qobject_cast<QTreeModel*>(q_func()->model()); }
    inline QModelIndex index(const QTreeWidgetItem *item, int column = 0) const
        { return model()->index(item, column); }
    inline QTreeWidgetItem *item(const QModelIndex &index) const
        { return model()->item(index); }
    void _q_emitItemPressed(const QModelIndex &index);
    void _q_emitItemClicked(const QModelIndex &index);
    void _q_emitItemDoubleClicked(const QModelIndex &index);
    void _q_emitItemActivated(const QModelIndex &index);
    void _q_emitItemEntered(const QModelIndex &index);
    void _q_emitItemChanged(const QModelIndex &index);
    void _q_emitItemExpanded(const QModelIndex &index);
    void _q_emitItemCollapsed(const QModelIndex &index);
    void _q_emitCurrentItemChanged(const QModelIndex &previous, const QModelIndex &index);
    void _q_sort();
    void _q_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

void QTreeWidgetPrivate::_q_emitItemPressed(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemPressed(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemClicked(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemClicked(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemDoubleClicked(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemDoubleClicked(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemActivated(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemActivated(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemEntered(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemEntered(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemChanged(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemChanged(item(index), index.column());
}

void QTreeWidgetPrivate::_q_emitItemExpanded(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemExpanded(item(index));
}

void QTreeWidgetPrivate::_q_emitItemCollapsed(const QModelIndex &index)
{
    Q_Q(QTreeWidget);
    emit q->itemCollapsed(item(index));
}

void QTreeWidgetPrivate::_q_emitCurrentItemChanged(const QModelIndex &current,
                                                const QModelIndex &previous)
{
    Q_Q(QTreeWidget);
    QTreeWidgetItem *currentItem = model()->item(current);
    QTreeWidgetItem *previousItem = model()->item(previous);
    emit q->currentItemChanged(currentItem, previousItem);
}

void QTreeWidgetPrivate::_q_sort()
{
    Q_Q(QTreeWidget);
    if (sortingEnabled) {
        int column = q->header()->sortIndicatorSection();
        Qt::SortOrder order = q->header()->sortIndicatorOrder();
        model()->sort(column, order);
    }
}

void QTreeWidgetPrivate::_q_dataChanged(const QModelIndex &topLeft,
                                        const QModelIndex &bottomRight)
{
    Q_Q(QTreeWidget);
    if (sortingEnabled && topLeft.isValid() && bottomRight.isValid()
        && !model()->sortPending) {
        int column = q->header()->sortIndicatorSection();
        if (column >= topLeft.column() && column <= bottomRight.column()) {
            Qt::SortOrder order = q->header()->sortIndicatorOrder();
            model()->ensureSorted(column, order, topLeft.row(),
                                  bottomRight.row(), topLeft.parent());
        }
    }
}

/*!
  \class QTreeWidget

  \brief The QTreeWidget class provides a tree view that uses a predefined
  tree model.

  \ingroup model-view
  \mainclass

  The QTreeWidget class is a convenience class that provides a standard
  tree widget with a classic item-based interface similar to that used by
  the QListView class in Qt 3.
  This class is based on Qt's Model/View architecture and uses a default
  model to hold items, each of which is a QTreeWidgetItem.

  Developers who do not need the flexibility of the Model/View framework
  can use this class to create simple hierarchical lists very easily. A more
  flexible approach involves combining a QTreeView with a standard item model.
  This allows the storage of data to be separated from its representation.

  In its simplest form, a tree widget can be constructed in the following way:

  \code
    QTreeWidget *treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(1);
    QList<QTreeWidgetItem *> items;
    for (int i = 0; i < 10; ++i)
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString("item: %1").arg(i))));
    treeWidget->insertTopLevelItems(0, items);
  \endcode

  Before items can be added to the tree widget, the number of columns must
  be set with setColumnCount(). This allows each item to have one or more
  labels or other decorations. The number of columns in use can be found
  with the columnCount() function.

  The tree can have a header that contains a section for each column in
  the widget. It is easiest to set up the labels for each section by
  supplying a list of strings with setHeaderLabels(), but a custom header
  can be constructed with a QTreeWidgetItem and inserted into the tree
  with the setHeaderItem() function.

  The items in the tree can be sorted by column according to a predefined
  sort order. If sorting is enabled, the user can sort the items by clicking
  on a column header. Sorting can be enabled or disabled by calling
  setSortingEnabled(). The isSortingEnabled() function indicates whether
  sorting is enabled.

  \table 100%
  \row \o \inlineimage windowsxp-treeview.png Screenshot of a Windows XP style tree widget
       \o \inlineimage macintosh-treeview.png Screenshot of a Macintosh style tree widget
       \o \inlineimage plastique-treeview.png Screenshot of a Plastique style tree widget
  \row \o A \l{Windows XP Style Widget Gallery}{Windows XP style} tree widget.
       \o A \l{Macintosh Style Widget Gallery}{Macintosh style} tree widget.
       \o A \l{Plastique Style Widget Gallery}{Plastique style} tree widget.
  \endtable

  \sa QTreeWidgetItem, QTreeView, {Model/View Programming}, {Settings Editor Example}
*/

/*!
    \property QTreeWidget::columnCount
    \brief the number of columns displayed in the tree widget
*/

/*!
    \fn void QTreeWidget::itemActivated(QTreeWidgetItem *item, int column)

    This signal is emitted when the user activates an item by single-
    or double-clicking (depending on the platform, i.e. on the
    QStyle::SH_ItemView_ActivateItemOnSingleClick style hint) or
    pressing a special key (e.g., \key Enter).

    The specified \a item is the item that was clicked, or 0 if no
    item was clicked. The \a column is the item's column that was
    clicked, or -1 if no item was clicked.
*/

/*!
    \fn void QTreeWidget::itemPressed(QTreeWidgetItem *item, int column)

    This signal is emitted when the user presses a mouse button inside
    the widget.

    The specified \a item is the item that was clicked, or 0 if no
    item was clicked. The \a column is the item's column that was
    clicked, or -1 if no item was clicked.
*/

/*!
    \fn void QTreeWidget::itemClicked(QTreeWidgetItem *item, int column)

    This signal is emitted when the user clicks inside the widget.

    The specified \a item is the item that was clicked, or 0 if no
    item was clicked. The \a column is the item's column that was
    clicked, or -1 if no item was clicked.
*/

/*!
    \fn void QTreeWidget::itemDoubleClicked(QTreeWidgetItem *item, int column)

    This signal is emitted when the user double clicks inside the
    widget.

    The specified \a item is the item that was clicked, or 0 if no
    item was clicked. The \a column is the item's column that was
    clicked, or -1 if no item was clicked.
*/

/*!
    \fn void QTreeWidget::itemExpanded(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is expanded so that
    all of its children are displayed.

    \sa isItemExpanded(), itemCollapsed(), expandItem()
*/

/*!
    \fn void QTreeWidget::itemCollapsed(QTreeWidgetItem *item)

    This signal is emitted when the specified \a item is collapsed so that
    none of its children are displayed.

    \sa isItemExpanded(), itemExpanded(), collapseItem()
*/

/*!
    \fn void QTreeWidget::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)

    This signal is emitted when the current item changes. The current
    item is specified by \a current, and this replaces the \a previous
    current item.

    \sa setCurrentItem()
*/

/*!
    \fn void QTreeWidget::itemSelectionChanged()

    This signal is emitted when the selection changes in the tree widget.
    The current selection can be found with selectedItems().
*/

/*!
    \fn void QTreeWidget::itemEntered(QTreeWidgetItem *item, int column)

    This signal is emitted when the mouse cursor enters an \a item over the
    specified \a column.
    QTreeWidget mouse tracking needs to be enabled for this feature to work.
*/

/*!
    \fn void QTreeWidget::itemChanged(QTreeWidgetItem *item, int column)

    This signal is emitted when the contents of the \a column in the specified
    \a item changes.
*/

/*!
  Constructs a tree widget with the given \a parent.
*/

QTreeWidget::QTreeWidget(QWidget *parent)
    : QTreeView(*new QTreeWidgetPrivate(), parent)
{
    QTreeView::setModel(new QTreeModel(1, this));
    connect(this, SIGNAL(pressed(QModelIndex)),
            SLOT(_q_emitItemPressed(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)),
            SLOT(_q_emitItemClicked(QModelIndex)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            SLOT(_q_emitItemDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(activated(QModelIndex)),
            SLOT(_q_emitItemActivated(QModelIndex)));
    connect(this, SIGNAL(entered(QModelIndex)),
            SLOT(_q_emitItemEntered(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)),
            SLOT(_q_emitItemExpanded(QModelIndex)));
    connect(this, SIGNAL(collapsed(QModelIndex)),
            SLOT(_q_emitItemCollapsed(QModelIndex)));
    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(_q_emitCurrentItemChanged(QModelIndex,QModelIndex)));
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SIGNAL(itemSelectionChanged()));
    connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(_q_emitItemChanged(QModelIndex)));
    QObject::connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                     this, SLOT(_q_dataChanged(QModelIndex,QModelIndex)));
    QObject::connect(model(), SIGNAL(columnsRemoved(QModelIndex,int,int)),
                     this, SLOT(_q_sort()));

    header()->setClickable(false);
}

/*!
    Destroys the tree widget and all its items.
*/

QTreeWidget::~QTreeWidget()
{
}

/*
  Retuns the number of header columns in the view.

  \sa sortColumn(), currentColumn(), topLevelItemCount()
*/

int QTreeWidget::columnCount() const
{
    Q_D(const QTreeWidget);
    return d->model()->columnCount();
}

/*
  Sets the number of header \a columns in the tree widget.
*/

void QTreeWidget::setColumnCount(int columns)
{
    Q_D(QTreeWidget);
    if (columns < 0)
        return;
    d->model()->setColumnCount(columns);
}

/*!
  \since 4.2

  Returns the tree widget's invisible root item.

  The invisible root item provides access to the tree widget's top-level items
  through the QTreeWidgetItem API, making it possible to write functions that
  can treat top-level items and their children in a uniform way; for example,
  recursive functions.
*/

QTreeWidgetItem *QTreeWidget::invisibleRootItem() const
{
    Q_D(const QTreeWidget);
    return d->model()->rootItem;
}

/*!
  Returns the top level item at the given \a index, or 0 if the item does
  not exist.

  \sa topLevelItemCount(), insertTopLevelItem()
*/

QTreeWidgetItem *QTreeWidget::topLevelItem(int index) const
{
    Q_D(const QTreeWidget);
    d->model()->executePendingSort();
    return d->model()->rootItem->child(index);
}

/*!\property QTreeWidget::topLevelItemCount
    \brief the number of top-level items

    \sa columnCount(), currentItem()
*/

int QTreeWidget::topLevelItemCount() const
{
    Q_D(const QTreeWidget);
    return d->model()->rootItem->childCount();
}

/*!
  Inserts the \a item at \a index in the top level in the view.

  If the item has already been inserted somewhere else it wont be inserted.

  \sa addTopLevelItem(), columnCount()
*/

void QTreeWidget::insertTopLevelItem(int index, QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    d->model()->rootItem->insertChild(index, item);
}

/*!
    \since 4.1

    Appends the \a item as a top-level item in the widget.

    \sa insertTopLevelItem()
*/
void QTreeWidget::addTopLevelItem(QTreeWidgetItem *item)
{
    insertTopLevelItem(topLevelItemCount(), item);
}

/*!
  Removes the top-level item at the given \a index in the tree and
  returns it, otherwise returns 0;

  \sa insertTopLevelItem(), topLevelItem(), topLevelItemCount()
*/

QTreeWidgetItem *QTreeWidget::takeTopLevelItem(int index)
{
    Q_D(QTreeWidget);
    return d->model()->rootItem->takeChild(index);
}

/*!
    \internal
*/
int QTreeWidget::indexOfTopLevelItem(QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    d->model()->executePendingSort();
    return d->model()->rootItem->indexOfChild(item);
}

/*!
  Returns the index of the given top-level \a item, or -1 if the item
  cannot be found.

  \sa sortItems(), topLevelItemCount()
 */
int QTreeWidget::indexOfTopLevelItem(QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    d->model()->executePendingSort();
    return d->model()->rootItem->indexOfChild(item);
}

/*!
  \since 4.1

  Inserts the list of \a items at \a index in the top level in the view.

  Items that have already been inserted somewhere else wont be inserted.

  \sa addTopLevelItems()
*/
void QTreeWidget::insertTopLevelItems(int index, const QList<QTreeWidgetItem*> &items)
{
    Q_D(QTreeWidget);
    d->model()->rootItem->insertChildren(index, items);
}

/*!
  Appends the list of \a items as a top-level items in the widget.

  \sa insertTopLevelItems()
*/
void QTreeWidget::addTopLevelItems(const QList<QTreeWidgetItem*> &items)
{
    insertTopLevelItems(topLevelItemCount(), items);
}

/*!
    Returns the item used for the tree widget's header.

    \sa setHeaderItem()
*/

QTreeWidgetItem *QTreeWidget::headerItem() const
{
    Q_D(const QTreeWidget);
    return d->model()->headerItem;
}

/*!
    Sets the header \a item for the tree widget. The label for each column in
    the header is supplied by the corresponding label in the item.

    The tree widget takes ownership of the item.

    \sa headerItem(), setHeaderLabels()
*/

void QTreeWidget::setHeaderItem(QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    if (!item)
        return;
    item->view = this;

    int oldCount = columnCount();
    if (oldCount < item->columnCount())
         d->model()->beginInsertColumns(QModelIndex(), oldCount, item->columnCount());
    else
         d->model()->beginRemoveColumns(QModelIndex(), item->columnCount(), oldCount);
    delete d->model()->headerItem;
    d->model()->headerItem = item;
    if (oldCount < item->columnCount())
        d->model()->endInsertColumns();
    else
        d->model()->endRemoveColumns();
    d->model()->headerDataChanged(Qt::Horizontal, 0, oldCount);
}


/*!
  Adds a column in the header for each item in the \a labels list, and sets
  the label for each column.

  Note that setHeaderLabels() won't remove existing columns.

  \sa setHeaderItem(), setHeaderLabel()
*/
void QTreeWidget::setHeaderLabels(const QStringList &labels)
{
    Q_D(QTreeWidget);
    if (columnCount() < labels.count())
        setColumnCount(labels.count());
    QTreeModel *model = d->model();
    QTreeWidgetItem *item = model->headerItem;
    for (int i = 0; i < labels.count(); ++i)
        item->setText(i, labels.at(i));
}

/*!
    \fn void QTreeWidget::setHeaderLabel(const QString &label)
    \since 4.2

    Same as setHeaderLabels(QStringList(\a label)).
*/

/*!
    Returns the current item in the tree widget.

    \sa setCurrentItem(), currentItemChanged()
*/
QTreeWidgetItem *QTreeWidget::currentItem() const
{
    Q_D(const QTreeWidget);
    return d->item(currentIndex());
}

/*!
    \since 4.1
    Returns the current column in the tree widget.

    \sa setCurrentItem(), columnCount()
*/
int QTreeWidget::currentColumn() const
{
    return currentIndex().column();
}

/*!
  Sets the current \a item in the tree widget.

  Depending on the current selection mode, the item may also be selected.

  \sa currentItem(), currentItemChanged()
*/
void QTreeWidget::setCurrentItem(QTreeWidgetItem *item)
{
    setCurrentItem(item, 0);
}

/*!
  \since 4.1
  Sets the current \a item in the tree widget and the curernt column to \a column.

  \sa currentItem()
*/
void QTreeWidget::setCurrentItem(QTreeWidgetItem *item, int column)
{
    Q_D(const QTreeWidget);
    setCurrentIndex(d->index(item, column));
}

/*!
  Returns a pointer to the item at the coordinates \a p.

  \sa visualItemRect()
*/
QTreeWidgetItem *QTreeWidget::itemAt(const QPoint &p) const
{
    Q_D(const QTreeWidget);
    return d->item(indexAt(p));
}

/*!
    \fn QTreeWidgetItem *QTreeWidget::itemAt(int x, int y) const
    \overload

    Returns a pointer to the item at the coordinates (\a x, \a y).
*/

/*!
  Returns the rectangle on the viewport occupied by the item at \a item.

  \sa itemAt()
*/
QRect QTreeWidget::visualItemRect(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    return visualRect(d->index(item));
}

/*!
  \since 4.1

  Returns the column used to sort the contents of the widget.

  \sa sortItems()
*/
int QTreeWidget::sortColumn() const
{
    return header()->sortIndicatorSection();
}

/*!
  Sorts the items in the widget in the specified \a order by the values in
  the given \a column.

  \sa sortColumn()
*/

void QTreeWidget::sortItems(int column, Qt::SortOrder order)
{
    Q_D(QTreeWidget);
    header()->setSortIndicator(column, order);
    d->model()->sort(column, order);
}

/*!
    \internal

    ### Qt 5: remove
*/
void QTreeWidget::setSortingEnabled(bool enable)
{
    QTreeView::setSortingEnabled(enable);
}

/*!
    \internal

    ### Qt 5: remove
*/
bool QTreeWidget::isSortingEnabled() const
{
    return QTreeView::isSortingEnabled();
}

/*!
  Starts editing the \a item in the given \a column if it is editable.
*/

void QTreeWidget::editItem(QTreeWidgetItem *item, int column)
{
    Q_D(QTreeWidget);
    edit(d->index(item, column));
}

/*!
  Opens a persistent editor for the \a item in the given \a column.

  \sa closePersistentEditor()
*/

void QTreeWidget::openPersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_D(QTreeWidget);
    QAbstractItemView::openPersistentEditor(d->index(item, column));
}

/*!
  Closes the persistent editor for the \a item in the given \a column.

  This function has no effect if no persistent editor is open for this
  combination of item and column.

  \sa openPersistentEditor()
*/

void QTreeWidget::closePersistentEditor(QTreeWidgetItem *item, int column)
{
    Q_D(QTreeWidget);
    QAbstractItemView::closePersistentEditor(d->index(item, column));
}

/*!
  \since 4.1

  Returns the widget displayed in the cell specified by \a item and the given \a column.
*/
QWidget *QTreeWidget::itemWidget(QTreeWidgetItem *item, int column) const
{
    Q_D(const QTreeWidget);
    return QAbstractItemView::indexWidget(d->index(item, column));
}

/*!
  \since 4.1

  Sets the given \a widget to be displayed in the cell specified by
  the given \a item and \a column.

  Note that the given \a widget's \l {QWidget}{autoFillBackground}
  property must be set to true, otherwise the widget's background will
  be transparent, showing both the model data and the tree widget
  item.

  This function should only be used to display static content in the
  place of a tree widget item. If you want to display custom dynamic
  content or implement a custom editor widget, use QTreeView and
  subclass QItemDelegate instead.

  \sa {Delegate Classes}
*/
void QTreeWidget::setItemWidget(QTreeWidgetItem *item, int column, QWidget *widget)
{
    Q_D(QTreeWidget);
    QAbstractItemView::setIndexWidget(d->index(item, column), widget);
}

/*!
  Returns true if the \a item is selected; otherwise returns false.

  \sa itemSelectionChanged()

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::isSelected()} instead.
*/
bool QTreeWidget::isItemSelected(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    return selectionModel()->isSelected(d->index(item));

}

/*!
  If \a select is true, the given \a item is selected; otherwise it is
  deselected.

  \sa itemSelectionChanged()

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::setSelected()} instead.
*/
void QTreeWidget::setItemSelected(const QTreeWidgetItem *item, bool select)
{
    Q_D(QTreeWidget);
    selectionModel()->select(d->index(item), (select ? QItemSelectionModel::Select
                                              : QItemSelectionModel::Deselect)
                             |QItemSelectionModel::Rows);
}

/*!
  Returns a list of all selected non-hidden items.

  \sa itemSelectionChanged()
*/
QList<QTreeWidgetItem*> QTreeWidget::selectedItems() const
{
    Q_D(const QTreeWidget);
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.count(); ++i) {
        QTreeWidgetItem *item = d->item(indexes.at(i));
        if (!items.contains(item)) // ### slow, optimize later
            items.append(item);
    }
    return items;
}

/*!
  Returns a list of items that match the given \a text, using the given \a flags, in the given \a column.
*/
QList<QTreeWidgetItem*> QTreeWidget::findItems(const QString &text, Qt::MatchFlags flags, int column) const
{
    Q_D(const QTreeWidget);
    QModelIndexList indexes = d->model()->match(model()->index(0, column, QModelIndex()),
                                                Qt::DisplayRole, text, -1, flags);
    QList<QTreeWidgetItem*> items;
    for (int i = 0; i < indexes.size(); ++i)
        items.append(d->item(indexes.at(i)));
    return items;
}

/*!
  Returns true if the \a item is explicitly hidden, otherwise returns false.

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::isHidden()} instead.
*/
bool QTreeWidget::isItemHidden(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    if (item == d->model()->headerItem)
        return header()->isHidden();
    const QModelIndex index = d->index(item);
    return isRowHidden(index.row(), index.parent());
}

/*!
  Hides the given \a item if \a hide is true; otherwise shows the item.

  \sa itemChanged()

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::setHidden()} instead.
*/
void QTreeWidget::setItemHidden(const QTreeWidgetItem *item, bool hide)
{
    Q_D(QTreeWidget);
    if (item == d->model()->headerItem) {
        header()->setHidden(hide);
    } else {
        const QModelIndex index = d->index(item);
        setRowHidden(index.row(), index.parent(), hide);
    }
}

/*!
  Returns true if the given \a item is open; otherwise returns false.

  \sa itemExpanded()

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::isExpanded()} instead.
*/
bool QTreeWidget::isItemExpanded(const QTreeWidgetItem *item) const
{
    Q_D(const QTreeWidget);
    return isExpanded(d->index(item));
}

/*!
    Sets the item referred to by \a item to either closed or opened,
    depending on the value of \a expand.

    \sa expandItem(), collapseItem(), itemExpanded()

  \obsolete

  This function is deprecated. Use \l{QTreeWidgetItem::setExpanded()} instead.
*/
void QTreeWidget::setItemExpanded(const QTreeWidgetItem *item, bool expand)
{
    Q_D(QTreeWidget);
    setExpanded(d->index(item), expand);
}

/*!
    Ensures that the \a item is visible, scrolling the view if necessary using
    the specified \a hint.

    \sa currentItem(), itemAt(), topLevelItem()
*/
void QTreeWidget::scrollToItem(const QTreeWidgetItem *item, QAbstractItemView::ScrollHint hint)
{
    Q_D(QTreeWidget);
    QTreeView::scrollTo(d->index(item), hint);
}

/*!
    Expands the \a item. This causes the tree containing the item's children
    to be expanded.

    \sa collapseItem(), currentItem(), itemAt(), topLevelItem(), itemExpanded()
*/
void QTreeWidget::expandItem(const QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    expand(d->index(item));
}

/*!
    Closes the \a item. This causes the tree containing the item's children
    to be collapsed.

    \sa expandItem(), currentItem(), itemAt(), topLevelItem()
*/
void QTreeWidget::collapseItem(const QTreeWidgetItem *item)
{
    Q_D(QTreeWidget);
    collapse(d->index(item));
}

/*!
    Clears the tree widget by removing all of its items and selections.

    \bold{Note:} Since each item is removed from the tree widget before being
    deleted, the return value of QTreeWidgetItem::treeWidget() will be invalid
    when called from an item's destructor.

    \sa takeTopLevelItem(), topLevelItemCount(), columnCount()
*/
void QTreeWidget::clear()
{
    Q_D(QTreeWidget);
    selectionModel()->clear();
    d->model()->clear();
}

/*!
    Returns a list of MIME types that can be used to describe a list of
    treewidget items.

    \sa mimeData()
*/
QStringList QTreeWidget::mimeTypes() const
{
    return model()->QAbstractItemModel::mimeTypes();
}

/*!
    Returns an object that contains a serialized description of the specified
    \a items. The format used to describe the items is obtained from the
    mimeTypes() function.

    If the list of items is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *QTreeWidget::mimeData(const QList<QTreeWidgetItem*>) const
{
    return d_func()->model()->internalMimeData();
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action in the \a index in the given \a parent item.

    The default implementation returns true if the drop was
    successfully handled by decoding the mime data and inserting it
    into the model; otherwise it returns false.

    \sa supportedDropActions()
*/
bool QTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index,
                               const QMimeData *data, Qt::DropAction action)
{
    QModelIndex idx;
    if (parent) idx = indexFromItem(parent);
    return model()->QAbstractItemModel::dropMimeData(data, action , index, 0, idx);
}

/*!
  Returns the drop actions supported by this view.

  \sa Qt::DropActions
*/
Qt::DropActions QTreeWidget::supportedDropActions() const
{
    return model()->QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

/*!
  Returns a list of pointers to the items contained in the \a data object.
  If the object was not created by a QTreeWidget in the same process, the list
  is empty.

*/
QList<QTreeWidgetItem*> QTreeWidget::items(const QMimeData *data) const
{
    const QTreeWidgetMimeData *twd = qobject_cast<const QTreeWidgetMimeData*>(data);
    if (twd)
        return twd->items;
    return QList<QTreeWidgetItem*>();
}

/*!
    Returns the QModelIndex assocated with the given \a item in the given \a column.

    \sa itemFromIndex(), topLevelItem()
*/
QModelIndex QTreeWidget::indexFromItem(QTreeWidgetItem *item, int column) const
{
    Q_D(const QTreeWidget);
    return d->index(item, column);
}

/*!
    Returns a pointer to the QTreeWidgetItem assocated with the given \a index.

    \sa indexFromItem()
*/
QTreeWidgetItem *QTreeWidget::itemFromIndex(const QModelIndex &index) const
{
    Q_D(const QTreeWidget);
    return d->item(index);
}

#ifndef QT_NO_DRAGANDDROP
/*! \reimp */
void QTreeWidget::dropEvent(QDropEvent *event) {
    Q_D(QTreeWidget);
    if (event->source() == this && (event->proposedAction() == Qt::MoveAction ||
                                    dragDropMode() == QAbstractItemView::InternalMove)) {
        QModelIndex topIndex;
        int col = -1;
        int row = -1;
        if (d->dropOn(event, &row, &col, &topIndex)) {
            QList<QModelIndex> idxs = selectedIndexes();
            QList<QPersistentModelIndex> indexes;
            for (int i = 0; i < idxs.count(); i++)
                indexes.append(idxs.at(i));

            if (indexes.contains(topIndex))
                return;

            // When removing items the drop location could shift
            QPersistentModelIndex dropRow = model()->index(row, col, topIndex);

            // Remove the items
            QList<QTreeWidgetItem *> taken;
            for (int i = indexes.count() - 1; i >= 0; --i) {
                QTreeWidgetItem *parent = itemFromIndex(indexes.at(i));
                if (!parent || !parent->parent()) {
                    taken.append(takeTopLevelItem(indexes.at(i).row()));
                } else {
                    taken.append(parent->parent()->takeChild(indexes.at(i).row()));
                }
            }

            // insert them back in at their new positions
            for (int i = 0; i < indexes.count(); ++i) {
                // Either at a specific point or appended
                if (row == -1) {
                    if (topIndex.isValid()) {
                        QTreeWidgetItem *parent = itemFromIndex(topIndex);
                        parent->insertChild(parent->childCount(), taken.takeFirst());
                    } else {
                        insertTopLevelItem(topLevelItemCount(), taken.takeFirst());
                    }
                } else {
                    int r = dropRow.row() >= 0 ? dropRow.row() : row;
                    if (topIndex.isValid()) {
                        QTreeWidgetItem *parent = itemFromIndex(topIndex);
                        parent->insertChild(qMin(r, parent->childCount()), taken.takeFirst());
                    } else {
                        insertTopLevelItem(qMin(r, topLevelItemCount()), taken.takeFirst());
                    }
                }
            }

            event->accept();
            // Don't want QAbstractItemView to delete it because it was "moved" we already did it
            event->setDropAction(Qt::CopyAction);
        }
    }

    QTreeView::dropEvent(event);
}
#endif

/*!
  \reimp
*/

void QTreeWidget::setModel(QAbstractItemModel * /*model*/)
{
    qFatal("QTreeWidget::setModel() - Changing the model of the QTreeWidget is not allowed.");
}

/*!
    \reimp
*/
bool QTreeWidget::event(QEvent *e)
{
    return QTreeView::event(e);
}

#include "moc_qtreewidget.cpp"
#endif // QT_NO_TREEWIDGET
