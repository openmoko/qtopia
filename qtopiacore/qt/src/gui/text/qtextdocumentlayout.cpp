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

#include "qtextdocumentlayout_p.h"
#include "qtextdocument_p.h"
#include "qtextimagehandler_p.h"
#include "qtexttable.h"
#include "qtextlist.h"

#include "qabstracttextdocumentlayout_p.h"

#include <qpainter.h>
#include <qrect.h>
#include <qpalette.h>
#include <qdebug.h>
#include <qvarlengtharray.h>
#include <limits.h>
#include <qstyle.h>
#include <qbasictimer.h>

// #define LAYOUT_DEBUG

#ifdef LAYOUT_DEBUG
#define LDEBUG qDebug()
#define INC_INDENT debug_indent += "  "
#define DEC_INDENT debug_indent = debug_indent.left(debug_indent.length()-2)
#else
#define LDEBUG if(0) qDebug()
#define INC_INDENT do {} while(0)
#define DEC_INDENT do {} while(0)
#endif

// ################ should probably add frameFormatChange notification!

struct QLayoutStruct;

class QTextFrameData : public QTextFrameLayoutData
{
public:
    QTextFrameData();

    // relative to parent frame
    QPointF position;
    QSizeF size;

    // contents starts at (margin+border/margin+border)
    qreal margin;
    qreal border;
    qreal padding;
    // contents width includes padding (as we need to treat this on a per cell basis for tables)
    qreal contentsWidth;
    qreal contentsHeight;

    qreal minimumWidth;
    qreal maximumWidth;

    QTextFrameFormat::Position flow_position;

    QLayoutStruct *currentLayoutStruct;

    bool sizeDirty;
    bool layoutDirty;

    QList<QPointer<QTextFrame> > floats;
};

QTextFrameData::QTextFrameData()
    : margin(0), border(0), padding(0), contentsWidth(0), contentsHeight(0),
      minimumWidth(0), maximumWidth(INT_MAX), currentLayoutStruct(0),
      sizeDirty(true), layoutDirty(true)
{
}

struct QLayoutStruct {
    QLayoutStruct() : contentsWidth(0), minimumWidth(0), maximumWidth(INT_MAX),
                      fullLayout(false), pageHeight(0.0),
                      pageBottom(0.0), pageMargin(0.0)
    {}
    QTextFrame *frame;
    qreal x_left;
    qreal x_right;
    qreal y;
    qreal contentsWidth;
    qreal minimumWidth;
    qreal maximumWidth;
    bool fullLayout;
    QList<QTextFrame *> pendingFloats;
    qreal pageHeight;
    qreal pageBottom;
    qreal pageMargin;
    QRectF updateRect;

    inline void newPage()
    { if (pageHeight == INT_MAX) return; pageBottom += pageHeight; y = pageBottom - pageHeight + 2 * pageMargin; }
};

class QTextTableData : public QTextFrameData
{
public:
    inline QTextTableData() : cellSpacing(0), cellPadding(0) {}
    qreal cellSpacing, cellPadding;
    QVector<qreal> minWidths;
    QVector<qreal> maxWidths;
    QVector<qreal> widths;
    QVector<qreal> heights;
    QVector<qreal> columnPositions;
    QVector<qreal> rowPositions;
    // rows that appear at the top of a page after a page break
    QVector<int> rowsAfterPageBreak;
    QVector<qreal> rowPositionsWithoutPageBreak;

    inline qreal cellWidth(int column, int colspan) const
    { return columnPositions.at(column + colspan - 1) + widths.at(column + colspan - 1)
             - columnPositions.at(column) - 2 * cellPadding; }

    inline void calcRowPosition(int row)
    {
        if (row > 0)
            rowPositions[row] = rowPositions.at(row - 1) + heights.at(row - 1) + border + cellSpacing + border;
    }

    QRectF cellRect(const QTextTableCell &cell) const;

    inline QPointF cellPosition(int row, int col) const
    { return QPointF(columnPositions.at(col) + cellPadding, rowPositions.at(row) + cellPadding); }
    inline QPointF cellPosition(const QTextTableCell &cell) const
    { return cellPosition(cell.row(), cell.column()); }

    void updateTableSize();
};

static QTextFrameData *createData(QTextFrame *f)
{
    QTextFrameData *data;
    if (qobject_cast<QTextTable *>(f))
        data = new QTextTableData;
    else
        data = new QTextFrameData;
    f->setLayoutData(data);
    return data;
}

static inline QTextFrameData *data(QTextFrame *f)
{
    QTextFrameData *data = static_cast<QTextFrameData *>(f->layoutData());
    if (!data)
        data = createData(f);
    return data;
}

void QTextTableData::updateTableSize()
{
    const qreal effectiveMargin = this->margin + border + padding;
    qreal height = contentsHeight == -1
                   ? rowPositions.last() + heights.last() + padding + border + cellSpacing + effectiveMargin
                   : contentsHeight + 2*effectiveMargin;
    size = QSizeF(contentsWidth + 2*effectiveMargin, height);
}

QRectF QTextTableData::cellRect(const QTextTableCell &cell) const
{
    const int row = cell.row();
    const int rowSpan = cell.rowSpan();
    const int column = cell.column();
    const int colSpan = cell.columnSpan();

    return QRectF(columnPositions.at(column),
                  rowPositions.at(row),
                  columnPositions.at(column + colSpan - 1) + widths.at(column + colSpan - 1) - columnPositions.at(column),
                  rowPositions.at(row + rowSpan - 1) + heights.at(row + rowSpan - 1) - rowPositions.at(row));
}

static inline bool isEmptyBlockBeforeTable(const QTextBlock &block, const QTextFrame::Iterator &nextIt)
{
    return !nextIt.atEnd()
           && qobject_cast<QTextTable *>(nextIt.currentFrame())
           && block.isValid()
           && block.length() == 1
           && !block.blockFormat().hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)
           && nextIt.currentFrame()->firstPosition() == block.position() + 1
           ;
}

static inline bool isEmptyBlockBeforeTable(QTextFrame::Iterator it)
{
    QTextFrame::Iterator next = it; ++next;
    return it.currentFrame() == 0
           && isEmptyBlockBeforeTable(it.currentBlock(), next);
}

static inline bool isEmptyBlockAfterTable(const QTextBlock &block, const QTextFrame *lastFrame)
{
    return qobject_cast<const QTextTable *>(lastFrame)
           && block.isValid()
           && block.length() == 1
           && lastFrame->lastPosition() == block.position() - 1
           ;
}

/*

Optimisation strategies:

HTML layout:

* Distinguish between normal and special flow. For normal flow the condition:
  y1 > y2 holds for all blocks with b1.key() > b2.key().
* Special flow is: floats, table cells

* Normal flow within table cells. Tables (not cells) are part of the normal flow.


* If blocks grows/shrinks in height and extends over whole page width at the end, move following blocks.
* If height doesn't change, no need to do anything

Table cells:

* If minWidth of cell changes, recalculate table width, relayout if needed.
* What about maxWidth when doing auto layout?

Floats:
* need fixed or proportional width, otherwise don't float!
* On width/height change relayout surrounding paragraphs.

Document width change:
* full relayout needed


Float handling:

* Floats are specified by a special format object.
* currently only floating images are implemented.

*/

/*

   On the table layouting:

   +---[ table border ]-------------------------
   |      [ cell spacing ]
   |  +------[ cell border ]-----+  +--------
   |  |                          |  |
   |  |
   |  |
   |  |
   |

   rowPositions[i] and columnPositions[i] point at the cell content
   position. So for example the left border is drawn at
   x = columnPositions[i] - fd->border and similar for y.

*/

enum {
    TextIndentValue = 40
};

struct QCheckPoint
{
    qreal y;
    int positionInFrame;
    qreal minimumWidth;
    qreal maximumWidth;
    qreal contentsWidth;
};
Q_DECLARE_TYPEINFO(QCheckPoint, Q_PRIMITIVE_TYPE);

static bool operator<(const QCheckPoint &checkPoint, qreal y)
{
    return checkPoint.y < y;
}

static bool operator<(const QCheckPoint &checkPoint, int pos)
{
    return checkPoint.positionInFrame < pos;
}

static void fillBackground(QPainter *p, const QRectF &rect, QBrush brush, QRectF gradientRect = QRectF())
{
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (gradientRect.isNull())
            gradientRect = rect;
        QMatrix m;
        m.translate(gradientRect.left(), gradientRect.top());
        m.scale(gradientRect.width(), gradientRect.height());
        brush.setMatrix(m);
    }
    p->fillRect(rect, brush);
}

class QTextDocumentLayoutPrivate : public QAbstractTextDocumentLayoutPrivate
{
    Q_DECLARE_PUBLIC(QTextDocumentLayout)
public:
    QTextDocumentLayoutPrivate();

    int blockTextFlags;
    QTextOption::WrapMode wordWrapMode;
#ifdef LAYOUT_DEBUG
    mutable QString debug_indent;
#endif

    int fixedColumnWidth;
    double tabStopWidth;
    int cursorWidth;

    mutable int currentLazyLayoutPosition;
    mutable int lazyLayoutStepSize;
    QBasicTimer layoutTimer;
    mutable QBasicTimer sizeChangedTimer;
    uint showLayoutProgress : 1;
    uint insideDocumentChange : 1;

    int lastPageCount;
    qreal idealWidth;

    qreal indent(QTextBlock bl) const;

    void drawFrame(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextFrame *f) const;
    void drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                  QTextFrame::Iterator it, QTextBlock *cursorBlockNeedingRepaint) const;
    void drawBlock(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                   QTextBlock bl) const;
    void drawListItem(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                      QTextBlock bl, const QTextCharFormat *selectionFormat) const;
    void drawTableCell(const QRectF &cellRect, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &cell_context,
                       QTextTable *table, QTextTableData *td, int r, int c,
                       QTextBlock *cursorBlockNeedingRepaint, QPointF *cursorBlockOffset) const;

    enum HitPoint {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextFrame *frame, const QPointF &point, int *position, QTextLayout **l) const;
    HitPoint hitTest(QTextFrame::Iterator it, HitPoint hit, const QPointF &p,
                     int *position, QTextLayout **l) const;
    HitPoint hitTest(QTextTable *table, const QPointF &point, int *position, QTextLayout **l) const;
    HitPoint hitTest(QTextBlock bl, const QPointF &point, int *position, QTextLayout **l) const;

    QLayoutStruct layoutCell(QTextTable *t, const QTextTableCell &cell, qreal width,
                            int layoutFrom, int layoutTo);
    void setCellPosition(QTextTable *t, const QTextTableCell &cell, const QPointF &pos);
    QRectF layoutTable(QTextTable *t, int layoutFrom, int layoutTo);

    void positionFloat(QTextFrame *frame, QTextLine *currentLine = 0);

    // calls the next one
    QRectF layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo);
    QRectF layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, qreal frameWidth, qreal frameHeight);

    void layoutBlock(const QTextBlock &bl, QLayoutStruct *layoutStruct, int layoutFrom, int layoutTo,
                     const QTextBlock &previousBlock);
    void layoutFlow(QTextFrame::Iterator it, QLayoutStruct *layoutStruct, int layoutFrom, int layoutTo);
    void pageBreakInsideTable(QTextTable *table, QLayoutStruct *layoutStruct);


    void floatMargins(qreal y, const QLayoutStruct *layoutStruct, qreal *left, qreal *right) const;
    qreal findY(qreal yFrom, const QLayoutStruct *layoutStruct, qreal requiredWidth) const;

    QVector<QCheckPoint> checkPoints;

    QTextFrame::Iterator frameIteratorForYPosition(qreal y) const;
    QTextFrame::Iterator frameIteratorForTextPosition(int position) const;

    void ensureLayouted(qreal y) const;
    void ensureLayoutedByPosition(int position) const;
    inline void ensureLayoutFinished() const
    { ensureLayoutedByPosition(INT_MAX); }
    void layoutStep() const;
};

