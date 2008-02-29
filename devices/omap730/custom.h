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

#ifndef QT_QWS_OMAP730
#define QT_QWS_OMAP730
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define QPE_USE_MALLOC_FOR_NEW
#endif

#define QPE_DEFAULT_TODAY_MODE "Daily"

// The serial device for AT command access to the phone
// hardware:
//
#define QTOPIA_PHONE_DEVICE "/dev/csmi/5"
//
// The baud rate for the serial connection:
//
#define QTOPIA_PHONE_RATE 115200

// Sets the phone vendor plugin to load (normally all plugins are loaded).
#define QTOPIA_PRELOAD_PHONE_PLUGIN "omap730vendor"

// Define this to use the "advanced" GSM 07.10 CMUX mode instead of "basic".
#define QTOPIA_ADVANCED_CMUX

// Define these to use direct serial ports for multiplexing channels.
// The primary AT command channel is defined by "QTOPIA_PHONE_DEVICE".
// Channels may be omitted if they aren't appropriate.
//
// This enables the use of CSMI on the TI device, instead of GMS 07.10.
#define QTOPIA_MUX_SECONDARY_DEVICE "/dev/csmi/6"
#define QTOPIA_MUX_DATA_DEVICE "/dev/csmi/8"
//#define QTOPIA_MUX_SPARE_DEVICE "/dev/ttyXX"
//#define QTOPIA_MUX_RAW_DEVICE "/dev/ttyXX"

// Define this if setup commands for GPRS data sessions should be sent
// on the primary command channel instead of the data channel.
#define QTOPIA_MUX_DATA_SETUP_ON_COMMAND_CHANNEL

#define QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#define QTOPIA_ENABLE_GLOBAL_BACKGROUNDS

// Disable media player visualization
#define NO_VISUALIZATION
