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
/* mangle.h - This file has some CPP macros to deal with different symbol
 * mangling across binary formats.
 * (c)2002 by Felix Buenemann <atmosfear at users.sourceforge.net>
 * File licensed under the GPL, see http://www.fsf.org/ for more info.
 */

#ifndef __MANGLE_H
#define __MANGLE_H

/* Feel free to add more to the list, eg. a.out IMO */
/* Use rip-relative addressing if compiling PIC code on x86-64. */
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__OS2__) || \
   (defined(__OpenBSD__) && !defined(__ELF__))
#if defined(ARCH_X86_64) && defined(PIC)
#define MANGLE(a) "_" #a"(%%rip)"
#else
#define MANGLE(a) "_" #a
#endif
#else
#if defined(ARCH_X86_64) && defined(PIC)
#define MANGLE(a) #a"(%%rip)"
#else
#define MANGLE(a) #a
#endif
#endif

#endif /* !__MANGLE_H */

