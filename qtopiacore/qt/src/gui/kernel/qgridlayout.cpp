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

#include "qgridlayout.h"


#include "qapplication.h"
#include "qwidget.h"
#include "qlist.h"
#include "qsizepolicy.h"
#include "qvector.h"

#include "qlayoutengine_p.h"
#include "qlayout_p.h"

/*
  Three internal classes related to QGridLayout: (1) QGridBox is a
  QLayoutItem with (row, column) information and (torow, tocolumn) information; (3) QGridLayoutData is
  the internal representation of a QGridLayout.
*/

class QGridBox
{
public:
    QGridBox(QLayoutItem *lit) { item_ = lit; }

    QGridBox(QWidget *wid) { item_ = new QWidgetItem(wid); }
    ~QGridBox() { delete item_; }

    QSize sizeHint() const { return item_->sizeHint(); }
    QSize minimumSize() const { return item_->minimumSize(); }
    QSize maximumSize() const { return item_->maximumSize(); }
    Qt::Orientations expandingDirections() const { return item_->expandingDirections(); }
    bool isEmpty() const { return item_->isEmpty(); }

    bool hasHeightForWidth() const { return item_->hasHeightForWidth(); }
    int heightForWidth(int w) const { return item_->heightForWidth(w); }

    void setAlignment(Qt::Alignment a) { item_->setAlignment(a); }
    void setGeometry(const QRect &r) { item_->setGeometry(r); }
    Qt::Alignment alignment() const { return item_->alignment(); }
    QLayoutItem *item() { return item_; }
    QLayoutItem *takeItem() { QLayoutItem *i = item_; item_ = 0; return i; }

    int hStretch() { return item_->widget() ?
                         item_->widget()->sizePolicy().horizontalStretch() : 0; }
    int vStretch() { return item_->widget() ?
                         item_->widget()->sizePolicy().verticalStretch() : 0; }

private:
    friend class QGridLayoutPrivate;

    QLayoutItem *item_;
    int row, col;
    int torow, tocol;
};

class QGridLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QGridLayout)
public:
    QGridLayoutPrivate();

    void add(QGridBox*, int row, int col);
    void add(QGridBox*, int row1, int row2, int col1, int col2);
    QSize sizeHint(int) const;
    QSize minimumSize(int) const;
    QSize maximumSize(int) const;

    Qt::Orientations expandingDirections(int spacing) const;

    void distribute(QRect, int);
    inline int numRows() const { return rr; }
    inline int numCols() const { return cc; }
    inline void expand(int rows, int cols)
        { setSize(qMax(rows, rr), qMax(cols, cc)); }
    inline void setRowStretch(int r, int s)
        { expand(r + 1, 0); rStretch[r] = s; setDirty(); }
    inline void setColStretch(int c, int s)
        { expand(0, c + 1); cStretch[c] = s; setDirty(); }
    inline int rowStretch(int r) const { return rStretch[r]; }
    inline int colStretch(int c) const { return cStretch[c]; }
    inline void setRowSpacing(int r, int s)
        { expand(r + 1, 0); rSpacing[r] = s; setDirty(); }
    inline void setColSpacing(int c, int s)
        { expand(0, c + 1); cSpacing[c] = s; setDirty(); }
    inline int rowSpacing(int r) const { return rSpacing[r]; }
    inline int colSpacing(int c) const { return cSpacing[c]; }

    inline void setReversed(bool r, bool c) { hReversed = c; vReversed = r; }
    inline bool horReversed() const { return hReversed; }
    inline bool verReversed() const { return vReversed; }
    inline void setDirty() { needRecalc = true; hfw_width = -1; }
    inline bool isDirty() const { return needRecalc; }
    bool hasHeightForWidth(int space);
    int heightForWidth(int, int, int);
    int minimumHeightForWidth(int, int, int);

    inline void getNextPos(int &row, int &col) { row = nextR; col = nextC; }
    inline int count() const { return things.count(); }
    QRect cellRect(int row, int col) const;

    inline QLayoutItem *itemAt(int index) const {
        if (index < things.count())
            return things.at(index)->item();
        else
            return 0;
    }
    inline QLayoutItem *takeAt(int index) {
        QLayoutItem *item = 0;
        if (index < things.count()) {
            QGridBox *b = things.takeAt(index);
            if (b) {
                item = b->takeItem();
                delete b;
            }
        }
        return item;
    }

    void getItemPosition(int index, int *row, int *column, int *rowSpan, int *columnSpan) {
        if (index < things.count()) {
            QGridBox *b =  things.at(index);
            int toRow = b->torow < 0 ? rr-1 : b->torow;
            int toCol = b->tocol  < 0 ? cc-1 : b->tocol;
            *row = b->row;
            *column = b->col;
            *rowSpan = toRow - *row + 1;
            *columnSpan = toCol - *column +1;
        }
    }
    void deleteAll();

private:
    void setNextPosAfter(int r, int c);
    void recalcHFW(int w, int s);
    void addHfwData(QGridBox *box, int width);
    void init();
    QSize findSize(int QLayoutStruct::*, int) const;
    void addData(QGridBox *b, bool r = true, bool c = true);
    void setSize(int rows, int cols);
    void setupLayoutData(int space);
    void setupHfwLayoutData(int space);

    int rr;
    int cc;
    QVector<QLayoutStruct> rowData;
    QVector<QLayoutStruct> colData;
    QVector<QLayoutStruct> *hfwData;
    QVector<int> rStretch;
    QVector<int> cStretch;
    QVector<int> rSpacing;
    QVector<int> cSpacing;
    QList<QGridBox *> things;

    int hfw_width;
    int hfw_height;
    int hfw_minheight;
    int nextR;
    int nextC;

    uint hReversed        : 1;
    uint vReversed        : 1;
    uint needRecalc        : 1;
    uint has_hfw        : 1;
    uint addVertical        : 1;
};

