/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QPE_FONT_HEIGHT_TO_ICONSIZE(x) (x+1)
#define QPE_DEFAULT_TODAY_MODE "Daily"

//#define QPE_HAVE_MEMALERTER
//#define QPE_LAZY_APPLICATION_SHUTDOWN

// The serial device for AT command access to the phone
// hardware:
//
#define QTOPIA_PHONE_DEVICE "/dev/ttyS0"
//
// Alternative for GSM0710 usage:
//#define QTOPIA_PHONE_DEVICE "/dev/ttyU0"
// The serial device for low-priority AT commands.
//#define QTOPIA_PHONE_DEVICE_LOW "/dev/ttyU1"
//
// The baud rate for the serial connection:
//
#define QTOPIA_PHONE_RATE 38400

// Displays the homescreen background in the phone launcher.
#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS

// Builds the calibration settings app into the server.
//#define QPE_NEED_CALIBRATION

