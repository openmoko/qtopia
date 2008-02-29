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
/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/*$Header: /usr/local/qtopiaroot/Source/qtopia-phone-2.1.1/src/3rdparty/applications/kphone/gsm/config.h,v 1.1 2005/09/05 10:57:17 pankaj Exp $*/

#ifndef	CONFIG_H
#define	CONFIG_H

//efine	SIGHANDLER_T	int 		/* signal handlers are void	*/
//efine HAS_SYSV_SIGNAL	1		/* sigs not blocked/reset?	*/

#define	HAS_STDLIB_H	1		/* /usr/include/stdlib.h	*/
//efine	HAS_LIMITS_H	1		/* /usr/include/limits.h	*/
#define	HAS_FCNTL_H	1		/* /usr/include/fcntl.h		*/
//efine	HAS_ERRNO_DECL	1		/* errno.h declares errno	*/

#define	HAS_FSTAT 	1		/* fstat syscall		*/
#define	HAS_FCHMOD 	1		/* fchmod syscall		*/
#define	HAS_CHMOD 	1		/* chmod syscall		*/
#define	HAS_FCHOWN 	1		/* fchown syscall		*/
#define	HAS_CHOWN 	1		/* chown syscall		*/
//efine	HAS__FSETMODE 	1		/* _fsetmode -- set file mode	*/

#define	HAS_STRING_H 	1		/* /usr/include/string.h 	*/
//efine	HAS_STRINGS_H	1		/* /usr/include/strings.h 	*/

#define	HAS_UNISTD_H	1		/* /usr/include/unistd.h	*/
#define	HAS_UTIME	1		/* POSIX utime(path, times)	*/
//efine	HAS_UTIMES	1		/* use utimes()	syscall instead	*/
#define	HAS_UTIME_H	1		/* UTIME header file		*/
//efine	HAS_UTIMBUF	1		/* struct utimbuf		*/
//efine	HAS_UTIMEUSEC   1		/* microseconds in utimbuf?	*/

#endif	/* CONFIG_H */
