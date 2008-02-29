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
 * mad - MPEG audio decoder
 * Copyright (C) 2000-2001 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: libmad_version.h,v 1.1 2001/09/14 07:23:42 jryland Exp $
 */

# ifndef LIBMAD_VERSION_H
# define LIBMAD_VERSION_H

# define MAD_VERSION_MAJOR	0
# define MAD_VERSION_MINOR	13
# define MAD_VERSION_PATCH	0
# define MAD_VERSION_EXTRA	" (beta)" // No tr

# define MAD_VERSION_STRINGIZE(str)	#str
# define MAD_VERSION_STRING(num)	MAD_VERSION_STRINGIZE(num)

# define MAD_VERSION		MAD_VERSION_STRING(MAD_VERSION_MAJOR) "."  \
				MAD_VERSION_STRING(MAD_VERSION_MINOR) "."  \
				MAD_VERSION_STRING(MAD_VERSION_PATCH)  \
				MAD_VERSION_EXTRA

# define MAD_PUBLISHYEAR	"2000-2001"
# define MAD_AUTHOR		"Robert Leslie" // No tr
# define MAD_EMAIL		"rob@mars.org"

extern char const mad_version[];
extern char const mad_copyright[];
extern char const mad_author[];
extern char const mad_build[];

# endif
