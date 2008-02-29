/****************************************************************************
**
** Copyright (C) 2004-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qt Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qformlayout.h"
#include <QPhoneStyle>
#include <QtCore/QDebug>
#include <QtCore/QRect>
#include <QtCore/QVector>
#include <QtGui/QWidgetItem>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QApplication>
#include <private/qlayout_p.h>
#include <private/qlayoutengine_p.h>

// since this isn't exported from QtGui
static int qSmratSpacing(const QLayout *layout, QStyle::PixelMetric pm)
{
    QObject *parent = layout->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

namespace {
    enum { ColumnCount = 2 };

    // Fixed column matrix, stores items as [i11, i12, i21, i22...],
    // with FORTRAN-style index operator(r, c). Long live FORTRAN!
    template <class T, int NumColumns>
        class FixedColumnMatrix {
        public:
            typedef QVector<T> Storage;

            FixedColumnMatrix() {}

            const T &operator()(int r, int c) const { return m_items[r * NumColumns + c]; }
            T &operator()(int r, int c) { return m_items[r * NumColumns + c]; }

            int rowCount() const { return m_items.size() / NumColumns; }
            void addRow(const T &value);
            void insertRow(int r, const T &value);
            void removeRow(int r);

            bool find(const T &value, int *row, int *col) const ;
            int count(const T &value) const { return m_items.count(value);  }

            // Hmmpf.. Some things are faster that way.
            const Storage &storage() const { return  m_items; }

            static void storageIndexToPosition(int idx, int *row, int *col);

        private:
            Storage m_items;
        };

template <class T, int NumColumns>
void FixedColumnMatrix<T, NumColumns>::addRow(const T &value)
{
    for (int i = 0; i < NumColumns; i++)
        m_items.push_back(value);
}

template <class T, int NumColumns>
void FixedColumnMatrix<T, NumColumns>::insertRow(int r, const T &value)
{
    Q_TYPENAME Storage::iterator it = m_items.begin();
    it += r * NumColumns;
    m_items.insert(it, NumColumns, value);
}

template <class T, int NumColumns>
void FixedColumnMatrix<T, NumColumns>::removeRow(int r)
{
    m_items.remove(r * NumColumns, NumColumns);
}

template <class T, int NumColumns>
bool FixedColumnMatrix<T, NumColumns>::find(const T &value, int *row, int *col) const
{
    const int idx = m_items.indexOf(value);
    if (idx == -1)
        return false;
    storageIndexToPosition(row, col);
    return true;
}

template <class T, int NumColumns>
void FixedColumnMatrix<T, NumColumns>::storageIndexToPosition(int idx, int *row, int *col)
{
    *row = idx / NumColumns;
    *col = idx % NumColumns;
}
} // namespace

// Porting hook
static inline bool wantTwoLineStyle()
{
    const QStyle *style = QApplication::style();
    return QFormLayout::QtopiaTwoLineStyle == style->styleHint((QStyle::StyleHint)QPhoneStyle::SH_FormStyle, 0, 0);
}

// -- our data structure for our items
// This owns the QLayoutItem
struct QFormLayoutItem
{
    QFormLayoutItem(QLayoutItem* i) : item(i), isHfw(false) {}
    ~QFormLayoutItem() {delete item;}

    QLayoutItem *item;

    // set by updateSizes
    bool isHfw;
    QSize minSize;
    QSize sizeHint;
    QSize maxSize;

    // also set by updateSizes
    int sbsHSpace; // only used for side by side, for the field item only (not label)
    int vSpace; // This is the spacing to the item in the row above

    // set by setupVerticalLayoutData
    bool sideBySide;
    int vLayoutIndex;

    // set by setupHorizontalLayoutData
    int layoutPos;
    int layoutWidth;

    // Wrappers
    QWidget *widget() {return item->widget();}
    QLayout *layout() {return item->layout();}

    bool hasHeightForWidth() const {return item->hasHeightForWidth();}
    int heightForWidth(int width) const {return item->heightForWidth(width);}
    int minimumHeightForWidth(int width) const {return item->minimumHeightForWidth(width);}
    Qt::Orientations expandingDirections() const {return item->expandingDirections();}
    QSizePolicy::ControlTypes controlTypes() const {return item->controlTypes();}

    void setGeometry(const QRect& r) {item->setGeometry(r);}
    QRect geometry() const {return item->geometry();}

    // For use with FixedColumnMatrix
    bool operator==(const QFormLayoutItem& other) {return item == other.item;}
};

// --------- QFormLayoutPrivate
class QFormLayoutPrivate : public QLayoutPrivate
{
    Q_DECLARE_PUBLIC(QFormLayout)
public:
    QFormLayoutPrivate();
    ~QFormLayoutPrivate() {};

    void addItem(QLayoutItem * item, int row, int column);
    void addLayout(QLayout *layout, int row, int column);
    void addWidget(QWidget *widget, int row, int column);

    void arrangeWidgets(const QVector<QLayoutStruct>& layouts, QRect &rect);

    enum SizeType {
        Min,
        Pref
    };

    typedef FixedColumnMatrix <QFormLayoutItem*, ColumnCount> ItemMatrix;
    ItemMatrix m_items;

    int layoutWidth;    // the last width that we called setupVerticalLayoutData on (for vLayouts)
    bool dirty; // have we (not) layed out yet?
    void setupVerticalLayoutData(int width);
    void setupHorizontalLayoutData(int width);

    bool sizesDirty; // have we (not) gathered layout item sizes?
    void updateSizes();

    bool has_hfw;
    int hfw_width;
    int hfw_height;
    int hfw_minheight;
    void recalcHFW(int w);
    void setupHfwLayoutData();

    int min_width;  // the width that gets turned into minSize (from updateSizes)
    int sh_width;   // the width that gets turned into prefSize (from updateSizes)
    int thresh_width; // the width that we start splitting label/field pairs at (from updateSizes)
    QSize minSize;
    QSize prefSize;
    void calcSizeHints();

    QVector<QLayoutStruct> vLayouts; // set by setupVerticalLayoutData;
    int vLayoutCount;               // Number of rows we calculated in setupVerticalLayoutData
    int maxLabelWidth;              // the label width we calculated in setupVerticalLayoutData

    QVector<QLayoutStruct> hfwLayouts;

    int hSpacing;
    int vSpacing;
};

QFormLayoutPrivate::QFormLayoutPrivate() :
    dirty(true), sizesDirty(true), has_hfw(false), hfw_width(-1), min_width(-1), sh_width(-1), thresh_width(QLAYOUTSIZE_MAX), minSize(-1,-1), prefSize(-1,-1), hSpacing(-1), vSpacing(-1)
{
}

// Skip 0 when looking up items
static int layoutIndexToStorageIndex(const QFormLayoutPrivate::ItemMatrix &m, int desiredIndex)
{
    const QFormLayoutPrivate::ItemMatrix::Storage &items = m.storage();

    int index = 0;
    const int count = items.count();
    for (int i = 0; i < count; i++) {
        if (items[i]) {
            if (index == desiredIndex)
                return i;
            index++;
        }
    }
    return -1;
}

static inline void updateFormLayoutItem(QFormLayoutItem* item, int userVSpacing)
{
    item->minSize = item->item->minimumSize();
    item->maxSize = item->item->maximumSize();
    item->sizeHint = item->item->sizeHint();
    item->isHfw = item->item->hasHeightForWidth();
    item->vSpace = userVSpacing;
}

/*
   Iterate over all the controls and gather their size
   information (min, sizeHint and max).  Also work out
   what the spacing between pairs of controls should be, and
   figure out the min and sizeHint widths
*/
void QFormLayoutPrivate::updateSizes()
{
    Q_Q(QFormLayout);

    if (sizesDirty) {
        bool twoLine = wantTwoLineStyle();
        int rr = m_items.rowCount();

        has_hfw = false;

        QFormLayoutItem *prevLbl = 0;
        QFormLayoutItem *prevFld = 0;

        QWidget *parent = q->parentWidget();
        QStyle *style = parent ? parent->style() : 0;

        int userVSpacing = q->verticalSpacing();
        int userHSpacing = twoLine ? 0 : q->horizontalSpacing();

        int maxMinLblWidth = 0;
        int maxMinFldWidth = 0; // field with label
        int maxMinIfldWidth = 0; // independent field

        int maxShLblWidth = 0;
        int maxShFldWidth = 0;
        int maxShIfldWidth = 0;

        for (int i = 0; i < rr; i++) {
            QFormLayoutItem *lbl =  m_items(i, 0);
            QFormLayoutItem *fld = m_items(i, 1);

            // Skip empty rows
            if (!lbl && !fld)
                continue;

            if (lbl) {
                updateFormLayoutItem(lbl, userVSpacing);
                if (lbl->isHfw)
                    has_hfw = true;
            }
            if (fld) {
                updateFormLayoutItem(fld, userVSpacing);
                fld->sbsHSpace = lbl ? userHSpacing : 0;
                if (fld->isHfw)
                    has_hfw = true;
            }

            // See if we need to calculate default spacings
            if ((userHSpacing < 0 || userVSpacing < 0) && style) {
                QSizePolicy::ControlTypes lbltypes = lbl ? lbl->controlTypes() : QSizePolicy::DefaultType;
                QSizePolicy::ControlTypes fldtypes = fld ? fld->controlTypes() : QSizePolicy::DefaultType;

                // VSpacing
                if (userVSpacing < 0) {
                    if (twoLine) {
                        // lbl spacing is to a previous item
                        QFormLayoutItem *lbltop = prevFld ? prevFld : prevLbl;
                        // fld spacing is to the lbl (or a previous item)
                        QFormLayoutItem *fldtop = lbl ? lbl : lbltop;
                        QSizePolicy::ControlTypes lbltoptypes = lbltop ? lbltop->controlTypes() : QSizePolicy::DefaultType;
                        QSizePolicy::ControlTypes fldtoptypes = fldtop ? fldtop->controlTypes() : QSizePolicy::DefaultType;
                        if(lbl && lbltop)
                            lbl->vSpace = style->combinedLayoutSpacing(lbltoptypes, lbltypes, Qt::Vertical, 0, parent);
                        if (fld && fldtop)
                            fld->vSpace = style->combinedLayoutSpacing(fldtoptypes, fldtypes, Qt::Vertical, 0, parent);
                    } else {
                        // Side by side..  we have to also consider the spacings to empty cells, which can strangely be more than
                        // non empty cells..
                        QFormLayoutItem *lbltop = prevLbl ? prevLbl : prevFld;
                        QFormLayoutItem *fldtop = prevFld;
                        QSizePolicy::ControlTypes lbltoptypes = lbltop ? lbltop->controlTypes() : QSizePolicy::DefaultType;
                        QSizePolicy::ControlTypes fldtoptypes = fldtop ? fldtop->controlTypes() : QSizePolicy::DefaultType;

                        // To be compatible to QGridLayout, we have to compare solitary labels & fields with both predecessors
                        if (lbl) {
                            if (!fld) {
                                int lblspacing = style->combinedLayoutSpacing(lbltoptypes, lbltypes, Qt::Vertical, 0, parent);
                                int fldspacing = style->combinedLayoutSpacing(fldtoptypes, lbltypes, Qt::Vertical, 0, parent);
                                lbl->vSpace = qMax(lblspacing, fldspacing);
                            } else
                                lbl->vSpace = style->combinedLayoutSpacing(lbltoptypes, lbltypes, Qt::Vertical, 0, parent);
                        }

                        if (fld) {
                            // check spacing against both the previous label and field
                            if (!lbl) {
                                int lblspacing = style->combinedLayoutSpacing(lbltoptypes, fldtypes, Qt::Vertical, 0, parent);
                                int fldspacing = style->combinedLayoutSpacing(fldtoptypes, fldtypes, Qt::Vertical, 0, parent);
                                fld->vSpace = qMax(lblspacing, fldspacing);
                            } else
                                fld->vSpace = style->combinedLayoutSpacing(fldtoptypes, fldtypes, Qt::Vertical, 0, parent);
                        }
                    }
                }

                // HSpacing
                if (userHSpacing < 0 && !twoLine && lbl && fld) {
                    fld->sbsHSpace = style->combinedLayoutSpacing(lbltypes, fldtypes, Qt::Horizontal, 0, parent);
                }
            }

            // Now update our min/sizehint widths
            // We choose to put the spacing in the field side in sbs, so
            // the right edge of the labels will align, but fields may
            // be a little ragged.. since different controls may have
            // different appearances, a slight raggedness in the left
            // edges of fields can be tolerated.
            // (Note - fld->sbsHSpace is 0 for twoLines mode)
            if (lbl) {
                maxMinLblWidth = qMax(maxMinLblWidth, lbl->minSize.width());
                maxShLblWidth = qMax(maxShLblWidth, lbl->sizeHint.width());
                if (fld) {
                    maxMinFldWidth = qMax(maxMinFldWidth, fld->minSize.width() + fld->sbsHSpace);
                    maxShFldWidth = qMax(maxShFldWidth, fld->sizeHint.width() + fld->sbsHSpace);
                }
            } else if (fld) {
                maxMinIfldWidth = qMax(maxMinIfldWidth, fld->minSize.width());
                maxShIfldWidth = qMax(maxShIfldWidth, fld->sizeHint.width());
            }

            prevLbl = lbl;
            prevFld = fld;
        }

        // Now, finally update the min/sizeHint widths
        if (twoLine) {
            sh_width = qMax(maxShLblWidth, qMax(maxShIfldWidth, maxShFldWidth));
            min_width = qMax(maxMinLblWidth, qMax(maxMinIfldWidth, maxMinFldWidth));
            // in two line, we don't care as much about the threshold width
            thresh_width = 0;
        } else {
            // This is just the max widths glommed together
            sh_width = qMax(maxShLblWidth + maxShFldWidth, maxShIfldWidth);
            min_width = qMax(maxMinLblWidth + maxMinFldWidth, maxMinIfldWidth);
            // We split a pair at label sh + field min (XXX for now..)
            thresh_width = maxShLblWidth + maxMinFldWidth;
        }
    }
    sizesDirty = false;
}

