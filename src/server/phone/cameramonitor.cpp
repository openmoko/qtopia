/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

// Local includes
#include "cameramonitor.h"

// Qtopia includes
#include <QtopiaFeatures>

// System includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Constants
static const char* const CAMERA_MONITOR_VIDEODEV        =   "/dev/video";
static const char* const CAMERA_MONITOR_CAMERA_FEATURE  =   "Camera";

// ============================================================================
//
//  CameraMonitor
//
// ============================================================================

/*!
  \class CameraMonitor
  \ingroup QtopiaServer::Task
  \brief The CameraMonitor class updates the camera feature

  The default implementation checks the camera availablity at startup and
  exports the "Camera" QtopiaFeature, if the camera can added and removed
  during device operation then update() should be called when this occurs.
*/

/*!
    Constructs the camera monitor task and attaches it to \a parent.
*/
CameraMonitor::CameraMonitor( QObject* parent )
:   QObject( parent )
{
    update();
}

/*!
    \internal
*/
CameraMonitor::~CameraMonitor()
{
}

/*!
    Updates the availability of the camera feature.
*/
void CameraMonitor::update()
{
    // TODO This code is duplicated in src/applications/camera/videocapturedevicefactory
    // so move into one convenient class
    int fd = open( CAMERA_MONITOR_VIDEODEV, O_RDWR );
    if ( fd != -1 ) {
        QtopiaFeatures::setFeature( CAMERA_MONITOR_CAMERA_FEATURE );
    } else {
        QtopiaFeatures::removeFeature( CAMERA_MONITOR_CAMERA_FEATURE );
    }
    close( fd );
}