QTextDocumentLayoutPrivate::QTextDocumentLayoutPrivate()
    : blockTextFlags(0), wordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere),
      fixedColumnWidth(-1),
      tabStopWidth(80), // same default as in qtextengine.cpp
      cursorWidth(1),
      currentLazyLayoutPosition(-1),
      lazyLayoutStepSize(1000),
      lastPageCount(-1)
{
    showLayoutProgress = true;
    insideDocumentChange = false;
    idealWidth = 0;
}

QTextFrame::Iterator QTextDocumentLayoutPrivate::frameIteratorForYPosition(qreal y) const
{
    Q_Q(const QTextDocumentLayout);

    const QTextDocumentPrivate *doc = q->document()->docHandle();
    QTextFrame *rootFrame = doc->rootFrame();

    if (checkPoints.isEmpty()
        || y < 0 || y > data(rootFrame)->size.height())
        return rootFrame->begin();

    QVector<QCheckPoint>::ConstIterator checkPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), y);
    if (checkPoint == checkPoints.end())
        return rootFrame->begin();

    if (checkPoint != checkPoints.begin())
        --checkPoint;

    const int position = rootFrame->firstPosition() + checkPoint->positionInFrame;
    return frameIteratorForTextPosition(position);
}

QTextFrame::Iterator QTextDocumentLayoutPrivate::frameIteratorForTextPosition(int position) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextDocumentPrivate *doc = q->document()->docHandle();
    QTextFrame *rootFrame = doc->rootFrame();

    const QTextDocumentPrivate::BlockMap &map = doc->blockMap();
    const int begin = map.findNode(rootFrame->firstPosition());
    const int end = map.findNode(rootFrame->lastPosition()+1);

    const int block = map.findNode(position);
    const int blockPos = map.position(block);

    QTextFrame::iterator it(rootFrame, block, begin, end);

    QTextFrame *containingFrame = doc->frameAt(blockPos);
    if (containingFrame != rootFrame) {
        while (containingFrame->parentFrame() != rootFrame) {
            containingFrame = containingFrame->parentFrame();
            Q_ASSERT(containingFrame);
        }

        it.cf = containingFrame;
        it.cb = 0;
    }

    return it;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame *frame, const QPointF &point, int *position, QTextLayout **l) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    // #########
    if (fd->layoutDirty)
        return PointAfter;
    Q_ASSERT(!fd->layoutDirty);
    Q_ASSERT(!fd->sizeDirty);
    const QPointF relativePoint = point - fd->position;

    QTextFrame *rootFrame = q->document()->rootFrame();

//     LDEBUG << "checking frame" << frame->firstPosition() << "point=" << point
//            << "position" << fd->position << "size" << fd->size;
    if (frame != rootFrame) {
        if (relativePoint.y() < 0 || relativePoint.x() < 0) {
            *position = frame->firstPosition() - 1;
//             LDEBUG << "before pos=" << *position;
            return PointBefore;
        } else if (relativePoint.y() > fd->size.height() || relativePoint.x() > fd->size.width()) {
            *position = frame->lastPosition() + 1;
//             LDEBUG << "after pos=" << *position;
            return PointAfter;
        }
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(frame))
        return hitTest(table, relativePoint, position, l);

    QTextFrame::Iterator it = frame->begin();

    if (frame == rootFrame) {
        it = frameIteratorForYPosition(relativePoint.y());

        Q_ASSERT(it.parentFrame() == frame);
    }

    if (it.currentFrame())
        *position = it.currentFrame()->firstPosition();
    else
        *position = it.currentBlock().position();

    return hitTest(it, PointBefore, relativePoint, position, l);
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextFrame::Iterator it, HitPoint hit, const QPointF &p,
                                    int *position, QTextLayout **l) const
{
    INC_INDENT;

    for (; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();
        HitPoint hp;
        int pos = -1;
        if (c) {
            hp = hitTest(c, p, &pos, l);
        } else {
            hp = hitTest(it.currentBlock(), p, &pos, l);
        }
        if (hp >= PointInside) {
            if (isEmptyBlockBeforeTable(it))
                continue;
            hit = hp;
            *position = pos;
            break;
        }
        if (hp == PointBefore && pos < *position) {
            *position = pos;
            hit = hp;
        } else if (hp == PointAfter && pos > *position) {
            *position = pos;
            hit = hp;
        }
    }

    DEC_INDENT;
//     LDEBUG << "inside=" << hit << " pos=" << *position;
    return hit;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextTable *table, const QPointF &point,
                                    int *position, QTextLayout **l) const
{
    QTextTableData *td = static_cast<QTextTableData *>(data(table));

    QVector<qreal>::ConstIterator rowIt = qLowerBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), point.y());
    if (rowIt == td->rowPositions.constEnd()) {
        rowIt = td->rowPositions.constEnd() - 1;
    } else if (rowIt != td->rowPositions.constBegin()) {
        --rowIt;
    }

    QVector<qreal>::ConstIterator colIt = qLowerBound(td->columnPositions.constBegin(), td->columnPositions.constEnd(), point.x());
    if (colIt == td->columnPositions.constEnd()) {
        colIt = td->columnPositions.constEnd() - 1;
    } else if (colIt != td->columnPositions.constBegin()) {
        --colIt;
    }

    QTextTableCell cell = table->cellAt(rowIt - td->rowPositions.constBegin(),
                                        colIt - td->columnPositions.constBegin());
    if (!cell.isValid())
        return PointBefore;

    *position = cell.firstPosition();

    HitPoint hp = hitTest(cell.begin(), PointInside, point - td->cellPosition(cell), position, l);

    if (hp == PointExact)
        return hp;
    if (hp == PointAfter)
        *position = cell.lastPosition();
    return PointInside;
}

QTextDocumentLayoutPrivate::HitPoint
QTextDocumentLayoutPrivate::hitTest(QTextBlock bl, const QPointF &point, int *position, QTextLayout **l) const
{
    QTextLayout *tl = bl.layout();
    QRectF textrect = tl->boundingRect();
    textrect.translate(tl->position());
//     LDEBUG << "    checking block" << bl.position() << "point=" << point
//            << "    tlrect" << textrect;
    *position = bl.position();
    if (point.y() < textrect.top()) {
//             LDEBUG << "    before pos=" << *position;
        return PointBefore;
    } else if (point.y() > textrect.bottom()) {
        *position += bl.length();
//             LDEBUG << "    after pos=" << *position;
        return PointAfter;
    }

    QPointF pos = point - textrect.topLeft();

    // ### rtl?

    HitPoint hit = PointInside;
    *l = tl;
    int off = 0;
    for (int i = 0; i < tl->lineCount(); ++i) {
        QTextLine line = tl->lineAt(i);
        const QRectF lr = line.naturalTextRect();
        if (lr.top() > pos.y()) {
            off = qMin(off, line.textStart());
        } else if (lr.bottom() <= pos.y()) {
            off = qMax(off, line.textStart() + line.textLength());
        } else {
            if (lr.left() <= pos.x() && lr.right() >= pos.x())
                hit = PointExact;
            off = line.xToCursor(pos.x());
            break;
        }
    }
    *position += off;

//     LDEBUG << "    inside=" << hit << " pos=" << *position;
    return hit;
}

// ### could be moved to QTextBlock
qreal QTextDocumentLayoutPrivate::indent(QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    QTextBlockFormat blockFormat = bl.blockFormat();
    qreal indent = blockFormat.indent();

    QTextObject *object = q->document()->objectForFormat(blockFormat);
    if (object)
        indent += object->format().toListFormat().indent();

    qreal scale = 1;
    if (q->paintDevice()) {
        extern int qt_defaultDpi();
        scale = qreal(q->paintDevice()->logicalDpiY()) / qreal(qt_defaultDpi());
    }

    return indent * TextIndentValue * scale;
}

static void drawFrameDecoration(QPainter *painter, QTextFrame *frame, QTextFrameData *fd, const QRectF &clip, const QRectF &rect)
{
    if (fd->border) {
        painter->save();
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        const qreal margin = fd->margin + fd->border;
        const qreal w = rect.width() - 2*margin;
        const qreal h = rect.height() - 2*margin;
        // left
        painter->drawRect(QRectF(rect.left() + fd->margin, rect.top() + fd->margin, fd->border, h + 2 * fd->border));
        // top
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border, rect.top() + fd->margin, w + fd->border, fd->border));

        painter->setBrush(Qt::darkGray);
        // right
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border + w, rect.top() + fd->margin + fd->border, fd->border, h));
        // bottom
        painter->drawRect(QRectF(rect.left() + fd->margin + fd->border, rect.top() + fd->margin + fd->border + h, w + fd->border, fd->border));

        painter->restore();
    }

    const QBrush bg = frame->frameFormat().background();
    if (bg != Qt::NoBrush) {
        QRectF bgRect = rect;
        const qreal margin = fd->margin + fd->border;
        bgRect.adjust(margin, margin, -margin, -margin);

        QRectF gradientRect; // invalid makes it default to bgRect
        if (!frame->parentFrame()) {
            bgRect = clip;
            gradientRect.setWidth(painter->device()->width());
            gradientRect.setHeight(painter->device()->height());
        }
        fillBackground(painter, bgRect, bg, gradientRect);
    }
}

