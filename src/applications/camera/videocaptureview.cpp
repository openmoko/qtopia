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

#include "videocaptureview.h"
#include "videocapturedevice.h"
#include "videocapturedevicefactory.h"
#include <math.h>

#include <qimage.h>
#include <qpainter.h>
#include <qevent.h>

#include <QDebug>

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


    m_doZoom = false;
    m_zoomlevel = 0;
    m_zoomfactor = 1.0;
    m_minZoom = 0;
    m_maxZoom = 2;

    m_force = false;
#ifdef QT_QWS_GREENPHONE
    m_still = false;
    syncNotifier = new QSocketNotifier(m_capture->getFD(), QSocketNotifier::Read, parent);
    connect(syncNotifier, SIGNAL(activated(int)), this, SLOT(imageReady(int)));  
#endif    
}

VideoCaptureView::~VideoCaptureView()
{
    delete m_capture;
#ifdef QT_QWS_GREENPHONE    
    delete syncNotifier;
#endif    
}

void VideoCaptureView::setLive(int period)
{
    if (m_tidUpdate)
        killTimer(m_tidUpdate);
    if (period == 0) {
        m_tidUpdate = startTimer(m_capture->minimumFramePeriod());
#ifdef QT_QWS_GREENPHONE        
        m_still = false;
#endif        
    }    
    else if ( period > 0 ) {
        m_tidUpdate = startTimer(period);
#ifdef QT_QWS_GREENPHONE        
        m_still =false;
#endif        
    }    
    else {
        m_tidUpdate = 0;
#ifdef QT_QWS_GREENPHONE        
        m_still = true;
#endif        
     }
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

        if (m_tidUpdate > 0 || m_force)
        {
#ifdef QT_QWS_GREENPHONE
        if(!m_still)
#endif        
                m_capture->getCameraImage(m_image);
            if (m_doZoom)
            {
                //Crop
                int i;
                float dw = 0.0,dh = 0.0;
                QRect r = m_image.rect();
                int w = (int)(r.width() * m_zoomfactor);
                int h = (int)(r.height() * m_zoomfactor);
                //center image
                for(i = m_zoomlevel; i > 0; i--) {
                    dw += w/(i<<1);
                    dh += h/(i<<1);
                }    
                QRect d (r.x() + (unsigned int)dw*m_zoomlevel, r.y() + (unsigned int)dh*m_zoomlevel , w, h); 
                QImage img2 = m_image.copy(d);
                m_image = img2; 
            }

        }
        
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

            painter.drawImage(QRect((width() - w) / 2, (height() - h) / 2, w, h),
                                     m_image,
                                     QRect(0, 0, m_image.width(), m_image.height()));
        }
    }
}

void VideoCaptureView::timerEvent(QTimerEvent*)
{
    m_cleared = true;
    repaint();
    m_cleared = false;
}

void VideoCaptureView::zoomIn()
{

    m_zoomlevel = (m_zoomlevel+1<=m_maxZoom)?++m_zoomlevel:m_zoomlevel;
    m_zoomfactor = 1.0 / ::pow(2,m_zoomlevel);
    doZoom();
}    

void VideoCaptureView::zoomOut()
{
    m_zoomlevel = (m_zoomlevel-1>=m_minZoom)?--m_zoomlevel:m_zoomlevel;
    m_zoomfactor = 1.0 / ::pow(2,m_zoomlevel);
    doZoom();
}

void VideoCaptureView::doZoom(void)
{
    if (m_zoomlevel == m_minZoom) 
    {
        // reset
        //m_capture->doZoom(m_zoomlevel);
        m_doZoom = false;
    }    
    else
    {
        
        //bool ret = m_capture->doZoom(m_zoomlevel);
       // if (ret)
        //    m_doZoom = false;
        //else
            m_doZoom = true;
    }    

    repaint();
}

#ifdef QT_QWS_GREENPHONE
void VideoCaptureView::imageReady(int fd)
{
    //NOTE: update() does not repaint immediatly, use repaint() instead
    Q_UNUSED(fd);
    m_force = true;
    repaint();
    m_force = false;
}
#endif


