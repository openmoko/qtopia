/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TASKMENU_GLOBAL_H
#define TASKMENU_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_TASKMENU_LIBRARY
# define QT_TASKMENU_EXPORT
#else
# define QT_TASKMENU_EXPORT
#endif
#else
#define QT_TASKMENU_EXPORT
#endif

#endif // TASKMENU_GLOBAL_H