void QTextDocumentLayoutPrivate::drawFrame(const QPointF &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextFrame *frame) const
{
    Q_Q(const QTextDocumentLayout);
    QTextFrameData *fd = data(frame);
    // #######
    if (fd->layoutDirty)
        return;
    Q_ASSERT(!fd->sizeDirty);
    Q_ASSERT(!fd->layoutDirty);

    const QPointF off = offset + fd->position;
    if (context.clip.isValid()
        && (off.y() > context.clip.bottom() || off.y() + fd->size.height() < context.clip.top()
            || off.x() > context.clip.right() || off.x() + fd->size.width() < context.clip.left()))
        return;

//     LDEBUG << debug_indent << "drawFrame" << frame->firstPosition() << "--" << frame->lastPosition() << "at" << offset;
//     INC_INDENT;

    // if the cursor is /on/ a table border we may need to repaint it
    // afterwards, as we usually draw the decoration first
    QTextBlock cursorBlockNeedingRepaint;
    QPointF offsetOfRepaintedCursorBlock = off;

    QTextTable *table = qobject_cast<QTextTable *>(frame);
    const QRectF frameRect(off, fd->size);

    if (table) {
        const int rows = table->rows();
        const int columns = table->columns();
        QTextTableData *td = static_cast<QTextTableData *>(data(table));

        QVarLengthArray<int> selectedTableCells(context.selections.size() * 4);
        for (int i = 0; i < context.selections.size(); ++i) {
            const QAbstractTextDocumentLayout::Selection &s = context.selections.at(i);
            int row_start = -1, col_start = -1, num_rows = -1, num_cols = -1;

            if (s.cursor.currentTable() == table)
                s.cursor.selectedTableCells(&row_start, &num_rows, &col_start, &num_cols);

            selectedTableCells[i * 4] = row_start;
            selectedTableCells[i * 4 + 1] = col_start;
            selectedTableCells[i * 4 + 2] = num_rows;
            selectedTableCells[i * 4 + 3] = num_cols;
        }

        if (td->rowsAfterPageBreak.isEmpty()) {
            drawFrameDecoration(painter, frame, fd, context.clip, frameRect);
        } else {
            Q_ASSERT(td->rowsAfterPageBreak.first() > 0);
            QRectF rect = frameRect;

            const qreal extraTableHeight = td->padding + td->border + td->cellSpacing // inter cell spacing
                                           + td->margin + td->border + td->padding; // effective table margin

            qreal tableHeaderHeight = 0;
            const QTextTableFormat format = table->format();
            const int headerRowCount = qMin(format.headerRowCount(), rows - 1);
            if (headerRowCount > 0)
                tableHeaderHeight = td->rowPositions.at(headerRowCount) - td->rowPositions.at(0);

            int lastVisibleRow = td->rowsAfterPageBreak.first() - 1;

            rect.setHeight(td->rowPositions.at(lastVisibleRow) + td->heights.at(lastVisibleRow) + extraTableHeight);
            drawFrameDecoration(painter, frame, fd, context.clip, rect);

            for (int i = 0; i < td->rowsAfterPageBreak.count(); ++i) {
                const int firstVisibleRow = td->rowsAfterPageBreak.at(i);
                if (i < td->rowsAfterPageBreak.count() - 1)
                    lastVisibleRow = td->rowsAfterPageBreak.at(i + 1) - 1;
                else
                    lastVisibleRow = rows - 1;

                rect.setTop(off.y() + td->rowPositions.at(firstVisibleRow) - extraTableHeight - tableHeaderHeight);
                rect.setBottom(off.y() + td->rowPositions.at(lastVisibleRow) + td->heights.at(lastVisibleRow) + extraTableHeight);

                drawFrameDecoration(painter, frame, fd, context.clip, rect);

                for (int r = 0; r < headerRowCount; ++r)
                    for (int c = 0; c < columns; ++c) {
                        QTextTableCell cell = table->cellAt(r, c);
                        QAbstractTextDocumentLayout::PaintContext cell_context = context;
                        for (int i = 0; i < context.selections.size(); ++i) {
                            int row_start = selectedTableCells[i * 4];
                            int col_start = selectedTableCells[i * 4 + 1];
                            int num_rows = selectedTableCells[i * 4 + 2];
                            int num_cols = selectedTableCells[i * 4 + 3];

                            if (row_start != -1) {
                                if (r >= row_start && r < row_start + num_rows
                                        && c >= col_start && c < col_start + num_cols) {
                                    cell_context.selections[i].cursor.setPosition(cell.firstPosition());
                                    cell_context.selections[i].cursor.setPosition(cell.lastPosition(), QTextCursor::KeepAnchor);
                                } else {
                                    cell_context.selections[i].cursor.clearSelection();
                                }
                            }
                        }
                        QRectF cellRect = td->cellRect(cell);

                        cellRect.translate(off);

                        cellRect.translate(0, td->rowPositions.at(firstVisibleRow) - extraTableHeight - tableHeaderHeight);

                        // we draw the right/bottom border at left() + width(), so for the clipping test
                        // we have to enlarge the cell rect by one pixel in both directions
                        if (cell_context.clip.isValid() && !cellRect.adjusted(0, 0, 1, 1).intersects(cell_context.clip))
                            continue;

                        drawTableCell(cellRect, painter, cell_context, table, td, r, c, &cursorBlockNeedingRepaint,
                                      &offsetOfRepaintedCursorBlock);
                    }
            }
        }

        int firstRow = 0;
        int lastRow = rows;

        if (context.clip.isValid()) {
            QVector<qreal>::ConstIterator rowIt = qLowerBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), context.clip.top() - off.y());
            if (rowIt != td->rowPositions.constEnd() && rowIt != td->rowPositions.constBegin()) {
                --rowIt;
                firstRow = rowIt - td->rowPositions.constBegin();
            }

            rowIt = qUpperBound(td->rowPositions.constBegin(), td->rowPositions.constEnd(), context.clip.bottom() - off.y());
            if (rowIt != td->rowPositions.constEnd()) {
                ++rowIt;
                lastRow = rowIt - td->rowPositions.constBegin();
            }
        }

        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(firstRow, c);
            firstRow = qMin(firstRow, cell.row());
        }

        for (int r = firstRow; r < lastRow; ++r) {
            for (int c = 0; c < columns; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                QAbstractTextDocumentLayout::PaintContext cell_context = context;
                for (int i = 0; i < context.selections.size(); ++i) {
                    int row_start = selectedTableCells[i * 4];
                    int col_start = selectedTableCells[i * 4 + 1];
                    int num_rows = selectedTableCells[i * 4 + 2];
                    int num_cols = selectedTableCells[i * 4 + 3];

                    if (row_start != -1) {
                        if (r >= row_start && r < row_start + num_rows
                            && c >= col_start && c < col_start + num_cols) {
                            cell_context.selections[i].cursor.setPosition(cell.firstPosition());
                            cell_context.selections[i].cursor.setPosition(cell.lastPosition(), QTextCursor::KeepAnchor);
                        } else {
                            cell_context.selections[i].cursor.clearSelection();
                        }
                    }
                }
                QRectF cellRect = td->cellRect(cell);

                cellRect.translate(off);
                // we draw the right/bottom border at left() + width(), so for the clipping test
                // we have to enlarge the cell rect by one pixel in both directions
                if (cell_context.clip.isValid() && !cellRect.adjusted(0, 0, 1, 1).intersects(cell_context.clip))
                    continue;

                drawTableCell(cellRect, painter, cell_context, table, td, r, c, &cursorBlockNeedingRepaint,
                              &offsetOfRepaintedCursorBlock);
            }
        }

    } else {
        drawFrameDecoration(painter, frame, fd, context.clip, frameRect);

        QTextFrame::Iterator it = frame->begin();

        if (frame == q->document()->rootFrame())
            it = frameIteratorForYPosition(context.clip.top());

        drawFlow(off, painter, context, it, &cursorBlockNeedingRepaint);
    }

    if (cursorBlockNeedingRepaint.isValid()) {
        const QPen oldPen = painter->pen();
        painter->setPen(context.palette.color(QPalette::Text));
        const int cursorPos = context.cursorPosition - cursorBlockNeedingRepaint.position();
        cursorBlockNeedingRepaint.layout()->drawCursor(painter, offsetOfRepaintedCursorBlock,
                                                       cursorPos, cursorWidth);
        painter->setPen(oldPen);
    }

//     DEC_INDENT;

    return;
}

void QTextDocumentLayoutPrivate::drawTableCell(const QRectF &cellRect, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &cell_context,
                                               QTextTable *table, QTextTableData *td, int r, int c,
                                               QTextBlock *cursorBlockNeedingRepaint, QPointF *cursorBlockOffset) const
{
    QTextTableCell cell = table->cellAt(r, c);
    int rspan = cell.rowSpan();
    int cspan = cell.columnSpan();
    if (rspan != 1) {
        int cr = cell.row();
        if (cr != r)
            return;
    }
    if (cspan != 1) {
        int cc = cell.column();
        if (cc != c)
            return;
    }

    if (td->border) {
        const QBrush oldBrush = painter->brush();
        const QPen oldPen = painter->pen();

        painter->setBrush(Qt::darkGray);
        painter->setPen(Qt::NoPen);

        // top border
        painter->drawRect(QRectF(cellRect.left(), cellRect.top() - td->border,
                    cellRect.width() + td->border, td->border));
        // left border
        painter->drawRect(QRectF(cellRect.left() - td->border, cellRect.top() - td->border,
                    td->border, cellRect.height() + 2 * td->border));

        painter->setBrush(Qt::lightGray);

        // bottom border
        painter->drawRect(QRectF(cellRect.left(), cellRect.top() + cellRect.height(),
                    cellRect.width() + td->border, td->border));
        // right border
        painter->drawRect(QRectF(cellRect.left() + cellRect.width(), cellRect.top(),
                    td->border, cellRect.height()));

        painter->setBrush(oldBrush);
        painter->setPen(oldPen);
    }

    {
        const QBrush bg = cell.format().background();
        if (bg != Qt::NoBrush)
            fillBackground(painter, cellRect, bg);
    }

    const QPointF cellPos = QPointF(cellRect.left() + td->cellPadding, cellRect.top() + td->cellPadding);

    QTextBlock repaintBlock;
    drawFlow(cellPos, painter, cell_context, cell.begin(), &repaintBlock);
    if (repaintBlock.isValid()) {
        *cursorBlockNeedingRepaint = repaintBlock;
        *cursorBlockOffset = cellPos;
    }
}

void QTextDocumentLayoutPrivate::drawFlow(const QPointF &offset, QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context,
                                          QTextFrame::Iterator it, QTextBlock *cursorBlockNeedingRepaint) const
{
    const bool inRootFrame = (!it.atEnd() && it.parentFrame() && it.parentFrame()->parentFrame() == 0);

    QVector<QCheckPoint>::ConstIterator lastVisibleCheckPoint = checkPoints.end();
    if (inRootFrame && context.clip.isValid()) {
        lastVisibleCheckPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), qreal(context.clip.bottom()));
    }

    QTextBlock lastBlock;

    for (; !it.atEnd(); ++it) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame && !checkPoints.isEmpty()) {
            int currentPosInDoc;
            if (c)
                currentPosInDoc = c->firstPosition();
            else
                currentPosInDoc = it.currentBlock().position();

            // if we're past what is already layouted then we're better off
            // not trying to draw things that may not be positioned correctly yet
            if (currentPosInDoc >= checkPoints.last().positionInFrame)
                break;

            if (lastVisibleCheckPoint != checkPoints.end()
                && context.clip.isValid()
                && currentPosInDoc >= lastVisibleCheckPoint->positionInFrame
               )
                break;
        }

        if (c)
            drawFrame(offset, painter, context, c);
        else
            drawBlock(offset, painter, context, it.currentBlock());

        // when entering a table and the previous block is empty
        // then layoutFlow 'hides' the block that just causes a
        // new line by positioning it /on/ the table border. as we
        // draw that block before the table itself the decoration
        // 'overpaints' the cursor and we need to paint it afterwards
        // again
        if (isEmptyBlockBeforeTable(lastBlock, it)
            && lastBlock.contains(context.cursorPosition)
           ) {
            *cursorBlockNeedingRepaint = lastBlock;
        }

        lastBlock = it.currentBlock();
    }
}

void QTextDocumentLayoutPrivate::drawBlock(const QPointF &offset, QPainter *painter,
                                           const QAbstractTextDocumentLayout::PaintContext &context,
                                           QTextBlock bl) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextLayout *tl = bl.layout();
    QRectF r = tl->boundingRect();
    r.translate(offset + tl->position());
    if (context.clip.isValid() && !r.intersects(context.clip))
        return;
