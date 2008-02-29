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

#include "qlayout.h"
#include "private/qlayoutengine_p.h"

#include "qvector.h"
#include "qwidget.h"

#include <qlist.h>
#include <qalgorithms.h>

#include <qdebug.h>
//#define QLAYOUT_EXTRA_DEBUG

typedef qint64 Fixed;
static inline Fixed toFixed(int i) { return (Fixed)i * 256; }
static inline int fRound(Fixed i) {
    return (i % 256 < 128) ? i / 256 : 1 + i / 256;
}

/*
  This is the main workhorse of the QGridLayout. It portions out
  available space to the chain's children.

  The calculation is done in fixed point: "fixed" variables are
  scaled by a factor of 256.

  If the layout runs "backwards" (i.e. RightToLeft or Up) the layout
  is computed mirror-reversed, and it's the caller's responsibility
  do reverse the values before use.

  chain contains input and output parameters describing the geometry.
  count is the count of items in the chain; pos and space give the
  interval (relative to parentWidget topLeft).
*/
void qGeomCalc(QVector<QLayoutStruct> &chain, int start, int count,
                         int pos, int space, int spacer)
{
    int cHint = 0;
    int cMin = 0;
    int cMax = 0;
    int sumStretch = 0;
    int spacerCount = 0;

    bool wannaGrow = false; // anyone who really wants to grow?
    //    bool canShrink = false; // anyone who could be persuaded to shrink?

    bool allEmptyNonstretch = true;
    int i;
    for (i = start; i < start + count; i++) {
        chain[i].done = false;
        cHint += chain[i].smartSizeHint();
        cMin += chain[i].minimumSize;
        cMax += chain[i].maximumSize;
        sumStretch += chain[i].stretch;
        if (!chain[i].empty)
            spacerCount++;
        wannaGrow = wannaGrow || chain[i].expansive || chain[i].stretch > 0;
        allEmptyNonstretch = allEmptyNonstretch && !wannaGrow && chain[i].empty;
    }

    int extraspace = 0;
    if (spacerCount)
        spacerCount--; // only spacers between things

    if (space < cMin + spacerCount * spacer) {
        /*
          Less space than minimumSize; take from the biggest first
        */

        int minSize =  cMin + spacerCount * spacer;

        //shrink the spacers proportionally
        spacer = minSize > 0 ? (spacer * space) / minSize : 0;

        QList<int> list;

        for (i = start; i < start+count; i++) {
            list << chain[i].minimumSize;
        }

        qSort(list);

        int space_left = space - spacerCount*spacer;

        int sum = 0;
        int idx = 0;
        int space_used=0;
        int current = 0;
        while (idx < count && space_used < space_left) {
            current = list.at(idx);
            space_used = sum + current * (count - idx);
            sum += current;
            ++idx;
        }
        --idx;
        int deficit = space_used - space_left;

        int items = count - idx;
        int maxval = current - deficit/items;

        for (i = start; i < start+count; i++) {
            chain[i].size = qMin(chain[i].minimumSize, maxval);
            chain[i].done = true;
        }
    } else if (space < cHint + spacerCount*spacer) {
        /*
          Less space than smartSizeHint(), but more than minimumSize.
          Currently take space equally from each, as in Qt 2.x.
          Commented-out lines will give more space to stretchier
          items.
        */
        int n = count;
        int space_left = space - spacerCount*spacer;
        int overdraft = cHint - space_left;

        // first give to the fixed ones:
        for (i = start; i < start + count; i++) {
            if (!chain[i].done
                 && chain[i].minimumSize >= chain[i].smartSizeHint()) {
                chain[i].size = chain[i].smartSizeHint();
                chain[i].done = true;
                space_left -= chain[i].smartSizeHint();
                // sumStretch -= chain[i].stretch;
                n--;
            }
        }
        bool finished = n == 0;
        while (!finished) {
            finished = true;
            Fixed fp_over = toFixed(overdraft);
            Fixed fp_w = 0;

            for (i = start; i < start+count; i++) {
                if (chain[i].done)
                    continue;
                // if (sumStretch <= 0)
                fp_w += fp_over / n;
                // else
                //    fp_w += (fp_over * chain[i].stretch) / sumStretch;
                int w = fRound(fp_w);
                chain[i].size = chain[i].smartSizeHint() - w;
                fp_w -= toFixed(w); // give the difference to the next
                if (chain[i].size < chain[i].minimumSize) {
                    chain[i].done = true;
                    chain[i].size = chain[i].minimumSize;
                    finished = false;
                    overdraft -= (chain[i].smartSizeHint()
                                   - chain[i].minimumSize);
                    // sumStretch -= chain[i].stretch;
                    n--;
                    break;
                }
            }
        }
    } else { // extra space
        int n = count;
        int space_left = space - spacerCount*spacer;
        // first give to the fixed ones, and handle non-expansiveness
        for (i = start; i < start + count; i++) {
            if (!chain[i].done
                && (chain[i].maximumSize <= chain[i].smartSizeHint()
                    || (wannaGrow && !chain[i].expansive && chain[i].stretch == 0)
                    || (!allEmptyNonstretch && chain[i].empty &&
                        !chain[i].expansive && chain[i].stretch == 0))) {
                chain[i].size = chain[i].smartSizeHint();
                chain[i].done = true;
                space_left -= chain[i].size;
                sumStretch -= chain[i].stretch;
                n--;
            }
        }
        extraspace = space_left;

        /*
          Do a trial distribution and calculate how much it is off.
          If there are more deficit pixels than surplus pixels, give
          the minimum size items what they need, and repeat.
          Otherwise give to the maximum size items, and repeat.

          Paul Olav Tvete has a wonderful mathematical proof of the
          correctness of this principle, but unfortunately this
          comment is too small to contain it.
        */
        int surplus, deficit;
        do {
            surplus = deficit = 0;
            Fixed fp_space = toFixed(space_left);
            Fixed fp_w = 0;
            for (i = start; i < start+count; i++) {
                if (chain[i].done)
                    continue;
                extraspace = 0;
                if (sumStretch <= 0)
                    fp_w += fp_space / n;
                else
                    fp_w += (fp_space * chain[i].stretch) / sumStretch;
                int w = fRound(fp_w);
                chain[i].size = w;
                fp_w -= toFixed(w); // give the difference to the next
                if (w < chain[i].smartSizeHint()) {
                    deficit +=  chain[i].smartSizeHint() - w;
                } else if (w > chain[i].maximumSize) {
                    surplus += w - chain[i].maximumSize;
                }
            }
            if (deficit > 0 && surplus <= deficit) {
                // give to the ones that have too little
                for (i = start; i < start+count; i++) {
                    if (!chain[i].done &&
                         chain[i].size < chain[i].smartSizeHint()) {
                        chain[i].size = chain[i].smartSizeHint();
                        chain[i].done = true;
                        space_left -= chain[i].smartSizeHint();
                        sumStretch -= chain[i].stretch;
                        n--;
                    }
                }
            }
            if (surplus > 0 && surplus >= deficit) {
                // take from the ones that have too much
                for (i = start; i < start+count; i++) {
                    if (!chain[i].done &&
                         chain[i].size > chain[i].maximumSize) {
                        chain[i].size = chain[i].maximumSize;
                        chain[i].done = true;
                        space_left -= chain[i].maximumSize;
                        sumStretch -= chain[i].stretch;
                        n--;
                    }
                }
            }
        } while (n > 0 && surplus != deficit);
        if (n == 0)
            extraspace = space_left;
    }

    /*
      As a last resort, we distribute the unwanted space equally
      among the spacers (counting the start and end of the chain). We
      could, but don't, attempt a sub-pixel allocation of the extra
      space.
    */
    int extra = extraspace / (spacerCount + 2);
    int p = pos + extra;
    for (i = start; i < start+count; i++) {
        chain[i].pos = p;
        p = p + chain[i].size;
        if (!chain[i].empty)
            p += spacer+extra;
    }


#ifdef QLAYOUT_EXTRA_DEBUG
    qDebug() << "qGeomCalc" << "start" << start <<  "count" << count <<  "pos" << pos <<  "space" << space <<  "spacer" << spacer;
    for (i = start; i < start + count; i++) {
        qDebug() << i << ":" << chain[i].minimumSize << chain[i].smartSizeHint() << chain[i].maximumSize
                 << "stretch" << chain[i].stretch << "empty" << chain[i].empty << "expansive" << chain[i].expansive;
        qDebug() << "result pos" << chain[i].pos << "size" << chain[i].size;
    }
#endif
}

