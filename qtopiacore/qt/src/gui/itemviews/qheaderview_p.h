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

#ifndef QHEADERVIEW_P_H
#define QHEADERVIEW_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qabstractitemview_p.h"

#ifndef QT_NO_ITEMVIEWS
#include "QtCore/qbitarray.h"
#include "QtGui/qapplication.h"
#include "QtGui/qlabel.h"

class QHeaderViewPrivate: public QAbstractItemViewPrivate
{
    Q_DECLARE_PUBLIC(QHeaderView)

public:
    QHeaderViewPrivate()
        : state(NoState),
          offset(0),
          sortIndicatorOrder(Qt::DescendingOrder),
          sortIndicatorSection(0),
          sortIndicatorShown(false),
          lastPos(-1),
          firstPos(-1),
          originalSize(-1),
          section(-1),
          target(-1),
          pressed(-1),
          hover(-1),
          length(0),
          sectionCount(0),
          movableSections(false),
          clickableSections(false),
          highlightSelected(false),
          stretchLastSection(false),
          cascadingResizing(false),
          stretchSections(0),
          contentsSections(0),
          minimumSectionSize(-1),
          lastSectionSize(0),
          sectionIndicatorOffset(0),
          sectionIndicator(0),
          globalResizeMode(QHeaderView::Interactive) {}

    int sectionHandleAt(int position);
    void setupSectionIndicator(int section, int position);
    void updateSectionIndicator(int section, int position);
    void resizeSections(QHeaderView::ResizeMode globalMode, bool useGlobalMode = false);
    void _q_sectionsRemoved(const QModelIndex &,int,int);

    bool isSectionSelected(int section) const;

    inline bool rowIntersectsSelection(int row) const {
        return (selectionModel ? selectionModel->rowIntersectsSelection(row, root) : false);
    }

    inline bool columnIntersectsSelection(int column) const {
        return (selectionModel ? selectionModel->columnIntersectsSelection(column, root) : false);
    }

    inline bool isRowSelected(int row) const {
        return (selectionModel ? selectionModel->isRowSelected(row, root) : false);
    }

    inline bool isColumnSelected(int column) const {
        return (selectionModel ? selectionModel->isColumnSelected(column, root) : false);
    }

    inline void prepareSectionSelected() {
        if (!selectionModel || !selectionModel->hasSelection())
            sectionSelected.clear();
        else if (sectionSelected.count() != sectionCount * 2)
            sectionSelected.fill(false, sectionCount * 2);
        else sectionSelected.fill(false);
    }

    inline bool reverse() const {
        return q_func()->isRightToLeft() && orientation == Qt::Horizontal;
    }

    inline int logicalIndex(int visualIndex) const {
        return logicalIndices.isEmpty() ? visualIndex : logicalIndices.at(visualIndex);
    }

    inline void setDefaultValues(Qt::Orientation o) {
        orientation = o;
        defaultSectionSize = (o == Qt::Horizontal ? 100
                              : qMax(q_func()->minimumSectionSize(), 30));
        defaultAlignment = (o == Qt::Horizontal
                            ? Qt::Alignment(Qt::AlignCenter)
                            : Qt::AlignLeft|Qt::AlignVCenter);
    }

    inline bool isVisualIndexHidden(int visual) const {
        return !sectionHidden.isEmpty() && sectionHidden.at(visual);
    }

    inline void setVisualIndexHidden(int visual, bool hidden) {
        if (!sectionHidden.isEmpty()) sectionHidden.setBit(visual, hidden);
    }

    inline QHeaderView::ResizeMode visualIndexResizeMode(int visual) const {
        return headerSectionResizeMode(visual);
    }

    inline bool hasAutoResizeSections() const {
        return stretchSections || stretchLastSection || contentsSections;
    }

    QStyleOptionHeader getStyleOption() const;

    inline void invalidateCachedSizeHint() const {
        cachedSizeHint = QSize();
    }

    inline void initializeIndexMapping() const {
        if (visualIndices.count() != sectionCount
            || logicalIndices.count() != sectionCount) {
            visualIndices.resize(sectionCount);
            logicalIndices.resize(sectionCount);
            for (int s = 0; s < sectionCount; ++s) {
                visualIndices[s] = s;
                logicalIndices[s] = s;
            }
        }
    }