//      LDEBUG << debug_indent << "drawBlock" << bl.position() << "at" << offset << "br" << tl->boundingRect();

    QTextBlockFormat blockFormat = bl.blockFormat();

    QBrush bg = blockFormat.background();
    if (bg != Qt::NoBrush) {
        // don't paint into the left margin. Right margin is already excluded by the line breaking
        QRectF contentsRect = r;
        contentsRect.setLeft(contentsRect.left() + bl.blockFormat().leftMargin());
        fillBackground(painter, contentsRect, bg);
    }

    QVector<QTextLayout::FormatRange> selections;
    int blpos = bl.position();
    int bllen = bl.length();
    const QTextCharFormat *selFormat = 0;
    for (int i = 0; i < context.selections.size(); ++i) {
        const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
        const int selStart = range.cursor.selectionStart() - blpos;
        const int selEnd = range.cursor.selectionEnd() - blpos;
        if (selStart < bllen && selEnd > 0
             && selEnd > selStart) {
            QTextLayout::FormatRange o;
            o.start = selStart;
            o.length = selEnd - selStart;
            o.format = range.format;
            selections.append(o);
        } else if (range.format.hasProperty(QTextFormat::FullWidthSelection)
                   && bl.contains(range.cursor.position())) {
            // for full width selections we don't require an actual selection, just
            // a position to specify the line. that's more convenience in usage.
            QTextLayout::FormatRange o;
            QTextLine l = tl->lineForTextPosition(range.cursor.position() - blpos);
            o.start = l.textStart();
            o.length = qMax(1, l.textLength());
            o.format = range.format;
            selections.append(o);
       }
        if (selStart <= 0 && selEnd >= 1)
            selFormat = &range.format;
    }

    QTextObject *object = q->document()->objectForFormat(bl.blockFormat());
    if (object && object->format().toListFormat().style() != QTextListFormat::ListStyleUndefined)
        drawListItem(offset, painter, context, bl, selFormat);

    QPen oldPen = painter->pen();
    painter->setPen(context.palette.color(QPalette::Text));

    tl->draw(painter, offset, selections, context.clip);

    if ((context.cursorPosition >= blpos && context.cursorPosition < blpos + bllen)
        || (context.cursorPosition < -1 && !tl->preeditAreaText().isEmpty())) {
        int cpos = context.cursorPosition;
        if (cpos < -1)
            cpos = tl->preeditAreaPosition() - (cpos + 2);
        else
            cpos -= blpos;
        tl->drawCursor(painter, offset, cpos, cursorWidth);
    }

    if (blockFormat.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
        const qreal width = blockFormat.lengthProperty(QTextFormat::BlockTrailingHorizontalRulerWidth).value(r.width());
        painter->setPen(context.palette.color(QPalette::Dark));
        qreal y = r.bottom();
        if (bl.length() == 1)
            y = r.top() + r.height() / 2;

        const qreal middleX = r.left() + r.width() / 2;
        painter->drawLine(QLineF(middleX - width / 2, y, middleX + width / 2, y));
    }

    painter->setPen(oldPen);
}


void QTextDocumentLayoutPrivate::drawListItem(const QPointF &offset, QPainter *painter,
                                              const QAbstractTextDocumentLayout::PaintContext &context,
                                              QTextBlock bl, const QTextCharFormat *selectionFormat) const
{
    Q_Q(const QTextDocumentLayout);
    const QTextBlockFormat blockFormat = bl.blockFormat();
    const QTextCharFormat charFormat = bl.charFormat();
    QFont font(charFormat.font());
    if (q->paintDevice())
        font = QFont(font, q->paintDevice());

    const QFontMetrics fontMetrics(font);
    QTextObject * const object = q->document()->objectForFormat(blockFormat);
    const QTextListFormat lf = object->format().toListFormat();
    const int style = lf.style();
    QString itemText;
    QSizeF size;

    QTextLayout *layout = bl.layout();
    if (layout->lineCount() == 0)
        return;
    QTextLine firstLine = layout->lineAt(0);
    Q_ASSERT(firstLine.isValid());
    QPointF pos = (offset + layout->boundingRect().topLeft() + layout->position()).toPoint();
    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    {
        QRectF textRect = firstLine.naturalTextRect();
        pos += textRect.topLeft().toPoint();
        if (dir == Qt::RightToLeft)
            pos.rx() += textRect.width();
    }

    switch (style) {
    case QTextListFormat::ListDecimal:
    case QTextListFormat::ListLowerAlpha:
    case QTextListFormat::ListUpperAlpha:
        itemText = static_cast<QTextList *>(object)->itemText(bl);
        size.setWidth(fontMetrics.width(itemText));
        size.setHeight(fontMetrics.height());
        break;

    case QTextListFormat::ListSquare:
    case QTextListFormat::ListCircle:
    case QTextListFormat::ListDisc:
        size.setWidth(fontMetrics.lineSpacing() / 3);
        size.setHeight(size.width());
        break;

    case QTextListFormat::ListStyleUndefined:
        return;
    default: return;
    }

    QRectF r(pos, size);

    qreal xoff = fontMetrics.width(QLatin1Char(' '));
    if (dir == Qt::LeftToRight)
        xoff = -xoff - size.width();
    r.translate( xoff, (fontMetrics.height() / 2 - size.height() / 2));

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    if (selectionFormat) {
        painter->setPen(QPen(selectionFormat->foreground(), 0));
        painter->fillRect(r, selectionFormat->background());
    } else {
        QBrush fg = charFormat.foreground();
        if (fg == Qt::NoBrush)
            fg = context.palette.text();
        painter->setPen(QPen(fg, 0));
    }

    QBrush brush = context.palette.brush(QPalette::Text);

    switch (style) {
    case QTextListFormat::ListDecimal:
    case QTextListFormat::ListLowerAlpha:
    case QTextListFormat::ListUpperAlpha: {
        QTextLayout layout(itemText, font, q->paintDevice());
        layout.setCacheEnabled(true);
        QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
        option.setTextDirection(dir);
        layout.setTextOption(option);
        layout.beginLayout();
        layout.createLine();
        layout.endLayout();
        layout.draw(painter, QPointF(r.left(), pos.y()));
        break;
    }
    case QTextListFormat::ListSquare:
        painter->fillRect(r, brush);
        break;
    case QTextListFormat::ListCircle:
        painter->drawEllipse(r);
        break;
    case QTextListFormat::ListDisc:
        painter->setBrush(brush);
        painter->drawEllipse(r);
        painter->setBrush(Qt::NoBrush);
        break;
    case QTextListFormat::ListStyleUndefined:
        break;
    default:
        break;
    }

    painter->restore();
}

static bool isFrameInCell(const QTextTableCell &cell, QTextFrame *frame)
{
    const int cellStart = cell.firstPosition();
    const int cellEnd = cell.lastPosition();
    const int frameStart = frame->firstPosition();
    const int frameEnd = frame->lastPosition();

    return cellStart <= frameStart && cellStart <= frameEnd
           && cellEnd >= frameStart && cellEnd >= frameEnd;
}

QLayoutStruct QTextDocumentLayoutPrivate::layoutCell(QTextTable *t, const QTextTableCell &cell, qreal width,
                                                    int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutCell";
    QLayoutStruct layoutStruct;
    layoutStruct.frame = t;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = INT_MAX;
    layoutStruct.y = 0;
    layoutStruct.x_left = 0;
    layoutStruct.x_right = width;
    // we get called with different widths all the time (for example for figuring
    // out the min/max widths), so we always have to do the full layout ;(
    // also when for example in a table layoutFrom/layoutTo affect only one cell,
    // making that one cell grow the available width of the other cells may change
    // (shrink) and therefore when layoutCell gets called for them they have to
    // be relayouted, even if layoutFrom/layoutTo is not in their range. Hence
    // this line:
    layoutStruct.fullLayout = true;

    QList<QTextFrame *> floats;

    // ### speed up
    // layout out child frames in that cell first
    for (int i = 0; i < t->childFrames().size(); ++i){
        QTextFrame *frame = t->childFrames().at(i);
        if (isFrameInCell(cell, frame)) {
            QTextFrameData *cd = data(frame);
            cd->sizeDirty = true;
                layoutFrame(frame, frame->firstPosition(), frame->lastPosition(), width, -1);
            layoutStruct.minimumWidth = qMax(layoutStruct.minimumWidth, cd->minimumWidth);
            layoutStruct.maximumWidth = qMin(layoutStruct.maximumWidth, cd->maximumWidth);

            if (cd->flow_position != QTextFrameFormat::InFlow)
                floats.append(frame);
        }
    }

    qreal floatMinWidth = layoutStruct.minimumWidth;

    layoutFlow(cell.begin(), &layoutStruct, layoutFrom, layoutTo);

    // floats that are located inside the text (like inline images) aren't taken into account by
    // layoutFlow with regards to the cell height (layoutStruct->y), so for a safety measure we
    // do that here. For example with <td><img align="right" src="..." />blah</td>
    // when the image happens to be higher than the text
    for (int i = 0; i < floats.size(); ++i)
        layoutStruct.y = qMax(layoutStruct.y, data(floats.at(i))->size.height());

    // constraint the maximumWidth by the minimum width of the fixed size floats, to
    // keep them visible
    layoutStruct.maximumWidth = qMax(layoutStruct.maximumWidth, floatMinWidth);

    // as floats in cells get added to the table's float list but must not affect
    // floats in other cells we must clear the list here.
    data(t)->floats.clear();

//    qDebug() << "layoutCell done";

    return layoutStruct;
}

