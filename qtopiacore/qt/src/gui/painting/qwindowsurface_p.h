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

#ifndef QWINDOWSURFACE_P_H
#define QWINDOWSURFACE_P_H

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

#include <qglobal.h>

class QPaintDevice;
class QRegion;
class QRect;
class QWidget;
class QPoint;

class QWindowSurface
{
public:
    virtual ~QWindowSurface() { }

    virtual QPaintDevice *paintDevice() = 0;
    virtual void flush(QWidget *widget, const QRegion &region, const QPoint &offset) = 0;
    virtual void setGeometry(const QRect &rect) = 0;
    virtual void release() = 0;
    virtual void scroll(const QRegion &area, int dx, int dy) = 0;

    virtual QRect geometry() const = 0;

    virtual void beginPaint(const QRegion &) { };
    virtual void endPaint(const QRegion &) { };

};

#endif // QWINDOWSURFACE_P_H
