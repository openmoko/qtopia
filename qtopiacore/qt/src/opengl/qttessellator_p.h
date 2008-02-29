/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtOpenGL module of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Qt Tesselator. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QTTESSELLATOR_H_H
#define QTTESSELLATOR_H_H

#include <QVector>
#include <QPointF>
#include <QRectF>

typedef int qt_XFixed;
typedef double	qt_XDouble;

#define qt_XDoubleToFixed(f)    ((qt_XFixed) ((f) * 65536))
#define qt_XFixedToDouble(f)    (((qt_XDouble) (f)) / 65536)

struct qt_XPointFixed {
    qt_XFixed  x, y;
};

struct qt_XLineFixed {
    qt_XPointFixed p1, p2;
};

struct qt_XTrapezoid {
    qt_XFixed     top, bottom;
    qt_XLineFixed left, right;
};

void qt_polygon_trapezoidation(QVector<qt_XTrapezoid> *traps,
                               const QPointF *pg, int pgSize,
                               bool winding, QRect *br);



#endif
