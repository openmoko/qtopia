/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qregion.h"
#include "qpolygon.h"
#include "qbuffer.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qt_windows.h"

static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}


struct QRegionPrivate {
    int numRects;
    QVector<QRect> rects;
    QRect extents;

    QRegionPrivate() { numRects = 0; }
    QRegionPrivate(const QRect &r) : rects(1) {
        numRects = 1;
        rects[0] = r;
        extents = r;
    }

    QRegionPrivate(const QRegionPrivate &r) {
        rects = r.rects.copy();
        numRects = r.numRects;
        extents = r.extents;
    }

    QRegionPrivate &operator=(const QRegionPrivate &r) {
        rects = r.rects.copy();
        numRects = r.numRects;
        extents = r.extents;
        return *this;
    }
};
static QRegionPrivate *PolygonRegion(QPoint *Pts, int Count, int rule);
#define EvenOddRule        0
#define WindingRule        1


QRegion::QRegion()
{
    if (!empty_region) {                        // avoid too many allocs
        qAddPostRoutine(cleanup_empty_region);
        empty_region = new QRegion(true);
    }
    data = empty_region->data;
    data->ref();
}

QRegion::QRegion(bool is_null)
{
    data = new QRegionData;
    data->rgn = 0;
    data->is_null = is_null;
}

QRegion::QRegion(const QRect &r, RegionType t)
{
    if (r.isEmpty()) {
        if (!empty_region) {                        // avoid too many allocs
            qAddPostRoutine(cleanup_empty_region);
            empty_region = new QRegion(true);
        }
        data = empty_region->data;
        data->ref();
    } else {
        data = new QRegionData;
        data->is_null = false;
        if (t == Rectangle) {                        // rectangular region
            data->rgn = CreateRectRgn(r.left(),     r.top(),
                                       r.right()+1, r.bottom()+1);
        } else if (t == Ellipse) {                // elliptic region
            QPolygon a;
            a.makeEllipse(r.x(), r.y(), r.width(), r.height());
            QRegionPrivate *rp = PolygonRegion((QPoint*)a.data(), a.size(),
                                                EvenOddRule);
            setRects(rp->rects.data(), rp->rects.count());
            delete rp;
        }
    }
}

QRegion::QRegion(const QPolygon &a, bool winding)
{
    if (a.size() > 0) {
        data = new QRegionData;
        data->is_null = false;
        QRegionPrivate *rp = PolygonRegion((QPoint*)a.data(), a.size(),
                                            winding ? WindingRule : EvenOddRule);
        setRects(rp->rects.data(), rp->rects.count());
        delete rp;
    } else {
        if (!empty_region) {                        // avoid too many allocs
            qAddPostRoutine(cleanup_empty_region);
            empty_region = new QRegion(true);
        }
        data = empty_region->data;
        data->ref();
    }
}

QRegion::QRegion(const QRegion &r)
{
    data = r.data;
    data->ref();
}

HRGN qt_win_bitmapToRegion(const QBitmap& bitmap)
{
    HRGN region=0;
    QImage image = bitmap.convertToImage();

    const int maxrect=1;
    struct RData {
        RGNDATAHEADER header;
        RECT rect[maxrect];
    };
    RData data;

// Windows CE doesn't have ExtCreateRegion, so we need
// to add each span immediately.
#define FlushSpans
#define AddSpan \
        { \
            HRGN r = CreateRectRgn(prev1, y, x, y+1); \
            if (region) { \
                CombineRgn(region, region, r, RGN_OR); \
                DeleteObject(r); \
            } else { \
                region = r; \
            } \
        }

    data.header.rcBound.top = 0;
    data.header.rcBound.left = 0;
    data.header.rcBound.right = image.width()-1;

    int x, y,
        n = 0, zero = 0;
    for (y=0; y<image.height(); y++) {
        uchar *line = image.scanLine(y);
        int w = image.width();
        uchar all=zero;
        int prev1 = -1;
        for (x=0; x<w;) {
            uchar byte = line[x/8];
            if (x>w-8 || byte!=all) {
                for (int b=8; b>0 && x<w; b--) {
                    if (!(byte&0x80) == !all) {
                        // More of the same
                    } else {
                        // A change.
                        if (all!=zero) {
                            AddSpan;
                            all = zero;
                        } else {
                            prev1 = x;
                            all = ~zero;
                        }
                    }
                    byte <<= 1;
                    x++;
                }
            } else {
                x+=8;
            }
        }
        if (all != zero) {
            AddSpan;
        }
    }
    if (n) {
        FlushSpans;
    }

    if (!region) {
        // Surely there is some better way.
        region = CreateRectRgn(0,0,1,1);
        CombineRgn(region, region, region, RGN_XOR);
    }

    return region;
}


