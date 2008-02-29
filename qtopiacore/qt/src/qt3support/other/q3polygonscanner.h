/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3POLYGONSCANNER_H
#define Q3POLYGONSCANNER_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3PointArray;
class QPoint;

class Q_COMPAT_EXPORT Q3PolygonScanner {
public:
    virtual ~Q3PolygonScanner() {}
    void scan(const Q3PointArray& pa, bool winding, int index=0, int npoints=-1);
    void scan(const Q3PointArray& pa, bool winding, int index, int npoints, bool stitchable);
    enum Edge { Left=1, Right=2, Top=4, Bottom=8 };
    void scan(const Q3PointArray& pa, bool winding, int index, int npoints, Edge edges);
    virtual void processSpans(int n, QPoint* point, int* width)=0;
};

QT_END_HEADER

#endif // Q3POLYGONSCANNER_H