void QFormLayoutPrivate::recalcHFW(int w)
{
    setupHfwLayoutData();

    int h = 0;
    int mh = 0;

    for (int r = 0; r < vLayoutCount; r++) {
        int spacing = hfwLayouts.at(r).spacing;
        h += hfwLayouts.at(r).sizeHint + spacing;
        mh += hfwLayouts.at(r).minimumSize + spacing;
    }

    hfw_width = w;
    hfw_height = qMin(QLAYOUTSIZE_MAX, h);
    hfw_minheight = qMin(QLAYOUTSIZE_MAX, mh);
}

void QFormLayoutPrivate::setupHfwLayoutData()
{
    // setupVerticalLayoutData must be called before this
    // setupHorizontalLayoutData must also be called before this
    // copies non hfw data into hfw
    // then updates size and min


    // Note: QGridLayout doesn't call minimumHeightForWidth,
    // but instead uses heightForWidth for both min and sizeHint.
    // For the common case where minimumHeightForWidth just calls
    // heightForWidth, we do the calculation twice, which can be
    // very expensive for word wrapped QLabels/QTextEdits, for example.
    // So we just use heightForWidth as well.
    int i;
    int rr = m_items.rowCount();

    hfwLayouts.clear();
    hfwLayouts.resize(vLayoutCount);
    for (i = 0; i < vLayoutCount; i++) {
        hfwLayouts[i] = vLayouts.at(i);
    }

    for (i = 0; i < rr; i++) {
        QFormLayoutItem *lbl = m_items(i,0);
        QFormLayoutItem *fld = m_items(i,1);

        if (lbl) {
            if (lbl->isHfw) {
                // We don't check sideBySide here, since a label is only
                // ever side by side with its field
                int hfw = lbl->heightForWidth(lbl->layoutWidth);
                hfwLayouts[lbl->vLayoutIndex].minimumSize = hfw;
                hfwLayouts[lbl->vLayoutIndex].sizeHint = hfw;
            } else {
                // Reset these here, so the fld can do a qMax below (the previous value may have
                // been the fields non-hfw values, which are often larger than hfw)
                hfwLayouts[lbl->vLayoutIndex].sizeHint = lbl->sizeHint.height();
                hfwLayouts[lbl->vLayoutIndex].minimumSize = lbl->minSize.height();
            }
        }

        if (fld) {
            int hfw = fld->isHfw ? fld->heightForWidth(fld->layoutWidth) : 0;
            int h = fld->isHfw ? hfw : fld->sizeHint.height();
            int mh = fld->isHfw ? hfw : fld->minSize.height();

            if (fld->sideBySide) {
                int oh = hfwLayouts.at(fld->vLayoutIndex).sizeHint;
                int omh = hfwLayouts.at(fld->vLayoutIndex).minimumSize;

                hfwLayouts[fld->vLayoutIndex].sizeHint = qMax(h, oh);
                hfwLayouts[fld->vLayoutIndex].minimumSize = qMax(mh, omh);
            } else {
                hfwLayouts[fld->vLayoutIndex].sizeHint = h;
                hfwLayouts[fld->vLayoutIndex].minimumSize = mh;
            }
        }
    }
}

