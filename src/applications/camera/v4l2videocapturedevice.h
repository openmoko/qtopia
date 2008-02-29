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

#ifndef __QTOPIA_V4L2VIDEDOCAPTUREDEVICE_H
#define __QTOPIA_V4L2VIDEDOCAPTUREDEVICE_H

#include <qsize.h>
#include <qlist.h>
#include <qmap.h>

#include "videocapturedevice.h"

namespace camera
{

class FormatConverter;

/*!
 * \class V4L2VideoCaptureDevice
 *
 */

class V4L2VideoCaptureDevice : public VideoCaptureDevice
{
public:

    V4L2VideoCaptureDevice(int fd);
    ~V4L2VideoCaptureDevice();

    bool hasCamera() const;
    void getCameraImage(QImage& img, bool copy = false);

    QList<QSize> photoSizes() const;
    QList<QSize> videoSizes() const;

    QSize recommendedPhotoSize() const;
    QSize recommendedVideoSize() const;
    QSize recommendedPreviewSize() const;

    QSize captureSize() const;
    void setCaptureSize(QSize size);

    uint refocusDelay() const;
    int minimumFramePeriod() const;

private:

    void setupCamera();
    void shutdown();
    void calcPhotoSizes();
    void beginCapture();
    void endCapture();

    int                 m_fd;
    int                 m_imageBufferLength;
    unsigned char       *m_imageBuffer;

    QSize               m_size;
    QImage              *m_currentImage;
    QMap<QSize, unsigned int>   m_imageTypes;
    FormatConverter     *m_converter;
};

}   // ns camera

#endif  // __QTOPIA_V4L2VIDEDOCAPTUREDEVICE_H

