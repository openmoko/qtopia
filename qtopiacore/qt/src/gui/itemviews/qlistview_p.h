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

#ifndef QLISTVIEW_P_H
#define QLISTVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemview_p.h"
#include "qrubberband.h"
#include "qbitarray.h"
#include "qbsptree_p.h"

#ifndef QT_NO_LISTVIEW

class QListViewItem
{
    friend class QListViewPrivate;
public:
    inline QListViewItem()
        : x(-1), y(-1), w(0), h(0), indexHint(-1), visited(0xffff) {}
    inline QListViewItem(const QListViewItem &other)
        : x(other.x), y(other.y), w(other.w), h(other.h),
          indexHint(other.indexHint), visited(other.visited) {}
    inline QListViewItem(QRect r, int i)
        : x(r.x()), y(r.y()), w(r.width()), h(r.height()),
          indexHint(i), visited(0xffff) {}
    inline bool operator==(const QListViewItem &other) const {
        return (x == other.x && y == other.y && w == other.w && h == other.h &&
                indexHint == other.indexHint); }
    inline bool operator!=(const QListViewItem &other) const
        { return !(*this == other); }
    inline bool isValid() const
        { return (x > -1) && (y > -1) && (w > 0) && (h > 0) && (indexHint > -1); }
    inline void invalidate()
        { x = -1; y = -1; w = 0; h = 0; }
    inline void resize(const QSize &size)
        { w = size.width(); h = size.height(); }
    inline void move(const QPoint &position)
        { x = position.x(); y = position.y(); }
    inline int width() const { return w; }
    inline int height() const { return h; }
private:
    inline QRect rect() const
        { return QRect(x, y, w, h); }
    int x, y;
    short w, h;
    mutable int indexHint;
    uint visited;
};

class QListViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QListView)
public:
    QListViewPrivate();
    ~QListViewPrivate() {}

    void clear();
    void prepareItemsLayout();

    QPoint initStaticLayout(const QRect &bounds, int spacing, int first);
    QPoint initDynamicLayout(const QRect &bounds, int spacing, int first);
    void initBspTree(const QSize &contents);

    bool doItemsLayout(int num);
    void doItemsLayout(const QRect &bounds, const QModelIndex &first, const QModelIndex &last);

    void doStaticLayout(const QRect &bounds, int first, int last);
    void doDynamicLayout(const QRect &bounds, int first, int last);

    void intersectingDynamicSet(const QRect &area) const;
    void intersectingStaticSet(const QRect &area) const;
    inline void intersectingSet(const QRect &area, bool doLayout = true) const {
        if (doLayout) executePostedLayout();
        QRect a = (q_func()->isRightToLeft() ? flipX(area.normalized()) : area.normalized());
        if (movement == QListView::Static) intersectingStaticSet(a);
        else intersectingDynamicSet(a);
    }

    void createItems(int to);
    void drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const;
    QRect itemsRect(const QVector<QModelIndex> &indexes) const;

    inline int flipX(int x) const
        { return qMax(viewport->width(), contentsSize.width()) - x; }
    inline QPoint flipX(const QPoint &p) const
        { return QPoint(flipX(p.x()), p.y()); }
    inline QRect flipX(const QRect &r) const
        { return QRect(flipX(r.x()) - r.width(), r.y(), r.width(), r.height()); }

    inline QRect viewItemRect(const QListViewItem &item) const
        { if (q_func()->isRightToLeft()) return flipX(item.rect()); return item.rect(); }

    QListViewItem indexToListViewItem(const QModelIndex &index) const;
    inline QModelIndex listViewItemToIndex(const QListViewItem &item) const
        { return model->index(itemIndex(item), column, root); }

    int itemIndex(const QListViewItem &item) const;
    static void addLeaf(QVector<int> &leaf, const QRect &area,
                        uint visited, QBspTree::Data data);

    void insertItem(int index);
    void removeItem(int index);
    void moveItem(int index, const QPoint &dest);

    QPoint snapToGrid(const QPoint &pos) const;
    QRect mapToViewport(const QRect &rect) const;
    QPoint draggedItemsDelta() const;
    QRect draggedItemsRect() const;

    QModelIndex closestIndex(const QPoint &target, const QVector<QModelIndex> &candidates) const;
    QSize itemSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int perItemScrollingPageSteps(int length, int bounds) const;

    bool selectionAllowed(const QModelIndex &index) const
    {
        if (movement == QListView::Static)
            return index.isValid();
        return true;
    }

    int perItemScrollToValue(int index, int value, int height,
                             QAbstractItemView::ScrollHint hint,
                             Qt::Orientation orientation) const;

    QStyleOptionViewItemV2 viewOptionsV2() const;

    QRect elasticBand;

    bool wrap;
    int spacing;
    QSize gridSize;
    QListView::Flow flow;
    QListView::Movement movement;
    QListView::ResizeMode resizeMode;
    QListView::LayoutMode layoutMode;
    QListView::ViewMode viewMode;

    // the properties controlling the
    // icon- or list-view modes
    enum ModeProperties {
        Wrap = 1,
        Spacing = 2,
        GridSize = 4,
        Flow = 8,
        Movement = 16,
        ResizeMode = 32
    };

    uint modeProperties : 8;

    QSize contentsSize;
    QRect layoutBounds;

    // used when laying out in batches
    int batchStartRow;
    int batchSavedDeltaSeg;
    int batchSavedPosition;

    // used for intersecting set
    mutable QVector<QModelIndex> intersectVector;

    // used when items are movable
    QBspTree tree;
    QVector<QListViewItem> items;
    QBitArray moved;

    // used when items are static
    QVector<int> flowPositions;
    QVector<int> segmentPositions;
    QVector<int> segmentStartRows;

    // timers
    QBasicTimer batchLayoutTimer;

    // used when dragging
    QVector<QModelIndex> draggedItems; // indices to the tree.itemVector
    mutable QPoint draggedItemsPos;

    // used for hidden items
    QVector<int> hiddenRows;

    int column;
    bool uniformItemSizes;
    mutable QSize cachedItemSize;
    int batchSize;
    bool wrapItemText;
};

#endif // QT_NO_LISTVIEW

#endif // QLISTVIEW_P_H