/*
  Given up to four items involved in a vertical spacing calculation
  (two rows * two columns), return the max vertical spacing for the
  row containing item1 (which may also include item2)
  We assume parent and item1 are not null.

  If a particular row is split, then the spacings for that row and
  the following row are affected, and this function should be
  called with recalculate = true for both rows (note: only rows with both
  a label and a field can be split).

  In particular:

  1) the split label's row vspace needs to be changed to qMax(label/prevLabel, label/prevField)
    [call with item1 = lbl, item2 = null, prevItem1 & prevItem2 as before]
  2) the split field's row vspace needs to be changed to the label/field spacing
    [call with item1 = fld, item2 = null, prevItem1 = lbl, prevItem2 = null]

 [if the next row has one item, 'item']
  3a) the following row's vspace needs to be changed to item/field spacing (would
      previously been the qMax(item/label, item/field) spacings)
    [call with item1 = item, item2 = null, prevItem1 = fld, prevItem2 = null]

  [if the next row has two items, 'label2' and 'field2']
  3b) the following row's vspace needs to be changed to be qMax(field/label2, field/field2) spacing
    [call with item1 = label2, item2 = field2, prevItem1 = fld, prevItem2 = null]

  In the (common) non split case, we can just use the precalculated vspace (possibly qMaxed between
  label and field).

  If recalculate is true, we expect:
  -  parent != null
  -  item1 != null
  -  item2 can be null
  -  prevItem1 can be null
  -  if item2 is not null, prevItem2 will be null (e.g. steps 1 or 3 above)
  -  if prevItem1 is null, prevItem2 will be null
*/
static inline int spacingHelper(QWidget* parent, QStyle *style, int userVSpacing, bool recalculate, QFormLayoutItem* item1, QFormLayoutItem* item2, QFormLayoutItem* prevItem1, QFormLayoutItem *prevItem2)
{
    int spacing = userVSpacing;
    if (spacing < 0) {
        if (!recalculate) {
            if (item1)
                spacing = item1->vSpace;
            if (item2)
                spacing = qMax(spacing, item2->vSpace);
        } else {
            if (style && prevItem1) {
                QSizePolicy::ControlTypes itemtypes = item1->controlTypes();
                int spacing2 = 0;

                spacing = style->combinedLayoutSpacing(itemtypes, prevItem1->controlTypes(), Qt::Vertical, 0, parent);

                // At most of one of item2 and prevItem2 will be non-null
                if (item2)
                    spacing2 = style->combinedLayoutSpacing(item2->controlTypes(), prevItem1->controlTypes(), Qt::Vertical, 0, parent);
                else if(prevItem2)
                    spacing2 = style->combinedLayoutSpacing(itemtypes, prevItem2->controlTypes(), Qt::Vertical, 0, parent);

                spacing = qMax(spacing, spacing2);
            }
        }
    } else {
        if (prevItem1) {
            QWidget *wid = prevItem1->item->widget();
            if (wid)
                spacing = qMax(spacing, prevItem1->geometry().top() - wid->geometry().top() );
        }
        if (prevItem2) {
            QWidget *wid = prevItem2->item->widget();
            if (wid)
                spacing = qMax(spacing, prevItem2->geometry().top() - wid->geometry().top() );
        }
    }
    return spacing;
}