QGridLayoutPrivate::QGridLayoutPrivate()
{
    addVertical = false;
    setDirty();
    rr = cc = 0;
    nextR = nextC = 0;
    hfwData = 0;
    hReversed = false;
    vReversed = false;
}

#if 0
QGridLayoutPrivate::QGridLayoutPrivate(int nRows, int nCols)
    : rowData(0), colData(0)
{
    init();
    if (nRows  < 0) {
        nRows = 1;
        addVertical = false;
    }
    if (nCols  < 0) {
        nCols = 1;
        addVertical = true;
    }
    setSize(nRows, nCols);
}
#endif

void QGridLayoutPrivate::deleteAll()
{
    while (!things.isEmpty())
        delete things.takeFirst();
    delete hfwData;
}

bool QGridLayoutPrivate::hasHeightForWidth(int spacing)
{
    setupLayoutData(spacing);
    return has_hfw;
}

/*
  Assumes that setupLayoutData() has been called, and that
  qGeomCalc() has filled in colData with appropriate values.
*/
void QGridLayoutPrivate::recalcHFW(int w, int spacing)
{
    /*
      Go through all children, using colData and heightForWidth()
      and put the results in hfwData.
    */
    if (!hfwData)
        hfwData = new QVector<QLayoutStruct>(rr);
    setupHfwLayoutData(spacing);
    QVector<QLayoutStruct> &rData = *hfwData;

    int h = 0;
    int mh = 0;
    int n = 0;
    for (int r = 0; r < rr; r++) {
        h += rData[r].sizeHint;
        mh += rData[r].minimumSize;
        if (!rData[r].empty)
            n++;
    }
    if (n) {
        h += (n - 1) * spacing;
        mh += (n - 1) * spacing;
    }

    hfw_width = w;
    hfw_height = qMin(QLAYOUTSIZE_MAX, h);
    hfw_minheight = qMin(QLAYOUTSIZE_MAX, mh);
}

int QGridLayoutPrivate::heightForWidth(int w, int margin, int spacing)
{
    setupLayoutData(spacing);
    if (!has_hfw)
        return -1;
    if (w + 2*margin != hfw_width) {
        qGeomCalc(colData, 0, cc, 0, w - 2*margin, spacing);
        recalcHFW(w - 2*margin, spacing);
    }
    return hfw_height + 2*margin;
}

int QGridLayoutPrivate::minimumHeightForWidth(int w, int margin, int spacing)
{
    (void) heightForWidth(w, margin, spacing);
    return has_hfw ? (hfw_minheight + 2*margin) : -1;
}

QSize QGridLayoutPrivate::findSize(int QLayoutStruct::*size, int spacer) const
{
    QGridLayoutPrivate *that = const_cast<QGridLayoutPrivate*>(this);
    that->setupLayoutData(spacer);

    int w = 0;
    int h = 0;
    int n = 0;

    for (int r = 0; r < rr; r++) {
        h = h + rowData[r].*size;
        if (!rowData[r].empty)
            n++;
    }
    if (n)
        h += (n - 1) * spacer;

    n = 0;
    for (int c = 0; c < cc; c++) {
        w = w + colData[c].*size;
        if (!colData[c].empty)
            n++;
    }
    if (n)
        w += (n - 1) * spacer;

    w = qMin(QLAYOUTSIZE_MAX, w);
    h = qMin(QLAYOUTSIZE_MAX, h);

    return QSize(w, h);
}

Qt::Orientations QGridLayoutPrivate::expandingDirections(int spacing) const
{
    QGridLayoutPrivate *that = const_cast<QGridLayoutPrivate*>(this);
    that->setupLayoutData(spacing);
    Qt::Orientations ret;

    for (int r = 0; r < rr; r++) {
        if (rowData[r].expansive) {
            ret |= Qt::Vertical;
            break;
        }
    }
    for (int c = 0; c < cc; c++) {
        if (colData[c].expansive) {
            ret |= Qt::Horizontal;
            break;
        }
    }
    return ret;
}

QSize QGridLayoutPrivate::sizeHint(int spacer) const
{
    return findSize(&QLayoutStruct::sizeHint, spacer);
}

QSize QGridLayoutPrivate::maximumSize(int spacer) const
{
    return findSize(&QLayoutStruct::maximumSize, spacer);
}

QSize QGridLayoutPrivate::minimumSize(int spacer) const
{
    return findSize(&QLayoutStruct::minimumSize, spacer);
}

void QGridLayoutPrivate::setSize(int r, int c)
{
    if ((int)rowData.size() < r) {
        int newR = qMax(r, rr * 2);
        rowData.resize(newR);
        rStretch.resize(newR);
        rSpacing.resize(newR);
        for (int i = rr; i < newR; i++) {
            rowData[i].init();
            rowData[i].maximumSize = 0;
            rowData[i].pos = 0;
            rowData[i].size = 0;
            rStretch[i] = 0;
            rSpacing[i] = 0;
        }
    }
    if ((int)colData.size() < c) {
        int newC = qMax(c, cc * 2);
        colData.resize(newC);
        cStretch.resize(newC);
        cSpacing.resize(newC);
        for (int i = cc; i < newC; i++) {
            colData[i].init();
            colData[i].maximumSize = 0;
            colData[i].pos = 0;
            colData[i].size = 0;
            cStretch[i] = 0;
            cSpacing[i] = 0;
        }
    }

    if (hfwData && (int)hfwData->size() < r) {
        delete hfwData;
        hfwData = 0;
        hfw_width = -1;
    }
    rr = r;
    cc = c;
}

