/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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