    inline void clearCascadingSections() {
        firstCascadingSection = sectionCount;
        lastCascadingSection = 0;
        cascadingSectionSize.clear();
    }

    inline void saveCascadingSectionSize(int visual, int size) {
        if (!cascadingSectionSize.contains(visual)) {
            cascadingSectionSize.insert(visual, size);
            firstCascadingSection = qMin(firstCascadingSection, visual);
            lastCascadingSection = qMax(lastCascadingSection, visual);
        }
    }

    inline bool sectionIsCascadable(int visual) {
        return visualIndexResizeMode(visual) == QHeaderView::Interactive;
    }


    void clear();
    void flipSortIndicator(int section);
    void cascadingResize(int visual, int newSize);

    enum State { NoState, ResizeSection, MoveSection } state;

    uint offset;
    Qt::Orientation orientation;
    Qt::SortOrder sortIndicatorOrder;
    int sortIndicatorSection;
    bool sortIndicatorShown;

    mutable QVector<int> visualIndices; // visualIndex = visualIndices.at(logicalIndex)
    mutable QVector<int> logicalIndices; // logicalIndex = row or column in the model
    mutable QBitArray sectionSelected;
    mutable QBitArray sectionHidden;
    mutable QHash<int, int> hiddenSectionSize; // from logical index to section size
    mutable QHash<int, int> cascadingSectionSize; // from visual index to section size
    mutable QSize cachedSizeHint;

    int firstCascadingSection;
    int lastCascadingSection;

    int lastPos;
    int firstPos;
    int originalSize;
    int section; // used for resizing and moving sections
    int target;
    int pressed;
    int hover;
    uint length;
    int sectionCount;
    bool movableSections;
    bool clickableSections;
    bool highlightSelected;
    bool stretchLastSection;
    bool cascadingResizing;
    int stretchSections;
    int contentsSections;
    int defaultSectionSize;
    int minimumSectionSize;
    int lastSectionSize; // $$$
    int sectionIndicatorOffset;
    Qt::Alignment defaultAlignment;
    QLabel *sectionIndicator;
    QHeaderView::ResizeMode globalResizeMode;

    // header section spans

    struct SectionSpan {
        uint size;
        int count;
        QHeaderView::ResizeMode resizeMode;
        inline SectionSpan() : size(0), count(0), resizeMode(QHeaderView::Interactive) {}
        inline SectionSpan(int length, int sections, QHeaderView::ResizeMode mode)
            : size(length), count(sections), resizeMode(mode) {}
        inline int sectionSize() const { return size / count; }
    };

    QVector<SectionSpan> sectionSpans;

    void createSectionSpan(int start, int end, int size, QHeaderView::ResizeMode mode);
    void removeSectionsFromSpans(int start, int end);
    void resizeSectionSpan(int visualIndex, uint oldSize, uint newSize);

    inline int headerSectionCount() const { // for debugging
        int count = 0;
        for (int i = 0; i < sectionSpans.count(); ++i)
            count += sectionSpans.at(i).count;
        return count;
    }

    inline int headerLength() const { // for debugging
        int len = 0;
        for (int i = 0; i < sectionSpans.count(); ++i)
            len += sectionSpans.at(i).size;
        return len;
    }

    inline void removeSpans(const QList<int> &spans) {
        for (int i = spans.count() - 1; i >= 0; --i) {
            length -= sectionSpans.at(spans.at(i)).size;
            sectionSpans.remove(spans.at(i));
        }
    }

    inline int sectionSpanIndex(int visual) const {
        int section_start = 0;
        for (int i = 0; i < sectionSpans.count(); ++i) {
            int section_end = section_start + sectionSpans.at(i).count - 1;
            if (visual >= section_start && visual <= section_end)
                return i;
            section_start = section_end + 1;
        }
        return -1;
    }

    int headerSectionSize(int visual) const;
    int headerSectionPosition(int visual) const;
    int headerVisualIndexAt(uint position) const;

    // resize mode
    void setHeaderSectionResizeMode(int visual, QHeaderView::ResizeMode mode);
    QHeaderView::ResizeMode headerSectionResizeMode(int visual) const;
    void setGlobalHeaderResizeMode(QHeaderView::ResizeMode mode);

    // other
    int viewSectionSizeHint(int logical) const;
    int adjustedVisualIndex(int visualIndex) const;

};

#endif // QT_NO_ITEMVIEWS

#endif // QHEADERVIEW_P_H
