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

#ifndef __QTOPIA_CAMERA_VIDEOCAPTURE_H
#define __QTOPIA_CAMERA_VIDEOCAPTURE_H

#include <qsize.h>
#include <qlist.h>
#include <qimage.h>


class VideoCapture
{
public:
    virtual ~VideoCapture() {}

    virtual bool hasCamera() const = 0;
    virtual void getCameraImage(QImage& img, bool copy=false) = 0;

    virtual QList<QSize> photoSizes() const = 0;
    virtual QList<QSize> videoSizes() const = 0;

    virtual QSize recommendedPhotoSize() const = 0;
    virtual QSize recommendedVideoSize() const = 0;
    virtual QSize recommendedPreviewSize() const = 0;

    virtual QSize captureSize() const = 0;
    virtual void setCaptureSize( QSize size ) = 0;

    virtual uint refocusDelay() const = 0;
    virtual int minimumFramePeriod() const = 0;

    static VideoCapture* instance();
};


#endif  // __QTOPIA_CAMERA_VIDEOCAPTURE_H

