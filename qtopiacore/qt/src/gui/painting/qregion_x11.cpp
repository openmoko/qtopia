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

#include <private/qt_x11_p.h>

#include <limits.h>

QRegion::QRegionData QRegion::shared_empty = {Q_ATOMIC_INIT(1), 0, 0, 0};

void QRegion::updateX11Region() const
{
    d->rgn = XCreateRegion();
    if (!d->qt_rgn)
        return;

    for(int i = 0; i < d->qt_rgn->numRects; ++i) {
        XRectangle r;
        const QRect &rect = d->qt_rgn->rects.at(i);
        r.x = qMax(SHRT_MIN, rect.x());
        r.y = qMax(SHRT_MIN, rect.y());
        r.width = qMin((int)USHRT_MAX, rect.width());
        r.height = qMin((int)USHRT_MAX, rect.height());
        XUnionRectWithRegion(&r, d->rgn, d->rgn);
    }
}

void *QRegion::clipRectangles(int &num) const
{
    if (!d->xrectangles && !(d == &shared_empty || d->qt_rgn->numRects == 0)) {
        XRectangle *r = static_cast<XRectangle*>(malloc(d->qt_rgn->numRects * sizeof(XRectangle)));
        d->xrectangles = r;
        for(int i = 0; i < d->qt_rgn->numRects; ++i) {
            const QRect &rect = d->qt_rgn->rects.at(i);
            r->x = qMax(SHRT_MIN, rect.x());
            r->y = qMax(SHRT_MIN, rect.y());
            r->width = qMin((int)USHRT_MAX, rect.width());
            r->height = qMin((int)USHRT_MAX, rect.height());
            ++r;
        }
    }
    if (d == &shared_empty || d->qt_rgn->numRects == 0)
        num = 0;
    else
        num = d->qt_rgn->numRects;
    return d->xrectangles;
}