void QGridLayoutPrivate::setNextPosAfter(int row, int col)
{
    if (addVertical) {
        if (col > nextC || col == nextC && row >= nextR) {
            nextR = row + 1;
            nextC = col;
            if (nextR >= rr) {
                nextR = 0;
                nextC++;
            }
        }
    } else {
        if (row > nextR || row == nextR && col >= nextC) {
            nextR = row;
            nextC = col + 1;
            if (nextC >= cc) {
                nextC = 0;
                nextR++;
            }
        }
    }
}

void QGridLayoutPrivate::add(QGridBox *box, int row, int col)
{
    expand(row+1, col+1);
    box->row = box->torow = row;
    box->col = box->tocol = col;
    things.append(box);
    setDirty();
    setNextPosAfter(row, col);
}

void QGridLayoutPrivate::add(QGridBox *box, int row1, int row2, int col1,
                           int col2 )
{
    if (row2 >= 0 && row2 < row1)
        qWarning("QGridLayout: Multi-cell fromRow greater than toRow");
    if (col2 >= 0 && col2 < col1)
        qWarning("QGridLayout: Multi-cell fromCol greater than toCol");
    if (row1 == row2 && col1 == col2) {
        add(box, row1, col1);
        return;
    }
    expand(row2+1, col2+1);
    box->row = row1;
    box->col = col1;

    box->torow = row2;
    box->tocol = col2;

    things.append(box);
    setDirty();
    if (col2 < 0)
        col2 = cc - 1;

    setNextPosAfter(row2, col2);
}

void QGridLayoutPrivate::addData(QGridBox *box, bool r, bool c)
{
    QSize hint = box->sizeHint();
    QSize minS = box->minimumSize();
    QSize maxS = box->maximumSize();
    const QWidget *widget = box->item()->widget();

    if (box->isEmpty() && widget)
        return;

    if (c) {
        if (!cStretch[box->col])
            colData[box->col].stretch = qMax(colData[box->col].stretch,
                                              box->hStretch());
        colData[box->col].sizeHint = qMax(hint.width(),
                                           colData[box->col].sizeHint);
        colData[box->col].minimumSize = qMax(minS.width(),
                                              colData[box->col].minimumSize);

        qMaxExpCalc(colData[box->col].maximumSize, colData[box->col].expansive, colData[box->col].empty,
                    maxS.width(), box->expandingDirections() & Qt::Horizontal, box->isEmpty());
    }
    if (r) {
        if (!rStretch[box->row])
            rowData[box->row].stretch = qMax(rowData[box->row].stretch,
                                              box->vStretch());
        rowData[box->row].sizeHint = qMax(hint.height(),
                                           rowData[box->row].sizeHint);
        rowData[box->row].minimumSize = qMax(minS.height(),
                                              rowData[box->row].minimumSize);

        qMaxExpCalc(rowData[box->row].maximumSize, rowData[box->row].expansive, rowData[box->row].empty,
                    maxS.height(), box->expandingDirections() & Qt::Vertical, box->isEmpty());
    }
}

static void distributeMultiBox(QVector<QLayoutStruct> &chain, int spacing, int start, int end,
                               int minSize, int sizeHint, QVector<int> &stretchArray, int stretch)
{
    int i;
    int w = 0;
    int wh = 0;
    int max = 0;
    for (i = start; i <= end; i++) {
        w += chain[i].minimumSize;
        wh += chain[i].sizeHint;
        if (chain[i].empty && chain[i].maximumSize == 0) //truly empty box
            chain[i].maximumSize = QWIDGETSIZE_MAX;
        max += chain[i].maximumSize;
        chain[i].empty = false;
        if (stretchArray[i] == 0)
            chain[i].stretch = qMax(chain[i].stretch,stretch);
    }
    w += spacing * (end - start);
    wh += spacing * (end - start);
    max += spacing * (end - start);

    if (max < minSize) { // implies w < minSize
        /*
          We must increase the maximum size of at least one of the
          items. qGeomCalc() will put the extra space in between the
          items. We must recover that extra space and put it
          somewhere. It does not really matter where, since the user
          can always specify stretch factors and avoid this code.
        */
        qGeomCalc(chain, start, end - start + 1, 0, minSize, spacing);
        int pos = 0;
        for (i = start; i <= end; i++) {
            int nextPos = (i == end) ? minSize - 1 : chain[i + 1].pos;
            int realSize = nextPos - pos;
            if (i != end)
                realSize -= spacing;
            if (chain[i].minimumSize < realSize)
                chain[i].minimumSize = realSize;
            if (chain[i].maximumSize < chain[i].minimumSize)
                chain[i].maximumSize = chain[i].minimumSize;
            pos = nextPos;
        }
    } else if (w < minSize) {
        qGeomCalc(chain, start, end - start + 1, 0, minSize, spacing);
        for (i = start; i <= end; i++) {
            if (chain[i].minimumSize < chain[i].size)
                chain[i].minimumSize = chain[i].size;
        }
    }

    if (wh < sizeHint) {
        qGeomCalc(chain, start, end - start + 1, 0, sizeHint, spacing);
        for (i = start; i <= end; i++) {
            if (chain[i].sizeHint < chain[i].size)
                chain[i].sizeHint = chain[i].size;
        }
    }
}

//#define QT_LAYOUT_DISABLE_CACHING