QRegion::QRegion(const QBitmap & bm)
{
    if (bm.isNull()) {
        if (!empty_region) {                        // avoid too many allocs
            qAddPostRoutine(cleanup_empty_region);
            empty_region = new QRegion(true);
        }
        data = empty_region->data;
        data->ref();
    } else {
        data = new QRegionData;
        data->is_null = false;
        data->rgn = qt_win_bitmapToRegion(bm);
    }
}


QRegion::~QRegion()
{
    if (data->deref()) {
        if (data->rgn)
            DeleteObject(data->rgn);
        delete data;
    }
}

QRegion &QRegion::operator=(const QRegion &r)
{
    r.data->ref();                                // beware of r = r
    if (data->deref()) {
        if (data->rgn)
            DeleteObject(data->rgn);
        delete data;
    }
    data = r.data;
    return *this;
}


QRegion QRegion::copy() const
{
    QRegion r(data->is_null);
    if (data->rgn) {
        r.data->rgn = CreateRectRgn(0, 0, 2, 2);
        CombineRgn(r.data->rgn, data->rgn, 0, RGN_COPY);
    }
    return r;
}


bool QRegion::isNull() const
{
    return data->is_null;
}

bool QRegion::isEmpty() const
{
    return data->is_null || data->rgn == 0;
}


bool QRegion::contains(const QPoint &p) const
{
    return data->rgn ? PtInRegion(data->rgn, p.x(), p.y()) : false;
}

bool QRegion::contains(const QRect &r) const
{
    if (!data->rgn)
        return false;
    RECT rect;
    SetRect(&rect, r.left(), r.top(), r.right(), r.bottom());
    return RectInRegion(data->rgn, &rect);
}


void QRegion::translate(int dx, int dy)
{
    if (!data->rgn)
        return;
    detach();
    OffsetRgn(data->rgn, dx, dy);
}


#define RGN_NOP -1

// duplicates of those in qregion.cpp
#define QRGN_OR               6
#define QRGN_AND              7
#define QRGN_SUB              8
#define QRGN_XOR              9

/*
  Performs the actual OR, AND, SUB and XOR operation between regions.
  Sets the resulting region handle to 0 to indicate an empty region.
*/

QRegion QRegion::winCombine(const QRegion &r, int op) const
{
    int both=RGN_NOP, left=RGN_NOP, right=RGN_NOP;
    switch (op) {
        case QRGN_OR:
            both = RGN_OR;
            left = right = RGN_COPY;
            break;
        case QRGN_AND:
            both = RGN_AND;
            break;
        case QRGN_SUB:
            both = RGN_DIFF;
            left = RGN_COPY;
            break;
        case QRGN_XOR:
            both = RGN_XOR;
            left = right = RGN_COPY;
            break;
        default:
            qWarning("QRegion: Internal error in winCombine");
    }

    QRegion result(false);
    int allCombineRgnResults = NULLREGION;
    result.data->rgn = CreateRectRgn(0, 0, 0, 0);
    if (data->rgn && r.data->rgn)
        allCombineRgnResults = CombineRgn(result.data->rgn, data->rgn, r.data->rgn, both);
    else if (data->rgn && left != RGN_NOP)
        allCombineRgnResults = CombineRgn(result.data->rgn, data->rgn, data->rgn, left); // 3rd param not used, but must be non-0
    else if (r.data->rgn && right != RGN_NOP)
        allCombineRgnResults = CombineRgn(result.data->rgn, r.data->rgn, r.data->rgn, right); // 3rd param not used, but must be non-0
    if (allCombineRgnResults == NULLREGION || allCombineRgnResults == ERROR)
        result.data->is_null = true;

    //##### do not delete this. A null pointer is different from an empty region in SelectClipRgn in qpainter_win! (M)
//     if (allCombineRgnResults == NULLREGION) {
//         if (result.data->rgn)
//             DeleteObject(result.data->rgn);
//         result.data->rgn = 0;                        // empty region
//     }
    return result;
}