static inline void initLayoutStruct(QLayoutStruct& sl, QFormLayoutItem* item)
{
    sl.init(0, item->minSize.height());
    sl.sizeHint = item->sizeHint.height();
    sl.maximumSize = item->maxSize.height();
    sl.expansive = (item->expandingDirections() & Qt::Vertical);
    sl.empty = false;
}

void QFormLayoutPrivate::setupVerticalLayoutData(int width)
{
    Q_Q(QFormLayout);

    // Early out if we have no changes that would cause a change in vertical layout
    if ((width == layoutWidth || (width >= thresh_width && layoutWidth >= thresh_width)) && !dirty && !sizesDirty)
        return;

    layoutWidth = width;

    int rr = m_items.rowCount();
    int vidx = 0;
    bool twoline = wantTwoLineStyle();
    vLayouts.clear();
    vLayouts.resize((2 * rr) + 1); // a max, some may be unused

    QStyle *style = 0;

    int userVSpacing = q->verticalSpacing();

    if (userVSpacing < 0) {
        if (QWidget *widget = q->parentWidget())
            style = widget->style();
    }

    // make sure our sizes are up to date
    updateSizes();

    // Grab the widest label width here
    // This might be different from the value computed during
    // sizeHint/minSize, since we don't count label/field pairs that
    // are split.
    maxLabelWidth = 0;
    if (!twoline) {
        for (int i = 0; i < rr; i++) {
            const QFormLayoutItem *lbl = m_items(i,0);
            const QFormLayoutItem *fld = m_items(i,1);
            if (lbl && (lbl->sizeHint.width()  + (fld ? fld->minSize.width() : 0) <= width)) {
                maxLabelWidth = qMax(maxLabelWidth, lbl->sizeHint.width());
            }
        }
    } else {
        maxLabelWidth = width;
    }

    QFormLayoutItem *prevItem1 = 0;
    QFormLayoutItem *prevItem2 = 0;
    bool prevRowSplit = false;

    for (int i = 0; i < rr; i++) {
        QFormLayoutItem *lbl =  m_items(i, 0);
        QFormLayoutItem *fld = m_items(i, 1);

        // Totally ignore empty rows...
        if (!lbl && !fld)
            continue;

        QSize min1;
        QSize min2;
        QSize sh1;
        QSize sh2;
        if (lbl) {
            min1 = lbl->minSize;
            sh1 = lbl->sizeHint;
        }
        if (fld) {
            min2 = fld->minSize;
            sh2 = fld->sizeHint;
        }

        // In separate lines, we make a vLayout for everything that isn't null
        // in side by side, we only separate label/field if we're going to wrap it
        bool splitSideBySide = !twoline && ((maxLabelWidth < sh1.width()) || (width < (maxLabelWidth + min2.width())));

        if (twoline || splitSideBySide) {

            if (lbl) {
                initLayoutStruct(vLayouts[vidx], lbl);

                if (vidx > 0)
                    vLayouts[vidx - 1].spacing = spacingHelper(q->parentWidget(), style, userVSpacing, splitSideBySide || prevRowSplit, lbl, 0, prevItem1, prevItem2);

                lbl->vLayoutIndex = vidx;
                lbl->sideBySide = false;

                prevItem1 = lbl;
                prevItem2 = 0;

                ++vidx;
            }

            if (fld) {
                initLayoutStruct(vLayouts[vidx], fld);

                if (vidx > 0)
                    vLayouts[vidx - 1].spacing = spacingHelper(q->parentWidget(), style, userVSpacing, splitSideBySide || prevRowSplit, fld, 0, prevItem1, prevItem2);

                fld->vLayoutIndex = vidx;
                fld->sideBySide = false;

                prevItem1 = fld;
                prevItem2 = 0;

                ++vidx;
            }

            prevRowSplit = splitSideBySide;
        } else {
            // we're in side by side mode, and we have enough space to do that
            QSize max1 (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            QSize max2 (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

            bool expanding = false;

            if (lbl) {
                max1 = lbl->maxSize;
                if (lbl->expandingDirections() & Qt::Vertical)
                    expanding = true;

                lbl->sideBySide = fld ? true : false;
                lbl->vLayoutIndex = vidx;
            }

            if (fld) {
                max2 = fld->maxSize;
                if (fld->expandingDirections() & Qt::Vertical)
                    expanding = true;

                fld->sideBySide = lbl ? true : false;
                fld->vLayoutIndex = vidx;
            }

            vLayouts[vidx].init(0, qMax(min1.height(), min2.height()));
            vLayouts[vidx].sizeHint = qMax(sh1.height(), sh2.height());
            vLayouts[vidx].maximumSize = qMin(max1.height(), max2.height());
            vLayouts[vidx].expansive = expanding;
            vLayouts[vidx].empty = false;

            if (vidx > 0)
                vLayouts[vidx - 1].spacing = spacingHelper(q->parentWidget(), style, userVSpacing, prevRowSplit, lbl, fld, prevItem1, prevItem2);

            if (lbl) {
                prevItem1 = lbl;
                prevItem2 = fld;
            } else {
                prevItem1 = fld;
                prevItem2 = 0;
            }

            prevRowSplit = false;
            ++vidx;
        }
    }

    // XXX Temporary hack to force neater layout on Qtopia (this is just an expanding spacer at the bottom)
    if (vidx > 0) {
        vLayouts[vidx].init(1, 0);
        vLayouts[vidx].expansive = true;
        ++vidx;
    }

    vLayoutCount = vidx;
    dirty = false;
}

void QFormLayoutPrivate::setupHorizontalLayoutData(int width)
{
    // requires setupVerticalLayoutData to be called first

    int rr = m_items.rowCount();
    bool twoline = wantTwoLineStyle();

    for (int i = 0; i < rr; i++) {
        QFormLayoutItem *lbl =  m_items(i, 0);
        QFormLayoutItem *fld = m_items(i, 1);

        // Totally ignore empty rows...
        if (!lbl && !fld)
            continue;

        if (lbl) {
            // if there is a field, and we're side by side, we use maxLabelWidth
            // otherwise we just use the sizehint
            lbl->layoutWidth = (fld && lbl->sideBySide) ? maxLabelWidth : lbl->sizeHint.width();
            lbl->layoutPos = 0;
        }

        if (fld) {
            // This is the default amount allotted to fields in sbs
            int fldwidth = width - maxLabelWidth - fld->sbsHSpace;

            // If we've split a row, we still decide to align
            // the field with all the other field if it will fit
            // Fields in sbs mode get the remnants of the maxLabelWidth
            if (!fld->sideBySide) {
                if (twoline || !lbl || fld->sizeHint.width() > fldwidth) {
                    fld->layoutWidth = width;
                    fld->layoutPos = 0;
                } else {
                    fld->layoutWidth = fldwidth;
                    fld->layoutPos = width - fldwidth;
                }
            } else {
                // We're sbs, so we should have a label
                fld->layoutWidth = fldwidth;
                fld->layoutPos = width - fldwidth;
            }
        }
    }
}

// debug layout
/*QDebug operator<<(QDebug d, const QFormLayout& l)
{
    const int c = l.count();
    int row, column;
    d << "Count " << c << '\n';
    for (int i = 0; i < c; i++) {
        QLayoutItem *item = l.itemAt(i);
        d << i << item->widget() << '\n';
    }
    return d;
}*/

void QFormLayoutPrivate::calcSizeHints()
{
    Q_Q(QFormLayout);

    int leftMargin, topMargin, rightMargin, bottomMargin;
    q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    updateSizes();
    setupVerticalLayoutData(QLAYOUTSIZE_MAX);
    // Don't need to call setupHorizontal here

    int h = topMargin + bottomMargin;
    int mh = topMargin + bottomMargin;

    // The following are set in updateSizes
    int w = sh_width + leftMargin + rightMargin;
    int mw = min_width + leftMargin + rightMargin;

    for (int i = 0; i < vLayoutCount; i++) {
        int spacing = vLayouts.at(i).spacing;
        h += vLayouts.at(i).sizeHint + spacing;
        mh += vLayouts.at(i).minimumSize + spacing;
    }

    minSize.rwidth() = qMin(mw, QLAYOUTSIZE_MAX);
    minSize.rheight() = qMin(mh, QLAYOUTSIZE_MAX);
    prefSize.rwidth() = qMin(w, QLAYOUTSIZE_MAX);
    prefSize.rheight() = qMin(h, QLAYOUTSIZE_MAX);
}

void QFormLayoutPrivate::addItem(QLayoutItem * item, int row, int column)
{
    Q_ASSERT(column == 0 || column == 1);

    if (row >= m_items.rowCount()) {
        m_items.addRow(0);
        row = m_items.rowCount() - 1;
    } else {
        // XXX occupied cell, adding twice, do qwarning
        if (m_items(row, column)) // Hmm, if occupied/insert?
            m_items.insertRow(row, 0);
    }
    if (item) {
        QFormLayoutItem *i = new QFormLayoutItem(item);
        m_items(row, column) = i;
    } else {
        // Null row.
        m_items(row, column) = 0;
    }
}

void QFormLayoutPrivate::addLayout(QLayout *layout, int row, int column)
{
    Q_Q(QFormLayout);
    q->addChildLayout(layout);
    addItem(layout, row, column);
}

void QFormLayoutPrivate::addWidget(QWidget *widget, int row, int column)
{
    Q_Q(QFormLayout);
    q->addChildWidget(widget);
    addItem(new QWidgetItem(widget), row, column);
}

/*!
    \class QFormLayout
    \preliminary
    \since 4.3
    \brief The QFormLayout class manages forms of input widgets with associated labels.

    \ingroup appearance
    \ingroup geomanagement

    \mainclass

    QFormLayout lays out widgets in a form. The way a form is laid out is based on the style hint
    QPhoneStyle::SH_FormStyle, which can have the values QtopiaDefaultStyle or QtopiaTwoLineStyle.
    QtopiaDefaultStyle places labels side by side with their buddy fields, while QtopiaTwoLineStyle
    places labels above their buddy fields. The following table shows the same interface in each style.

    \table 50%
    \header
        \o QtopiaDefaultStyle
        \o QtopiaTwoLineStyle
    \row
        \o \image qtopiadefaultstyle.png
        \o \image qtopiatwolinestyle.png
    \endtable

    In QtopiaDefaultStyle mode, labels will be given enough horizontal space to fit the widest label, and the
    rest of the space will be given to the fields. If the minimum size of a label/field pair is wider than
    the available space, the field will be wrapped to the next line.

    \sa QBoxLayout, QGridLayout, QStackedLayout
*/

/*!
    \enum QFormLayout::FormStyle

    This enum specifies the different looks supported by QFormLayout.

    \value QtopiaDefaultStyle Labels are placed side by side with their buddy fields.
    \value QtopiaTwoLineStyle Labels are placed above their buddy fields
*/

/*!
    Constructs a new QFormLayout with parent widget \a parent.
*/
QFormLayout::QFormLayout(QWidget *parent)
    : QLayout(*new QFormLayoutPrivate,0,parent)
{
}

/*!
    Constructs a new QFormLayout.
*/
QFormLayout::QFormLayout()
    : QLayout(*new QFormLayoutPrivate,0,0)
{
}

/*!
    Destroys the form layout.
*/
QFormLayout::~QFormLayout()
{
    //Note: child layouts are deleted
    Q_D(QFormLayout);

    // Convoluted destruction to make sure that
    // the m_item array entries are zeroed before deletion,
    // since we can get recursive.
    int rr = d->m_items.rowCount();
    for (int i = 0; i < rr; i++) {
        QFormLayoutItem* item = d->m_items(i, 0);
        if (item) {
            d->m_items(i,0) = 0;
            delete item;
        }
        item = d->m_items(i, 1);
        if (item) {
            d->m_items(i, 1) = 0;
            delete item;
        }
    }
}

/*!
    Adds \a field to the end of this form layout with label \a label.
    In \c SideBySide mode, if \a label is null the widget will take all
    available horizontal space.
*/
void QFormLayout::addRow(const QString &label, QWidget *field)
{
    insertRow(rowCount(), label, field);
}

/*!
    Adds \a field to the end of this form layout with label \a label.
    In \c SideBySide mode, if \a label is null the widget will take all
    available horizontal space.
*/
void QFormLayout::addRow(QWidget *label, QWidget *field)
{
    insertRow(rowCount(), label, field);
}

/*!
    Inserts \a field at position \a index with label \a label.
    If \a index is negative, the widget is added at the end.
    In \c SideBySide mode, if \a label is null the widget will take all
    available horizontal space.
*/
void QFormLayout::insertRow(int index, const QString &label, QWidget *field)
{
    if (index < 0)
        index = rowCount();

    // Auto create the label the way we like em
    QLabel *l = label.isNull() ? 0 : new QLabel(label);
    if (l) {
        if (wantTwoLineStyle())
            l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        else
            l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (field)
            l->setBuddy(field);
    }
    insertRow(index, l, field);
}

/*!
    Inserts \a field at position \a index with label \a label.
    If \a index is negative, the widget is added at the end.
    In \c SideBySide mode, if \a label is null the widget will take all
    available horizontal space.
*/
void QFormLayout::insertRow(int index, QWidget *label, QWidget *field)
{
    // XXX index is row, not real index
    if (index < 0)
        index = rowCount();

    // Realistically, we want at least one non-null widget
    // (this path is only used by add/insert)
    if (!label && !field) {
        qWarning("QFormLayout: cannot add null label and field to %s", objectName().toLocal8Bit().data());
    } else {
        Q_D(QFormLayout);
        // one could be null
        if(label)
            d->addWidget(label, index, 0);
        if(field)
            d->addWidget(field, index, 1);
        invalidate();
    }
}

/*!
    Adds \a layout to the end of this form layout with label \a label.
    In \c SideBySide mode, if \a label is null the layout will take all
    available horizontal space.
*/
void QFormLayout::addRow(const QString &label, QLayout *layout)
{
    insertRow(rowCount(), label, layout);
}

/*!
    Adds \a layout to the end of this form layout with label \a label.
    In \c SideBySide mode, if \a label is null the layout will take all
    available horizontal space.
*/
void QFormLayout::addRow(QWidget *label, QLayout *layout)
{
    insertRow(rowCount(), label, layout);
}

/*!
    Inserts \a layout at position \a index with label \a label.
    If \a index is negative, the layout is added at the end.
    In \c SideBySide mode, if \a label is null the layout will take all
    available horizontal space.
*/
void QFormLayout::insertRow(int index, const QString &label, QLayout *layout)
{
    if (index < 0)
        index = rowCount();

    QLabel *l = label.isNull() ? 0 : new QLabel(label);
    if (l) {
        if (wantTwoLineStyle())
            l->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        else
            l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    insertRow(index, l, layout);
}

/*!
    Inserts \a layout at position \a index with label \a label.
    If \a index is negative, the layout is added at the end.
    In \c SideBySide mode, if \a label is null the layout will take all
    available horizontal space.
*/
void QFormLayout::insertRow(int index, QWidget *label, QLayout *layout)
{
    if (!layout)
        return;

    if (index < 0)
        index = rowCount();

    Q_D(QFormLayout);
    if (label)
        d->addWidget(label, index, 0);
    d->addLayout(layout, index, 1);
    invalidate();
}

/*!
    \reimp

    Use addRow() instead.
*/
void QFormLayout::addItem(QLayoutItem *item)
{
    Q_D(QFormLayout);
    d->addItem(item, d->m_items.rowCount(), 1);
}

/*!
    \reimp
*/
int QFormLayout::count() const
{
    Q_D(const QFormLayout);
    return d->m_items.rowCount() * ColumnCount -  d->m_items.count(0);
}

/*!
    \reimp
*/
QLayoutItem *QFormLayout::itemAt(int desiredIndex) const
{
    Q_D(const QFormLayout);

    const int storageIndex = layoutIndexToStorageIndex(d->m_items, desiredIndex);
    return storageIndex == -1 ? 0 : d->m_items.storage()[storageIndex]->item;
}

/*!
    \reimp
*/
QLayoutItem *QFormLayout::takeAt(int index)
{
    Q_D(QFormLayout);

    const int storageIndex = layoutIndexToStorageIndex(d->m_items, index);
    if (storageIndex == -1) {
        qWarning() << "QFormLayout::takeAt: Invalid index " << index;
        return 0;
    }

    int row, col;
    QFormLayoutPrivate::ItemMatrix::storageIndexToPosition(storageIndex, &row, &col);
    Q_ASSERT(d->m_items(row, col));

    QFormLayoutItem *item = d->m_items(row, col);
    Q_ASSERT(item);
    d->m_items(row, col) = 0;

    invalidate();

    // grab ownership back from the QFormLayoutItem
    QLayoutItem *i = item->item;
    item->item = NULL;
    delete item;
    return i;
}

/*!
    \reimp
*/
Qt::Orientations QFormLayout::expandingDirections() const
{
    return (Qt::Vertical | Qt::Horizontal);
}

/*!
    \reimp
*/
bool QFormLayout::hasHeightForWidth() const
{
    // We can't definitively say that we're not
    // hfw (unless we're separate lines and have no hfw, but that needs updateSizes()
    return true;
}

/*!
    \reimp
*/
int QFormLayout::heightForWidth(int width) const
{
    Q_D(const QFormLayout);
    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    int targetWidth = width - leftMargin - rightMargin;
    if (d->hfw_width != targetWidth) {
        QFormLayoutPrivate *dat = const_cast<QFormLayoutPrivate *>(d);
        dat->setupVerticalLayoutData(targetWidth);
        dat->setupHorizontalLayoutData(targetWidth);
        dat->recalcHFW(targetWidth);
    }
    return d->hfw_height + topMargin + bottomMargin;
}

/*!
    \reimp
*/
void QFormLayout::setGeometry(const QRect &rect)
{
    Q_D(QFormLayout);
    if (d->dirty || rect != geometry()) {
        QRect cr = rect;
        int leftMargin, topMargin, rightMargin, bottomMargin;
        getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
        cr.adjust(+leftMargin, +topMargin, -rightMargin, -bottomMargin);

        bool hfw = hasHeightForWidth();
        d->setupVerticalLayoutData(cr.width());
        d->setupHorizontalLayoutData(cr.width());
        if (hfw && d->hfw_width != cr.width()) {
            d->recalcHFW(cr.width());
        }
        if (hfw) {
            qGeomCalc(d->hfwLayouts, 0, d->vLayoutCount, cr.y(), cr.height());
            d->arrangeWidgets(d->hfwLayouts, cr);
        } else {
            qGeomCalc(d->vLayouts, 0, d->vLayoutCount, cr.y(), cr.height());
            d->arrangeWidgets(d->vLayouts, cr);
        }
        QLayout::setGeometry(rect);
    }
}

/*!
    \reimp
*/
QSize QFormLayout::sizeHint() const
{
    Q_D(const QFormLayout);
    if (!d->prefSize.isValid()) {
        QFormLayoutPrivate *dat = const_cast<QFormLayoutPrivate *>(d);
        dat->calcSizeHints();
    }
    return d->prefSize;
}

/*!
    \reimp
*/
QSize QFormLayout::minimumSize() const
{
    Q_D(const QFormLayout);
    if (!d->minSize.isValid()) {
        QFormLayoutPrivate *dat = const_cast<QFormLayoutPrivate *>(d);
        dat->calcSizeHints();
    }
    return d->minSize;
}

/*!
    \reimp
*/
void QFormLayout::invalidate()
{
    Q_D(QFormLayout);
    d->dirty = true;
    d->sizesDirty = true;
    d->minSize = QSize(-1,-1);     //"dirty"
    d->prefSize = QSize(-1,-1);    //"dirty"
    d->hfw_width = -1;
    d->layoutWidth = -1;
    QLayout::invalidate();
}

/*!
    Returns the number of rows in the form.
*/
int QFormLayout::rowCount() const
{
    Q_D(const QFormLayout);
    return d->m_items.rowCount();
}

/*!
    \property QFormLayout::horizontalSpacing
    \brief the spacing between widgets that are laid out side by side

    If no value is explicitly set, the layout's horizontal spacing is
    inherited from the parent layout, or from the style settings for
    the parent widget.

    \sa verticalSpacing, QStyle::pixelMetric(), {QStyle::}{PM_LayoutHorizontalSpacing}
*/
void QFormLayout::setHorizontalSpacing(int spacing)
{
    Q_D(QFormLayout);
    if (spacing != d->hSpacing) {
        d->hSpacing = spacing;
        invalidate();
    }
}

int QFormLayout::horizontalSpacing() const
{
    Q_D(const QFormLayout);
    if (d->hSpacing >= 0) {
        return d->hSpacing;
    } else {
        return qSmratSpacing(this, QStyle::PM_LayoutHorizontalSpacing);
    }
}

/*!
    \property QFormLayout::verticalSpacing
    \brief the spacing between widgets that are laid out on top of each other

    If no value is explicitly set, the layout's vertical spacing is
    inherited from the parent layout, or from the style settings for
    the parent widget.

    \sa horizontalSpacing, QStyle::pixelMetric(), {QStyle::}{PM_LayoutHorizontalSpacing}
*/
void QFormLayout::setVerticalSpacing(int spacing)
{
    Q_D(QFormLayout);
    if (spacing != d->vSpacing) {
        d->vSpacing = spacing;
        invalidate();
    }
}

int QFormLayout::verticalSpacing() const
{
    Q_D(const QFormLayout);
    if (d->vSpacing >= 0) {
        return d->vSpacing;
    } else {
        return qSmratSpacing(this, QStyle::PM_LayoutVerticalSpacing);
    }
}

void QFormLayoutPrivate::arrangeWidgets(const QVector<QLayoutStruct>& layouts, QRect &rect)
{
    Q_Q(QFormLayout);

    int i;
    const int rr = m_items.rowCount();
    const Qt::LayoutDirection visualDir = q->parentWidget() && q->parentWidget()->isRightToLeft() ? Qt::RightToLeft :  Qt::LeftToRight;

    for (i = 0; i < rr; i++) {
        QFormLayoutItem *lbl =  m_items(i, 0);
        QFormLayoutItem *fld = m_items(i, 1);

        if (lbl) {
            QSize sz(lbl->layoutWidth, layouts.at(lbl->vLayoutIndex).size);
            QPoint p(lbl->layoutPos + rect.x(), layouts.at(lbl->vLayoutIndex).pos);
            // XXX expansion & sizepolicy stuff

            if (lbl->maxSize.isValid())
                sz = sz.boundedTo(lbl->maxSize);
            lbl->setGeometry(QStyle::visualRect(visualDir, rect, QRect(p, sz)));
        }

        if (fld) {
            QSize sz(fld->layoutWidth, layouts.at(fld->vLayoutIndex).size);
            QPoint p(fld->layoutPos + rect.x(), layouts.at(fld->vLayoutIndex).pos);
/*
            if ((fld->widget() && fld->widget()->sizePolicy().horizontalPolicy() & (QSizePolicy::GrowFlag | QSizePolicy::ExpandFlag | QSizePolicy::IgnoreFlag))
                || (fld->layout() && sz.width() < fld->maxSize.width())) {
                sz.rwidth() = fld->layoutWidth;
            }
*/
            if (fld->maxSize.isValid())
                sz = sz.boundedTo(fld->maxSize);

            fld->setGeometry(QStyle::visualRect(visualDir, rect, QRect(p, sz)));
        }
    }
}

/*!
    Adds \a widget to the end of this form layout with no label.
*/
void QFormLayout::addRow(QWidget *widget)
{
    addRow(0, widget);
}

/*!
    Adds \a layout to the end of this form layout with no label.
*/
void QFormLayout::addRow(QLayout *layout)
{
    addRow(0, layout);
}

/*!
    Inserts \a widget at position \a index with no label.
    If \a index is negative, the widget is added at the end.
*/
void QFormLayout::insertRow(int index, QWidget *widget)
{
    insertRow(index, 0, widget);
}

/*!
    Inserts \a layout at position \a index with no label.
    If \a index is negative, the layout is added at the end.
*/
void QFormLayout::insertRow(int index, QLayout *layout)
{
    insertRow(index, 0, layout);
}

