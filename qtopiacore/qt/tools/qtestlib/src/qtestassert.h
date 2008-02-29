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

#ifndef QTESTASSERT_H
#define QTESTASSERT_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#define QTEST_ASSERT(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)

#define QTEST_ASSERT_X(cond, where, what) do {if(!(cond))qt_assert_x(where, what,__FILE__,__LINE__);} while (0)

QT_END_HEADER

#endif
