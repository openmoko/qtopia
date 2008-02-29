/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEST_GUI_H
#define QTEST_GUI_H

#include <QtTest/qtestassert.h>
#include <QtTest/qtest.h>
#include <QtTest/qtestevent.h>
#include <QtTest/qtestmouse.h>
#include <QtTest/qtestkeyboard.h>

#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

QT_BEGIN_HEADER

namespace QTest
{

template<>
inline bool qCompare(QIcon const &t1, QIcon const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    QTEST_ASSERT(sizeof(QIcon) == sizeof(void *));
    return qCompare<void *>(*reinterpret_cast<void * const *>(&t1),
                   *reinterpret_cast<void * const *>(&t2), actual, expected, file, line);
}

template<>
inline bool qCompare(QPixmap const &t1, QPixmap const &t2, const char *actual, const char *expected,
                    const char *file, int line)
{
    return qCompare(t1.toImage(), t2.toImage(), actual, expected, file, line);
}

}

/* compatibility */

inline static bool pixmapsAreEqual(const QPixmap *actual, const QPixmap *expected)
{
    if (!actual && !expected)
        return true;
    if (!actual || !expected)
        return false;
    if (actual->isNull() && expected->isNull())
        return true;
    if (actual->isNull() || expected->isNull() || actual->size() != expected->size())
        return false;
    return actual->toImage() == expected->toImage();
}

QT_END_HEADER

#endif
