/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef _OS_SUPPORT_H
#define _OS_SUPPORT_H

/**
 * @file os_support.h
 * miscellaneous OS support macros and functions.
 *
 * - usleep() (Win32, BeOS, OS/2)
 * - floatf() (OS/2)
 * - strcasecmp() (OS/2)
 */

#ifdef __MINGW32__
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
// #  include <windows.h>
#  define usleep(t)    Sleep((t) / 1000)
#endif

#ifdef __BEOS__
#  ifndef usleep
#    include <OS.h>
#    define usleep(t)  snooze((bigtime_t)(t))
#  endif
#endif

#if defined(CONFIG_OS2)
#include <stdlib.h>
static inline int usleep(unsigned int t) { return _sleep2(t / 1000); }
static inline int strcasecmp(const char* s1, const char* s2) { return stricmp(s1,s2); }
#endif

#endif /* _OS_SUPPORT_H */
