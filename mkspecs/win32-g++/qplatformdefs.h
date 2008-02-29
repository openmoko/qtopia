/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

// Get Qt defines/settings

#include "qglobal.h"

#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <windows.h>

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT-0 < 0x0500)
typedef enum {
    NameUnknown		  = 0, 
    NameFullyQualifiedDN  = 1, 
    NameSamCompatible	  = 2, 
    NameDisplay		  = 3, 
    NameUniqueId	  = 6, 
    NameCanonical	  = 7, 
    NameUserPrincipal	  = 8, 
    NameCanonicalEx	  = 9, 
    NameServicePrincipal  = 10, 
    NameDnsDomain	  = 12
} EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT;
#endif

#define Q_FS_FAT
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_STATBUF		struct _stati64		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stati64		// non-ANSI defs
#define QT_STAT			::_stati64
#define QT_FSTAT		::_fstati64
#else
#define QT_STATBUF		struct _stat		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stat		// non-ANSI defs
#define QT_STAT			::_stat
#define QT_FSTAT		::_fstat
#endif
#define QT_STAT_REG		_S_IFREG
#define QT_STAT_DIR		_S_IFDIR
#define QT_STAT_MASK		_S_IFMT
#if defined(_S_IFLNK)
#  define QT_STAT_LNK		_S_IFLNK
#endif
#define QT_FILENO		_fileno
#define QT_OPEN			::_open
#define QT_CLOSE		::_close
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_LSEEK		::_lseeki64
#define QT_TSTAT		::_tstati64
#else
#define QT_LSEEK		::_lseek
#define QT_TSTAT		::_tstat
#endif
#define QT_READ			::_read
#define QT_WRITE		::_write
#define QT_ACCESS		::_access
#define QT_GETCWD		::_getcwd
#define QT_CHDIR		::_chdir
#define QT_MKDIR		::_mkdir
#define QT_RMDIR		::_rmdir
#define QT_OPEN_RDONLY		_O_RDONLY
#define QT_OPEN_WRONLY		_O_WRONLY
#define QT_OPEN_RDWR		_O_RDWR
#define QT_OPEN_CREAT		_O_CREAT
#define QT_OPEN_TRUNC		_O_TRUNC
#define QT_OPEN_APPEND		_O_APPEND
#if defined(O_TEXT)
# define QT_OPEN_TEXT		_O_TEXT
# define QT_OPEN_BINARY		_O_BINARY
#endif

#define QT_SIGNAL_ARGS		int

#define QT_VSNPRINTF		::_vsnprintf
#define QT_SNPRINTF		::_snprintf

# define F_OK	0
# define X_OK	1
# define W_OK	2
# define R_OK	4


#endif // QPLATFORMDEFS_H