QRectF QTextDocumentLayoutPrivate::layoutTable(QTextTable *table, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutTable";
    QTextTableData *td = static_cast<QTextTableData *>(data(table));
    Q_ASSERT(td->sizeDirty);
    const int rows = table->rows();
    const int columns = table->columns();

    const QTextTableFormat fmt = table->format();

    QVector<QTextLength> columnWidthConstraints = fmt.columnWidthConstraints();
    if (columnWidthConstraints.size() != columns)
        columnWidthConstraints.resize(columns);
    Q_ASSERT(columnWidthConstraints.count() == columns);

    const qreal cellSpacing = td->cellSpacing = fmt.cellSpacing();
    td->cellPadding = fmt.cellPadding();
    const qreal margin = td->margin + td->border + td->padding;

    qreal totalWidth = fmt.width().value(td->contentsWidth);
    // two (vertical) borders per cell per column
    totalWidth -= columns * 2 * td->border;
    // inter-cell spacing
    totalWidth -= (columns - 1) * cellSpacing;
    // cell spacing at the left and right hand side
    totalWidth -= 2 * cellSpacing;
    // remember the width used to distribute to percentaged columns
    qreal initialTotalWidth = totalWidth;

    td->widths.resize(columns);
    td->widths.fill(0);

    td->minWidths.resize(columns);
    // start with a minimum width of 0. totally empty
    // cells of default created tables are invisible otherwise
    // and therefore hardly editable
    td->minWidths.fill(1);

    td->maxWidths.resize(columns);
    td->maxWidths.fill(INT_MAX);

    td->rowsAfterPageBreak.clear();

    // calculate minimum and maximum sizes of the columns
    for (int i = 0; i < columns; ++i) {
        for (int row = 0; row < rows; ++row) {
            const QTextTableCell cell = table->cellAt(row, i);
            const int cspan = cell.columnSpan();

            if (cspan > 1 && i != cell.column())
                continue;

            // to figure out the min and the max width lay out the cell at
            // maximum width. otherwise the maxwidth calculation sometimes
            // returns wrong values
            QLayoutStruct layoutStruct = layoutCell(table, cell, INT_MAX, layoutFrom, layoutTo);

            // distribute the minimum width over all columns the cell spans
            qreal widthToDistribute = layoutStruct.minimumWidth + 2 * td->cellPadding;
            for (int n = 0; n < cspan; ++n) {
                const int col = i + n;
                qreal w = widthToDistribute / (cspan - n);
                td->minWidths[col] = qMax(td->minWidths.at(col), w);
                widthToDistribute -= td->minWidths.at(col);
                if (widthToDistribute <= 0)
                    break;
            }

            // ### colspans
            qreal maxW = td->maxWidths.at(i);
            if (layoutStruct.maximumWidth != INT_MAX) {
                if (maxW == INT_MAX)
                    maxW = layoutStruct.maximumWidth + 2 * td->cellPadding;
                else
                    maxW = qMax(maxW, layoutStruct.maximumWidth + 2 * td->cellPadding);
            }
            td->maxWidths[i] = qMax(td->minWidths.at(i), maxW);
        }
    }

    // set fixed values, figure out total percentages used and number of
    // variable length cells. Also assign the minimum width for variable columns.
    qreal totalPercentage = 0;
    int variableCols = 0;
    for (int i = 0; i < columns; ++i) {
        const QTextLength &length = columnWidthConstraints.at(i);
        if (length.type() == QTextLength::FixedLength) {
            td->widths[i] = qMax(length.rawValue(), td->minWidths.at(i));
            totalWidth -= td->widths.at(i);
        } else if (length.type() == QTextLength::PercentageLength) {
            totalPercentage += length.rawValue();
        } else if (length.type() == QTextLength::VariableLength) {
            variableCols++;

            td->widths[i] = td->minWidths.at(i);
            totalWidth -= td->minWidths.at(i);
        }
    }

    // set percentage values
    {
        const qreal totalPercentagedWidth = initialTotalWidth * totalPercentage / 100;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::PercentageLength) {
                const qreal percentWidth = totalPercentagedWidth * columnWidthConstraints.at(i).rawValue() / totalPercentage;
                td->widths[i] = qMax(percentWidth, td->minWidths.at(i));
                totalWidth -= td->widths.at(i);
            }
    }

    // for variable columns distribute the remaining space
    if (variableCols > 0 && totalWidth > 0) {
        QVarLengthArray<int> columnsWithProperMaxSize;
        for (int i = 0; i < columns; ++i)
            if (columnWidthConstraints.at(i).type() == QTextLength::VariableLength
                && td->maxWidths.at(i) != INT_MAX)
                columnsWithProperMaxSize.append(i);

        qreal lastTotalWidth = totalWidth;
        while (totalWidth > 0) {
            for (int k = 0; k < columnsWithProperMaxSize.count(); ++k) {
                const int col = columnsWithProperMaxSize[k];
                const int colsLeft = columnsWithProperMaxSize.count() - k;
                const qreal w = qMin(td->maxWidths.at(col) - td->widths.at(col), totalWidth / colsLeft);
                td->widths[col] += w;
                totalWidth -= w;
            }
            if (totalWidth == lastTotalWidth)
                break;
            lastTotalWidth = totalWidth;
        }

        if (totalWidth > 0
            // don't unnecessarily grow variable length sized tables
            && fmt.width().type() != QTextLength::VariableLength) {
            const qreal widthPerAnySizedCol = totalWidth / variableCols;
            for (int col = 0; col < columns; ++col) {
                if (columnWidthConstraints.at(col).type() == QTextLength::VariableLength)
                    td->widths[col] += widthPerAnySizedCol;
            }
        }
    }


    td->columnPositions.resize(columns);
    td->columnPositions[0] = margin /*includes table border*/ + cellSpacing + td->border;

    for (int i = 1; i < columns; ++i)
        td->columnPositions[i] = td->columnPositions.at(i-1) + td->widths.at(i-1) + td->border + cellSpacing + td->border;

    td->heights.resize(rows);
    td->heights.fill(0);

    td->rowPositions.resize(rows);
    td->rowPositions[0] = margin /*includes table border*/ + cellSpacing + td->border;

    bool haveRowSpannedCells = false;

    // now that we have the column widths we can lay out all cells with the right
    // width, to calculate the row heights. we have to use two passes though, cells
    // which span more than one row have to be processed later to avoid them enlarging
    // other calls too much
    //
    // ### this could be made faster by iterating over the cells array of QTextTable
    for (int r = 0; r < rows; ++r) {
        td->calcRowPosition(r);

        for (int c = 0; c < columns; ++c) {
            QTextTableCell cell = table->cellAt(r, c);
            const int rspan = cell.rowSpan();
            const int cspan = cell.columnSpan();

            if (cspan > 1 && cell.column() != c)
                continue;

            if (rspan > 1) {
                haveRowSpannedCells = true;
                continue;
            }

            const qreal width = td->cellWidth(c, cspan);
//            qDebug() << "layoutCell for cell at row" << r << "col" << c;
            QLayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo);

            td->heights[r] = qMax(td->heights.at(r), layoutStruct.y + 2 * td->cellPadding);
        }
    }

    if (haveRowSpannedCells) {
        for (int r = 0; r < rows; ++r) {
            td->calcRowPosition(r);

            for (int c = 0; c < columns; ++c) {
                QTextTableCell cell = table->cellAt(r, c);
                const int rspan = cell.rowSpan();
                const int cspan = cell.columnSpan();

                if (cspan > 1 && cell.column() != c)
                    continue;

                if (rspan == 1)
                    continue;

                if (cell.row() != r)
                    continue;

                const qreal width = td->cellWidth(c, cspan);
                QLayoutStruct layoutStruct = layoutCell(table, cell, width, layoutFrom, layoutTo);

                // the last row gets all the remaining space
                qreal heightToDistribute = layoutStruct.y + 2 * td->cellPadding;
                for (int n = 0; n < rspan - 1; ++n) {
                    const int row = r + n;
                    heightToDistribute -= td->heights.at(row) + td->border + cellSpacing + td->border;
                    if (heightToDistribute <= 0)
                        break;
                }

                if (heightToDistribute > 0) {
                    const int lastRow = r + rspan - 1;
                    td->heights[lastRow] = qMax(td->heights.at(lastRow), heightToDistribute);
                }
            }
        }
    }

    // - margin to compensate the + margin in columnPositions[0]
//    td->contentsWidth = qMax(td->contentsWidth,
//                             td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin);
    td->contentsWidth = td->columnPositions.last() + td->widths.last() + td->padding + td->border + cellSpacing - margin;

    td->minimumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i) {
        td->minimumWidth += td->minWidths.at(i) + td->border + cellSpacing + td->border;
    }
    td->minimumWidth += margin - td->border;

    td->maximumWidth = td->columnPositions.at(0);
    for (int i = 0; i < columns; ++i)
        if (td->maxWidths.at(i) != INT_MAX)
            td->maximumWidth += td->maxWidths.at(i) + td->border + cellSpacing + td->border;
    td->maximumWidth += margin - td->border;

    td->rowPositionsWithoutPageBreak = td->rowPositions;

    td->updateTableSize();
    td->sizeDirty = false;
    return QRectF(); // invalid rect -> update everything
}

void QTextDocumentLayoutPrivate::positionFloat(QTextFrame *frame, QTextLine *currentLine)
{
    QTextFrameData *fd = data(frame);

    QTextFrame *parent = frame->parentFrame();
    Q_ASSERT(parent);
    QTextFrameData *pd = data(parent);
    Q_ASSERT(pd && pd->currentLayoutStruct);

    if (!pd->floats.contains(frame))
        pd->floats.append(frame);
    fd->layoutDirty = true;
    Q_ASSERT(!fd->sizeDirty);

//     qDebug() << "positionFloat:" << frame << "width=" << fd->size.width();
    qreal y = pd->currentLayoutStruct->y;
    if (currentLine) {
        qreal left, right;
        floatMargins(y, pd->currentLayoutStruct, &left, &right);
//         qDebug() << "have line: right=" << right << "left=" << left << "textWidth=" << currentLine->textWidth();
        if (right - left < currentLine->naturalTextWidth() + fd->size.width()) {
            pd->currentLayoutStruct->pendingFloats.append(frame);
//             qDebug() << "    adding to pending list";
            return;
        }
    }

    if (!parent->parentFrame() /* float in root frame */
        && y + fd->size.height() > pd->currentLayoutStruct->pageBottom) {
        y = pd->currentLayoutStruct->pageBottom;
    }

    y = findY(y, pd->currentLayoutStruct, fd->size.width());

    qreal left, right;
    floatMargins(y, pd->currentLayoutStruct, &left, &right);

    if (fd->flow_position == QTextFrameFormat::FloatLeft)
        fd->position = QPointF(left, y);
    else
        fd->position = QPointF(right - fd->size.width(), y);

//     qDebug()<< "float positioned at " << fd->position;
    fd->layoutDirty = false;
}

QRectF QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo)
{
    LDEBUG << "layoutFrame (pre)";
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameFormat fformat = f->frameFormat();

    QTextFrame *parent = f->parentFrame();
    const QTextFrameData *pd = parent ? data(parent) : 0;

    const qreal maximumWidth = qMax(qreal(0), pd ? pd->contentsWidth : q_func()->document()->pageSize().width());

    const qreal width = fformat.width().value(maximumWidth);

    QTextLength height = fformat.height();
    qreal h = height.value(pd ? pd->contentsHeight : -1);

    return layoutFrame(f, layoutFrom, layoutTo, width, h);
}

QRectF QTextDocumentLayoutPrivate::layoutFrame(QTextFrame *f, int layoutFrom, int layoutTo, qreal frameWidth, qreal frameHeight)
{
    LDEBUG << "layoutFrame from=" << layoutFrom << "to=" << layoutTo;
    Q_Q(QTextDocumentLayout);
    Q_ASSERT(data(f)->sizeDirty);
//     qDebug("layouting frame (%d--%d), parent=%p", f->firstPosition(), f->lastPosition(), f->parentFrame());

    QTextFrameData *fd = data(f);
    const qreal oldContentsWidth = fd->contentsWidth;
    qreal newContentsWidth;

    {
        QTextFrameFormat fformat = f->frameFormat();
        // set sizes of this frame from the format
        fd->margin = fformat.margin();
        fd->border = fformat.border();
        fd->padding = fformat.padding();

        newContentsWidth = frameWidth - 2*(fd->margin + fd->border + fd->padding);

        if (frameHeight != -1) {
            fd->contentsHeight = frameHeight - 2*(fd->margin + fd->border + fd->padding);
        } else {
            fd->contentsHeight = frameHeight;
        }

        fd->flow_position = fformat.position();
    }

    int startPos = f->firstPosition();
    int endPos = f->lastPosition();
    if (startPos > endPos) {
        fd->contentsWidth = newContentsWidth;
        // inline image
        QTextCharFormat format = q->format(startPos - 1);
        QTextObjectInterface *iface = q->handlerForObject(format.objectType());
        if (iface)
            fd->size = iface->intrinsicSize(q->document(), startPos - 1, format).toSize();
        fd->sizeDirty = false;
        return QRectF();
    }

    if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
        fd->contentsWidth = newContentsWidth;
        return layoutTable(table, layoutFrom, layoutTo);
    }

    // set fd->contentsWidth temporarily, so that layoutFrame for the children
    // picks the right width. We'll initialize it properly at the end of this
    // function.
    fd->contentsWidth = newContentsWidth;
    qreal maxChildFrameWidth = 0;
    // layout child frames
    QList<QTextFrame *> children = f->childFrames();
    for (int i = 0; i < children.size(); ++i) {
        QTextFrame *c = children.at(i);
        QTextFrameData *cd = data(c);
        if (cd->sizeDirty) {
            layoutFrame(c, layoutFrom, layoutTo);
        }
        maxChildFrameWidth = qMax(maxChildFrameWidth, cd->size.width());
    }

    qreal margin = fd->margin + fd->border + fd->padding;
    QLayoutStruct layoutStruct;
    layoutStruct.frame = f;
    layoutStruct.x_left = margin;
    layoutStruct.x_right = layoutStruct.x_left + newContentsWidth;
    layoutStruct.y = margin;
    layoutStruct.contentsWidth = 0;
    layoutStruct.minimumWidth = 0;
    layoutStruct.maximumWidth = INT_MAX;
    layoutStruct.fullLayout = oldContentsWidth != newContentsWidth;
    layoutStruct.updateRect = QRectF(QPointF(0, 0), QSizeF(INT_MAX, INT_MAX));
    LDEBUG << "layoutStruct: x_left" << layoutStruct.x_left << "x_right" << layoutStruct.x_right
           << "fullLayout" << layoutStruct.fullLayout;

    if (!f->parentFrame()) {
        layoutStruct.pageHeight = q->document()->pageSize().height();
        if (layoutStruct.pageHeight < 0)
            layoutStruct.pageHeight = INT_MAX;
        layoutStruct.pageBottom = layoutStruct.pageHeight - fd->margin;
        layoutStruct.pageMargin = fd->margin;
        idealWidth = 0; // reset
    }

    QTextFrame::Iterator it = f->begin();
    layoutFlow(it, &layoutStruct, layoutFrom, layoutTo);

    if (!f->parentFrame())
        idealWidth = qMax(maxChildFrameWidth, layoutStruct.contentsWidth);

    qreal actualWidth = qMax(newContentsWidth, qMax(maxChildFrameWidth, layoutStruct.contentsWidth));
    fd->contentsWidth = actualWidth;
    if (newContentsWidth <= 0) { // nowrap layout?
        fd->contentsWidth = newContentsWidth;
    }

    fd->minimumWidth = layoutStruct.minimumWidth;
    fd->maximumWidth = layoutStruct.maximumWidth;

    qreal height = fd->contentsHeight == -1
                 ? layoutStruct.y + margin
                 : fd->contentsHeight + 2*margin;
    fd->size = QSizeF(actualWidth + 2*margin, height);
    fd->sizeDirty = false;
    return layoutStruct.updateRect;
}

