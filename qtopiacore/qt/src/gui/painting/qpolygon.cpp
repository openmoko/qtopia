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

#include "qpolygon.h"
#include "qrect.h"
#include "qdatastream.h"
#include "qmatrix.h"
#include "qdebug.h"
#include "qpainterpath.h"
#include "qvariant.h"
#include "qpainterpath_p.h"
#include "qbezier_p.h"

#include <stdarg.h>

/*!
    \class QPolygon
    \brief The QPolygon class provides a vector of points using
    integer precision.

    \reentrant

    \ingroup multimedia
    \ingroup shared

    A QPolygon object is a QVector<QPoint>.  The easiest way to add
    points to a QPolygon is to use QVector's streaming operator, as
    illustrated below:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto STREAM
    \skipto QPolygon
    \printuntil QPoint

    In addition to the functions provided by QVector, QPolygon
    provides some point-specific functions.

    Each point in a polygon can be retrieved by passing its index to
    the point() function. To populate the polygon, QPolygon provides
    the setPoint() function to set the point at a given index, the
    setPoints() function to set all the points in the polygon
    (resizing it to the given number of points), and the putPoints()
    function which copies a number of given points into the polygon
    from a specified index (resizing the polygon if necessary).

    QPolygon provides the boundingRect() and translate() functions for
    geometry functions. Use the QMatrix::map() function for more
    general transformations of QPolygons.

    The QPolygon class is \l {Implicit Data Sharing}{implicitly
    shared}.

    \sa QVector, QPolygonF, QLine
*/


/*****************************************************************************
  QPolygon member functions
 *****************************************************************************/

/*!
    \fn QPolygon::QPolygon()

    Constructs a polygon with no points.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(int size)

    Constructs a polygon of the given \a size. Creates an empty
    polygon if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygon::QPolygon(const QPolygon &polygon)

    Constructs a copy of the given \a polygon.

    \sa setPoints()
*/

/*!
    \fn QPolygon::QPolygon(const QVector<QPoint> &points)

    Constructs a polygon containing the specified \a points.

    \sa setPoints()
*/

/*!
    \fn QPolygon::QPolygon(const QRect &rectangle, bool closed)

    Constructs a polygon from the given \a rectangle.  If \a closed is
    false, the polygon just contains the four points of the rectangle
    ordered clockwise, otherwise the polygon's fifth point is set to
    \a {rectangle}.topLeft().

    Note that the bottom-right corner of the rectangle is located at
    (rectangle.x() + rectangle.width(), rectangle.y() +
    rectangle.height()).

    \sa setPoints()
*/

QPolygon::QPolygon(const QRect &r, bool closed)
{
    reserve(closed ? 5 : 4);
    *this << QPoint(r.x(), r.y())
          << QPoint(r.x() + r.width(), r.y())
          << QPoint(r.x() + r.width(), r.y() + r.height())
          << QPoint(r.x(), r.y() + r.height());
    if (closed)
        *this << QPoint(r.left(), r.top());
}

/*!
    \internal
    Constructs a point array with \a nPoints points, taken from the
    \a points array.

    Equivalent to setPoints(nPoints, points).
*/

QPolygon::QPolygon(int nPoints, const int *points)
{
    setPoints(nPoints, points);
}


/*!
    \fn QPolygon::~QPolygon()

    Destroys the polygon.
*/


/*!
    Translates all points in the polygon by (\a{dx}, \a{dy}).
*/

void QPolygon::translate(int dx, int dy)
{
    register QPoint *p = data();
    register int i = size();
    QPoint pt(dx, dy);
    while (i--) {
        *p += pt;
        ++p;
    }
}

/*!
    \fn void QPolygon::translate(const QPoint &offset)
    \overload

    Translates all points in the polygon by the given \a offset.
*/


/*!
    Extracts the coordinates of the point at the given \a index to
    *\a{x} and *\a{y} (if they are valid pointers).

    \sa setPoint()
*/

void QPolygon::point(int index, int *x, int *y) const
{
    QPoint p = at(index);
    if (x)
        *x = (int)p.x();
    if (y)
        *y = (int)p.y();
}

/*!
    \fn QPoint QPolygon::point(int index) const
    \overload

    Returns the point at the given \a index.
*/

/*!
    \fn void QPolygon::setPoint(int index, const QPoint &point)
    \overload

    Sets the point at the given \a index to the given \a point.
*/

/*!
    \fn void QPolygon::setPoint(int index, int x, int y)

    Sets the point at the given \a index to the point specified by
    (\a{x}, \a{y}).

    \sa point(), putPoints(), setPoints(),
*/

