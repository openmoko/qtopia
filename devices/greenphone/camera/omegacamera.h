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

#ifndef __QTOPIA_CAMERA_OMEGACAMERA_H
#define __QTOPIA_CAMERA_OMEGACAMERA_H

#include "videocapturedevice.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

namespace camera
{

class OmegaCamera : public VideoCaptureDevice
{
public:
    OmegaCamera();
    ~OmegaCamera();

    bool hasCamera() const;
    void getCameraImage( QImage& img, bool copy = false);

    QList<QSize> photoSizes() const;
    QList<QSize> videoSizes() const;

    QSize recommendedPhotoSize() const;
    QSize recommendedVideoSize() const;
    QSize recommendedPreviewSize() const;

    QSize captureSize() const;
    void setCaptureSize( QSize size );

    uint refocusDelay() const;
    int minimumFramePeriod() const;

private:
    int fd;
    int width, height;
    struct video_capability caps;
    struct video_mbuf mbuf;
    unsigned char *frames;
    quint16*    m_imageBuf;
    int currentFrame;

    void setupCamera( QSize size );
    void shutdown();
};


}   // ns camera

#endif  // __QTOPIA_CAMERA_OMEGACAMERA_H
