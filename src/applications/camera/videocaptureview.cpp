/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "videocaptureview.h"
#include "videocapturedevice.h"
#include "videocapturedevicefactory.h"

#include <qimage.h>
#include <qpainter.h>
#include <qevent.h>

#include <qdebug.h>

VideoCaptureView::VideoCaptureView(QWidget *parent, Qt::WFlags fl):
    QWidget(parent, fl),
    m_cleared(false),
    m_tidUpdate(0)
{
    m_capture = camera::VideoCaptureDeviceFactory::createVideoCaptureDevice();

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    // Optimize paint event
    setAttribute(Qt::WA_NoSystemBackground);

    QPalette    pal(palette());
    pal.setBrush(QPalette::Window, Qt::black);
    setPalette(pal);

    setLive();
}

VideoCaptureView::~VideoCaptureView()
{
    delete m_capture;
}

void VideoCaptureView::setLive(int period)
{
    if (m_tidUpdate)
        killTimer(m_tidUpdate);
    if (period == 0)
        m_tidUpdate = startTimer(m_capture->minimumFramePeriod());
    else if ( period > 0 )
        m_tidUpdate = startTimer(period);
    else
        m_tidUpdate = 0;
}

void VideoCaptureView::setStill(const QImage& i)
{
    setLive(-1);
    m_image = i;
    repaint();
}

QList<QSize> VideoCaptureView::photoSizes() const
{
    return m_capture->photoSizes();
}

QList<QSize> VideoCaptureView::videoSizes() const
{
    return m_capture->videoSizes();
}

QSize VideoCaptureView::recommendedPhotoSize() const
{
    return m_capture->recommendedPhotoSize();
}

QSize VideoCaptureView::recommendedVideoSize() const
{
    return m_capture->recommendedVideoSize();
}

QSize VideoCaptureView::recommendedPreviewSize() const
{
    return m_capture->recommendedPreviewSize();
}

QSize VideoCaptureView::captureSize() const
{
    return m_capture->captureSize();
}

void VideoCaptureView::setCaptureSize( QSize size )
{
    m_capture->setCaptureSize(size);
}

uint VideoCaptureView::refocusDelay() const
{
    return m_capture->refocusDelay();
}

bool VideoCaptureView::available() const
{
    return m_capture->hasCamera();
}

void VideoCaptureView::moveEvent(QMoveEvent*)
{
    m_cleared = false;
}

void VideoCaptureView::resizeEvent(QResizeEvent*)
{
    m_cleared = false;
}

void VideoCaptureView::paintEvent(QPaintEvent* paintEvent)
{
    QPainter    painter(this);

    if (!m_cleared)
    {
        QPoint      brushOrigin = painter.brushOrigin();

        // Paint window background
        painter.setBrushOrigin(-mapToGlobal(QPoint(0, 0)));
        painter.fillRect(paintEvent->rect(), window()->palette().brush(QPalette::Window));

        // Reset origin
        painter.setBrushOrigin(brushOrigin);

        m_cleared = true;
    }

    if (m_tidUpdate && !available()) {

        painter.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap, tr("No Camera"));

        killTimer(m_tidUpdate);
    }
    else
    {
        if (m_tidUpdate > 0)
            m_capture->getCameraImage(m_image);

        int w = m_image.width();
        int h = m_image.height();

        if (!(w == 0 || h == 0))
        {
            if (width() * w > height() * h) {
                w = w * height() / h;
                h = height();
            }
            else {
                h = h * width() / w;
                w = width();
            }

            QPainter(this).drawImage(QRect((width() - w) / 2, (height() - h) / 2, w, h),
                                     m_image,
                                     QRect(0, 0, m_image.width(), m_image.height()));
        }
    }
}

void VideoCaptureView::timerEvent(QTimerEvent*)
{
    repaint();
}

