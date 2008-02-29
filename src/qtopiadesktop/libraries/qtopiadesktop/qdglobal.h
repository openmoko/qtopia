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
#ifndef QPE_GLOBAL_H
#define QPE_GLOBAL_H

#include <qglobal.h>
#include <qplugin.h>

// The _EXPORT macros...

#ifdef Q_OS_WIN32
# if defined(QTOPIADESKTOP_MAKEDLL)
#  define QD_EXPORT __declspec(dllexport)
# elif defined(QTOPIADESKTOP_DLL)
#  define QD_EXPORT __declspec(dllimport)
# endif
#endif
#ifndef QD_EXPORT
#define QD_EXPORT
#endif

#endif
