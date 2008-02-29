/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the window classes of the Qt Toolkit.
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

#include "qt_windows.h"
#include "qbytearray.h"
#include "qstring.h"
#include "qvector.h"

/*
  This file contains the code in the qtmain library for Windows.
  qtmain contains the Windows startup code and is required for
  linking to the Qt DLL.

  When a Windows application starts, the WinMain function is
  invoked. WinMain calls qWinMain in the Qt DLL/library, which
  initializes Qt.
*/

#if defined(Q_OS_TEMP)
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#endif

#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
#ifdef Q_OS_TEMP
extern "C" int __cdecl main(int, char **);
#else
extern "C" int main(int, char **);
#endif
#endif

/*
  WinMain() - Initializes Windows and calls user's startup function main().
  NOTE: WinMain() won't be called if the application was linked as a "console"
  application.
*/

#ifdef Q_OS_TEMP
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPWSTR /*wCmdParam*/, int cmdShow)
#else
extern "C"
int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR /*cmdParamarg*/, int cmdShow)
#endif
{
    QByteArray cmdParam;
    QT_WA({
        LPTSTR cmdline = GetCommandLineW();
        cmdParam = QString::fromUtf16((const unsigned short *)cmdline).toLocal8Bit();
    }, {
        cmdParam = GetCommandLineA();
    });

    int argc = 0;
    QVector<char *> argv(8);
    qWinMain(instance, prevInstance, cmdParam.data(), cmdShow, argc, argv);

#ifdef Q_OS_TEMP
    TCHAR uniqueAppID[256];
    GetModuleFileName(0, uniqueAppID, 255);
    QString uid = QString::fromUcs2(uniqueAppID).lower().remove('\\');

    // If there exists an other instance of this application
    // it will be the owner of a mutex with the unique ID.
    HANDLE mutex = CreateMutex(NULL, TRUE, uid.ucs2());
    if (mutex && ERROR_ALREADY_EXISTS == GetLastError()) {
        CloseHandle(mutex);

        // The app is already running, so we use the unique
        // ID to create a unique messageNo, which is used
        // as the registered class name for the windows
        // created. Set the first instance's window to the
        // foreground, else just terminate.
        UINT msgNo = RegisterWindowMessage(uid.ucs2());
        HWND aHwnd = FindWindow(QString::number(msgNo).ucs2(), 0);
        if (aHwnd)
            SetForegroundWindow(aHwnd);
        return 0;
    }
#endif // Q_OS_TEMP

    int result = main(argc, argv.data());
    return result;
}
