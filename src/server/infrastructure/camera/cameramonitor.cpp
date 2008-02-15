/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

// Local includes
#include "cameramonitor.h"
#include "qtopiaserverapplication.h"

// Qtopia includes
#include <QtopiaFeatures>
#include <QValueSpaceItem>
#include <QTimer>
#include <custom.h>

// System includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Constants
static const char* const CAMERA_MONITOR_VIDEODEV        =   V4L_VIDEO_DEVICE;
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
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
*/

/*!
    Constructs the camera monitor task and attaches it to \a parent.
*/
CameraMonitor::CameraMonitor( QObject* parent )
:   QObject( parent )
{
    //delay the update to minimize startup time
    serverWidgetVsi = new QValueSpaceItem("/System/ServerWidgets/Initialized", this);
    connect( serverWidgetVsi, SIGNAL(contentsChanged()), this, SLOT(delayedUpdate()) );
    delayedUpdate(); //in case its visible already
}

/*!
    \internal
*/
CameraMonitor::~CameraMonitor()
{
}

/*!
  \internal
  */
void CameraMonitor::delayedUpdate()
{
    if ( serverWidgetVsi && serverWidgetVsi->value( QByteArray(), false ).toBool() ) {
        serverWidgetVsi->disconnect();
        serverWidgetVsi->deleteLater();
        serverWidgetVsi = 0;
        QTimer::singleShot( 5000, this, SLOT(update()) );
    }
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

