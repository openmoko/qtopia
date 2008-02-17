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

#include <qimage.h>

#include "dummyvideocapturedevice.h"


#ifdef QTOPIA_CAMERA_DEBUG

// Dummy implementation for systems without video.

DummyVideoCaptureDevice::DummyVideoCaptureDevice()
{
}

DummyVideoCaptureDevice::~DummyVideoCaptureDevice()
{
}

bool DummyVideoCaptureDevice::hasCamera() const
{
    return FALSE;
}

QSize DummyVideoCaptureDevice::captureSize() const
{
    return QSize(640, 480);
}

uint DummyVideoCaptureDevice::refocusDelay() const
{
    return 0;
}

int DummyVideoCaptureDevice::minimumFramePeriod() const
{
    return 100;
}

static unsigned int nextrand()
{
#define A 16807
#define M 2147483647
#define Q 127773
#define R 2836
    static unsigned int rnd=1;
    unsigned long hi = rnd / Q;
    unsigned long lo = rnd % Q;
    unsigned long test = A*lo - R*hi;
    if (test > 0)
            rnd = test;
    else
            rnd = test + M;

    return rnd;
}

void DummyVideoCaptureDevice::getCameraImage(QImage& img, bool)
{
    // Just generate something dynamic (rectangles)
    static QImage   cimg;
    int             x, y, w, h;

    if (cimg.isNull()) {
        x = y = 0;
        w = 640; h = 480;
        cimg.create(w, h, 32);
    } else {
        w = nextrand() % (cimg.width() - 10) + 10;
        h = nextrand() % (cimg.height() - 10) + 10;
        x = nextrand() % (cimg.width() - w);
        y = nextrand() % (cimg.height() - h);
    }

    QRgb c = qRgb(nextrand() % 255,nextrand() % 255,nextrand() % 255);

    for (int j = 0; j < h; j++) {
        QRgb* l = (QRgb*)cimg.scanLine(y+j)+x;

        for (int i = 0; i < w; i++)
            l[i] = c;
    }

    img = cimg;
}

QList<QSize> DummyVideoCaptureDevice::photoSizes() const
{
    QList<QSize> list;

    list.append(QSize(640, 480));
    list.append(QSize(320, 240));

    return list;
}

QList<QSize> DummyVideoCaptureDevice::videoSizes() const
{
    QList<QSize> list;

    list.append(QSize(640, 480));
    list.append(QSize(320, 240));

    return list;
}

QSize DummyVideoCaptureDevice::recommendedPhotoSize() const
{
    return QSize(640, 480);
}

QSize DummyVideoCaptureDevice::recommendedVideoSize() const
{
    return QSize(320, 240);
}

QSize DummyVideoCaptureDevice::recommendedPreviewSize() const
{
    return QSize(320, 240);
}

void DummyVideoCaptureDevice::setCaptureSize(QSize size)
{
}

#endif  // QTOPIA_CAMERA_DEBUG
