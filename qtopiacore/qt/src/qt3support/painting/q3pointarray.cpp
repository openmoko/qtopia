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

#include "q3pointarray.h"
#include "private/qbezier_p.h"
#include "private/qpainterpath_p.h"

/*!
    \class Q3PointArray
    The Q3PointArray class provides an array of points.

    \compat

    Q3PointArray is a QPolygon subclass that provides functions
    to make it more source compatible with the \c QPointArray class
    in Qt 3.

    In Qt 4, we recommend that you use QPainterPath for representing
    arcs, ellipses, and Bezier curves, rather than QPolygon.
*/

/*!
    Sets the points of the array to those describing an arc of an
    ellipse with size, width \a w by height \a h, and position (\a x,
    \a y), starting from angle \a a1 and spanning by angle \a a2. The
    resulting array has sufficient resolution for pixel accuracy (see
    the overloaded function which takes an additional QMatrix
    parameter).

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16*360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero degrees
    is at the 3 o'clock position.
*/
#ifndef QT_NO_WMATRIX
void Q3PointArray::makeArc(int x, int y, int w, int h, int a1, int a2)
{
    QRectF r(x, y, w, h);
    QPointF startPoint;
    qt_find_ellipse_coords(r, a1 / 16.0, a2 / 16.0, &startPoint, 0);

    QPainterPath path(startPoint);
    path.arcTo(r, a1 / 16.0, a2 / 16.0);

    if (path.isEmpty())
        *this = Q3PointArray();
    else
        *this = path.toSubpathPolygons().at(0).toPolygon();
}
#endif

#ifndef QT_NO_TRANSFORMATIONS
/*!
    \overload

    Sets the points of the array to those describing an arc of an
    ellipse with width \a w and height \a h and position (\a x, \a y),
    starting from angle \a a1, and spanning angle by \a a2, and
    transformed by the matrix \a xf. The resulting array has
    sufficient resolution for pixel accuracy.

    Angles are specified in 16ths of a degree, i.e. a full circle
    equals 5760 (16 * 360). Positive values mean counter-clockwise,
    whereas negative values mean the clockwise direction. Zero
    degrees is at the 3 o'clock position.
*/
void Q3PointArray::makeArc(int x, int y, int w, int h, int a1, int a2, const QMatrix &xf)
{
    QRectF r(x, y, w, h);
    QPointF startPoint;
    qt_find_ellipse_coords(r, a1 / 16.0, a2 / 16.0, &startPoint, 0);

    QPainterPath path(startPoint);
    path.arcTo(r, a1 / 16.0, a2 / 16.0);
    path = path * xf;
    if (path.isEmpty())
        *this = Q3PointArray();
    else
        *this = path.toSubpathPolygons().at(0).toPolygon();
}

#endif // QT_NO_TRANSFORMATIONS

/*!
    \fn Q3PointArray::Q3PointArray()

    Constructs an empty Q3PointArray.
*/

/*!
    \fn Q3PointArray::Q3PointArray(const QRect &r, bool closed)

    Constructs a point array from the rectangle \a r.

    If \a closed is false, then the point array just contains the
    following four points of the rectangle ordered clockwise. The
    bottom-right point is located at (r.x() + r.width(), r.y() +
    r.height()).
*/

/*!
    \fn Q3PointArray::Q3PointArray(const QPolygon& other)

    Constructs a copy of \a other.
*/

/*!
    \fn Q3PointArray Q3PointArray::copy() const

    Returns a copy of this Q3PointArray.
*/

/*!
    \fn bool Q3PointArray::isNull()

    Same as isEmpty().
*/

/*!
    Sets the points of the array to those describing an ellipse with
    size, width \a w by height \a h, and position (\a x, \a y).

    The returned array has sufficient resolution for use as pixels.
*/
void Q3PointArray::makeEllipse(int x, int y, int w, int h)
{
    QPainterPath path;
    path.addEllipse(x, y, w, h);
    *this = path.toSubpathPolygons().at(0).toPolygon();
}

#ifndef QT_NO_BEZIER

/*!
    Returns the Bezier points for the four control points in this
    array.
*/
Q3PointArray Q3PointArray::cubicBezier() const
{
    if (size() != 4) {
	qWarning( "Q3PointArray::bezier: The array must have 4 control points" );
        return QPolygon();
    }
    QPolygonF polygon = QBezier::fromPoints(at(0), at(1), at(2), at(3)).toPolygon();
    return polygon.toPolygon();
}
#endif //QT_NO_BEZIER