/*!
    Resizes the polygon to \a nPoints and populates it with the given
    \a points.

    The example code creates a polygon with two points (10, 20) and
    (30, 40):

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto SETPOINTS
    \skipto static
    \printuntil setPoints

    \sa setPoint() putPoints()
*/

void QPolygon::setPoints(int nPoints, const int *points)
{
    resize(nPoints);
    int i = 0;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}

/*!
    \overload

    Resizes the polygon to \a nPoints and populates it with the points
    specified by the variable argument list.  The points are given as a
    sequence of integers, starting with \a firstx then \a firsty, and
    so on.

    The example code creates a polygon with two points (10, 20) and
    (30, 40):

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto SETPOINTS2
    \skipto QPolygon
    \printuntil setPoints
*/

void QPolygon::setPoints(int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
    resize(nPoints);
    setPoint(0, firstx, firsty);
    int i = 0, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}

/*!
    \overload
    \internal

    Copies \a nPoints points from the \a points coord array into this
    point array, and resizes the point array if \c{index+nPoints}
    exceeds the size of the array.

    \sa setPoint()
*/

void QPolygon::putPoints(int index, int nPoints, const int *points)
{
    if (index + nPoints > size())
        resize(index + nPoints);
    int i = index;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}

/*!
    Copies \a nPoints points from the variable argument list into this
    polygon from the given \a index.

    The points are given as a sequence of integers, starting with \a
    firstx then \a firsty, and so on. The polygon is resized if
    \c{index+nPoints} exceeds its current size.

    The example code creates a polygon with three points (4,5), (6,7)
    and (8,9), by expanding the polygon from 1 to 3 points:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS
    \skipto QPolygon
    \printuntil putPoints

    The following code has the same result, but here the putPoints()
    function overwrites rather than extends:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS2
    \skipto QPolygon
    \printuntil putPoints(1, 1

    \sa setPoints()
*/

