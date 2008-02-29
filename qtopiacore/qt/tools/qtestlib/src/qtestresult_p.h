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

#ifndef QTESTRESULT_P_H
#define QTESTRESULT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtTest/qtest_global.h>

class QTestResultPrivate;
class QTestData;

class QTestResult
{
public:
    enum TestLocation { NoWhere = 0, DataFunc = 1, InitFunc = 2, Func = 3, CleanupFunc = 4 };

    static const char *currentTestObjectName();
    static bool currentTestFailed();
    static bool allDataPassed();
    static QTestData *currentTestData();
    static QTestData *currentGlobalTestData();
    static const char *currentTestFunction();
    static TestLocation currentTestLocation();
    static const char *currentDataTag();
    static const char *currentGlobalDataTag();
    static void finishedCurrentTestFunction();
    static void reset();

    static int passCount();
    static int failCount();
    static int skipCount();

    static void ignoreMessage(QtMsgType type, const char *msg);

    static void addFailure(const char *message, const char *file, int line);
    static bool compare(bool success, const char *msg, const char *file, int line);
    static bool compare(bool success, const char *msg, char *val1, char *val2,
                        const char *actual, const char *expected, const char *file, int line);

    static void setCurrentGlobalTestData(QTestData *data);
    static void setCurrentTestData(QTestData *data);
    static void setCurrentTestFunction(const char *func);
    static void setCurrentTestLocation(TestLocation loc);
    static void setCurrentTestObject(const char *name);
    static void addSkip(const char *message, QTest::SkipMode mode,
                        const char *file, int line);
    static bool expectFail(const char *dataIndex, const char *comment,
                           QTest::TestFailMode mode, const char *file, int line);
    static bool verify(bool statement, const char *statementStr, const char *extraInfo,
                       const char *file, int line);
};

#endif
