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

#ifndef QT_WINDOWS_H
#define QT_WINDOWS_H

#if defined(Q_CC_BOR)
// Borland's windows.h does not set these correctly, resulting in
// unusable WinSDK standard dialogs
#ifndef WINVER
#define WINVER 0x400
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif
#endif

//#include "qwindowdefs.h"
#include "qnamespace.h"

#include <windows.h>
#if defined(Q_CC_BOR)
// windef.h (included by windows.h) defines FALSE and TRUE to 0 resp. 1 -- with
// these defines, Borland Builder 6 has problems distinguishing the different
// overloads of the QBitmap constructors.
#undef TRUE
#undef FALSE
#endif

// already defined when compiled with WINVER >= 0x0500
// and we only use them in WV_2000 and WV_98
#ifndef SPI_SETMENUANIMATION
#define SPI_SETMENUANIMATION 0x1003
#endif
#ifndef SPI_SETMENUFADE
#define SPI_SETMENUFADE 0x1013
#endif
#ifndef SPI_SETCOMBOBOXANIMATION
#define SPI_SETCOMBOBOXANIMATION 0x1005
#endif
#ifndef SPI_SETTOOLTIPANIMATION
#define SPI_SETTOOLTIPANIMATION 0x1017
#endif
#ifndef SPI_SETTOOLTIPFADE
#define SPI_SETTOOLTIPFADE 0x1019
#endif
#ifndef SPI_SETUIEFFECTS 
#define SPI_SETUIEFFECTS 0x103F
#endif
#ifndef SPI_GETMENUANIMATION
#define SPI_GETMENUANIMATION 0x1002
#endif
#ifndef SPI_GETMENUFADE
#define SPI_GETMENUFADE 0x1012
#endif
#ifndef SPI_GETCOMBOBOXANIMATION
#define SPI_GETCOMBOBOXANIMATION 0x1004
#endif
#ifndef SPI_GETTOOLTIPANIMATION
#define SPI_GETTOOLTIPANIMATION 0x1016
#endif
#ifndef SPI_GETTOOLTIPFADE
#define SPI_GETTOOLTIPFADE 0x1018
#endif
#ifndef SPI_GETUIEFFECTS 
#define SPI_GETUIEFFECTS 0x103E
#endif
#ifndef IDC_HAND
#define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

#endif // QT_WINDOWS_H