QRegion QRegion::unite(const QRegion &r) const
{
    return winCombine(r, QRGN_OR);
}

QRegion QRegion::intersect(const QRegion &r) const
{
     return winCombine(r, QRGN_AND);
}

QRegion QRegion::subtract(const QRegion &r) const
{
    return winCombine(r, QRGN_SUB);
}

QRegion QRegion::eor(const QRegion &r) const
{
    return winCombine(r, QRGN_XOR);
}


QRect QRegion::boundingRect() const
{
    RECT r;
    int result = data->rgn ? GetRgnBox(data->rgn, &r) : 0;
    if (result == 0 || result == NULLREGION)
        return QRect(0,0,0,0);
    else
        return QRect(r.left, r.top, r.right-r.left, r.bottom-r.top);
}


QVector<QRect> QRegion::rects() const
{
    QVector<QRect> a;
    if (data->rgn == 0)
        return a;

    int numBytes = data->rgn ? GetRegionData(data->rgn, 0, 0) : 0;
    if (numBytes == 0)
        return a;

    char *buf = new char[numBytes];
    if (buf == 0)
        return a;

    RGNDATA *rd = (RGNDATA*)buf;
    if (GetRegionData(data->rgn, numBytes, rd) == 0) {
        delete [] buf;
        return a;
    }

    a = QVector<QRect>(rd->rdh.nCount);
    RECT *r = (RECT*)rd->Buffer;
    for (int i=0; i<(int)a.size(); i++) {
        a[i].setCoords(r->left, r->top, r->right-1, r->bottom-1);
        r++;
    }

    delete [] buf;

    return a;
}

void QRegion::setRects(const QRect *rects, int num)
{
    // Could be optimized
    *this = QRegion();
    for (int i=0; i<num; i++)
        *this |= rects[i];
}


bool QRegion::operator==(const QRegion &r) const
{
    if (data == r.data)                        // share the same data
        return true;
    if ((data->rgn == 0) ^ (r.data->rgn == 0)) // one is empty, not both
        return false;
    return data->rgn == 0 ?
        true :                                        // both empty
        EqualRgn(data->rgn, r.data->rgn);        // both non-empty
}




// Windows CE special region handling --------------------------------


// inline QRect::setCoords
inline void qt_setCoords(QRect *r, int xp1, int yp1, int xp2, int yp2)
{
    r->x1 = xp1;
    r->y1 = yp1;
    r->x2 = xp2;
    r->y2 = yp2;
}


#define RectangleOut        0
#define RectangleIn        1
#define RectanglePart        2

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200

/*
 * used to allocate buffers for points and link
 * the buffers together
 */
typedef struct _POINTBLOCK {
    QPoint pts[NUMPTSTOBUFFER];
    struct _POINTBLOCK *next;
} POINTBLOCK;


// START OF poly.h extract
/* $XConsortium: poly.h,v 1.4 94/04/17 20:22:19 rws Exp $ */
/************************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

/*
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape,
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */


/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}


