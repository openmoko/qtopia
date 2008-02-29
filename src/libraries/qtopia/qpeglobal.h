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

#ifndef QPE_GLOBAL_H
#define QPE_GLOBAL_H

#include <qglobal.h>

#if ( defined(Q_OS_WIN32) || defined(Q_OS_WIN64) ) && defined(PALMTOPCENTER)
// #  if defined(QT_NODLL)
//#    undef QTOPIA_MAKEDLL
//#    undef QTOPIA_DLL
#  if defined(QTOPIA_MAKEDLL)	/* create a Qt DLL library */
#    if defined(QTOPIA_DLL)
#      undef QTOPIA_DLL
#    endif
#    define QTOPIA_EXPORT  __declspec(dllexport)
#    define QTOPIA_TEMPLATEEXTERN
#    define QTOPIA_TEMPLATEDLL
#    undef  QTOPIA_DISABLE_COPY	/* avoid unresolved externals */
#  elif defined(QTOPIA_DLL)		/* use a Qt DLL library */
#    define QTOPIA_EXPORT  __declspec(dllimport)
#    define QTOPIA_TEMPLATEEXTERN extern
#    define QTOPIA_TEMPLATEDLL
#    undef  QTOPIA_DISABLE_COPY	/* avoid unresolved externals */
#  endif
#else
#  undef QTOPIA_MAKEDLL		/* ignore these for other platforms */
#  undef QTOPIA_DLL
#endif

#ifndef QTOPIA_EXPORT
#  define QTOPIA_EXPORT
#endif

#endif
