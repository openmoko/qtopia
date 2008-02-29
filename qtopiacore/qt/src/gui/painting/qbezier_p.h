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

#ifndef QBEZIER_P_H
#define QBEZIER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qpoint.h"
#include "QtCore/qline.h"
#include "QtCore/qrect.h"

class QPolygonF;

class Q_GUI_EXPORT QBezier
{
public:
    static QBezier fromPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4);

    inline QPointF pointAt(qreal t) const;
    inline QPointF normalVector(qreal t) const;

    QPolygonF toPolygon() const;
    void addToPolygon(QPolygonF *p) const;
    QRectF bounds() const;

    QPointF pt1() const { return QPointF(x1, y1); }
    QPointF pt2() const { return QPointF(x2, y2); }
    QPointF pt3() const { return QPointF(x3, y3); }
    QPointF pt4() const { return QPointF(x4, y4); }

    inline QPointF midPoint() const;
    inline QLineF midTangent() const;

    void split(QBezier *firstHalf, QBezier *secondHalf) const;
    int shifted(QBezier *curveSegments, int maxSegmets, qreal offset, float threshold) const;

    qreal x1, y1, x2, y2, x3, y3, x4, y4;
};

inline QPointF QBezier::midPoint() const
{
    return QPointF((x1 + x4 + 3*(x2 + x3))/8., (y1 + y4 + 3*(y2 + y3))/8.);
}

inline QLineF QBezier::midTangent() const
{
    QPointF mid = midPoint();
    QLineF dir(QLineF(x1, y1, x2, y2).pointAt(0.5), QLineF(x3, y3, x4, y4).pointAt(0.5));
    return QLineF(mid.x() - dir.dx(), mid.y() - dir.dy(),
                  mid.x() + dir.dx(), mid.y() + dir.dy());
}

inline QPointF QBezier::pointAt(qreal t) const
{
    Q_ASSERT(t >= 0);
    Q_ASSERT(t <= 1);
#if 1
    qreal a, b, c, d;
    qreal m_t = 1. - t;
    b = m_t * m_t;
    c = t * t;
    d = c * t;
    a = b * m_t;
    b *= 3. * t;
    c *= 3. * m_t;

    return QPointF(a*x1 + b*x2 + c*x3 + d*x4, a*y1 + b*y2 + c*y3 + d*y4);
#else
    // numerically more stable:
    qreal m_t = 1. - t;
    qreal a = x1*m_t + x2*t;
    qreal b = x2*m_t + x3*t;
    qreal c = x3*m_t + x4*t;
    a = a*m_t + b*t;
    b = b*m_t + c*t;
    qreal x = a*m_t + b*t;
    qreal a = y1*m_t + y2*t;
    qreal b = y2*m_t + y3*t;
    qreal c = y3*m_t + y4*t;
    a = a*m_t + b*t;
    b = b*m_t + c*t;
    qreal y = a*m_t + b*t;
    return QPointF(x, y);
#endif
}

inline QPointF QBezier::normalVector(qreal t) const
{
    qreal m_t = 1. - t;
    qreal a = m_t * m_t;
    qreal b = t * m_t;
    qreal c = t * t;

    return QPointF((y2-y1) * a + (y3-y2) * b + (y4-y3) * c,  -(x2-x1) * a - (x3-x2) * b - (x4-x3) * c);
}

#endif // QBEZIER_P_H
