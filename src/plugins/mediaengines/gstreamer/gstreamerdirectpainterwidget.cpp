/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QSize>
#include <QRect>
#include <QPainter>
#include <QPaintEvent>
#include <QtGui/qscreen_qws.h>
#include <QDebug>

#include "gstreamerqtopiavideosink.h"

#include "gstreamerdirectpainterwidget.h"


namespace gstreamer
{

static inline bool isQVFb()
{
    static bool result, test = true;

    if (test)
    {
        test = false;

        QByteArray display_spec(getenv("QWS_DISPLAY"));

        result = display_spec.startsWith("QVFb");
    }

    return result;
}

/*!
    \class gstreamer::DirectPainterWidget
    \internal
*/

DirectPainterWidget::DirectPainterWidget(QWidget* parent):
    QDirectPainter(parent, QDirectPainter::NonReserved),
    m_isVisible(false),
    m_firstPaintCalc(true),
    m_painting(false),
    m_black(0)
{
}

DirectPainterWidget::~DirectPainterWidget()
{
    g_object_unref(m_sink);
}

GstElement* DirectPainterWidget::element()
{
    m_sink = GST_ELEMENT(g_object_new(QtopiaVideoSinkClass::get_type(), NULL));

    if (m_sink != 0)
    {
        QtopiaVideoSink*  sink = reinterpret_cast<QtopiaVideoSink*>(m_sink);

        sink->widget = this;
    }

    return m_sink;
}

int DirectPainterWidget::displayDepth() const
{
    return qt_screen->depth();
}

void DirectPainterWidget::setVideoSize(int w, int h)
{
    m_videoSize = QSize(w, h);
}

void DirectPainterWidget::paint(QImage const& frame)
{
    if (m_painting)
        return;

    m_painting = true;

    if (!frame.isNull() && !m_firstPaintCalc)
    {
        startPainting();

        QRegion paintRegion = allocatedRegion();

        if (m_black != 0)
        {
            qt_screen->solidFill(Qt::black, paintRegion & m_blackRegion);
            --m_black;
        }

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
        qt_screen->blit(frame.scaled(m_destSize),
                        m_destTopLeft,
                        paintRegion);
#else
        qt_screen->blit(frame, m_destTopLeft, paintRegion);
#endif
        endPainting();

        if (isQVFb())
        {
            // Inform QVFb of change in framebuffer
            QScreen::instance()->setDirty(m_destRect);
        }
    }

    m_painting = false;
}

// private
void DirectPainterWidget::regionChanged(const QRegion &exposedRegion)
{
    calc(exposedRegion);
}

void DirectPainterWidget::calc(QRegion const& region)
{
    m_isVisible = false;

    if (!region.isEmpty() && m_videoSize.isValid())
    {
        if (m_firstPaintCalc)   // XXX: Assume original exposed region is max widget size!
        {
            QRect bounds = region.boundingRect();
            QRect imageRect = QRect(QPoint(0, 0), m_videoSize);

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
            m_destSize = m_videoSize;
            m_destSize.scale(bounds.size(), Qt::KeepAspectRatio);

            imageRect.setSize(m_destSize);
#endif
            imageRect.moveCenter(bounds.center());

            m_destRect = bounds;
            m_destTopLeft = imageRect.topLeft();
            m_blackRegion = region ^ QRegion(imageRect);

            m_firstPaintCalc = false;
        }
        else
        {
            // Fill if paused, this is not the best. But the image data may
            // not be retrievable at this time. TODO:
            qt_screen->solidFill(Qt::black, region);
        }

        m_black = m_blackRegion.isEmpty() ? 0 : 2;

        m_isVisible = true;
    }

    m_savedRegion = region;
}

}   // ns gstreamer
