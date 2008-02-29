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

#ifndef __QTOPIA_V4L1VIDEDOCAPTUREDEVICE_H
#define __QTOPIA_V4L1VIDEDOCAPTUREDEVICE_H

#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <linux/videodev.h>

#include "videocapturedevice.h"

namespace camera
{

/*!
 * \class V4L1VideoCaptureDevice
 *
 */

class V4L1VideoCaptureDevice : public VideoCaptureDevice
{
public:

    V4L1VideoCaptureDevice();
    ~V4L1VideoCaptureDevice();

    bool hasCamera() const;
    void getCameraImage( QImage& img, bool copy=FALSE );

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

    int                 fd;
    int                 width, height;
    video_capability    caps;
    video_mbuf          mbuf;
    unsigned char       *frames;
    int                 currentFrame;

    void setupCamera( QSize size );
    void shutdown();
};

}   // ns camera

#endif  // __QTOPIA_V4L1VIDEDOCAPTUREDEVICE_H


