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

#ifndef QPE_GLOBAL_H
#define QPE_GLOBAL_H

#include <qglobal.h>

#if defined(QWS) && defined(_WS_QWS_) && !defined(Q_WS_QWS)
#   define Q_WS_QWS
#endif

#if defined(_OS_WIN32_) && !defined(Q_OS_WIN32)
#   define Q_OS_WIN32
#   ifndef Q_WS_WIN32
#	define Q_WS_WIN32    
#   endif    
#endif
#if defined(_OS_UNIX_) && !defined(Q_OS_UNIX)
#   define Q_OS_UNIX
#   ifndef Q_WS_UNIX
#	define Q_WS_UNIX    
#   endif    
#endif

#if ( defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) 
// #  if defined(QT_NODLL)
//#    undef QTOPIA_MAKEDLL
//#    undef QTOPIA_DLL
#  if defined(QTOPIA_MAKEDLL)	/* create a Qt DLL library */
#    if defined(QTOPIA_DLL)
#      undef QTOPIA_DLL
#    endif
#    define QTOPIA_EXPORT  __declspec(dllexport)
#    define QTOPIA_TEMPLATE_EXTERN
#    define QTOPIA_TEMPLATEDLL
#    undef  QTOPIA_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined(QTOPIA_DLL) || defined (QT_DLL)		/* use a Qt DLL library */
#    define QTOPIA_EXPORT  __declspec(dllimport)
#    define QTOPIA_TEMPLATE_EXTERN /*extern*/
#    define QTOPIA_TEMPLATEDLL
#    undef  QTOPIA_DISABLE_COPY	/* avoid unresolved externals */
#  endif

#  if defined(QPC_MAKEDLL)	/* create a Qt DLL library for qtopia deskop*/
#    if defined(QPC_DLL)
#      undef QPC_DLL
#    endif
#    define QPC_EXPORT  __declspec(dllexport)
#    define QPC_TEMPLATE_EXTERN
#    define QPC_TEMPLATEDLL
#    undef  QPC_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined (QPC_DLL) || defined(QT_DLL) 		/* use a Qt DLL library */
#    define QPC_EXPORT  __declspec(dllimport)
#    define QPC_TEMPLATE_EXTERN /*extern*/
#    define QPC_TEMPLATEDLL
#    undef  QPC_DISABLE_COPY	/* avoid unresolved externals */
#  endif

#   ifdef QWS
#	ifndef Q_WS_QWS 
#	    define Q_WS_QWS
#	endif
#	ifndef _WS_QWS
#	    define _WS_QWS      
#	endif
#  endif

#else
#   undef QTOPIA_MAKEDLL		// ignore these for other platforms
#   undef QTOPIA_DLL
#endif


#ifndef QTOPIA_EXPORT
#  define QTOPIA_EXPORT
#endif

#ifndef QPC_EXPORT
#  define QPC_EXPORT
#endif


// These are the additional Qt templates we export
#if defined(QTOPIA_TEMPLATE_EXTERN)
#include <qdict.h>
#if QT_VERSION >= 0x030000
#include <qlibrary.h>
#else
#include <qtopia/qlibrary.h>
#endif

#include <qguardedptr.h>
#include <qwidget.h>
#include <qmap.h>
#include <qcanvas.h>
#define QTOPIA_TEMPLATE_EXPORT QTOPIA_TEMPLATE_EXTERN template class QTOPIA_EXPORT
#if (QT_VERSION >= 0x030000) && (QT_VERSION < 0x030200)
QTOPIA_TEMPLATE_EXPORT QDict<QLibrary>;
QTOPIA_TEMPLATE_EXPORT QGuardedPtr<QWidget>;
QTOPIA_TEMPLATE_EXPORT QMap<int, int>;
QTOPIA_TEMPLATE_EXPORT QMap<int, bool>;
QTOPIA_TEMPLATE_EXPORT QMap<QString, int>;
QTOPIA_TEMPLATE_EXPORT QMap<int, QString>;
#endif
#endif


#endif