void QGridLayoutPrivate::setupLayoutData(int spacing)
{
#ifndef QT_LAYOUT_DISABLE_CACHING
    if (!needRecalc)
        return;
#endif
    has_hfw = false;
    int i;

    for (i = 0; i < rr; i++) {
        rowData[i].init(rStretch[i], rSpacing[i]);
        rowData[i].maximumSize = rStretch[i] ? QLAYOUTSIZE_MAX : rSpacing[i];
    }
    for (i = 0; i < cc; i++) {
        colData[i].init(cStretch[i], cSpacing[i]);
        colData[i].maximumSize = cStretch[i] ? QLAYOUTSIZE_MAX : cSpacing[i];
    }
    for (int pass = 0; pass < 2; ++pass) {
        for (i = 0; i < things.size(); ++i) {
            QGridBox *box = things.at(i);
            int r1 = box->row;
            int c1 = box->col;
            int r2 = box->torow;
            int c2 = box->tocol;
            if (r2 < 0)
                r2 = rr - 1;
            if (c2 < 0)
                c2 = cc - 1;

            QSize hint = box->sizeHint();
            QSize min = box->minimumSize();
            if (box->hasHeightForWidth())
                has_hfw = true;

            if (r1 == r2) {
                if (pass == 0)
                    addData(box, true, false);
            } else {
                if (pass == 1)
                    distributeMultiBox(rowData, spacing, r1, r2, min.height(), hint.height(),
                                       rStretch, box->vStretch());
            }
            if (c1 == c2) {
                if (pass == 0)
                    addData(box, false, true);
            } else {
                if (pass == 1)
                    distributeMultiBox(colData, spacing, c1, c2, min.width(), hint.width(),
                                       cStretch, box->hStretch());
            }
        }
    }

    for (i = 0; i < rr; i++)
        rowData[i].expansive = rowData[i].expansive || rowData[i].stretch > 0;
    for (i = 0; i < cc; i++)
        colData[i].expansive = colData[i].expansive || colData[i].stretch > 0;

    needRecalc = false;
}

void QGridLayoutPrivate::addHfwData(QGridBox *box, int width)
{
    QVector<QLayoutStruct> &rData = *hfwData;
    if (box->hasHeightForWidth()) {
        int hint = box->heightForWidth(width);
        rData[box->row].sizeHint = qMax(hint, rData[box->row].sizeHint);
        rData[box->row].minimumSize = qMax(hint, rData[box->row].minimumSize);
    } else {
        QSize hint = box->sizeHint();
        QSize minS = box->minimumSize();
        rData[box->row].sizeHint = qMax(hint.height(), rData[box->row].sizeHint);
        rData[box->row].minimumSize = qMax(minS.height(), rData[box->row].minimumSize);
    }
}

/*
  Similar to setupLayoutData(), but uses heightForWidth(colData)
  instead of sizeHint(). Assumes that setupLayoutData() and
  qGeomCalc(colData) has been called.
*/
void QGridLayoutPrivate::setupHfwLayoutData(int spacing)
{
    QVector<QLayoutStruct> &rData = *hfwData;
    for (int i = 0; i < rr; i++) {
        rData[i] = rowData[i];
        rData[i].minimumSize = rData[i].sizeHint = rSpacing[i];
    }

    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < things.size(); ++i) {
            QGridBox *box = things.at(i);
            int r1 = box->row;
            int c1 = box->col;
            int r2 = box->torow;
            int c2 = box->tocol;
            if (r2 < 0)
                r2 = rr-1;
            if (c2 < 0)
                c2 = cc-1;
            int w = colData[c2].pos + colData[c2].size - colData[c1].pos;

            if (r1 == r2) {
                if (pass == 0)
                    addHfwData(box, w);
            } else {
                if (pass == 1) {
                    QSize hint = box->sizeHint();
                    QSize min = box->minimumSize();
                    if (box->hasHeightForWidth()) {
                        int hfwh = box->heightForWidth(w);
                        if (hfwh > hint.height())
                            hint.setHeight(hfwh);
                        if (hfwh > min.height())
                            min.setHeight(hfwh);
                    }
                    distributeMultiBox(rData, spacing, r1, r2,
                                       min.height(), hint.height(),
                                       rStretch, box->vStretch());
                }
            }
        }
    }
    for (int i = 0; i < rr; i++)
        rData[i].expansive = rData[i].expansive || rData[i].stretch > 0;
}

void QGridLayoutPrivate::distribute(QRect r, int spacing)
{
    Q_Q(QGridLayout);
    bool visualHReversed = hReversed;
    QWidget *parent = q->parentWidget();
    if (parent && parent->isRightToLeft())
        visualHReversed = !visualHReversed;

    setupLayoutData(spacing);

    qGeomCalc(colData, 0, cc, r.x(), r.width(), spacing);
    QVector<QLayoutStruct> *rDataPtr;
    if (has_hfw) {
        recalcHFW(r.width(), spacing);
        qGeomCalc(*hfwData, 0, rr, r.y(), r.height(), spacing);
        rDataPtr = hfwData;
    } else {
        qGeomCalc(rowData, 0, rr, r.y(), r.height(), spacing);
        rDataPtr = &rowData;
    }
    QVector<QLayoutStruct> &rData = *rDataPtr;
    int i;

    bool reverse = ((r.bottom() > rect.bottom()) || (r.bottom() == rect.bottom()
                                                     && ((r.right() > rect.right()) != visualHReversed)));
    int n = things.size();
    for (i = 0; i < n; ++i) {
        QGridBox *box = things.at(reverse ? n-i-1 : i);
        int r2 = box->torow;
        int c2 = box->tocol;
        if (r2 < 0)
            r2 = rr-1;
        if (c2 < 0)
            c2 = cc-1;

        int x = colData[box->col].pos;
        int y = rData[box->row].pos;
        int x2p = colData[c2].pos + colData[c2].size; // x2+1
        int y2p = rData[r2].pos + rData[r2].size;    // y2+1
        int w = x2p - x;
        int h = y2p - y;

        if (visualHReversed)
            x = r.left() + r.right() - x - w + 1;
        if (vReversed)
            y = r.top() + r.bottom() - y - h + 1;
        box->setGeometry(QRect(x, y, w, h));
    }
}

