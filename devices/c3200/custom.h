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

#ifndef QT_QWS_C3200
#define QT_QWS_C3200
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QPE_DEFAULT_TODAY_MODE "Daily"

#define QPE_NEED_CALIBRATION

//#define NO_WIRELESS_LAN

#define QGLOBAL_PIXMAP_CACHE_LIMIT 5242880   //use 5M instead of default 1M

#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#undef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#endif
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
#undef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
#endif
#define NO_VISUALIZATION

// Define the devices whose packages are compatible with this device,
// by convention the first device listed is this device.
#define QTOPIA_COMPATIBLE_DEVICES "c3200"

// Define the name of the Video4Linux device to use for the camera.
#ifndef V4L_VIDEO_DEVICE
#define V4L_VIDEO_DEVICE            "/dev/video"
#endif
