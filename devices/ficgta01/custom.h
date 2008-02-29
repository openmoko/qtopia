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

#ifndef QT_QWS_FICGTA01
#define QT_QWS_FICGTA01
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif


//#define QT_NO_LOG_STREAM

#define QPE_NEED_CALIBRATION

#define QTOPIA_PHONE_DEVICE "/dev/ttySAC0"
//#define QTOPIA_PHONE_VENDOR "ficgta01"

#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
#define NO_VISUALIZATION
#define QGLOBAL_PIXMAP_CACHE_LIMIT 2097152



// Define the devices whose packages are compatible with this device,
// by convention the first device listed is this device.
#define QTOPIA_COMPATIBLE_DEVICES "ficgta01"

// Define the name of the Video4Linux device to use for the camera.
#ifndef V4L_VIDEO_DEVICE
#define V4L_VIDEO_DEVICE            "" //no video
#endif
