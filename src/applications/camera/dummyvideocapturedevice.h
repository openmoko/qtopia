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

#ifndef __QTOPIA_DUMMYVIDEDOCAPTUREDEVICE_H
#define __QTOPIA_DUMMYVIDEDOCAPTUREDEVICE_H

#include "videocapturedevice.h"

namespace camera
{


/*!
 * \class DummyVideoCaptureDevice
 *
 */

class DummyVideoCaptureDevice : public VideoCaptureDevice
{
public:

    DummyVideoCaptureDevice();
    ~DummyVideoCaptureDevice();

    bool hasCamera() const;
    void getCameraImage(QImage& img, bool copy = false );

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

    int                 m_fd;
    int                 m_imageBufferLength;
    unsigned char       *m_imageBuffer;

    QSize               m_size;
    QImage              *m_currentImage;
};

}   // ns camera

#endif  // __QTOPIA_DUMMYVIDEDOCAPTUREDEVICE_H



