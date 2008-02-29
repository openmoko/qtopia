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

#include "qbezier_p.h"
#include <qdebug.h>
#include <qline.h>
#include <qpolygon.h>

#include <private/qnumeric_p.h>
#include <private/qmath_p.h>
/*!
  \internal
*/
QBezier QBezier::fromPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4)
{
    QBezier b;
    b.x1 = p1.x();
    b.y1 = p1.y();
    b.x2 = p2.x();
    b.y2 = p2.y();
    b.x3 = p3.x();
    b.y3 = p3.y();
    b.x4 = p4.x();
    b.y4 = p4.y();
    return b;
}

/*!
  \internal
*/
QPolygonF QBezier::toPolygon() const
{
    // flattening is done by splitting the bezier until we can replace the segment by a straight
    // line. We split further until the control points are close enough to the line connecting the
    // boundary points.
    //
    // the Distance of a point p from a line given by the points (a,b) is given by:
    //
    // d = abs( (bx - ax)(ay - py) - (by - ay)(ax - px) ) / line_length
    //
    // We can stop splitting if both control points are close enough to the line.
    // To make the algorithm faster we use the manhattan length of the line.

    QPolygonF polygon;
    polygon.append(QPointF(x1, y1));
    addToPolygon(&polygon);
    return polygon;
}

void QBezier::addToPolygon(QPolygonF *polygon) const
{
    QBezier beziers[32];
    beziers[0] = *this;
    QBezier *b = beziers;
    while (b >= beziers) {
        // check if we can pop the top bezier curve from the stack
        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
        qreal d;
        if (l > 1.) {
            d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2) - (b->y4 - b->y1)*(b->x1 - b->x2) )
                + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3) - (b->y4 - b->y1)*(b->x1 - b->x3) );
        } else {
            d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
            l = 1.;
        }
        if (d < .5*l || b == beziers + 31) {
            // good enough, we pop it off and add the endpoint
            polygon->append(QPointF(b->x4, b->y4));
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
            ++b;
        }
    }
}

void QBezier::split(QBezier *firstHalf, QBezier *secondHalf) const
{
    Q_ASSERT(firstHalf);
    Q_ASSERT(secondHalf);

    qreal c = (x2 + x3)/2;
    firstHalf->x2 = (x1 + x2)/2;
    secondHalf->x3 = (x3 + x4)/2;
    firstHalf->x1 = x1;
    secondHalf->x4 = x4;
    firstHalf->x3 = (firstHalf->x2 + c)/2;
    secondHalf->x2 = (secondHalf->x3 + c)/2;
    firstHalf->x4 = secondHalf->x1 = (firstHalf->x3 + secondHalf->x2)/2;

    c = (y2 + y3)/2;
    firstHalf->y2 = (y1 + y2)/2;
    secondHalf->y3 = (y3 + y4)/2;
    firstHalf->y1 = y1;
    secondHalf->y4 = y4;
    firstHalf->y3 = (firstHalf->y2 + c)/2;
    secondHalf->y2 = (secondHalf->y3 + c)/2;
    firstHalf->y4 = secondHalf->y1 = (firstHalf->y3 + secondHalf->y2)/2;
}


QRectF QBezier::bounds() const
{
    qreal xmin = x1;
    qreal xmax = x1;
    if (x2 < xmin)
        xmin = x2;
    else if (x2 > xmax)
        xmax = x2;
    if (x3 < xmin)
        xmin = x3;
    else if (x3 > xmax)
        xmax = x3;
    if (x4 < xmin)
        xmin = x4;
    else if (x4 > xmax)
        xmax = x4;

    qreal ymin = y1;
    qreal ymax = y1;
    if (y2 < ymin)
        ymin = y2;
    else if (y2 > ymax)
        ymax = y2;
    if (y3 < ymin)
        ymin = y3;
    else if (y3 > ymax)
        ymax = y3;
    if (y4 < ymin)
        ymin = y4;
    else if (y4 > ymax)
        ymax = y4;
    return QRectF(xmin, ymin, xmax-xmin, ymax-ymin);
}


enum ShiftResult {
    Ok,
    Discard,
    Split,
    Circle
};

static ShiftResult good_offset(const QBezier *b1, const QBezier *b2, qreal offset, qreal threshold)
{
    const qreal o2 = offset*offset;
    const qreal max_dist_line = threshold*offset*offset;
    const qreal max_dist_normal = threshold*offset;
    const qreal spacing = 0.25;
    for (qreal i = spacing; i < 0.99; i += spacing) {
        QPointF p1 = b1->pointAt(i);
        QPointF p2 = b2->pointAt(i);
        qreal d = (p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y());
        if (qAbs(d - o2) > max_dist_line)
            return Split;

        QPointF normalPoint = b1->normalVector(i);
        qreal l = qAbs(normalPoint.x()) + qAbs(normalPoint.y());
        if (l != 0.) {
            d = qAbs( normalPoint.x()*(p1.y() - p2.y()) - normalPoint.y()*(p1.x() - p2.x()) ) / l;
            if (d > max_dist_normal)
                return Split;
        }
    }
    return Ok;
}