void QPolygon::putPoints(int index, int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
    if (index + nPoints > size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    setPoint(index, firstx, firsty);
    int i = index, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}


/*!
    \fn void QPolygon::putPoints(int index, int nPoints, const QPolygon &fromPolygon, int fromIndex)
    \overload

    Copies \a nPoints points from the given \a fromIndex ( 0 by
    default) in \a fromPolygon into this polygon, starting at the
    specified \a index. For example:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto PUTPOINTS3
    \skipto QPolygon
    \printto }
*/

void QPolygon::putPoints(int index, int nPoints, const QPolygon & from, int fromIndex)
{
    if (index + nPoints > size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    int n = 0;
    while(n < nPoints) {
        setPoint(index + n, from[fromIndex+n]);
        ++n;
    }
}


/*!
    Returns the bounding rectangle of the polygon, or QRect(0, 0, 0,
    0) if the polygon is empty.

    \sa QVector::isEmpty()
*/

QRect QPolygon::boundingRect() const
{
    if (isEmpty())
        return QRect(0, 0, 0, 0);
    register const QPoint *pd = constData();
    int minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    ++pd;
    for (int i = 1; i < size(); ++i) {
        if (pd->x() < minx)
            minx = pd->x();
        else if (pd->x() > maxx)
            maxx = pd->x();
        if (pd->y() < miny)
            miny = pd->y();
        else if (pd->y() > maxy)
            maxy = pd->y();
        ++pd;
    }
    return QRect(QPoint(minx,miny), QPoint(maxx,maxy));
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPolygon &a)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QPolygon(";
    for (int i = 0; i < a.count(); ++i)
        dbg.nospace() << a.at(i);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QPolygon to QDebug");
    return dbg;
    Q_UNUSED(a);
#endif
}
#endif


/*!
    \class QPolygonF
    \brief The QPolygonF class provides a vector of points using
    floating point precision.

    \reentrant
    \ingroup multimedia
    \ingroup shared

    A QPolygonF is a QVector<QPointF>. The easiest way to add points
    to a QPolygonF is to use its streaming operator, as illustrated
    below:

    \quotefromfile snippets/polygon/polygon.cpp
    \skipto STREAMF
    \skipto QPolygonF
    \printuntil QPointF

    In addition to the functions provided by QVector, QPolygonF
    provides the boundingRect() and translate() functions for geometry
    operations. Use the QMatrix::map() function for more general
    transformations of QPolygonFs.

    QPolygonF also provides the isClosed() function to determine
    whether a polygon's start and end points are the same, and the
    toPolygon() function returning an integer precision copy of this
    polygon.

    The QPolygonF class is \l {Implicit Data Sharing}{implicitly
    shared}.

    \sa QVector, QPolygon, QLineF
*/


/*****************************************************************************
  QPolygonF member functions
 *****************************************************************************/

/*!
    \fn QPolygonF::QPolygonF()

    Constructs a polygon with no points.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygonF::QPolygonF(int size)

    Constructs a polygon of the given \a size. Creates an empty
    polygon if \a size == 0.

    \sa QVector::isEmpty()
*/

/*!
    \fn QPolygonF::QPolygonF(const QPolygonF &polygon)

    Constructs a copy of the given \a polygon.
*/

/*!
    \fn QPolygonF::QPolygonF(const QVector<QPointF> &points)

    Constructs a polygon containing the specified \a points.
*/

/*!
    \fn QPolygonF::QPolygonF(const QRectF &rectangle)

    Constructs a closed polygon from the specified \a rectangle.

    The polygon contains the four vertices of the rectangle in
    clockwise order starting and ending with the top-left vertex.

    \sa isClosed()
*/

QPolygonF::QPolygonF(const QRectF &r)
{
    reserve(5);
    append(QPointF(r.x(), r.y()));
    append(QPointF(r.x() + r.width(), r.y()));
    append(QPointF(r.x() + r.width(), r.y() + r.height()));
    append(QPointF(r.x(), r.y() + r.height()));
    append(QPointF(r.x(), r.y()));
}

/*!
    \fn QPolygonF::QPolygonF(const QPolygon &polygon)

    Constructs a float based polygon from the specified integer based
    \a polygon.

    \sa toPolygon()
*/

QPolygonF::QPolygonF(const QPolygon &a)
{
    reserve(a.size());
    for (int i=0; i<a.size(); ++i)
        append(a.at(i));
}

/*!
    \fn QPolygonF::~QPolygonF()

    Destroys the polygon.
*/


/*!
    Translate all points in the polygon by the given \a offset.
*/

void QPolygonF::translate(const QPointF &offset)
{
    register QPointF *p = data();
    register int i = size();
    while (i--) {
        *p += offset;
        ++p;
    }
}

/*!
    \fn void QPolygonF::translate(qreal dx, qreal dy)
    \overload

    Translates all points in the polygon by (\a{dx}, \a{dy}).
*/

/*!
    \fn bool QPolygonF::isClosed() const

    Returns true if the polygon is closed; otherwise returns false.

    A polygon is said to be closed if its start point and end point are equal.

    \sa QVector::first(), QVector::last()
*/

/*!
    Returns the bounding rectangle of the polygon, or QRectF(0,0,0,0)
    if the polygon is empty.

    \sa QVector::isEmpty()
*/

QRectF QPolygonF::boundingRect() const
{
    if (isEmpty())
        return QRectF(0, 0, 0, 0);
    register const QPointF *pd = constData();
    qreal minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    ++pd;
    for (int i = 1; i < size(); ++i) {
        if (pd->x() < minx)
            minx = pd->x();
        else if (pd->x() > maxx)
            maxx = pd->x();
        if (pd->y() < miny)
            miny = pd->y();
        else if (pd->y() > maxy)
            maxy = pd->y();
        ++pd;
    }
    return QRectF(minx,miny, maxx - minx, maxy - miny);
}

/*!
    Creates and returns a QPolygon by converting each QPointF to a
    QPoint.

    \sa QPointF::toPoint()
*/

QPolygon QPolygonF::toPolygon() const
{
    QPolygon a;
    a.reserve(size());
    for (int i=0; i<size(); ++i)
        a.append(at(i).toPoint());
    return a;
}

/*!
   Returns the polygon as a QVariant
*/
QPolygon::operator QVariant() const
{
    return QVariant(QVariant::Polygon, this);
}

/*****************************************************************************
  QPolygonF stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QPolygonF &polygon)
    \relates QPolygonF

    Writes the given \a polygon to the given \a stream, and returns a
    reference to the stream.

    \sa {Format of the QDataStream Operators}
*/

QDataStream &operator<<(QDataStream &s, const QPolygonF &a)
{
    quint32 len = a.size();
    uint i;

    s << len;
    for (i = 0; i < len; ++i)
        s << a.at(i);
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QPolygonF &polygon)
    \relates QPolygonF

    Reads a polygon from the given \a stream into the given \a
    polygon, and returns a reference to the stream.

    \sa {Format of the QDataStream Operators}
*/

QDataStream &operator>>(QDataStream &s, QPolygonF &a)
{
    quint32 len;
    uint i;

    s >> len;
    a.reserve(a.size() + (int)len);
    QPointF p;
    for (i = 0; i < len; ++i) {
        s >> p;
        a.insert(i, p);
    }
    return s;
}
#endif //QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QPolygonF &a)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QPolygonF(";
    for (int i = 0; i < a.count(); ++i)
        dbg.nospace() << a.at(i);
    dbg.nospace() << ')';
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QPolygonF to QDebug");
    return dbg;
    Q_UNUSED(a);
#endif
}
#endif

