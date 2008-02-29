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

#ifndef QTEST_GLOBAL_H
#define QTEST_GLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

#ifdef QTEST_EMBED
# define Q_TESTLIB_EXPORT
#elif !defined(QT_SHARED)
# define Q_TESTLIB_EXPORT
#else
# ifdef QTESTLIB_MAKEDLL
#  define Q_TESTLIB_EXPORT Q_DECL_EXPORT
# else
#  define Q_TESTLIB_EXPORT Q_DECL_IMPORT
# endif
#endif

#if (defined (Q_CC_MSVC) && _MSC_VER < 1310) || defined (Q_CC_SUN) || defined (Q_CC_XLC) || (defined (Q_CC_GNU) && (__GNUC__ - 0 < 3))
# define QTEST_NO_SPECIALIZATIONS
#endif

#if (defined Q_CC_HPACC) && (defined __ia64)
# ifdef Q_TESTLIB_EXPORT
#  undef Q_TESTLIB_EXPORT
# endif
# define Q_TESTLIB_EXPORT
#endif

#define QTEST_VERSION     0x040302
#define QTEST_VERSION_STR "4.3.2"

namespace QTest
{
    enum SkipMode { SkipSingle = 1, SkipAll = 2 };
    enum TestFailMode { Abort = 1, Continue = 2 };

    int Q_TESTLIB_EXPORT qt_snprintf(char *str, int size, const char *format, ...);
}

QT_END_HEADER

#endif