QRect QGridLayoutPrivate::cellRect(int row, int col) const
{
    if (row < 0 || row >= rr || col < 0 || col >= cc)
        return QRect();

    const QVector<QLayoutStruct> *rDataPtr;
    if (has_hfw && hfwData)
        rDataPtr = hfwData;
    else
        rDataPtr = &rowData;
    return QRect(colData[col].pos, (*rDataPtr)[row].pos, colData[col].size, (*rDataPtr)[row].size);
}

/*!
    \class QGridLayout

    \brief The QGridLayout class lays out widgets in a grid.

    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    QGridLayout takes the space made available to it (by its parent
    layout or by the parentWidget()), divides it up into rows and
    columns, and puts each widget it manages into the correct cell.

    Columns and rows behave identically; we will discuss columns, but
    there are equivalent functions for rows.

    Each column has a minimum width and a stretch factor. The minimum
    width is the greatest of that set using setColumnMinimumWidth() and the
    minimum width of each widget in that column. The stretch factor is
    set using setColumnStretch() and determines how much of the available
    space the column will get over and above its necessary minimum.

    Normally, each managed widget or layout is put into a cell of its
    own using addWidget(). It is also possible for a widget to occupy
    multiple cells using the row and column spanning overloads of
    addItem() and addWidget(). If you do this, QGridLayout will guess
    how to distribute the size over the columns/rows (based on the
    stretch factors).

    To remove a widget from a layout, call remove(). Calling
    QWidget::hide() on a widget also effectively removes the widget
    from the layout until QWidget::show() is called.

    This illustration shows a fragment of a dialog with a five-column,
    three-row grid (the grid is shown overlaid in magenta):

    \image gridlayout.png A grid layout

    Columns 0, 2 and 4 in this dialog fragment are made up of a
    QLabel, a QLineEdit, and a QListBox. Columns 1 and 3 are
    placeholders made with setColumnMinimumWidth(). Row 0 consists of three
    QLabel objects, row 1 of three QLineEdit objects and row 2 of
    three QListBox objects. We used placeholder columns (1 and 3) to
    get the right amount of space between the columns.

    Note that the columns and rows are not equally wide or tall. If
    you want two columns to have the same width, you must set their
    minimum widths and stretch factors to be the same yourself. You do
    this using setColumnMinimumWidth() and setColumnStretch().

    If the QGridLayout is not the top-level layout (i.e. does not
    manage all of the widget's area and children), you must add it to
    its parent layout when you create it, but before you do anything
    with it. The normal way to add a layout is by calling
    addLayout() on the parent layout.

    Once you have added your layout you can start putting widgets and
    other layouts into the cells of your grid layout using
    addWidget(), addItem(), and addLayout().

    QGridLayout also includes two margin widths: the \l margin and
    the \l spacing. The margin is the width of the reserved space
    along each of the QGridLayout's four sides. The spacing is the
    width of the automatically allocated spacing between neighboring
    boxes.

    The default \l margin and \l spacing values are provided by the
    style. The default margin Qt styles specify is 9 for child
    widgets and 11 for windows. The spacing defaults to the same as
    the margin width for a top-level layout, or to the same as the
    parent layout.

    \sa QBoxLayout, QStackedLayout, {Layout Classes}, {Basic Layouts Example}
*/


/*!
    Constructs a new QGridLayout with parent widget, \a parent.  The
    layout has one row and one column initially, and will expand when
    new items are inserted.
*/
QGridLayout::QGridLayout(QWidget *parent)
    : QLayout(*new QGridLayoutPrivate, 0, parent)
{
    Q_D(QGridLayout);
    d->expand(1, 1);
}

/*!
    Constructs a new grid layout.

    You must insert this grid into another layout. You can insert
    widgets and layouts into this layout at any time, but laying out
    will not be performed before this is inserted into another layout.
*/
QGridLayout::QGridLayout()
    : QLayout(*new QGridLayoutPrivate, 0, 0)
{
    Q_D(QGridLayout);
    d->expand(1, 1);
}


#ifdef QT3_SUPPORT
/*!
  \obsolete
    Constructs a new QGridLayout with \a nRows rows, \a nCols columns
    and parent widget, \a  parent. \a parent may not be 0. The grid
    layout is called \a name.

    \a margin is the number of pixels between the edge of the widget
    and its managed children. \a space is the default number of pixels
    between cells. If \a space is -1, the value of \a margin is used.
*/
QGridLayout::QGridLayout(QWidget *parent, int nRows, int nCols, int margin,
                          int space, const char *name)
    : QLayout(*new QGridLayoutPrivate, 0, parent)
{
    Q_D(QGridLayout);
    d->expand(nRows, nCols);
    setMargin(margin);
    setSpacing(space<0 ? margin : space);
    setObjectName(QString::fromAscii(name));
}

/*!
  \obsolete
    Constructs a new grid with \a nRows rows and \a nCols columns. If
    \a spacing is -1, this QGridLayout inherits its parent's
    spacing(); otherwise \a spacing is used. The grid layout is called
    \a name.

    You must insert this grid into another layout. You can insert
    widgets and layouts into this layout at any time, but laying out
    will not be performed before this is inserted into another layout.
*/
QGridLayout::QGridLayout(QLayout *parentLayout, int nRows, int nCols,
                          int spacing, const char *name)
    : QLayout(*new QGridLayoutPrivate, parentLayout, 0)
{
    Q_D(QGridLayout);
    d->expand(nRows, nCols);
    setSpacing(spacing);
    setObjectName(QString::fromAscii(name));
}