void QTextDocumentLayoutPrivate::layoutFlow(QTextFrame::Iterator it, QLayoutStruct *layoutStruct,
                                            int layoutFrom, int layoutTo)
{
    Q_Q(QTextDocumentLayout);
    LDEBUG << "layoutFlow from=" << layoutFrom << "to=" << layoutTo;
    QTextFrameData *fd = data(layoutStruct->frame);

    fd->currentLayoutStruct = layoutStruct;

    QTextFrame::Iterator previousIt;

    const bool inRootFrame = (it.parentFrame() == q->document()->rootFrame());
    if (inRootFrame) {
        bool redoCheckPoints = layoutStruct->fullLayout || checkPoints.isEmpty();

        if (!redoCheckPoints) {
            QVector<QCheckPoint>::Iterator checkPoint = qLowerBound(checkPoints.begin(), checkPoints.end(), layoutFrom);
            if (checkPoint != checkPoints.end()) {
                if (checkPoint != checkPoints.begin())
                    --checkPoint;

                layoutStruct->y = checkPoint->y;
                layoutStruct->minimumWidth = checkPoint->minimumWidth;
                layoutStruct->maximumWidth = checkPoint->maximumWidth;
                layoutStruct->contentsWidth = checkPoint->contentsWidth;

                if (layoutStruct->pageHeight > 0.0) {
                    int page = int(layoutStruct->y / layoutStruct->pageHeight);
                    layoutStruct->pageBottom = (page + 1) * layoutStruct->pageHeight - layoutStruct->pageMargin;
                }

                it = frameIteratorForTextPosition(checkPoint->positionInFrame);
                checkPoints.resize(checkPoint - checkPoints.begin() + 1);

                if (checkPoint != checkPoints.begin()) {
                    previousIt = it;
                    --previousIt;
                }
            } else {
                redoCheckPoints = true;
            }
        }

        if (redoCheckPoints) {
            checkPoints.clear();
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.positionInFrame = 0;
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
        }
    }

    while (!it.atEnd()) {
        QTextFrame *c = it.currentFrame();

        if (inRootFrame) {
            int docPos;
            if (it.currentFrame())
                docPos = it.currentFrame()->firstPosition();
            else
                docPos = it.currentBlock().position();

            if (qAbs(layoutStruct->y - checkPoints.last().y) > 2000) {
                qreal left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                if (left == layoutStruct->x_left && right == layoutStruct->x_right) {
                    QCheckPoint p;
                    p.y = layoutStruct->y;
                    p.positionInFrame = docPos;
                    p.minimumWidth = layoutStruct->minimumWidth;
                    p.maximumWidth = layoutStruct->maximumWidth;
                    p.contentsWidth = layoutStruct->contentsWidth;
                    checkPoints.append(p);

                    if (currentLazyLayoutPosition != -1
                        && docPos > currentLazyLayoutPosition + lazyLayoutStepSize)
                        break;

                    if (layoutTo != -1
                        && docPos > layoutTo)
                        break;
                }
            }
        }

        if (c) {
            // position child frame
            QTextFrameData *cd = data(c);
            Q_ASSERT(!cd->sizeDirty);
            if (cd->flow_position == QTextFrameFormat::InFlow) {
                if (c->frameFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
                    layoutStruct->newPage();

                qreal left, right;
                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, layoutStruct->x_left);
                right = qMin(right, layoutStruct->x_right);

                if (right - left < cd->size.width()) {
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, cd->size.width());
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                }

                QPointF pos(left, layoutStruct->y);

                Qt::Alignment align = Qt::AlignLeft;

                QTextTable *table = qobject_cast<QTextTable *>(c);

                if (table)
                    align = table->format().alignment() & Qt::AlignHorizontal_Mask;

                // align only if there is space for alignment
                if (right - left > cd->size.width()) {
                    if (align & Qt::AlignRight)
                        pos.rx() += layoutStruct->x_right - cd->size.width();
                    else if (align & Qt::AlignHCenter)
                        pos.rx() += (layoutStruct->x_right - cd->size.width()) / 2;
                }

                cd->position = pos;
                layoutStruct->y += cd->size.height();
                cd->layoutDirty = false;

                if (table) {
                    QTextTableData *td = static_cast<QTextTableData *>(data(table));
                    // if the table was previously broken across a page boundary
                    // (due to lazy layouting) then we need to reset the row positions
                    // and the table height (from the row positions) and call
                    // pageBreakInsideTable again.
                    if (!td->rowsAfterPageBreak.isEmpty()) {
                        td->rowsAfterPageBreak.clear();
                        td->rowPositions = td->rowPositionsWithoutPageBreak;
                        td->updateTableSize();
                    }
                }

                if (inRootFrame
                    && cd->position.y() + cd->size.height() > layoutStruct->pageBottom
                   ) {

                    if (table && cd->size.height() > layoutStruct->pageHeight / 2) {
                        pageBreakInsideTable(table, layoutStruct);
                    } else {
                        layoutStruct->newPage();
                        cd->position.setY(layoutStruct->y);
                        layoutStruct->y += cd->size.height();
                    }
                }

                if (c->frameFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
                    layoutStruct->newPage();
            } else {
                positionFloat(c);
            }
            previousIt = it;
            ++it;
        } else {
            QTextFrame::Iterator lastIt;
            if (!previousIt.atEnd())
                lastIt = previousIt;
            previousIt = it;
            QTextBlock block = it.currentBlock();
            ++it;

            if (block.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysBefore)
                layoutStruct->newPage();

            const qreal origY = layoutStruct->y;

            // layout and position child block
            layoutBlock(block, layoutStruct, layoutFrom, layoutTo, lastIt.currentBlock());

            // if the block right before a table is empty 'hide' it by
            // positioning it into the table border
            if (isEmptyBlockBeforeTable(block, it)) {
                layoutStruct->y = origY;
                continue;
            }

            // if the block right after a table is empty then 'hide' it, too
            if (isEmptyBlockAfterTable(block, lastIt.currentFrame())) {
                QTextTableData *td = static_cast<QTextTableData *>(data(lastIt.currentFrame()));
                QTextLayout *layout = block.layout();

                QPointF pos(td->position.x() + td->size.width(),
                            td->position.y() + td->size.height() - layout->boundingRect().height());

                layout->setPosition(pos);
                layoutStruct->y = origY;
            }

            if (block.blockFormat().pageBreakPolicy() & QTextFormat::PageBreak_AlwaysAfter)
                layoutStruct->newPage();
        }
    }

    // a float at the bottom of a frame may make it taller, hence the qMax() for layoutStruct->y.
    // we don't need to do it for tables though because floats in tables are per table
    // and not per cell and layoutCell already takes care of doing the same as we do here
    if (!qobject_cast<QTextTable *>(layoutStruct->frame)) {
        QList<QTextFrame *> children = layoutStruct->frame->childFrames();
        for (int i = 0; i < children.count(); ++i) {
            QTextFrameData *fd = data(children.at(i));
            if (!fd->layoutDirty && fd->flow_position != QTextFrameFormat::InFlow)
                layoutStruct->y = qMax(layoutStruct->y, fd->position.y() + fd->size.height());
        }
    }

    if (inRootFrame) {
        if (it.atEnd()) {
            //qDebug() << "layout done!";
            currentLazyLayoutPosition = -1;
            QCheckPoint cp;
            cp.y = layoutStruct->y;
            cp.positionInFrame = q->document()->docHandle()->length();
            cp.minimumWidth = layoutStruct->minimumWidth;
            cp.maximumWidth = layoutStruct->maximumWidth;
            cp.contentsWidth = layoutStruct->contentsWidth;
            checkPoints.append(cp);
        } else {
            currentLazyLayoutPosition = checkPoints.last().positionInFrame;
            // #######
            //checkPoints.last().positionInFrame = q->document()->docHandle()->length();
        }
    }


    fd->currentLayoutStruct = 0;
}

