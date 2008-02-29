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

#ifndef QDESIGNER_COMPONENTS_GLOBAL_H
#define QDESIGNER_COMPONENTS_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#define QDESIGNER_COMPONENTS_EXTERN Q_DECL_EXPORT
#define QDESIGNER_COMPONENTS_IMPORT Q_DECL_IMPORT

#ifdef QT_DESIGNER_STATIC
#  define QDESIGNER_COMPONENTS_EXPORT
#elif defined(QDESIGNER_COMPONENTS_LIBRARY)
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_EXTERN
#else
#  define QDESIGNER_COMPONENTS_EXPORT QDESIGNER_COMPONENTS_IMPORT
#endif

QT_END_HEADER

#endif // QDESIGNER_COMPONENTS_GLOBAL_H