/*!
  \obsolete
    Constructs a new grid with \a nRows rows and \a nCols columns. If
    \a spacing is -1, this QGridLayout inherits its parent's
    spacing(); otherwise \a spacing is used. The grid layout is called
    \a name.

    You must insert this grid into another layout. You can insert
    widgets and layouts into this layout at any time, but laying out
    will not be performed before this is inserted into another layout.
*/
QGridLayout::QGridLayout(int nRows, int nCols,
                          int spacing, const char *name)
    : QLayout(*new QGridLayoutPrivate, 0, 0)
{
    Q_D(QGridLayout);
    d->expand(nRows, nCols);
    setSpacing(spacing);
    setObjectName(QString::fromAscii(name));
}
#endif


/*!
\internal (mostly)

Sets the positioning mode used by addItem(). If \a orient is
Qt::Horizontal, this layout is expanded to \a n columns, and items
will be added columns-first. Otherwise it is expanded to \a n rows and
items will be added rows-first.
*/

void QGridLayout::setDefaultPositioning(int n, Qt::Orientation orient)
{
    Q_D(QGridLayout);
    if (orient == Qt::Horizontal) {
        d->expand(1, n);
        d->addVertical = false;
    } else {
        d->expand(n,1);
        d->addVertical = true;
    }
}


/*!
    Destroys the grid layout. Geometry management is terminated if
    this is a top-level grid.

    The layout's widgets aren't destroyed.
*/
QGridLayout::~QGridLayout()
{
    Q_D(QGridLayout);
    d->deleteAll();
}

/*!
    Returns the number of rows in this grid.
*/
int QGridLayout::rowCount() const
{
    Q_D(const QGridLayout);
    return d->numRows();
}

/*!
    Returns the number of columns in this grid.
*/
int QGridLayout::columnCount() const
{
    Q_D(const QGridLayout);
    return d->numCols();
}

/*!
    \reimp
*/
QSize QGridLayout::sizeHint() const
{
    Q_D(const QGridLayout);
    int m = margin();
    return d->sizeHint(spacing()) + QSize(2 * m, 2 * m);
}

/*!
    \reimp
*/
QSize QGridLayout::minimumSize() const
{
    Q_D(const QGridLayout);
    int m = margin();
    return d->minimumSize(spacing()) + QSize(2 * m, 2 * m);
}

/*!
    \reimp
*/
QSize QGridLayout::maximumSize() const
{
    Q_D(const QGridLayout);
    int m = margin();
    QSize s = d->maximumSize(spacing()) +
              QSize(2 * m, 2 * m);
    s = s.boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));
    if (alignment() & Qt::AlignHorizontal_Mask)
        s.setWidth(QLAYOUTSIZE_MAX);
    if (alignment() & Qt::AlignVertical_Mask)
        s.setHeight(QLAYOUTSIZE_MAX);
    return s;
}

/*!
    \reimp
*/
bool QGridLayout::hasHeightForWidth() const
{
    return ((QGridLayout*)this)->d_func()->hasHeightForWidth(spacing());
}

/*!
    \reimp
*/
int QGridLayout::heightForWidth(int w) const
{
    QGridLayout *that = (QGridLayout*)this;
    return that->d_func()->heightForWidth(w, margin(), spacing());
}

/*!
    \reimp
*/
int QGridLayout::minimumHeightForWidth(int w) const
{
    QGridLayout *that = (QGridLayout*)this;
    return that->d_func()->minimumHeightForWidth(w, margin(), spacing());
}

#ifdef QT3_SUPPORT
/*!
    \compat

    Searches for widget \a w in this layout (not including child
    layouts). If \a w is found, it sets \c{*}\a{row} and
    \c{*}\a{column} to the row and column that the widget
    occupies and returns true; otherwise returns false.

    If the widget spans multiple rows/columns, the top-left cell
    is returned.

    Use indexOf() and getItemPosition() instead.
*/
bool QGridLayout::findWidget(QWidget* w, int *row, int *column)
{
    Q_D(QGridLayout);
    int index = indexOf(w);
    if (index < 0)
        return false;
    int dummy1, dummy2;
    d->getItemPosition(index, row, column, &dummy1, &dummy2);
    return true;
}
#endif
/*!
    \reimp
*/
int QGridLayout::count() const
{
    Q_D(const QGridLayout);
    return d->count();
}


/*!
    \reimp
*/
QLayoutItem *QGridLayout::itemAt(int index) const
{
    Q_D(const QGridLayout);
    return d->itemAt(index);
}


/*!
    \reimp
*/
QLayoutItem *QGridLayout::takeAt(int index)
{
    Q_D(QGridLayout);
    return d->takeAt(index);
}

/*!
  Returns the position information of the item with the given \a index.

  The variables passed as \a row and \a column are updated with the position of the
  item in the layout, and the \a rowSpan and \a columnSpan variables are updated
  with the vertical and horizontal spans of the item.
*/
void QGridLayout::getItemPosition(int index, int *row, int *column, int *rowSpan, int *columnSpan)
{
    Q_D(QGridLayout);
    d->getItemPosition(index, row, column, rowSpan, columnSpan);
}


/*!
    \reimp
*/
void QGridLayout::setGeometry(const QRect &rect)
{
    Q_D(QGridLayout);
    if (d->isDirty() || rect != geometry()) {
        QRect cr = alignment() ? alignmentRect(rect) : rect;
        int m = margin();
        QRect s(cr.x() + m, cr.y() + m,
                 cr.width() - 2 * m, cr.height() - 2 * m);
        d->distribute(s, spacing());
        QLayout::setGeometry(rect);
    }
}

