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

#ifndef ATCUSTOM_H
#define ATCUSTOM_H

#include <custom.h>
#include <version.h>

// Values that can be used to customise manufacturer information.
#ifndef QTOPIA_AT_MANUFACTURER
#define QTOPIA_AT_MANUFACTURER      "QTOPIA PHONE"
#endif
#ifndef QTOPIA_AT_MODEL
#define QTOPIA_AT_MODEL             "QTOPIA PHONE"
#endif
#ifndef QTOPIA_AT_REVISION
#define QTOPIA_AT_REVISION          QTOPIA_VERSION_STR
#endif
#ifndef QTOPIA_AT_SERIAL_NUMBER
#define QTOPIA_AT_SERIAL_NUMBER     "1234567890"
#endif

// Define this to get a strict GSM AT command set with no AT*Q extensions.
//#define QTOPIA_AT_STRICT 1

#endif // ATCUSTOM_H