void QTextDocumentLayoutPrivate::layoutBlock(const QTextBlock &bl, QLayoutStruct *layoutStruct,
                                             int layoutFrom, int layoutTo, const QTextBlock &previousBlock)
{
    Q_Q(QTextDocumentLayout);

    const QTextDocument *doc = q->document();
    QTextBlockFormat blockFormat = bl.blockFormat();
    QTextLayout *tl = bl.layout();

    LDEBUG << "layoutBlock from=" << layoutFrom << "to=" << layoutTo;

    Qt::LayoutDirection dir = blockFormat.layoutDirection();
    if (blockTextFlags & ((int)QTextDocumentLayout::LTR|(int)QTextDocumentLayout::RTL)) {
        if (!blockFormat.hasProperty(QTextFormat::LayoutDirection))
            dir = blockTextFlags & QTextDocumentLayout::LTR ? Qt::LeftToRight : Qt::RightToLeft;
    }
    Qt::Alignment align = QStyle::visualAlignment(dir, blockFormat.alignment());
    if (blockTextFlags & Qt::AlignHorizontal_Mask) {
        if (!blockFormat.hasProperty(QTextFormat::BlockAlignment))
            align = (Qt::Alignment)(blockTextFlags & Qt::AlignHorizontal_Mask);
    }
    QTextOption option(align);
    option.setTextDirection(dir);
    if (blockTextFlags & Qt::TextSingleLine
        || blockFormat.nonBreakableLines()
        || doc->pageSize().width() < 0)
        option.setWrapMode(QTextOption::ManualWrap);
    else
        option.setWrapMode(wordWrapMode);
    option.setTabStop(tabStopWidth);
    option.setUseDesignMetrics(doc->useDesignMetrics());
    tl->setTextOption(option);

    const bool haveWordOrAnyWrapMode = (option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere);

//    qDebug() << "layoutBlock; width" << layoutStruct->x_right - layoutStruct->x_left << "(maxWidth is btw" << tl->maximumWidth() << ")";

    if (previousBlock.isValid()) {
        qreal margin = qMax(blockFormat.topMargin(), previousBlock.blockFormat().bottomMargin());
        if (margin > 0 && q->paintDevice()) {
            extern int qt_defaultDpi();
            margin *= qreal(q->paintDevice()->logicalDpiY()) / qreal(qt_defaultDpi());
        }
        layoutStruct->y += margin;
    }

    //QTextFrameData *fd = data(layoutStruct->frame);

    const qreal indent = this->indent(bl);
    const qreal totalLeftMargin = blockFormat.leftMargin() + (dir == Qt::RightToLeft ? 0 : indent);
    const qreal totalRightMargin = blockFormat.rightMargin() + (dir == Qt::RightToLeft ? indent : 0);

    const QPointF oldPosition = tl->position();
    tl->setPosition(QPointF(layoutStruct->x_left, layoutStruct->y));

    if (layoutStruct->fullLayout
        || (bl.position() + bl.length() > layoutFrom && bl.position() <= layoutTo)
        // force relayout if we cross a page boundary
        || (layoutStruct->pageHeight > 0.0 && layoutStruct->y + tl->boundingRect().height() > layoutStruct->pageBottom)) {

//         qDebug() << "    layouting block at" << bl.position();
        const qreal cy = layoutStruct->y;
        const qreal l = layoutStruct->x_left  + totalLeftMargin;
        const qreal r = layoutStruct->x_right - totalRightMargin;

        tl->beginLayout();
        bool firstLine = true;
        while (1) {
            QTextLine line = tl->createLine();
            if (!line.isValid())
                break;

            qreal left, right;
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            qreal text_indent = 0;
            if (firstLine) {
                text_indent = blockFormat.textIndent();
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
                firstLine = false;
            }
//         qDebug() << "layout line y=" << currentYPos << "left=" << left << "right=" <<right;

            if (fixedColumnWidth != -1)
                line.setNumColumns(fixedColumnWidth);
            else
                line.setLineWidth(right - left);

//        qDebug() << "layoutBlock; layouting line with width" << right - left << "->textWidth" << line.textWidth();
            floatMargins(layoutStruct->y, layoutStruct, &left, &right);
            left = qMax(left, l);
            right = qMin(right, r);
            if (dir == Qt::LeftToRight)
                left += text_indent;
            else
                right -= text_indent;

            if (fixedColumnWidth == -1 && line.naturalTextWidth() > right-left) {
                // float has been added in the meantime, redo
                layoutStruct->pendingFloats.clear();

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WrapAnywhere);
                    tl->setTextOption(option);
                }

                line.setLineWidth(right-left);
                if (line.naturalTextWidth() > right-left) {
                    layoutStruct->pendingFloats.clear();
                    // lines min width more than what we have
                    layoutStruct->y = findY(layoutStruct->y, layoutStruct, line.naturalTextWidth());
                    floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                    left = qMax(left, l);
                    right = qMin(right, r);
                    if (dir == Qt::LeftToRight)
                        left += text_indent;
                    else
                        right -= text_indent;
                    line.setLineWidth(qMax<qreal>(line.naturalTextWidth(), right-left));
                }

                if (haveWordOrAnyWrapMode) {
                    option.setWrapMode(QTextOption::WordWrap);
                    tl->setTextOption(option);
                }
            }

            qreal lineHeight = line.height();
            if (layoutStruct->pageHeight > 0.0 && layoutStruct->y + lineHeight > layoutStruct->pageBottom) {
                layoutStruct->newPage();

                floatMargins(layoutStruct->y, layoutStruct, &left, &right);
                left = qMax(left, l);
                right = qMin(right, r);
                if (dir == Qt::LeftToRight)
                    left += text_indent;
                else
                    right -= text_indent;
            }

            line.setPosition(QPointF(left - layoutStruct->x_left, layoutStruct->y - cy));
            layoutStruct->y += lineHeight;
            layoutStruct->contentsWidth
                = qMax<qreal>(layoutStruct->contentsWidth, line.x() + line.naturalTextWidth() + totalRightMargin);

            // position floats
            for (int i = 0; i < layoutStruct->pendingFloats.size(); ++i) {
                QTextFrame *f = layoutStruct->pendingFloats.at(i);
                positionFloat(f);
            }
            layoutStruct->pendingFloats.clear();
        }
        tl->endLayout();
    } else {
        const int cnt = tl->lineCount();
        for (int i = 0; i < cnt; ++i) {
            QTextLine line = tl->lineAt(i);
            layoutStruct->contentsWidth
                = qMax(layoutStruct->contentsWidth, line.x() + tl->lineAt(i).naturalTextWidth() + totalRightMargin);
            const qreal lineHeight = line.height();
            if (layoutStruct->pageHeight > 0.0 && layoutStruct->y + lineHeight > layoutStruct->pageBottom)
                layoutStruct->newPage();
            line.setPosition(QPointF(line.position().x(), layoutStruct->y - tl->position().y()));
            layoutStruct->y += lineHeight;
        }
        if (layoutStruct->updateRect.isValid()
            && bl.length() > 1) {
            if (layoutFrom >= bl.position() + bl.length()) {
                // if our height didn't change and the change in the document is
                // in one of the later paragraphs, then we don't need to repaint
                // this one
                layoutStruct->updateRect.setTop(qMax(layoutStruct->updateRect.top(), layoutStruct->y));
            } else if (layoutTo < bl.position()
                       && oldPosition == tl->position()) {
                // if the change in the document happened earlier in the document
                // and our position did /not/ change because none of the earlier paragraphs
                // or frames changed their height, then we don't need to repaint
                // this one
                layoutStruct->updateRect.setBottom(qMin(layoutStruct->updateRect.bottom(), tl->position().y()));
            }
        }
    }

    // ### doesn't take floats into account. would need to do it per line. but how to retrieve then? (Simon)
    layoutStruct->minimumWidth = qMax(layoutStruct->minimumWidth, tl->minimumWidth() + blockFormat.leftMargin() + indent);

    const qreal maxW = tl->maximumWidth() + blockFormat.leftMargin() + indent;
    if (maxW > 0) {
        if (layoutStruct->maximumWidth == INT_MAX)
            layoutStruct->maximumWidth = maxW;
        else
            layoutStruct->maximumWidth = qMax(layoutStruct->maximumWidth, maxW);
    }
}

void QTextDocumentLayoutPrivate::pageBreakInsideTable(QTextTable *table, QLayoutStruct *layoutStruct)
{
    QTextTableData *td = static_cast<QTextTableData *>(data(table));
    const int rows = table->rows();
    Q_ASSERT(rows > 0);
    qreal origY = td->position.y();
    // y positions relative to top of table, as rowPositions is relative, too
    qreal pageBottom = layoutStruct->pageBottom - td->position.y();
    // distance from cell content boundary (top or bottom) to end of table (top or bottom)
    const qreal extraTableHeight = td->padding + td->border + td->cellSpacing // inter cell spacing
                                   + td->margin + td->border + td->padding; // effective table margin

    td->rowsAfterPageBreak.clear();

    qreal tableHeaderHeight = 0;
    const QTextTableFormat format = table->format();
    const int headerRowCount = qMin(format.headerRowCount(), rows - 1);
    if (headerRowCount > 0)
        tableHeaderHeight = td->rowPositions.at(headerRowCount) - td->rowPositions.at(0);

    // if the header and the first row of data is already taller than the remaining height
    // move the whole table to the next page
    if (tableHeaderHeight + td->rowPositions.at(headerRowCount) + td->heights.at(headerRowCount) + extraTableHeight > pageBottom) {
        layoutStruct->newPage();
        origY = layoutStruct->y;
        td->position.setY(layoutStruct->y);
        pageBottom = layoutStruct->pageBottom - td->position.y();
    }

    qreal offset = 0.0;
    for (int r = 2; r < rows; ++r) {
        if (td->rowPositions[r] + offset > pageBottom) {
            offset += pageBottom - td->rowPositions[r - 1] + 2 * layoutStruct->pageMargin;
            offset += extraTableHeight; // make sure there's enough space for the table margin/border
            offset += tableHeaderHeight;
            layoutStruct->newPage();
            td->rowPositions[r - 1] = layoutStruct->y + extraTableHeight + tableHeaderHeight - td->position.y();

            td->rowsAfterPageBreak.append(r - 1);
            pageBottom = layoutStruct->pageBottom - td->position.y();
        }
        td->rowPositions[r] += offset;
    }

    if (rows > 1 && td->rowPositions.last() + td->heights.last() + extraTableHeight > pageBottom) {
        td->rowsAfterPageBreak.append(rows - 1);
        layoutStruct->newPage();
        td->rowPositions.last() = layoutStruct->y + extraTableHeight - td->position.y();
    }

    // calc new total height of table
    td->updateTableSize();
    layoutStruct->y = origY + td->size.height();
}

void QTextDocumentLayoutPrivate::floatMargins(qreal y, const QLayoutStruct *layoutStruct,
                                              qreal *left, qreal *right) const
{
//     qDebug() << "floatMargins y=" << y;
    *left = layoutStruct->x_left;
    *right = layoutStruct->x_right;
    QTextFrameData *lfd = data(layoutStruct->frame);
    for (int i = 0; i < lfd->floats.size(); ++i) {
        QTextFrameData *fd = data(lfd->floats.at(i));
        if (!fd->layoutDirty) {
            if (fd->position.y() <= y && fd->position.y() + fd->size.height() > y) {
//                 qDebug() << "adjusting with float" << f << fd->position.x()<< fd->size.width();
                if (fd->flow_position == QTextFrameFormat::FloatLeft)
                    *left = qMax(*left, fd->position.x() + fd->size.width());
                else
                    *right = qMin(*right, fd->position.x());
            }
        }
    }
//     qDebug() << "floatMargins: left="<<*left<<"right="<<*right<<"y="<<y;
}


qreal QTextDocumentLayoutPrivate::findY(qreal yFrom, const QLayoutStruct *layoutStruct, qreal requiredWidth) const
{
    qreal right, left;
    requiredWidth = qMin(requiredWidth, layoutStruct->x_right - layoutStruct->x_left);

//     qDebug() << "findY:" << yFrom;
    while (1) {
        floatMargins(yFrom, layoutStruct, &left, &right);
//         qDebug() << "    yFrom=" << yFrom<<"right=" << right << "left=" << left << "requiredWidth=" << requiredWidth;
        if (right-left >= requiredWidth)
            break;

        // move float down until we find enough space
        qreal newY = INT_MAX;
        QTextFrameData *lfd = data(layoutStruct->frame);
        for (int i = 0; i < lfd->floats.size(); ++i) {
            QTextFrameData *fd = data(lfd->floats.at(i));
            if (!fd->layoutDirty) {
                if (fd->position.y() <= yFrom && fd->position.y() + fd->size.height() > yFrom)
                    newY = qMin(newY, fd->position.y() + fd->size.height());
            }
        }
        if (newY == INT_MAX)
            break;
        yFrom = newY;
    }
    return yFrom;
}

QTextDocumentLayout::QTextDocumentLayout(QTextDocument *doc)
    : QAbstractTextDocumentLayout(*new QTextDocumentLayoutPrivate, doc)
{
    registerHandler(QTextFormat::ImageObject, new QTextImageHandler(this));
}


void QTextDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    Q_D(QTextDocumentLayout);
    QTextFrame *frame = document()->rootFrame();
    if(data(frame)->sizeDirty)
        return;
    if (context.clip.isValid()) {
        d->ensureLayouted(context.clip.bottom());
    } else {
        d->ensureLayoutFinished();
    }
    d->drawFrame(QPointF(), painter, context, frame);
}

static void markFrames(QTextFrame *current, int from, int oldLength, int length)
{
    int end = qMax(oldLength, length) + from;

    if (current->firstPosition() >= end || current->lastPosition() < from)
        return;

    QTextFrameData *fd = data(current);
    for (int i = 0; i < fd->floats.size(); ++i) {
        QTextFrame *f = fd->floats[i];
        if (!f) {
            // float got removed in editing operation
            fd->floats.removeAt(i);
            --i;
        }
    }

    fd->layoutDirty = true;
    fd->sizeDirty = true;

//     qDebug("    marking frame (%d--%d) as dirty", current->firstPosition(), current->lastPosition());
    QList<QTextFrame *> children = current->childFrames();
    for (int i = 0; i < children.size(); ++i)
        markFrames(children.at(i), from, oldLength, length);
}