/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct {
    int minor_axis;        /* minor axis        */
    int d;                /* decision variable */
    int m, m1;                /* slope and slope+1 */
    int incr1, incr2;        /* error increments */
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
        BRESINITPGON(dmaj, min1, min2, bres.minor_axis, bres.d, \
                     bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
        BRESINCRPGON(bres.d, bres.minor_axis, bres.m, bres.m1, bres.incr1, bres.incr2)



/*
 *     These are the data structures needed to scan
 *     convert regions.  Two different scan conversion
 *     methods are available -- the even-odd method, and
 *     the winding number method.
 *     The even-odd rule states that a point is inside
 *     the polygon if a ray drawn from that point in any
 *     direction will pass through an odd number of
 *     path segments.
 *     By the winding number rule, a point is decided
 *     to be inside the polygon if a ray drawn from that
 *     point in any direction passes through a different
 *     number of clockwise and counter-clockwise path
 *     segments.
 *
 *     These data structures are adapted somewhat from
 *     the algorithm in (Foley/Van Dam) for scan converting
 *     polygons.
 *     The basic algorithm is to start at the top (smallest y)
 *     of the polygon, stepping down to the bottom of
 *     the polygon by incrementing the y coordinate.  We
 *     keep a list of edges which the current scanline crosses,
 *     sorted by x.  This list is called the Active Edge Table (AET)
 *     As we change the y-coordinate, we update each entry in
 *     in the active edge table to reflect the edges new xcoord.
 *     This list must be sorted at each scanline in case
 *     two edges intersect.
 *     We also keep a data structure known as the Edge Table (ET),
 *     which keeps track of all the edges which the current
 *     scanline has not yet reached.  The ET is basically a
 *     list of ScanLineList structures containing a list of
 *     edges which are entered at a given scanline.  There is one
 *     ScanLineList per scanline at which an edge is entered.
 *     When we enter a new edge, we move it from the ET to the AET.
 *
 *     From the AET, we can implement the even-odd rule as in
 *     (Foley/Van Dam).
 *     The winding number rule is a little trickier.  We also
 *     keep the EdgeTableEntries in the AET linked by the
 *     nextWETE (winding EdgeTableEntry) link.  This allows
 *     the edges to be linked just as before for updating
 *     purposes, but only uses the edges linked by the nextWETE
 *     link as edges representing spans of the polygon to
 *     drawn (as with the even-odd rule).
 */

/*
 * for the winding number rule
 */
#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1

typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;


/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = 1; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres) \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres) \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}
// END OF poly.h extract




// START OF PolyReg.c extract
/* $XConsortium: PolyReg.c,v 11.23 94/11/17 21:59:37 converse Exp $ */
/************************************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/
/* $XFree86: xc/lib/X11/PolyReg.c,v 1.1.1.2.8.2 1998/10/04 15:22:49 hohndel Exp $ */

#define LARGE_COORDINATE 1000000
#define SMALL_COORDINATE -LARGE_COORDINATE


/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */

static void
InsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE, int scanline,
                ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    register EdgeTableEntry *start, *prev;
    register ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock =
                  (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor_axis < ETE->bres.minor_axis))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
}


/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons.
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure,
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

static void
CreateETandAET(register int count, register QPoint *pts,
        EdgeTable *ET, EdgeTableEntry *AET, register EdgeTableEntry *pETEs,
        ScanLineListBlock   *pSLLBlock)
{
    register QPoint *top, *bottom;
    register QPoint *PrevPt, *CurrPt;
    int iSLLBlock = 0;
    int dy;

    if (count < 2)  return;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor_axis = SMALL_COORDINATE;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = SMALL_COORDINATE;
    ET->ymin = LARGE_COORDINATE;
    pSLLBlock->next = (ScanLineListBlock *)NULL;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--)
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y() > CurrPt->y())
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if (bottom->y() != top->y())
        {
            pETEs->ymax = bottom->y()-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y() - top->y();
            BRESINITPGONSTRUCT(dy, top->x(), bottom->x(), pETEs->bres)

            InsertEdgeInET(ET, pETEs, top->y(), &pSLLBlock, &iSLLBlock);

            if (PrevPt->y() > ET->ymax)
                ET->ymax = PrevPt->y();
            if (PrevPt->y() < ET->ymin)
                ET->ymin = PrevPt->y();
            pETEs++;
        }

        PrevPt = CurrPt;
    }
}


/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table,
 *     leaving them sorted by smaller x coordinate.
 *
 */

static void
loadAET(register EdgeTableEntry *AET, register EdgeTableEntry *ETEs)
{
    register EdgeTableEntry *pPrevAET;
    register EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor_axis < ETEs->bres.minor_axis))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}


/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    |
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */

