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

// Datebook on Qtopia Desktop sends the "current time"
// to Qtopia after it has finished synchronizing.
// This define tells Qtopia to set it's clock from the
// time received so that the clock on the PDA is kept
// in sync with the desktop.
#define QPE_SYNC_CLOCK_FROM_QD

// Do all the processing but don't actually write the
// clock. (eg. Qtopia is running on a PC under QVfb)
#define QPE_SYNC_CLOCK_FROM_QD_FAKE

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

// Displays the homescreen background in all applications - experimental
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Builds the calibration settings app into the server.
//#define QPE_NEED_CALIBRATION

// Sets the phone vendor plugin to load (normally all plugins are loaded).
//#define QTOPIA_PRELOAD_PHONE_PLUGIN "foovendor"

// Set the maximum GSM 07.10 multiplexing frame size (default is 31).
//#define MAX_GSM0710_FRAME_SIZE 31

// Set additional parameters to add to the AT+CMUX command after frame size.
//#define QTOPIA_EXTRA_CMUX_PARAMS ""

// Define this to use the "advanced" GSM 07.10 CMUX mode instead of "basic".
//#define QTOPIA_ADVANCED_CMUX

// Define these to use direct serial ports for multiplexing channels.
// The primary AT command channel is defined by "QTOPIA_PHONE_DEVICE".
// Channels may be omitted if they aren't appropriate.
//#define QTOPIA_MUX_SECONDARY_DEVICE "/dev/ttyXX"
//#define QTOPIA_MUX_DATA_DEVICE "/dev/ttyXX"
//#define QTOPIA_MUX_SPARE_DEVICE "/dev/ttyXX"
//#define QTOPIA_MUX_RAW_DEVICE "/dev/ttyXX"

// Define this if setup commands for GPRS data sessions should be sent
// on the primary command channel instead of the data channel.
//#define QTOPIA_MUX_DATA_SETUP_ON_COMMAND_CHANNEL

// Define this if Wavecom-style WMUX multiplexing should be supported.
#define	QTOPIA_WAVECOM_MUX

// Define this if you do not have a working /dev/mixer (PDA Edition only)
#define QTOPIA_USE_QSS_VOLUME
