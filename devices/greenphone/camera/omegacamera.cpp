/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qimage.h>
#include <qpainter.h>

#include "omegacamera.h"

#define	VIDEO_DEVICE	    "/dev/video0"


namespace camera
{

bool OmegaCamera::hasCamera() const
{
    return ( fd != -1 );
}

QSize OmegaCamera::captureSize() const
{
    return QSize( width, height );
}

uint OmegaCamera::refocusDelay() const
{
    return 250;
}

int OmegaCamera::minimumFramePeriod() const
{
    return 40; // milliseconds
}

OmegaCamera::OmegaCamera():
    m_imageBuf(0)
{
    setupCamera( QSize( 0, 0 ) );
}

OmegaCamera::~OmegaCamera()
{
    shutdown();
}

void OmegaCamera::setupCamera( QSize size )
{
    // Clear important variables.
    frames = 0;
    currentFrame = 0;
    caps.minwidth = 144;
    caps.minheight = 176;
    caps.maxwidth = 1024;
    caps.maxheight = 1280;

    // Open the video device.
    fd = open( VIDEO_DEVICE, O_RDWR );
    if ( fd == -1 ) {
        qWarning( "%s: %s", VIDEO_DEVICE, strerror( errno ) );
        return;
    }

    // Set palette mode
    struct { int r1; int r2; } _pm = { 15, 15 };
    ioctl(fd, 215, &_pm);

    // Determine the capture size to use.  Zero indicates "preview mode".
    if ( size.width() == 0 ) {
        size = QSize(144, 176);
    }

    width = size.height();
    height = size.width();

    // Set the new capture window.
    struct video_window wind;
    memset(&wind, 0, sizeof(wind));

    wind.width = width;
    wind.height = height;

    if ( ioctl( fd, VIDIOCSWIN, &wind ) < 0 ) {
        qWarning("%s: could not set the capture window", VIDEO_DEVICE);
    }

    m_imageBuf = (quint16*) malloc(width * height * 2);

    // Enable mmap-based access to the camera.
    memset( &mbuf, 0, sizeof( mbuf ) );
    if ( ioctl( fd, VIDIOCGMBUF, &mbuf ) < 0 ) {
        qWarning( "%s: mmap-based camera access is not available", VIDEO_DEVICE );
        close( fd );
        fd = -1;
        return;
    }

    // Mmap the designated memory region.
    frames = (unsigned char *)mmap( 0, mbuf.size, PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0 );
    if ( !frames || frames == (unsigned char *)(long)(-1) ) {
        qWarning( "%s: could not mmap the device", VIDEO_DEVICE );
        close( fd );
        fd = -1;
        return;
    }

    // Start capturing of the first frame.
    ioctl(fd, VIDIOCCAPTURE, 0);
}

void OmegaCamera::shutdown()
{
    if ( frames != 0 ) {
        munmap( frames, mbuf.size );
        frames = 0;
    }

    if ( fd != -1 ) {
        ioctl(fd, VIDIOCCAPTURE, -1);
        close( fd );
        fd = -1;
    }

    if (m_imageBuf)
    {
        free(m_imageBuf);
        m_imageBuf = 0;
    }
}

#define CLAMP(x) x < 0 ? 0 : x & 0xff
inline void yuv2rgb(int y, int u, int v, quint16* rgb)
{
    register int r, g, b;

    r = y + (v * 1370705 / 1000000);
    g = y - (v * 698001 / 1000000) - (u * 337633 / 1000000);
    b = y + (u * 1732446 / 1000000);

    r = CLAMP(r);
    g = CLAMP(g);
    b = CLAMP(b);

    *rgb = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}


void OmegaCamera::getCameraImage( QImage& img, bool copy )
{
    Q_UNUSED(copy);

    if ( fd == -1 ) {
        if ( img.isNull() ) {
            img = QImage(height, width, QImage::Format_RGB16);
        }
        return;
    }

    // Start capturing the next frame (we alternate between 0 and 1).
    int frame = currentFrame;
    struct video_mmap capture;
    memset(&capture, 0, sizeof(capture));

    if ( mbuf.frames > 1 ) {
        currentFrame = !currentFrame;
        capture.frame = currentFrame;
        capture.width = width;
        capture.height = height;

        ioctl( fd, VIDIOCMCAPTURE, &capture );
    }

    // Wait for the current frame to complete.
    ioctl(fd, VIDIOCSYNC, &frame);

    // {{{ Convert 
    /*
    unsigned char*  buf = frames + mbuf.offsets[frame];
    unsigned char*  end = buf + width * height * 2;
    quint16*        dst = m_imageBuf;

    while (buf < end)
    {
        int u = buf[0] - 128;
        int v = buf[2] - 128;

        yuv2rgb(buf[1] - 16, u, v, dst++);
        yuv2rgb(buf[3] - 16, u, v, dst++);

        buf += 4;
    }
    */
    // }}}

    // {{{ Convert + rotate
    int             wp = height;
    int             hp = width;
    unsigned char*  buf = frames + mbuf.offsets[frame];

    for (int x = wp - 1; x >= 0; --x)
    {
        for (int y = 0; y < hp;)
        {
            int u = buf[0] - 128;
            int v = buf[2] - 128;

            yuv2rgb(buf[1] - 16, u, v, m_imageBuf + (y++ * wp) + x);
            yuv2rgb(buf[3] - 16, u, v, m_imageBuf + (y++ * wp) + x);

            buf += 4;
        }
    }
    // }}}

    img = QImage((uchar*) m_imageBuf, height, width, QImage::Format_RGB16);

    // Queue up another frame if the device only supports one at a time.
    if ( mbuf.frames <= 1 ) {
        capture.frame = currentFrame;
        capture.width = width;
        capture.height = height;

        ioctl( fd, VIDIOCMCAPTURE, &capture );
    }
}

QList<QSize> OmegaCamera::photoSizes() const
{
    QList<QSize> list;

    /*
    list << QSize(176, 144) << QSize(160, 120) <<
            QSize(320, 240) << QSize(352, 288) <<
            QSize(640, 480) << QSize(1280, 1024);
    */
    list << QSize(480, 640) << QSize(240, 320) << QSize(144, 176);

    return list;
}

QList<QSize> OmegaCamera::videoSizes() const
{
    // We use the same sizes for both.
    return photoSizes();
}

QSize OmegaCamera::recommendedPhotoSize() const
{
    return QSize(480, 640);
}

QSize OmegaCamera::recommendedVideoSize() const
{
    return QSize(480, 640);
}

QSize OmegaCamera::recommendedPreviewSize() const
{
    return QSize(144, 176);
}

void OmegaCamera::setCaptureSize( QSize size )
{
    if ( size.width() != height || size.height() != width) {
        shutdown();
        setupCamera( size );
    }
}

} // ns camera

