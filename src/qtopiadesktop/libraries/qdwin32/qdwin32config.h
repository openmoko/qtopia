/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QDWIN32CONFIG_H
#define QDWIN32CONFIG_H

#include <qglobal.h>
#include <qplugin.h>

// The _EXPORT macros...

#ifdef Q_OS_WIN32
# if defined(QDWIN32_MAKEDLL)
#  define QDWIN32_EXPORT __declspec(dllexport)
# elif defined(QDWIN32_DLL)
#  define QDWIN32_EXPORT __declspec(dllimport)
# endif
#endif
#ifndef QDWIN32_EXPORT
#define QDWIN32_EXPORT
#endif

#endif
