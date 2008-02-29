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

#include "qttessellator_p.h"

#include <QPointF>
#include <QVector>
#include <QList>
#include <QVariant>
#include <QVarLengthArray>

#include "private/qmath_p.h"
#include "private/qnumeric_p.h"

/*
 * Polygon tesselator - can probably be optimized a bit more
 */

//#define QT_DEBUG_TESSELATOR
#define FloatToXFixed(i) (int)((i) * 65536)
#define IntToXFixed(i) ((i) << 16)

Q_DECLARE_TYPEINFO(qt_XTrapezoid, Q_PRIMITIVE_TYPE);

// used by the edge point sort algorithm
static qreal currentY = 0.f;

struct QEdge {
    qt_XPointFixed p1, p2;
    qreal m;
    qreal b;
    signed char winding;
    qreal intersection;
};

Q_DECLARE_TYPEINFO(QEdge, Q_PRIMITIVE_TYPE);

static inline bool compareEdges(const QEdge *e1, const QEdge *e2)
{
    return e1->p1.y < e2->p1.y;
}

static inline bool isEqual(const qt_XPointFixed &p1, const qt_XPointFixed &p2)
{
    return ((p1.x == p2.x) && (p1.y == p2.y));
}

struct QIntersectionPoint {
    qreal x;
    const QEdge *edge;
};
Q_DECLARE_TYPEINFO(QIntersectionPoint, Q_PRIMITIVE_TYPE);

static inline bool compareIntersections(const QEdge *i1, const QEdge *i2)
{
    if (qAbs(i1->intersection - i2->intersection) > 0.01) { // x != other.x in 99% of the cases
        return i1->intersection < i2->intersection;
    } else {
        qreal x1 = !qIsFinite(i1->b) ? qt_XFixedToDouble(i1->p1.x) :
                   (currentY+1.f - i1->b)*i1->m;
        qreal x2 = !qIsFinite(i2->b) ? qt_XFixedToDouble(i2->p1.x) :
                   (currentY+1.f - i2->b)*i2->m;
        return x1 < x2;
    }
}

#define qrealToXFixed FloatToXFixed

static qt_XTrapezoid QT_FASTCALL toXTrapezoid(qt_XFixed y1, qt_XFixed y2, const QEdge &left, const QEdge &right)
{
    qt_XTrapezoid trap;
    trap.top = y1;
    trap.bottom = y2;
    trap.left.p1.y = left.p1.y;
    trap.left.p2.y = left.p2.y;
    trap.right.p1.y = right.p1.y;
    trap.right.p2.y = right.p2.y;
    trap.left.p1.x = left.p1.x;
    trap.left.p2.x = left.p2.x;
    trap.right.p1.x = right.p1.x;
    trap.right.p2.x = right.p2.x;
    return trap;
}

#ifdef QT_DEBUG_TESSELATOR
static QPointF xf_to_qt(qt_XPointFixed pt)
{
    return QPointF(qt_XFixedToDouble(pt.x), qt_XFixedToDouble(pt.y));
}

static void dump_edges(const QList<const QEdge *> &et)
{
    for (int x = 0; x < et.size(); ++x) {
        qDebug() << "edge#" << x << xf_to_qt(et.at(x)->p1) << xf_to_qt(et.at(x)->p2) << "b: " << et.at(x)->b << "m:" << et.at(x)->m << et.at(x);
    }
}

static void dump_trap(const qt_XTrapezoid &t)
{
    qDebug() << "trap# t=" << qt_XFixedToDouble(t.top) << "b=" << qt_XFixedToDouble(t.bottom)  << "h="
             << qt_XFixedToDouble(t.bottom - t.top) << "\tleft p1: ("
             << qt_XFixedToDouble(t.left.p1.x) << ","<< qt_XFixedToDouble(t.left.p1.y)
             << ")" << "\tleft p2: (" << qt_XFixedToDouble(t.left.p2.x) << ","
             << qt_XFixedToDouble(t.left.p2.y) << ")" << "\n\t\t\t\tright p1:("
             << qt_XFixedToDouble(t.right.p1.x) << "," << qt_XFixedToDouble(t.right.p1.y) << ")"
             << "\tright p2:(" << qt_XFixedToDouble(t.right.p2.x) << ","
             << qt_XFixedToDouble(t.right.p2.y) << ")";
}
#endif

