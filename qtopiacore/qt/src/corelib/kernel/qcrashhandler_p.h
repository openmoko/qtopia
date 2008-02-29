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

#ifndef QCRASHHANDLER_P_H
#define QCRASHHANDLER_P_H

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

typedef void (*QtCrashHandler)();

class Q_CORE_EXPORT QSegfaultHandler
{
    friend void qt_signal_handler(int);
    static QtCrashHandler callback;
public:
    static void initialize(char **, int);

    inline static void installCrashHandler(QtCrashHandler h) { callback = h; }
    inline static QtCrashHandler crashHandler() { return callback; }

private:
};

#endif // QCRASHHANDLER_P_H
