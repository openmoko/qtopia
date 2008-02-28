/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
    delete m_frameBufferImage;
#endif
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

        if (m_black != 0) {
            qt_screen->solidFill(Qt::black, paintRegion & m_blackRegion);
            --m_black;
        }

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
        QPainter    p(m_frameBufferImage);
        p.setClipRegion(paintRegion);
        p.setWindow(m_windowRect);
        p.setViewport(m_viewPort);
        p.setWorldTransform(m_transform);
        p.drawImage(0, 0, frame);
#else
        qt_screen->blit(frame, m_destTopLeft, paintRegion);
#endif
        endPainting();

        qt_screen->setDirty(paintRegion.boundingRect());
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

    if (!region.isEmpty() && m_videoSize.isValid()) {
        if (m_firstPaintCalc) {
            QRect bounds = geometry();
            m_destRect = QRect(QPoint(0, 0), m_videoSize);

#ifndef QTOPIA_NO_MEDIAVIDEOSCALING
            m_frameBufferImage = new QImage(frameBuffer(),
                                            screenWidth(),
                                            screenHeight(),
                                            screenDepth() == 16 ? QImage::Format_RGB16 :
                                                                  QImage::Format_RGB32);

            const double fourfifths = double(4) / 5;
            if ((m_destRect.width() < fourfifths * bounds.width() &&
                 m_destRect.height() < fourfifths * bounds.height()) ||
                (m_destRect.width() > bounds.width() ||
                 m_destRect.height() > bounds.height()))
            {
                QSize scaled = m_videoSize;
                scaled.scale(bounds.size(), Qt::KeepAspectRatio);
                m_destRect.setSize(scaled);
            }

            m_destRect.moveCenter(bounds.center());

            if (qt_screen->isTransformed()) {
                m_windowRect = QRect(QPoint(0, 0), qt_screen->mapToDevice(m_videoSize));
                m_viewPort = qt_screen->mapToDevice(m_destRect, QSize(qt_screen->width(), qt_screen->height()));
                switch (qt_screen->transformOrientation()) {
                    case 1: m_transform.translate(0, m_windowRect.height()); break;
                    case 2: m_transform.translate(m_windowRect.width(), m_windowRect.height()); break;
                    case 3: m_transform.translate(m_windowRect.width(), 0); break;
                }
                m_transform.rotate(360 - qt_screen->transformOrientation() * 90);
            }
            else {
                m_windowRect = QRect(QPoint(0, 0), m_videoSize);
                m_viewPort = m_destRect;
            }
#else
            m_destRect.moveCenter(bounds.center());
            m_destTopLeft = m_destRect.topLeft();
#endif
            m_blackRegion = requestedRegion() ^ QRegion(m_destRect);

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
