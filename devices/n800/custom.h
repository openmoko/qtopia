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

#ifndef QT_QWS_N800
#define QT_QWS_N800
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Disable media player visualization
#define NO_VISUALIZATION
#define QPE_NEED_CALIBRATION

#ifndef QT_QWS_N810
#define V4L_RADIO_DEVICE "/dev/radio0"
#endif
#define V4L_VIDEO_DEVICE "/dev/video0"