void QTextDocumentLayout::documentChanged(int from, int oldLength, int length)
{
    Q_D(QTextDocumentLayout);

    const QSizeF pageSize = document()->pageSize();
    if (pageSize.isNull())
        return;

    QRectF updateRect;

    const QSizeF oldSize = dynamicDocumentSize();

    d->lazyLayoutStepSize = 1000;
    d->sizeChangedTimer.stop();
    d->insideDocumentChange = true;

    const int documentLength = document()->docHandle()->length();
    const bool fullLayout = (oldLength == 0 && length == documentLength);
    const bool smallChange = documentLength > 0
                             && (qMax(length, oldLength) * 100 / documentLength) < 5;

    // don't show incremental layout progress (avoid scrollbar flicker)
    // if we see only a small change in the document and we're either starting
    // a layout run or we're already in progress for that and we haven't seen
    // any bigger change previously (showLayoutProgress already false)
    if (smallChange
        && (d->currentLazyLayoutPosition == -1 || d->showLayoutProgress == false))
        d->showLayoutProgress = false;
    else
        d->showLayoutProgress = true;

    if (fullLayout) {
        d->currentLazyLayoutPosition = 0;
        d->checkPoints.clear();
        d->layoutStep();
    } else {
        d->ensureLayoutedByPosition(from);
        updateRect = doLayout(from, oldLength, length);
    }

    if (!d->layoutTimer.isActive() && d->currentLazyLayoutPosition != -1)
        d->layoutTimer.start(10, this);

    d->insideDocumentChange = false;

    if (d->showLayoutProgress) {
        const QSizeF newSize = dynamicDocumentSize();
        if (newSize != oldSize)
            emit documentSizeChanged(newSize);
    }

    if (!updateRect.isValid()) {
        // don't use the frame size, it might have shrunken
        updateRect = QRectF(QPointF(0, 0), QSizeF(INT_MAX, INT_MAX));
    }

    emit update(updateRect);
}

QRectF QTextDocumentLayout::doLayout(int from, int oldLength, int length)
{
    Q_D(QTextDocumentLayout);

//     qDebug("documentChange: from=%d, oldLength=%d, length=%d", from, oldLength, length);

    // mark all frames between f_start and f_end as dirty
    markFrames(document()->rootFrame(), from, oldLength, length);

    QRectF updateRect;

    QTextFrame *root = document()->rootFrame();
    if(data(root)->sizeDirty)
        updateRect = d->layoutFrame(root, from, from + length);
    data(root)->layoutDirty = false;

    if (d->currentLazyLayoutPosition == -1)
        layoutFinished();
    else if (d->showLayoutProgress)
        d->sizeChangedTimer.start(0, this);

    return updateRect;
}

int QTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayouted(point.y());
    QTextFrame *f = document()->rootFrame();
    int position = 0;
    QTextLayout *l = 0;
    QTextDocumentLayoutPrivate::HitPoint p = d->hitTest(f, point, &position, &l);
    if (accuracy == Qt::ExactHit && p < QTextDocumentLayoutPrivate::PointExact)
        return -1;

    // ensure we stay within document bounds
    int lastPos = f->lastPosition();
    if (l && !l->preeditAreaText().isEmpty())
        lastPos += l->preeditAreaText().length();
    if (position > lastPos)
        position = lastPos;
    else if (position < 0)
        position = 0;

    return position;
}

void QTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QSizeF intrinsic = handler.iface->intrinsicSize(document(), posInDocument, format);

    QTextFrameFormat::Position pos = QTextFrameFormat::InFlow;
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    if (frame) {
        pos = frame->frameFormat().position();
        data(frame)->sizeDirty = false;
        data(frame)->size = intrinsic.toSize();
    }

    item.setDescent(0);
    QSizeF inlineSize = (pos == QTextFrameFormat::InFlow ? intrinsic : QSizeF(0, 0));
    item.setWidth(inlineSize.width());
    item.setAscent(inlineSize.height());
}

void QTextDocumentLayout::positionInlineObject(QTextInlineObject item, int posInDocument, const QTextFormat &format)
{
    Q_D(QTextDocumentLayout);
    Q_UNUSED(posInDocument);
    if (item.width() != 0)
        // inline
        return;

    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextObjectHandler handler = d->handlers.value(f.objectType());
    if (!handler.component)
        return;

    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    if (!frame)
        return;

    QTextBlock b = document()->findBlock(frame->firstPosition());
    QTextLine line;
    if (b.position() <= frame->firstPosition() && b.position() + b.length() > frame->lastPosition())
        line = b.layout()->lineAt(b.layout()->lineCount()-1);
//     qDebug() << "layoutObject: line.isValid" << line.isValid() << b.position() << b.length() <<
//         frame->firstPosition() << frame->lastPosition();
    d->positionFloat(frame, line.isValid() ? &line : 0);
}

void QTextDocumentLayout::drawInlineObject(QPainter *p, const QRectF &rect, QTextInlineObject item,
                                           int posInDocument, const QTextFormat &format)
{
    QTextCharFormat f = format.toCharFormat();
    Q_ASSERT(f.isValid());
    QTextFrame *frame = qobject_cast<QTextFrame *>(document()->objectForFormat(f));
    QRectF r = rect;
    if (frame) {
        QTextFrameData *fd = data(frame);
        if (fd->flow_position != QTextFrameFormat::InFlow) {
            r = QRectF(fd->position, fd->size);
            r.translate(data(frame->parentFrame())->position);
        }
    }
//    qDebug() << "drawObject at" << r;
    QAbstractTextDocumentLayout::drawInlineObject(p, r, item, posInDocument, format);
}

int QTextDocumentLayout::dynamicPageCount() const
{
    const QSizeF pgSize = document()->pageSize();
    if (pgSize.height() < 0)
        return 1;
    return (int)(dynamicDocumentSize().height()
                 / pgSize.height()) + 1;
}

QSizeF QTextDocumentLayout::dynamicDocumentSize() const
{
    return data(document()->rootFrame())->size;
}

int QTextDocumentLayout::pageCount() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return dynamicPageCount();
}

QSizeF QTextDocumentLayout::documentSize() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return dynamicDocumentSize();
}

void QTextDocumentLayoutPrivate::ensureLayouted(qreal y) const
{
    Q_Q(const QTextDocumentLayout);
    if (currentLazyLayoutPosition == -1)
        return;
    const QSizeF oldSize = q->dynamicDocumentSize();

    if (checkPoints.isEmpty())
        layoutStep();

    while (currentLazyLayoutPosition != -1
           && checkPoints.last().y < y)
        layoutStep();
}

void QTextDocumentLayoutPrivate::ensureLayoutedByPosition(int position) const
{
    if (currentLazyLayoutPosition == -1)
        return;
    if (position < currentLazyLayoutPosition)
        return;
    while (currentLazyLayoutPosition != -1
           && currentLazyLayoutPosition < position) {
        const_cast<QTextDocumentLayout *>(q_func())->doLayout(currentLazyLayoutPosition, 0, INT_MAX - currentLazyLayoutPosition);
    }
}

void QTextDocumentLayoutPrivate::layoutStep() const
{
    ensureLayoutedByPosition(currentLazyLayoutPosition + lazyLayoutStepSize);
    lazyLayoutStepSize = qMin(200000, lazyLayoutStepSize * 2);
}

void QTextDocumentLayout::setBlockTextFlags(int flags)
{
    Q_D(QTextDocumentLayout);
    d->blockTextFlags = flags;
}

int QTextDocumentLayout::blockTextFlags() const
{
    Q_D(const QTextDocumentLayout);
    return d->blockTextFlags;
}

void QTextDocumentLayout::setWordWrapMode(QTextOption::WrapMode mode)
{
    Q_D(QTextDocumentLayout);
    d->wordWrapMode = mode;
}

QTextOption::WrapMode QTextDocumentLayout::wordWrapMode() const
{
    Q_D(const QTextDocumentLayout);
    return d->wordWrapMode;
}

void QTextDocumentLayout::setTabStopWidth(double width)
{
    Q_D(QTextDocumentLayout);
    if (width < 0)
        return;
    d->tabStopWidth = width;
}

double QTextDocumentLayout::tabStopWidth() const
{
    Q_D(const QTextDocumentLayout);
    return d->tabStopWidth;
}

void QTextDocumentLayout::setCursorWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->cursorWidth = width;
}

int QTextDocumentLayout::cursorWidth() const
{
    Q_D(const QTextDocumentLayout);
    return d->cursorWidth;
}

void QTextDocumentLayout::setFixedColumnWidth(int width)
{
    Q_D(QTextDocumentLayout);
    d->fixedColumnWidth = width;
}

QRectF QTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    QPointF pos;
    d_func()->ensureLayoutFinished();
    const int framePos = frame->firstPosition();
    QTextFrame *f = frame;
    while (f) {
        QTextFrameData *fd = data(f);
        pos += fd->position;

        if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
            QTextTableCell cell = table->cellAt(framePos);
            if (cell.isValid())
                pos += static_cast<QTextTableData *>(fd)->cellPosition(cell.row(), cell.column());
        }

        f = f->parentFrame();
    }
    return QRectF(pos, data(frame)->size);
}

QRectF QTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    d_func()->ensureLayoutedByPosition(block.position() + block.length());
    QTextFrame *frame = document()->frameAt(block.position());
    QPointF offset;
    const int blockPos = block.position();

    while (frame) {
        QTextFrameData *fd = data(frame);
        offset += fd->position;

        if (QTextTable *table = qobject_cast<QTextTable *>(frame)) {
            QTextTableCell cell = table->cellAt(blockPos);
            if (cell.isValid())
                offset += static_cast<QTextTableData *>(fd)->cellPosition(cell.row(), cell.column());
        }

        frame = frame->parentFrame();
    }

    const QTextLayout *layout = block.layout();
    QRectF rect = layout->boundingRect();
    rect.translate(layout->position() + offset);
    return rect;
}

int QTextDocumentLayout::layoutStatus() const
{
    int pos = d_func()->currentLazyLayoutPosition;
    if (pos == -1)
        return 100;
    return pos * 100 / document()->docHandle()->length();
}

void QTextDocumentLayout::timerEvent(QTimerEvent *e)
{
    Q_D(QTextDocumentLayout);
    if (e->timerId() == d->layoutTimer.timerId()) {
        if (d->currentLazyLayoutPosition != -1)
            d->layoutStep();
    } else if (e->timerId() == d->sizeChangedTimer.timerId()) {
        emit documentSizeChanged(dynamicDocumentSize());
        d->sizeChangedTimer.stop();

        if (d->currentLazyLayoutPosition == -1) {
            const int newCount = dynamicPageCount();
            if (newCount != d->lastPageCount) {
                d->lastPageCount = newCount;
                emit pageCountChanged(newCount);
            }
        }
    } else {
        QAbstractTextDocumentLayout::timerEvent(e);
    }
}

void QTextDocumentLayout::layoutFinished()
{
    Q_D(QTextDocumentLayout);
    d->layoutTimer.stop();
    if (!d->insideDocumentChange)
        d->sizeChangedTimer.start(0, this);
    // reset
    d->showLayoutProgress = true;
}

void QTextDocumentLayout::ensureLayouted(qreal y)
{
    d_func()->ensureLayouted(y);
}

qreal QTextDocumentLayout::idealWidth() const
{
    Q_D(const QTextDocumentLayout);
    d->ensureLayoutFinished();
    return d->idealWidth;
}

#include "moc_qtextdocumentlayout_p.cpp"
