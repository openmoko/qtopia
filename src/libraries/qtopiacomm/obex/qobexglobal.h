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
#ifndef __QOBEXGLOBAL_H__
#define __QOBEXGLOBAL_H__


#include <qglobal.h>
#include <qplugin.h>

#ifdef QTOPIA_OBEX
#   include <qtopiaglobal.h>
#else
#   ifndef QTOPIA_AUTOTEST_EXPORT
#       define QTOPIA_AUTOTEST_EXPORT
#   endif
#endif


#if defined(QT_VISIBILITY_AVAILABLE)
#   define QTOPIAOBEX_EXPORT __attribute__((visibility("default")))
#else
#   define QTOPIAOBEX_EXPORT
#endif


#endif
