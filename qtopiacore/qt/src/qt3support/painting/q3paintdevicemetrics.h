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

#ifndef Q3PAINTDEVICEMETRICS_H
#define Q3PAINTDEVICEMETRICS_H

#include <QtGui/qpaintdevice.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3PaintDeviceMetrics                        // paint device metrics
{
public:
    Q3PaintDeviceMetrics(const QPaintDevice *device) : pdev(device) {}

    int width() const { return pdev->width(); }
    int height() const { return pdev->height(); }
    int widthMM() const { return pdev->widthMM(); }
    int heightMM() const { return pdev->heightMM(); }
    int logicalDpiX() const { return pdev->logicalDpiX(); }
    int logicalDpiY() const { return pdev->logicalDpiY(); }
    int physicalDpiX() const { return pdev->physicalDpiX(); }
    int physicalDpiY() const { return pdev->physicalDpiY(); }
    int numColors() const { return pdev->numColors(); }
    int depth() const { return pdev->depth(); }

private:
    const QPaintDevice *pdev;
};

QT_END_HEADER

#endif // Q3PAINTDEVICEMETRICS_H
