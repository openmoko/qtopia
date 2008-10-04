/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the window classes of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
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

QT_BEGIN_NAMESPACE

#if defined(Q_OS_WINCE)
extern void __cdecl qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#else
extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);
#endif

QT_END_NAMESPACE

QT_USE_NAMESPACE


#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
#ifdef Q_OS_WINCE
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

#ifdef Q_OS_WINCE
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

#if defined(Q_OS_WINCE)
    TCHAR appName[256];
    GetModuleFileName(0, appName, 255);
    cmdParam = QString(QLatin1String("\"%1\" ")).arg(QString::fromUtf16((const unsigned short *)appName)).toLocal8Bit() + cmdParam;
#endif

    int argc = 0;
    QVector<char *> argv(8);
    qWinMain(instance, prevInstance, cmdParam.data(), cmdShow, argc, argv);

#if defined(Q_OS_WINCE)
    TCHAR uniqueAppID[256];
    GetModuleFileName(0, uniqueAppID, 255);
    QString uid = QString::fromUtf16((const unsigned short *)uniqueAppID).toLower().replace(QString(QLatin1String("\\")), QString(QLatin1String("_")));

    // If there exists an other instance of this application
    // it will be the owner of a mutex with the unique ID.
    HANDLE mutex = CreateMutex(NULL, TRUE, (LPCWSTR)uid.utf16());
    if (mutex && ERROR_ALREADY_EXISTS == GetLastError()) {
        CloseHandle(mutex);

        // The app is already running, so we use the unique
        // ID to create a unique messageNo, which is used
        // as the registered class name for the windows
        // created. Set the first instance's window to the
        // foreground, else just terminate.
        // Use bitwise 0x01 OR to reactivate window state if
        // it was hidden
        UINT msgNo = RegisterWindowMessage((LPCWSTR)uid.utf16());
        HWND aHwnd = FindWindow((LPCWSTR)QString::number(msgNo).utf16(), 0);
        if (aHwnd)
            SetForegroundWindow((HWND)(((ULONG)aHwnd) | 0x01));
        return 0;
    }
#endif // Q_OS_WINCE

    int result = main(argc, argv.data());
#if defined(Q_OS_WINCE)
    CloseHandle(mutex);
#endif
    return result;
}