static inline QLineF qline_shifted(const QPointF &p1, const QPointF &p2, qreal offset)
{
    QLineF l(p1, p2);
    QLineF ln = l.normalVector().unitVector();
    l.translate(ln.dx() * offset, ln.dy() * offset);
    return l;
}

static bool qbezier_is_line(QPointF *points, int pointCount)
{
    Q_ASSERT(pointCount > 2);

    qreal dx13 = points[2].x() - points[0].x();
    qreal dy13 = points[2].y() - points[0].y();

    qreal dx12 = points[1].x() - points[0].x();
    qreal dy12 = points[1].y() - points[0].y();

    if (pointCount == 3) {
        if (dx13 * dx12 != 0)
            return qFuzzyCompare(dy12 / dx12, dy13 / dx13);
        else
            return qFuzzyCompare(dx12 / dy12, dx13 / dy13);

    } else if (pointCount == 4) {
        qreal dx14 = points[3].x() - points[0].x();
        qreal dy14 = points[3].y() - points[0].y();

        if (dx14*dx13*dx12 != 0) {
            qreal b14 = dy14 / dx14;
            qreal b13 = dy13 / dx13;
            qreal b12 = dy12 / dx12;
            return qFuzzyCompare(b14, b13) && qFuzzyCompare(b14, b12);

        } else {
            qreal a14 = dx14 / dy14;
            qreal a13 = dx13 / dy13;
            qreal a12 = dx12 / dy12;
            return qFuzzyCompare(a14, a13) && qFuzzyCompare(a14, a12);
        }
    }

    return false;
}

static ShiftResult shift(const QBezier *orig, QBezier *shifted, qreal offset, qreal threshold)
{
    int map[4];
    bool p1_p2_equal = (orig->x1 == orig->x2 && orig->y1 == orig->y2);
    bool p2_p3_equal = (orig->x2 == orig->x3 && orig->y2 == orig->y3);
    bool p3_p4_equal = (orig->x3 == orig->x4 && orig->y3 == orig->y4);

    QPointF points[4];
    int np = 0;
    points[np] = QPointF(orig->x1, orig->y1);
    map[0] = 0;
    ++np;
    if (!p1_p2_equal) {
        points[np] = QPointF(orig->x2, orig->y2);
        ++np;
    }
    map[1] = np - 1;
    if (!p2_p3_equal) {
        points[np] = QPointF(orig->x3, orig->y3);
        ++np;
    }
    map[2] = np - 1;
    if (!p3_p4_equal) {
        points[np] = QPointF(orig->x4, orig->y4);
        ++np;
    }
    map[3] = np - 1;
    if (np == 1)
        return Discard;

    // We need to specialcase lines of 3 or 4 points due to numerical
    // instability in intersections below
    if (np > 2 && qbezier_is_line(points, np)) {
        QLineF l = qline_shifted(points[0], points[np-1], offset);
        *shifted = QBezier::fromPoints(l.p1(), l.pointAt(0.33), l.pointAt(0.66), l.p2());
        return Ok;
    }

    QRectF b = orig->bounds();
    if (np == 4 && b.width() < .1*offset && b.height() < .1*offset) {
        qreal l = (orig->x1 - orig->x2)*(orig->x1 - orig->x2) +
                  (orig->y1 - orig->y2)*(orig->y1 - orig->y1) *
                  (orig->x3 - orig->x4)*(orig->x3 - orig->x4) +
                  (orig->y3 - orig->y4)*(orig->y3 - orig->y4);
        qreal dot = (orig->x1 - orig->x2)*(orig->x3 - orig->x4) +
                    (orig->y1 - orig->y2)*(orig->y3 - orig->y4);
        if (dot < 0 && dot*dot < 0.8*l)
            // the points are close and reverse dirction. Approximate the whole
            // thing by a semi circle
            return Circle;
    }

    QPointF points_shifted[4];

    QLineF prev = QLineF(QPointF(), points[1] - points[0]);
    QPointF prev_normal = prev.normalVector().unitVector().p2();

    points_shifted[0] = points[0] + offset * prev_normal;

    for (int i = 1; i < np - 1; ++i) {
        QLineF next = QLineF(QPointF(), points[i + 1] - points[i]);
        QPointF next_normal = next.normalVector().unitVector().p2();

        QPointF normal_sum = prev_normal + next_normal;

        qreal r = 1.0 + prev_normal.x() * next_normal.x()
                      + prev_normal.y() * next_normal.y();

        if (qFuzzyCompare(r, (qreal)0.0)) {
            points_shifted[i] = points[i] + offset * prev_normal;
        } else {
            qreal k = offset / r;
            points_shifted[i] = points[i] + k * normal_sum;
        }

        prev_normal = next_normal;
    }

    points_shifted[np - 1] = points[np - 1] + offset * prev_normal;

    *shifted = QBezier::fromPoints(points_shifted[map[0]], points_shifted[map[1]],
                                   points_shifted[map[2]], points_shifted[map[3]]);

    return good_offset(orig, shifted, offset, threshold);
}

