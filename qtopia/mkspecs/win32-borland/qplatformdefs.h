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

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

// Get Qt defines/settings

#include "qglobal.h"
#define Q_FS_FAT

#define _POSIX_
#include <limits.h>
#undef _POSIX_

#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dos.h>
#include <stdlib.h>
#include <windows.h>

#if __BORLANDC__ >= 0x550
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_STATBUF		struct stati64		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stati64		// non-ANSI defs
#define QT_STAT			::stati64
#define QT_FSTAT		::fstati64
#else
#define QT_STATBUF		struct stat		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stat		// non-ANSI defs
#define QT_STAT			::stat
#define QT_FSTAT		::fstat
#endif
# define QT_STAT_REG		_S_IFREG
# define QT_STAT_DIR		_S_IFDIR
# define QT_STAT_MASK		_S_IFMT
# if defined(_S_IFLNK)
#  define QT_STAT_LNK		_S_IFLNK
# endif
# define QT_FILENO		_fileno
# define QT_OPEN		::open
# define QT_CLOSE		::_close
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_LSEEK		::_lseeki64
#define QT_TSTAT		::_tstati64
#else
#define QT_LSEEK		::_lseek
#define QT_TSTAT		::_tstat
#endif
# define QT_READ		::_read
# define QT_WRITE		::_write
# define QT_ACCESS		::_access
# define QT_GETCWD		::_getcwd
# define QT_CHDIR		::chdir
# define QT_MKDIR		::_mkdir
# define QT_RMDIR		::_rmdir
# define QT_OPEN_RDONLY		_O_RDONLY
# define QT_OPEN_WRONLY		_O_WRONLY
# define QT_OPEN_RDWR		_O_RDWR
# define QT_OPEN_CREAT		_O_CREAT
# define QT_OPEN_TRUNC		_O_TRUNC
# define QT_OPEN_APPEND		_O_APPEND
# if defined(O_TEXT)
#  define QT_OPEN_TEXT		_O_TEXT
#  define QT_OPEN_BINARY	_O_BINARY
# endif
#else						// all other systems
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_STATBUF		struct stati64		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct stati64		// non-ANSI defs
#define QT_STAT			::stati64
#define QT_FSTAT		::fstati64
#else
#define QT_STATBUF		struct stat		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct stat		// non-ANSI defs
#define QT_STAT			::stat
#define QT_FSTAT		::fstat
#endif
# define QT_STAT_REG		S_IFREG
# define QT_STAT_DIR		S_IFDIR
# define QT_STAT_MASK		S_IFMT
# if defined(S_IFLNK)
#  define QT_STAT_LNK		S_IFLNK
# endif
# define QT_FILENO		fileno
# define QT_OPEN		::open
# define QT_CLOSE		::close
#ifdef QT_LARGE_FILE_SUPPORT
#define QT_LSEEK		::lseeki64
#define QT_TSTAT		::tstati64
#else
#define QT_LSEEK		::lseek
#define QT_TSTAT		::tstat
#endif
# define QT_READ		::read
# define QT_WRITE		::write
# define QT_ACCESS		::access
# if defined(Q_OS_OS2EMX)
// This is documented in the un*x to OS/2-EMX Porting FAQ:
// 	http://homepages.tu-darmstadt.de/~st002279/os2/porting.html
#  define QT_GETCWD		::_getcwd2
#  define QT_CHDIR		::_chdir2
# else
#  define QT_GETCWD		::getcwd
#  define QT_CHDIR		::chdir
# endif
# define QT_MKDIR		::mkdir
# define QT_RMDIR		::rmdir
# define QT_OPEN_RDONLY		O_RDONLY
# define QT_OPEN_WRONLY		O_WRONLY
# define QT_OPEN_RDWR		O_RDWR
# define QT_OPEN_CREAT		O_CREAT
# define QT_OPEN_TRUNC		O_TRUNC
# define QT_OPEN_APPEND		O_APPEND
# if defined(O_TEXT)
#  define QT_OPEN_TEXT		O_TEXT
#  define QT_OPEN_BINARY	O_BINARY
# endif
#endif

#define QT_SIGNAL_ARGS		int

#define QT_VSNPRINTF		::_vsnprintf
#define QT_SNPRINTF		::_snprintf

# define F_OK	0
# define X_OK	1
# define W_OK	2
# define R_OK	4


#endif // QPLATFORMDEFS_H
