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

#include <qglobal.h> // for Q_WS_WIN define (non-PCH)
#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

#include <QtGui/QPaintDevice>
#include <QtGui/QWidget>

#include "private/qwindowsurface_raster_p.h"
#include "private/qpaintengine_raster_p.h"

class MetricAccessor : public QWidget {
public:
    int metric(PaintDeviceMetric m) const { return QWidget::metric(m); }
};

class QRasterPaintDevice : public QPaintDevice
{
public:
    QRasterPaintDevice()
    {
        m_window = 0;
        m_engine.setFlushOnEnd(false);
    }

    QWidget *window() const
    {
        return m_window;
    }

    void setWindow(QWidget *window)
    {
        m_window = window;
    }

    QSize size() const
    {
        return m_engine.size();
    }

    virtual int metric(PaintDeviceMetric m) const
    {
        Q_ASSERT(m_window);
        return ((MetricAccessor *) m_window)->metric(m);
    }

    QPaintEngine *paintEngine() const
    {
        return const_cast<QRasterPaintEngine *>(&m_engine);
    }

private:
    QRasterPaintEngine m_engine;
    QWidget *m_window;
};


struct QRasterWindowSurfacePrivate
{
    QRasterPaintDevice device;
};

QRasterWindowSurface::QRasterWindowSurface(QWidget *window)
    : d_ptr(new QRasterWindowSurfacePrivate)
{
    Q_ASSERT(window->isTopLevel());
    d_ptr->device.setWindow(window);
}


QRasterWindowSurface::~QRasterWindowSurface()
{
    delete d_ptr;
}

QPaintDevice *QRasterWindowSurface::paintDevice()
{
    return &d_ptr->device;
}


void QRasterWindowSurface::flush(QWidget *widget, const QRegion &rgn, const QPoint &offset)
{
#ifdef Q_WS_WIN
    QPoint wOffset = qt_qwidget_data(widget)->wrect.topLeft();

    QRasterPaintEngine *engine = static_cast<QRasterPaintEngine *>(d_ptr->device.paintEngine());
    HDC engine_dc = engine->getDC();
    HDC widget_dc = widget->getDC();

    QRect br = rgn.boundingRect();
    QRect wbr = br.translated(-wOffset);
    BitBlt(widget_dc, wbr.x(), wbr.y(), wbr.width(), wbr.height(),
           engine_dc, br.x() + offset.x(), br.y() + offset.y(), SRCCOPY);

    widget->releaseDC(widget_dc);
    engine->releaseDC(engine_dc);
#else
    Q_UNUSED(widget);
    Q_UNUSED(rgn);
    Q_UNUSED(offset);
#endif

}

void QRasterWindowSurface::setGeometry(const QRect &rect)
{
    const QSize size = static_cast<QRasterPaintEngine*>(d_ptr->device.paintEngine())->size();
    if (size == rect.size())
        return;
    QRasterWindowSurface::release();
}

void QRasterWindowSurface::release()
{
    static_cast<QRasterPaintEngine *>(d_ptr->device.paintEngine())->releaseBuffer();
}


void QRasterWindowSurface::scroll(const QRegion &area, int dx, int dy)
{
#ifdef Q_WS_WIN
    QRect rect = area.boundingRect();

    QRasterPaintEngine *engine = static_cast<QRasterPaintEngine *>(d_ptr->device.paintEngine());
    HDC engine_dc = engine->getDC();
    if (!engine_dc)
        return;

    BitBlt(engine_dc, rect.x()+dx, rect.y()+dy, rect.width(), rect.height(),
           engine_dc, rect.x(), rect.y(), SRCCOPY);

    engine->releaseDC(engine_dc);
#else
    Q_UNUSED(area);
    Q_UNUSED(dx);
    Q_UNUSED(dy);
#endif
}

QRect QRasterWindowSurface::geometry() const
{
    const QPoint offset = d_ptr->device.window()->geometry().topLeft();
    const QSize size = static_cast<QRasterPaintEngine *>(d_ptr->device.paintEngine())->size();
    return QRect(offset, size);
}
