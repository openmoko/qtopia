/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BUDDYEDITOR_GLOBAL_H
#define BUDDYEDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef Q_OS_WIN
#ifdef QT_BUDDYEDITOR_LIBRARY
# define QT_BUDDYEDITOR_EXPORT
#else
# define QT_BUDDYEDITOR_EXPORT
#endif
#else
#define QT_BUDDYEDITOR_EXPORT
#endif

#endif // BUDDYEDITOR_GLOBAL_H