static void
computeWAET(register EdgeTableEntry *AET)
{
    register EdgeTableEntry *pWETE;
    register int inside = 1;
    register int isInside = 0;

    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            (inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}


/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

static int
InsertionSort(register EdgeTableEntry *AET)
{
    register EdgeTableEntry *pETEchase;
    register EdgeTableEntry *pETEinsert;
    register EdgeTableEntry *pETEchaseBackTMP;
    register int changed = 0;

    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor_axis > AET->bres.minor_axis)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return changed;
}


/*
 *     Clean up our act.
 */

static void
FreeStorage(register ScanLineListBlock   *pSLLBlock)
{
    register ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock)
    {
        tmpSLLBlock = pSLLBlock->next;
        free((char *)pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}


/*
 *     Create an array of rectangles from a list of points.
 *     If indeed these things (POINTS, RECTS) are the same,
 *     then this proc is still needed, because it allocates
 *     storage for the array, which was allocated on the
 *     stack by the calling procedure.
 *
 */

static int PtsToRegion(register int numFullPtBlocks, register int iCurPtBlock,
                        POINTBLOCK *FirstPtBlock, QRegionPrivate *reg)
{
    register QRect  *rects;
    register QPoint *pts;
    register POINTBLOCK *CurPtBlock;
    register int i;
    register QRect *extents;
    register int numRects;

    extents = &reg->extents;

    numRects = ((numFullPtBlocks * NUMPTSTOBUFFER) + iCurPtBlock) >> 1;

    reg->rects.resize(numRects);

    CurPtBlock = FirstPtBlock;
    rects = reg->rects.data() - 1;
    numRects = 0;
    extents->setLeft(INT_MAX);
    extents->setRight(INT_MIN);

    for (; numFullPtBlocks >= 0; numFullPtBlocks--) {
        /* the loop uses 2 points per iteration */
        i = NUMPTSTOBUFFER >> 1;
        if (!numFullPtBlocks)
            i = iCurPtBlock >> 1;
        for (pts = CurPtBlock->pts; i--; pts += 2) {
            if (pts->x() > pts[1].x())
                continue;
            if (numRects && pts->x() == rects->left() && pts->y() == rects->bottom() &&
                pts[1].x() == rects->right() &&
                (numRects == 1 || rects[-1].top() != rects->top()) &&
                (i && pts[2].y() > pts[1].y())) {
                rects->setBottom(pts[1].y());
                continue;
            }
            numRects++;
            rects++;
            qt_setCoords(rects, pts->x(), pts->y(), pts[1].x(), pts[1].y());
            if (rects->left() < extents->left())
                extents->setLeft(rects->left());
            if (rects->right() > extents->right())
                extents->setRight(rects->right());
        }
        CurPtBlock = CurPtBlock->next;
    }

    if (numRects) {
        extents->setTop(reg->rects[0].top());
        extents->setBottom(rects->bottom());
    } else {
        qt_setCoords(extents, 0, 0, 0, 0);
    }
    reg->numRects = numRects;

    return true;
}


/*
 *     polytoregion
 *
 *     Scan converts a polygon by returning a run-length
 *     encoding of the resultant bitmap -- the run-length
 *     encoding is in the form of an array of rectangles.
 */

static QRegionPrivate *PolygonRegion(QPoint *Pts, int Count, int rule)
                                    //Point *Pts;   - the pts
                                    //int    Count; - number of pts
                                    //int    rule;  - winding rule
{
    QRegionPrivate *region;
    register EdgeTableEntry *pAET;   /* Active Edge Table       */
    register int y;                  /* current scanline        */
    register int iPts = 0;           /* number of pts in buffer */
    register EdgeTableEntry *pWETE;  /* Winding Edge Table Entry*/
    register ScanLineList *pSLL;     /* current scanLineList    */
    register QPoint *pts;             /* output buffer           */
    EdgeTableEntry *pPrevAET;        /* ptr to previous AET     */
    EdgeTable ET;                    /* header node for ET      */
    EdgeTableEntry AET;              /* header node for AET     */
    EdgeTableEntry *pETEs;           /* EdgeTableEntries pool   */
    ScanLineListBlock SLLBlock;      /* header for scanlinelist */
    int fixWAET = false;
    POINTBLOCK FirstPtBlock, *curPtBlock; /* PtBlock buffers    */
    POINTBLOCK *tmpPtBlock;
    int numFullPtBlocks = 0;

    if (!(region = new QRegionPrivate))
        return 0;

    /* special case a rectangle */
    pts = Pts;
    if (((Count == 4) ||
         ((Count == 5) && (pts[4].x() == pts[0].x()) && (pts[4].y() == pts[0].y()))) &&
        (((pts[0].y() == pts[1].y()) &&
          (pts[1].x() == pts[2].x()) &&
          (pts[2].y() == pts[3].y()) &&
          (pts[3].x() == pts[0].x())) ||
         ((pts[0].x() == pts[1].x()) &&
          (pts[1].y() == pts[2].y()) &&
          (pts[2].x() == pts[3].x()) &&
          (pts[3].y() == pts[0].y())))) {
        region->extents.setLeft(qMin(pts[0].x(), pts[2].x()));
        region->extents.setTop(qMin(pts[0].y(), pts[2].y()));
        region->extents.setRight(qMax(pts[0].x(), pts[2].x()));
        region->extents.setBottom(qMax(pts[0].y(), pts[2].y()));
        if ((region->extents.left() <= region->extents.right()) &&
            (region->extents.top() <= region->extents.bottom())) {
            region->numRects = 1;
            region->rects.resize(1);
            region->rects[0] = region->extents;
        }
        return region;
    }

    if (! (pETEs = (EdgeTableEntry *)
           malloc((unsigned) (sizeof(EdgeTableEntry) * Count))))
        return 0;

    pts = FirstPtBlock.pts;
    CreateETandAET(Count, Pts, &ET, &AET, pETEs, &SLLBlock);
    pSLL = ET.scanlines.next;
    curPtBlock = &FirstPtBlock;

    if (rule == EvenOddRule) {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++) {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL != NULL && y == pSLL->scanline) {
                loadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;

            /*
             *  for each active edge
             */
            while (pAET) {
                pts->setX(pAET->bres.minor_axis), pts->setY(y);
                pts++, iPts++;

                /*
                 *  send out the buffer
                 */
                if (iPts == NUMPTSTOBUFFER) {
                    tmpPtBlock = (POINTBLOCK *)malloc(sizeof(POINTBLOCK));
                    curPtBlock->next = tmpPtBlock;
                    curPtBlock = tmpPtBlock;
                    pts = curPtBlock->pts;
                    numFullPtBlocks++;
                    iPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
            }
            (void) InsertionSort(&AET);
        }
    }
    else {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++) {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL != NULL && y == pSLL->scanline) {
                loadAET(&AET, pSLL->edgelist);
                computeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;

            /*
             *  for each active edge
             */
            while (pAET) {
                /*
                 *  add to the buffer only those edges that
                 *  are in the Winding active edge table.
                 */
                if (pWETE == pAET) {
                    pts->setX(pAET->bres.minor_axis), pts->setY(y);
                    pts++, iPts++;

                    /*
                     *  send out the buffer
                     */
                    if (iPts == NUMPTSTOBUFFER) {
                        tmpPtBlock = (POINTBLOCK *)malloc(sizeof(POINTBLOCK));
                        curPtBlock->next = tmpPtBlock;
                        curPtBlock = tmpPtBlock;
                        pts = curPtBlock->pts;
                        numFullPtBlocks++;    iPts = 0;
                    }
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET)
            }

            /*
             *  recompute the winding active edge table if
             *  we just resorted or have exited an edge.
             */
            if (InsertionSort(&AET) || fixWAET) {
                computeWAET(&AET);
                fixWAET = false;
            }
        }
    }
    FreeStorage(SLLBlock.next);
    (void) PtsToRegion(numFullPtBlocks, iPts, &FirstPtBlock, region);
    for (curPtBlock = FirstPtBlock.next; --numFullPtBlocks >= 0;) {
        tmpPtBlock = curPtBlock->next;
        free((char *)curPtBlock);
        curPtBlock = tmpPtBlock;
    }
    free((char *)pETEs);
    return region;
}
// END OF PolyReg.c extract
