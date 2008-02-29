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

#ifndef QTABLEVIEW_H
#define QTABLEVIEW_H

#include <QtGui/qabstractitemview.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_TABLEVIEW

class QHeaderView;
class QTableViewPrivate;

class Q_GUI_EXPORT QTableView : public QAbstractItemView
{
    Q_OBJECT
    Q_PROPERTY(bool showGrid READ showGrid WRITE setShowGrid)
    Q_PROPERTY(Qt::PenStyle gridStyle READ gridStyle WRITE setGridStyle)
    Q_PROPERTY(bool sortingEnabled READ isSortingEnabled WRITE setSortingEnabled)

public:
    explicit QTableView(QWidget *parent = 0);
    ~QTableView();

    void setModel(QAbstractItemModel *model);
    void setRootIndex(const QModelIndex &index);
    void setSelectionModel(QItemSelectionModel *selectionModel);

    QHeaderView *horizontalHeader() const;
    QHeaderView *verticalHeader() const;
    void setHorizontalHeader(QHeaderView *header);
    void setVerticalHeader(QHeaderView *header);

    int rowViewportPosition(int row) const;
    int rowAt(int y) const;

    void setRowHeight(int row, int height);
    int rowHeight(int row) const;

    int columnViewportPosition(int column) const;
    int columnAt(int x) const;

    void setColumnWidth(int column, int width);
    int columnWidth(int column) const;

    bool isRowHidden(int row) const;
    void setRowHidden(int row, bool hide);

    bool isColumnHidden(int column) const;
    void setColumnHidden(int column, bool hide);

    void setSortingEnabled(bool enable);
    bool isSortingEnabled() const;

    bool showGrid() const;

    Qt::PenStyle gridStyle() const;
    void setGridStyle(Qt::PenStyle style);

    QRect visualRect(const QModelIndex &index) const;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
    QModelIndex indexAt(const QPoint &p) const;

    void setSpan(int row, int column, int rowSpan, int columnSpan);
    int rowSpan(int row, int column) const;
    int columnSpan(int row, int column) const;

    void sortByColumn(int column, Qt::SortOrder order);

public Q_SLOTS:
    void selectRow(int row);
    void selectColumn(int column);
    void hideRow(int row);
    void hideColumn(int column);
    void showRow(int row);
    void showColumn(int column);
    void resizeRowToContents(int row);
    void resizeRowsToContents();
    void resizeColumnToContents(int column);
    void resizeColumnsToContents();
    void sortByColumn(int column);
    void setShowGrid(bool show);

protected Q_SLOTS:
    void rowMoved(int row, int oldIndex, int newIndex);
    void columnMoved(int column, int oldIndex, int newIndex);
    void rowResized(int row, int oldHeight, int newHeight);
    void columnResized(int column, int oldWidth, int newWidth);
    void rowCountChanged(int oldCount, int newCount);
    void columnCountChanged(int oldCount, int newCount);

protected:
    QTableView(QTableViewPrivate &, QWidget *parent);
    void scrollContentsBy(int dx, int dy);

    QStyleOptionViewItem viewOptions() const;
    void paintEvent(QPaintEvent *e);

    void timerEvent(QTimerEvent *event);

    int horizontalOffset() const;
    int verticalOffset() const;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);
    QRegion visualRegionForSelection(const QItemSelection &selection) const;
    QModelIndexList selectedIndexes() const;

    void updateGeometries();

    int sizeHintForRow(int row) const;
    int sizeHintForColumn(int column) const;

    void verticalScrollbarAction(int action);
    void horizontalScrollbarAction(int action);

    bool isIndexHidden(const QModelIndex &index) const;

private:
    Q_DECLARE_PRIVATE(QTableView)
    Q_DISABLE_COPY(QTableView)
};

#endif // QT_NO_TABLEVIEW

QT_END_HEADER

#endif // QTABLEVIEW_H
