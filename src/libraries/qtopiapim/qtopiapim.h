/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTOPIAPIM_GLOBAL_H
#define QTOPIAPIM_GLOBAL_H

#include <qtopia/qpeglobal.h>

#if (defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) 

#  if defined(QTOPIAPIM_MAKEDLL)	/* create a Qt DLL library for qtopia */
#    if defined(QTOPIAPIM_DLL)
#      undef QTOPIAPIM_DLL
#    endif
#    define QTOPIAPIM_EXPORT  __declspec(dllexport)
#    define QTOPIAPIM_TEMPLATE_EXTERN
#    define QTOPIAPIM_TEMPLATEDLL
#    undef  QTOPIAPIM_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined(QTOPIAPIM_DLL) || defined (QT_DLL)		/* use a Qt DLL library */
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
