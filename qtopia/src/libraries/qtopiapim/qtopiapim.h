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

#ifndef QTOPIAPIM_GLOBAL_H
#define QTOPIAPIM_GLOBAL_H

#include <qtopia/qpeglobal.h>

#if (defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) 
#  if defined(QTOPIAPIM_MAKEDLL)	/* create a Qtopia Pim DLL library */
#    if defined(QTOPIAPIM_DLL)
#      undef QTOPIAPIM_DLL
#    endif
#    define QTOPIAPIM_EXPORT  __declspec(dllexport)
#    define QTOPIAPIM_TEMPLATE_EXTERN
#    define QTOPIAPIM_TEMPLATEDLL
#    undef  QTOPIAPIM_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined(QTOPIAPIM_DLL) || defined (QT_DLL)		/* use a Qtopia Pim DLL library */
#    define QTOPIAPIM_EXPORT  __declspec(dllimport)
#    define QTOPIAPIM_TEMPLATE_EXTERN /*extern*/
#    define QTOPIAPIM_TEMPLATEDLL
#    undef  QTOPIAPIM_DISABLE_COPY	/* avoid unresolved externals */
#  endif

#else // (defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) 
#   undef QTOPIAPIM_MAKEDLL		/* ignore these for other platforms */
#   undef QTOPIAPIM_DLL
#endif // (defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) 

#ifndef QTOPIAPIM_EXPORT
#  define QTOPIAPIM_EXPORT
#endif

#endif
