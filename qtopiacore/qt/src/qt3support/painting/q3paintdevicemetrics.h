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