void qt_polygon_trapezoidation(QVector<qt_XTrapezoid> *traps,
                               const QPointF *pg, int pgSize,
                               bool winding, QRect *br)
{
      QVector<QEdge> edges;
    edges.reserve(128);
    qreal ymin(INT_MAX/256);
    qreal ymax(INT_MIN/256);
    qreal xmin(INT_MAX/256);
    qreal xmax(INT_MIN/256);

    Q_ASSERT(pg[0] == pg[pgSize-1]);
    // generate edge table
    for (int x = 0; x < pgSize-1; ++x) {
	QEdge edge;
	edge.winding = pg[x].y() > pg[x+1].y() ? 1 : -1;
        QPointF p1, p2;
	if (edge.winding > 0) {
	    p1 = pg[x+1];
	    p2 = pg[x];
	} else {
	    p1 = pg[x];
	    p2 = pg[x+1];
	}
        edge.p1.x = qt_XDoubleToFixed(p1.x());
        edge.p1.y = qt_XDoubleToFixed(p1.y());
        edge.p2.x = qt_XDoubleToFixed(p2.x());
        edge.p2.y = qt_XDoubleToFixed(p2.y());

	edge.m = (p1.y() - p2.y()) / (p1.x() - p2.x()); // line derivative
	edge.b = p1.y() - edge.m * p1.x(); // intersection with y axis
	edge.m = edge.m != 0.0 ? 1.0 / edge.m : 0.0; // inverted derivative
	edges.append(edge);
        xmin = qMin(xmin, qt_XFixedToDouble(edge.p1.x));
        xmax = qMax(xmax, qt_XFixedToDouble(edge.p2.x));
        ymin = qMin(ymin, qt_XFixedToDouble(edge.p1.y));
        ymax = qMax(ymax, qt_XFixedToDouble(edge.p2.y));
    }
    br->setX(qRound(xmin));
    br->setY(qRound(ymin));
    br->setWidth(qRound(xmax - xmin));
    br->setHeight(qRound(ymax - ymin));

    QList<const QEdge *> et; 	    // edge list
    for (int i = 0; i < edges.size(); ++i)
        et.append(&edges.at(i));

    // sort edge table by min y value
    qSort(et.begin(), et.end(), compareEdges);

    // eliminate shared edges
    for (int i = 0; i < et.size(); ++i) {
	for (int k = i+1; k < et.size(); ++k) {
            const QEdge *edgeI = et.at(i);
            const QEdge *edgeK = et.at(k);
            if (edgeK->p1.y > edgeI->p1.y)
                break;
   	    if (edgeI->winding != edgeK->winding &&
                isEqual(edgeI->p1, edgeK->p1) && isEqual(edgeI->p2, edgeK->p2)
		) {
 		et.removeAt(k);
		et.removeAt(i);
		--i;
		break;
	    }
	}
    }

    if (ymax <= ymin)
	return;
    QList<const QEdge *> aet; 	    // edges that intersects the current scanline

    currentY = ymin; // used by the less than op
    QMap<qreal, qreal> segInters;
    for (qreal y = ymin; y < ymax;) {
        qt_XFixed yf = qt_XDoubleToFixed(y);
        
	// fill active edge table with edges that intersect the current line
	for (int i = 0; i < et.size(); ++i) {
            const QEdge *edge = et.at(i);
            if (edge->p1.y > yf)
                break;
            aet.append(edge);
            et.removeAt(i);
            --i;
	}

	// remove processed edges from active edge table
	for (int i = 0; i < aet.size(); ++i) {
	    if (aet.at(i)->p2.y <= yf) {
		aet.removeAt(i);
 		--i;
	    }
	}
        if (aet.size()%2 != 0) {
            qWarning("QX11PaintEngine: aet out of sync - this should not happen.");
            return;
        }

	// done?
	if (!aet.size()) {
            if (!et.size()) {
                break;
	    } else {
 		y = qt_XFixedToDouble(et.at(0)->p1.y);
                continue;
	    }
        }

        // calculate the next y where we have to start a new set of trapezoids
	qreal next_y(INT_MAX/256);
 	for (int i = 0; i < aet.size(); ++i) {
            const QEdge *edge = aet.at(i);
 	    if (qt_XFixedToDouble(edge->p2.y) < next_y)
 		next_y = qt_XFixedToDouble(edge->p2.y);
        }

	if (et.size() && next_y > qt_XFixedToDouble(et.at(0)->p1.y))
	    next_y = qt_XFixedToDouble(et.at(0)->p1.y);

	// calc intersection points
 	for (int i = 0; i < aet.size(); ++i) {
            QEdge *edge = const_cast<QEdge*>(aet.at(i));
            edge->intersection = (edge->p1.x != edge->p2.x) ?
                                 ((y - edge->b)*edge->m) : qt_XFixedToDouble(edge->p1.x);
	}
        qSort(aet.begin(), aet.end(), compareIntersections);
        
        int aetSize = aet.size();
	for (int i = 0; i < aetSize; ++i) {
            int aetEnd = qMin(aetSize, i+2);
	    for (int k = i+1; k < aetEnd; ++k) {
                const QEdge *edgeI = aet.at(i);
                const QEdge *edgeK = aet.at(k);
		qreal m1 = edgeI->m;
		qreal b1 = edgeI->b;
		qreal m2 = edgeK->m;
		qreal b2 = edgeK->b;

		if (qAbs(m1 - m2) < 0.001)
                    continue;

                // ### intersect is not calculated correctly when optimized with -O2 (gcc)
                volatile qreal intersect = 0;
                if (!qIsFinite(b1))
                    intersect = (1.f / m2) * qt_XFixedToDouble(edgeI->p1.x) + b2;
                else if (!qIsFinite(b2))
                    intersect = (1.f / m1) * qt_XFixedToDouble(edgeK->p1.x) + b1;
                else
                    intersect = (b1*m1 - b2*m2) / (m1 - m2);

                if (intersect > y && intersect < next_y) {
                    next_y = intersect;
                }
	    }
	}

        qt_XFixed next_yf = qrealToXFixed(next_y);

        if (yf == next_yf) {
            y = currentY = next_y;
            continue;
        }

        if (winding) {
            // winding fill rule
            for (int i = 0; i < aet.size();) {
                int winding = 0;
                const QEdge *left = aet[i];
                const QEdge *right = 0;
                winding += aet[i]->winding;
                for (++i; i < aet.size() && winding != 0; ++i) {
                    winding += aet[i]->winding;
                    right = aet[i];
                }
                if (!left || !right)
                    break;
                traps->append(toXTrapezoid(yf, next_yf, *left, *right));
            }
        } else {
            // odd-even fill rule
            for (int i = 0; i < aet.size()-1; i += 2) {
                traps->append(toXTrapezoid(yf, next_yf, *aet[i], *aet[i+1]));
            }
        }
	y = currentY = next_y;
    }
}
