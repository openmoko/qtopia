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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qglpaintdevice_qws_p.h>
#include <private/qgl_p.h>
#include <private/qpaintengine_opengl_p.h>
#include <private/qglwindowsurface_qws_p.h>

class QWSGLPaintDevicePrivate
{
public:
    QWidget *widget;
};

class QMetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) {
        return QWidget::metric(m);
    }
};

QWSGLPaintDevice::QWSGLPaintDevice(QWidget *widget) :
    d_ptr(new QWSGLPaintDevicePrivate)
{
    Q_D(QWSGLPaintDevice);
    d->widget = widget;
}

QWSGLPaintDevice::~QWSGLPaintDevice()
{
    Q_D(QWSGLPaintDevice);
    delete d;
}

QPaintEngine* QWSGLPaintDevice::paintEngine() const
{
    return qt_qgl_paint_engine();
}

int QWSGLPaintDevice::metric(PaintDeviceMetric m) const
{
    Q_D(const QWSGLPaintDevice);
    Q_ASSERT(d->widget);

    return ((QMetricAccessor *) d->widget)->metric(m);
}

QWSGLWindowSurface* QWSGLPaintDevice::windowSurface() const
{
     Q_D(const QWSGLPaintDevice);
     return static_cast<QWSGLWindowSurface*>(d->widget->windowSurface());
}