/*!
    Returns the geometry of the cell with row \a row and column \a column
    in the grid. Returns an invalid rectangle if \a row or \a column is
    outside the grid.

    \warning in the current version of Qt this function does not
    return valid results until setGeometry() has been called, i.e.
    after the parentWidget() is visible.
*/
QRect QGridLayout::cellRect(int row, int column) const
{
    Q_D(const QGridLayout);
    return d->cellRect(row, column);
}
#ifdef QT3_SUPPORT
/*!
  \obsolete
    Expands this grid so that it will have \a nRows rows and \a nCols
    columns. Will not shrink the grid. You should not need to call
    this function because QGridLayout expands automatically as new
    items are inserted.
*/
void QGridLayout::expand(int nRows, int nCols)
{
    Q_D(QGridLayout);
    d->expand(nRows, nCols);
}
#endif

/*!
    \reimp
*/
void QGridLayout::addItem(QLayoutItem *item)
{
    Q_D(QGridLayout);
    int r, c;
    d->getNextPos(r, c);
    addItem(item, r, c);
}

/*!
    Adds \a item at position \a row, \a column, spanning \a rowSpan
    rows and \a columnSpan columns, and aligns it according to \a
    alignment. If \a rowSpan and/or \a columnSpan is -1, then the item
    will extend to the bottom and/or right edge, respectively. The
    layout takes ownership of the \a item.

    \warning Do not use this function to add child layouts or child
    widget items. Use addLayout() or addWidget() instead.
*/
void QGridLayout::addItem(QLayoutItem *item, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    Q_D(QGridLayout);
    QGridBox *b = new QGridBox(item);
    b->setAlignment(alignment);
    d->add(b, row, (rowSpan < 0) ? -1 : row + rowSpan - 1, column, (columnSpan < 0) ? -1 : column + columnSpan - 1);
    invalidate();
}

/*
  Returns true if the widget \a w can be added to the layout \a l;
  otherwise returns false.
*/
static bool checkWidget(QLayout *l, QWidget *w)
{
    if (!w) {
        qWarning("QLayout: Cannot add null widget to %s/%s", l->metaObject()->className(),
                  l->objectName().toLocal8Bit().data());
        return false;
    }
    return true;
}

/*!
    Adds the given \a widget to the cell grid at \a row, \a column. The
    top-left position is (0, 0) by default.

    The alignment is specified by \a alignment. The default
    alignment is 0, which means that the widget fills the entire cell.

*/
void QGridLayout::addWidget(QWidget *widget, int row, int column, Qt::Alignment alignment)
{
    if (!checkWidget(this, widget))
        return;
    if (row < 0 || column < 0) {
        qWarning("QGridLayout: Cannot add %s/%s to %s/%s at row %d column %d",
                 widget->metaObject()->className(), widget->objectName().toLocal8Bit().data(),
                 metaObject()->className(), objectName().toLocal8Bit().data(), row, column);
        return;
    }
    addChildWidget(widget);
    QWidgetItem *b = new QWidgetItem(widget);
    addItem(b, row, column, 1, 1, alignment);
}

/*!
    \overload

    This version adds the given \a widget to the cell grid, spanning
    multiple rows/columns. The cell will start at \a fromRow, \a
    fromColumn spanning \a rowSpan rows and \a columnSpan columns. The
    grid will have the given \a alignment.

    If \a rowSpan and/or \a columnSpan is -1, then the widget will
    extend to the bottom and/or right edge, respectively.

*/
void QGridLayout::addWidget(QWidget *widget, int fromRow, int fromColumn,
                            int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    Q_D(QGridLayout);
    if (!checkWidget(this, widget))
        return;
    int toRow = (rowSpan < 0) ? -1 : fromRow + rowSpan - 1;
    int toColumn = (columnSpan < 0) ? -1 : fromColumn + columnSpan - 1;
    addChildWidget(widget);
    QGridBox *b = new QGridBox(widget);
    b->setAlignment(alignment);
    d->add(b, fromRow, toRow, fromColumn, toColumn);
    invalidate();
}

/*!
    \fn void QGridLayout::addWidget(QWidget *widget)

    \overload
    \internal
*/

/*!
    Places the \a layout at position (\a row, \a column) in the grid. The
    top-left position is (0, 0).

    The alignment is specified by \a alignment. The default
    alignment is 0, which means that the widget fills the entire cell.

    A non-zero alignment indicates that the layout should not grow to
    fill the available space but should be sized according to
    sizeHint().


    \a layout becomes a child of the grid layout.
*/
void QGridLayout::addLayout(QLayout *layout, int row, int column, Qt::Alignment alignment)
{
    Q_D(QGridLayout);
    addChildLayout(layout);
    QGridBox *b = new QGridBox(layout);
    b->setAlignment(alignment);
    d->add(b, row, column);
}

/*!
  \overload
    This version adds the layout \a layout to the cell grid, spanning multiple
    rows/columns. The cell will start at \a row, \a column spanning \a
    rowSpan rows and \a columnSpan columns.

    If \a rowSpan and/or \a columnSpan is -1, then the layout will extend to the bottom
    and/or right edge, respectively.
*/
void QGridLayout::addLayout(QLayout *layout, int row, int column,
                                      int rowSpan, int columnSpan, Qt::Alignment alignment)
{
    Q_D(QGridLayout);
    addChildLayout(layout);
    QGridBox *b = new QGridBox(layout);
    b->setAlignment(alignment);
    d->add(b, row, (rowSpan < 0) ? -1 : row + rowSpan - 1, column, (columnSpan < 0) ? -1 : column + columnSpan - 1);
}

/*!
    Sets the stretch factor of row \a row to \a stretch. The first row
    is number 0.

    The stretch factor is relative to the other rows in this grid.
    Rows with a higher stretch factor take more of the available
    space.

    The default stretch factor is 0. If the stretch factor is 0 and no
    other row in this table can grow at all, the row may still grow.

    \sa rowStretch(), setRowMinimumHeight(), setColumnStretch()
*/
void QGridLayout::setRowStretch(int row, int stretch)
{
    Q_D(QGridLayout);
    d->setRowStretch(row, stretch);
    invalidate();
}