Q_GUI_EXPORT QSize qSmartMinSize(const QWidgetItem *i)
{
    QWidget *w = ((QWidgetItem *)i)->widget();

    QSize s(0, 0);
    QSize msh(w->minimumSizeHint());
    QSize sh(w->sizeHint());

    if (w->sizePolicy().horizontalPolicy() != QSizePolicy::Ignored) {
        if (w->sizePolicy().horizontalPolicy() & QSizePolicy::ShrinkFlag)
            s.setWidth(msh.width());
        else
            s.setWidth(qMax(sh.width(), msh.width()));
    }

    if (w->sizePolicy().verticalPolicy() != QSizePolicy::Ignored) {
        if (w->sizePolicy().verticalPolicy() & QSizePolicy::ShrinkFlag) {
            s.setHeight(msh.height());
        } else {
            s.setHeight(qMax(sh.height(), msh.height()));
        }
    }

    s = s.boundedTo(w->maximumSize());
    QSize min = w->minimumSize();
    if (min.width() > 0)
        s.setWidth(min.width());
    if (min.height() > 0)
        s.setHeight(min.height());

    return s.expandedTo(QSize(0,0));
}

Q_GUI_EXPORT QSize qSmartMinSize(const QWidget *w)
{
    QWidgetItem item(const_cast<QWidget *>(w));
    return qSmartMinSize(&item);
}

Q_GUI_EXPORT QSize qSmartMaxSize(const QWidgetItem *i, Qt::Alignment align)
{
    QWidget *w = ((QWidgetItem*)i)->widget();
    if (align & Qt::AlignHorizontal_Mask && align & Qt::AlignVertical_Mask)
        return QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX);
    QSize s = w->maximumSize();
    if (s.width() == QWIDGETSIZE_MAX && !(align & Qt::AlignHorizontal_Mask))
        if (!(w->sizePolicy().horizontalPolicy() & QSizePolicy::GrowFlag))
            s.setWidth(w->sizeHint().width());

    if (s.height() == QWIDGETSIZE_MAX && !(align & Qt::AlignVertical_Mask))
        if (!(w->sizePolicy().verticalPolicy() & QSizePolicy::GrowFlag))
            s.setHeight(w->sizeHint().height());

    s = s.expandedTo(w->minimumSize());

    if (align & Qt::AlignHorizontal_Mask)
        s.setWidth(QLAYOUTSIZE_MAX);
    if (align & Qt::AlignVertical_Mask)
        s.setHeight(QLAYOUTSIZE_MAX);
    return s;
}

Q_GUI_EXPORT QSize qSmartMaxSize(const QWidget *w, Qt::Alignment align)
{
    QWidgetItem item(const_cast<QWidget *>(w));
    return qSmartMaxSize(&item, align);
}

