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

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QPE_DEFAULT_TODAY_MODE "Daily"

//#define QPE_HAVE_MEMALERTER
//#define QPE_LAZY_APPLICATION_SHUTDOWN

// The serial device for AT command access to the phone
// hardware:
//
#define QTOPIA_PHONE_DEVICE "/dev/ttyS0"
//
// The baud rate for the serial connection:
//
#define QTOPIA_PHONE_RATE 38400

// Displays the homescreen background in the phone launcher.
#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS

// Displays the homescreen background in all applications.
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Builds the calibration settings app into the server.
//#define QPE_NEED_CALIBRATION

// Define the phone vendor.  This indicates to load "libfoovendor.so"
// as the phone vendor plugin, and "libfoomultiplex.so" as the multiplexer.
//#define QTOPIA_PHONE_VENDOR "foo"

// Define this if wireless LAN support should be removed from the lan plugin
// Removing Wireless LAN support reduces the size of the lan plugin by about 1 MB.
// Extended Wireless LAN support (scanning and active reconnection) requires Wireless extension v14+ and will only be enabled if the 
// device supports WE v14+
//#define NO_WIRELESS_LAN
