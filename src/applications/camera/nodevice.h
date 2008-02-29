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

#ifndef __QTOPIA_CAMERA_NODEVICE_H
#define __QTOPIA_CAMERA_NODEVICE_H

namespace camera
{

class NoDevice : public VideoCaptureDevice
{
public:

    NoDevice() {}
    ~NoDevice() {}

    bool hasCamera() const { return false; }
    void getCameraImage(QImage&, bool = false ) {}

    QList<QSize> photoSizes() const { return QList<QSize>(); }
    QList<QSize> videoSizes() const { return QList<QSize>(); }

    QSize recommendedPhotoSize() const { return QSize(); }
    QSize recommendedVideoSize() const { return QSize(); }
    QSize recommendedPreviewSize() const { return QSize(); }

    QSize captureSize() const { return QSize(); }
    void setCaptureSize(QSize) {}

    uint refocusDelay() const { return 0; }
    int minimumFramePeriod() const { return 0; }
};


}   // ns camera

#endif  // __QTOPIA_CAMERA_NODEVICE_H
