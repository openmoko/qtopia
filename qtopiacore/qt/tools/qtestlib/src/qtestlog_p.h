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

#ifndef QTESTLOG_P_H
#define QTESTLOG_P_H

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

class QTestLog
{
public:
    enum LogMode { Plain = 0, XML, LightXML };

    static void enterTestFunction(const char* function);
    static void leaveTestFunction();

    static void addPass(const char *msg);
    static void addFail(const char *msg, const char *file, int line);
    static void addXFail(const char *msg, const char *file, int line);
    static void addXPass(const char *msg, const char *file, int line);
    static void addSkip(const char *msg, QTest::SkipMode mode,
                        const char *file, int line);
    static void addIgnoreMessage(QtMsgType type, const char *msg);
    static int unhandledIgnoreMessages();
    static void printUnhandledIgnoreMessages();

    static void warn(const char *msg);
    static void info(const char *msg, const char *file, int line);

    static void startLogging();
    static void stopLogging();

    static void setLogMode(LogMode mode);
    static LogMode logMode();

    static void setVerboseLevel(int level);
    static int verboseLevel();

    static void redirectOutput(const char *fileName);
    static const char *outputFileName();

    static void setMaxWarnings(int max);

private:
    QTestLog();
    ~QTestLog();
};

#endif
