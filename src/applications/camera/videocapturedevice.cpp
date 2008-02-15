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


/*!
    \class camera::VideoCaptureDevice
    \brief The VideoCaptureDevice class provides a basic interface for Video Capture Devices such as cameras
    
    This class represents a basic capture device such as a camera on the Linux platorm. It main function is to set up the device for image capture
    and store the most recent captured image in a buffer for retrieval.
    \internal
*/    

/*!
    \fn bool VideoCaptureDevice::hasCamera() const = 0
    Returns true if a camera is detected, false otherwise
    \internal


*/    
  
/*!  
    \fn void VideoCaptureDevice::getCameraImage(QImage& img, bool copy = false) = 0

    Stores a recently captured image frame in \c{img}
    \internal

*/

/*!
    \fn QList<QSize> VideoCaptureDevice::photoSizes() const = 0
    Returns the  available sizes that the device supports in capture mode
    \internal

*/

/*!
    \fn QList<QSize> VideoCaptureDevice::videoSizes() const = 0
     Returns the  available sizes that the device supports in video mode
    \internal

*/ 

/*!
    \fn QSize VideoCaptureDevice::recommendedPhotoSize() const = 0
    Returns the optimal photo size for the device
    \internal

*/

/*!
    \fn QSize VideoCaptureDevice::recommendedVideoSize() const = 0
    Returns the optimal video size for the device
    \internal

*/    
    
/*!
    \fn QSize VideoCaptureDevice::recommendedPreviewSize() const = 0
    Returns the preview size for the device
    \internal

*/    

/*!    
    \fn QSize VideoCaptureDevice::captureSize() const = 0
    Returns the current capture size
    \internal

*/    
    
/*!  
    \fn void VideoCaptureDevice::setCaptureSize(QSize size) = 0
    Sets the  capture size
    \internal

*/
    
/*!
    \fn uint VideoCaptureDevice::refocusDelay() const = 0
    Returns the delay in milliseconds that the device needs to refocus
    \internal

*/

/*!    
    \fn int VideoCaptureDevice::minimumFramePeriod() const = 0
    Returns the rate at which a frame become available in milliseconds
    \internal

*/

/*!
    \fn int VideoCaptureDevice::getFD() = 0
    Returns the device file descriptor
    \internal

*/    
}
