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

#ifndef Q3POINTARRAY_H
#define Q3POINTARRAY_H

#include <QtGui/qpolygon.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3PointArray : public QPolygon
{
public:
    inline Q3PointArray() : QPolygon() {}
    inline Q3PointArray(const QRect &r, bool closed=false) : QPolygon(r, closed) {}
    inline Q3PointArray(const QPolygon& a) : QPolygon(a) {}

    inline Q3PointArray copy() const { return *this; }
    inline bool isNull() { return isEmpty(); }
    void makeEllipse(int x, int y, int w, int h);
#ifndef QT_NO_WMATRIX
    void makeArc(int x, int y, int w, int h, int a1, int a2);
    void makeArc(int x, int y, int w, int h, int a1, int a2, const QMatrix &matrix);
#endif
    Q3PointArray cubicBezier() const;
};

QT_END_HEADER

#endif // Q3POINTARRAY_H