/*!
    Returns the stretch factor for row \a row.

    \sa setRowStretch()
*/
int QGridLayout::rowStretch(int row) const
{
    Q_D(const QGridLayout);
    return d->rowStretch(row);
}

/*!
    Returns the stretch factor for column \a column.

    \sa setColumnStretch()
*/
int QGridLayout::columnStretch(int column) const
{
    Q_D(const QGridLayout);
    return d->colStretch(column);
}

/*!
    Sets the stretch factor of column \a column to \a stretch. The first
    column is number 0.

    The stretch factor is relative to the other columns in this grid.
    Columns with a higher stretch factor take more of the available
    space.

    The default stretch factor is 0. If the stretch factor is 0 and no
    other column in this table can grow at all, the column may still
    grow.

    An alternative approach is to add spacing using addItem() with a
    QSpacerItem.

    \sa columnStretch(), setRowStretch()
*/
void QGridLayout::setColumnStretch(int column, int stretch)
{
    Q_D(QGridLayout);
    d->setColStretch(column, stretch);
    invalidate();
}



/*!
    Sets the minimum height of row \a row to \a minSize pixels.

    \sa rowMinimumHeight(), setColumnMinimumWidth()
*/
void QGridLayout::setRowMinimumHeight(int row, int minSize)
{
    Q_D(QGridLayout);
    d->setRowSpacing(row, minSize);
    invalidate();
}

/*!
    Returns the minimum width set for row \a row.

    \sa setRowMinimumHeight()
*/
int QGridLayout::rowMinimumHeight(int row) const
{
    Q_D(const QGridLayout);
    return d->rowSpacing(row);
}

/*!
    Sets the minimum width of column \a column to \a minSize pixels.

    \sa columnMinimumWidth(), setRowMinimumHeight()
*/
void QGridLayout::setColumnMinimumWidth(int column, int minSize)
{
    Q_D(QGridLayout);
    d->setColSpacing(column, minSize);
    invalidate();
}

/*!
    Returns the column spacing for column \a column.

    \sa setColumnMinimumWidth()
*/
int QGridLayout::columnMinimumWidth(int column) const
{
    Q_D(const QGridLayout);
    return d->colSpacing(column);
}

/*!
    \reimp
*/
Qt::Orientations QGridLayout::expandingDirections() const
{
    Q_D(const QGridLayout);
    return d->expandingDirections(spacing());
}

/*!
    Sets the grid's origin corner, i.e. position (0, 0), to \a corner.
*/
void QGridLayout::setOriginCorner(Qt::Corner corner)
{
    Q_D(QGridLayout);
    d->setReversed(corner == Qt::BottomLeftCorner || corner == Qt::BottomRightCorner,
                   corner == Qt::TopRightCorner || corner == Qt::BottomRightCorner);
}

/*!
    Returns the corner that's used for the grid's origin, i.e. for
    position (0, 0).
*/
Qt::Corner QGridLayout::originCorner() const
{
    Q_D(const QGridLayout);
    if (d->horReversed()) {
        return d->verReversed() ? Qt::BottomRightCorner : Qt::TopRightCorner;
    } else {
        return d->verReversed() ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
    }
}

/*!
    \reimp
*/
void QGridLayout::invalidate()
{
    Q_D(QGridLayout);
    d->setDirty();
    QLayout::invalidate();
}

/*!
    \fn void QGridLayout::addRowSpacing(int row, int minsize)

    Use addItem(new QSpacerItem(0, minsize), row, 0) instead.
*/

/*!
    \fn void QGridLayout::addColSpacing(int col, int minsize)

    Use addItem(new QSpacerItem(minsize, 0), 0, col) instead.
*/

/*!
    \fn void QGridLayout::addMultiCellWidget(QWidget *widget, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)

    Use an addWidget() overload that allows you to specify row and
    column spans instead.
*/

/*!
    \fn void QGridLayout::addMultiCell(QLayoutItem *l, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)

    Use an addItem() overload that allows you to specify row and
    column spans instead.
*/

/*!
    \fn void QGridLayout::addMultiCellLayout(QLayout *layout, int fromRow, int toRow, int fromCol, int toCol, Qt::Alignment align = 0)

    Use an addLayout() overload that allows you to specify row and
    column spans instead.
*/

/*!
    \fn int QGridLayout::numRows() const

    Use rowCount() instead.
*/

/*!
    \fn int QGridLayout::numCols() const

    Use columnCount() instead.
*/

/*!
    \fn void QGridLayout::setColStretch(int col, int stretch)

    Use setColumnStretch() instead.
*/

/*!
    \fn int QGridLayout::colStretch(int col) const

    Use columnStretch() instead.
*/

/*!
    \fn void QGridLayout::setColSpacing(int col, int minSize)

    Use setColumnMinimumWidth() instead.
*/

/*!
    \fn int QGridLayout::colSpacing(int col) const

    Use columnSpacing() instead.
*/

/*!
    \fn void QGridLayout::setRowSpacing(int row, int minSize)

    Use setRowMinimumHeight(\a row, \a minSize) instead.
*/

/*!
    \fn int QGridLayout::rowSpacing(int row) const

    Use rowMinimumHeight(\a row) instead.
*/

/*!
    \fn QRect QGridLayout::cellGeometry(int row, int column) const

    Use cellRect(\a row, \a column) instead.
*/

/*!
    \fn void QGridLayout::setOrigin(Qt::Corner corner)

    Use setOriginCorner(\a corner) instead.
*/

/*!
    \fn Qt::Corner QGridLayout::origin() const

    Use originCorner() instead.
*/

