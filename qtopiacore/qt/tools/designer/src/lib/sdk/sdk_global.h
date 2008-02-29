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

#ifndef SDK_GLOBAL_H
#define SDK_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#define QDESIGNER_SDK_EXTERN Q_DECL_EXPORT
#define QDESIGNER_SDK_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_SDK_EXPORT
#elif defined(QDESIGNER_SDK_LIBRARY)
#  define QDESIGNER_SDK_EXPORT QDESIGNER_SDK_EXTERN
#else
#  define QDESIGNER_SDK_EXPORT QDESIGNER_SDK_IMPORT
#endif

QT_END_HEADER

#endif // SDK_GLOBAL_H