// This value is used to determine the length of control point vectors
// when approximating arc segments as curves. The factor is multiplied
// with the radius of the circle.
#define KAPPA 0.5522847498


static void addCircle(const QBezier *b, qreal offset, QBezier *o)
{
    QPointF normals[3];

    normals[0] = QPointF(b->y2 - b->y1, b->x1 - b->x2);
    qreal dist = sqrt(normals[0].x()*normals[0].x() + normals[0].y()*normals[0].y());
    if (qFuzzyCompare(dist, 0))
        return;
    normals[0] /= dist;
    normals[2] = QPointF(b->y4 - b->y3, b->x3 - b->x4);
    dist = sqrt(normals[2].x()*normals[2].x() + normals[2].y()*normals[2].y());
    if (qFuzzyCompare(dist, 0))
        return;
    normals[2] /= dist;

    normals[1] = QPointF(b->x1 - b->x2 - b->x3 + b->x4, b->y1 - b->y2 - b->y3 + b->y4);
    normals[1] /= -1*sqrt(normals[1].x()*normals[1].x() + normals[1].y()*normals[1].y());

    qreal angles[2];
    qreal sign = 1.;
    for (int i = 0; i < 2; ++i) {
        qreal cos_a = normals[i].x()*normals[i+1].x() + normals[i].y()*normals[i+1].y();
        if (cos_a > 1.)
            cos_a = 1.;
        if (cos_a < -1.)
            cos_a = -1;
        angles[i] = acos(cos_a)/Q_PI;
    }

    if (angles[0] + angles[1] > 1.) {
        // more than 180 degrees
        normals[1] = -normals[1];
        angles[0] = 1. - angles[0];
        angles[1] = 1. - angles[1];
        sign = -1.;

    }

    QPointF circle[3];
    circle[0] = QPointF(b->x1, b->y1) + normals[0]*offset;
    circle[1] = QPointF(0.5*(b->x1 + b->x4), 0.5*(b->y1 + b->y4)) + normals[1]*offset;
    circle[2] = QPointF(b->x4, b->y4) + normals[2]*offset;

    for (int i = 0; i < 2; ++i) {
        qreal kappa = 2.*KAPPA * sign * offset * angles[i];

        o->x1 = circle[i].x();
        o->y1 = circle[i].y();
        o->x2 = circle[i].x() - normals[i].y()*kappa;
        o->y2 = circle[i].y() + normals[i].x()*kappa;
        o->x3 = circle[i+1].x() + normals[i+1].y()*kappa;
        o->y3 = circle[i+1].y() - normals[i+1].x()*kappa;
        o->x4 = circle[i+1].x();
        o->y4 = circle[i+1].y();

        ++o;
    }
}

int QBezier::shifted(QBezier *curveSegments, int maxSegments, qreal offset, float threshold) const
{
    Q_ASSERT(curveSegments);
    Q_ASSERT(maxSegments > 0);

    if (x1 == x2 && x1 == x3 && x1 == x4 &&
        y1 == y2 && y1 == y3 && y1 == y4)
        return 0;

    --maxSegments;
    QBezier beziers[10];
redo:
    beziers[0] = *this;
    QBezier *b = beziers;
    QBezier *o = curveSegments;

    while (b >= beziers) {
        if (b - beziers == 9 || o - curveSegments == maxSegments) {
            threshold *= 1.5;
            if (threshold > 2.)
                goto give_up;
            goto redo;
        }
        ShiftResult res = shift(b, o, offset, threshold);
        if (res == Discard) {
            --b;
        } else if (res == Ok) {
            ++o;
            --b;
            continue;
        } else if (res == Circle && maxSegments - (o - curveSegments) >= 2) {
            // add semi circle
            addCircle(b, offset, o);
            --b;
            o += 2;
        } else {
            b->split(b+1, b);
            ++b;
        }
    }

give_up:
    while (b >= beziers) {
        shift(b, o, offset, threshold);
        ++o;
        --b;
    }

    Q_ASSERT(o - curveSegments <= maxSegments);
    return o - curveSegments;
}
