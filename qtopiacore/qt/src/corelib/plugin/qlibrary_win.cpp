/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "qlibrary_p.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"

#include "qt_windows.h"

extern QString qt_error_string(int code);

bool QLibraryPrivate::load_sys()
{
    QString attempt = fileName;

    //avoid 'Bad Image' message box
    UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    QT_WA({
        pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
    } , {
        pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
    });

    
    if (pluginState != IsAPlugin) {
        if (!pHnd && ::GetLastError() == ERROR_MOD_NOT_FOUND) {
            attempt += QLatin1String(".dll");
            QT_WA({
                pHnd = LoadLibraryW((TCHAR*)attempt.utf16());
            } , {
                pHnd = LoadLibraryA(QFile::encodeName(attempt).data());
            });
        }
    }

    SetErrorMode(oldmode);
    if (!pHnd) {
        errorString = QLibrary::tr("QLibrary::load_sys: Cannot load %1 (%2)").arg(fileName).arg(::qt_error_string());
    }
    if (pHnd) {
        errorString.clear();
        QT_WA({
            TCHAR buffer[MAX_PATH + 1];
            ::GetModuleFileNameW(pHnd, buffer, MAX_PATH);
            attempt = QString::fromUtf16(reinterpret_cast<const ushort *>(&buffer));
        }, {
            char buffer[MAX_PATH + 1];
            ::GetModuleFileNameA(pHnd, buffer, MAX_PATH);
            attempt = QString::fromLocal8Bit(buffer);
        });
        const QDir dir =  QFileInfo(fileName).dir();
        const QString realfilename = attempt.mid(attempt.lastIndexOf(QLatin1Char('\\')) + 1);
        if (dir.path() == QLatin1String("."))
            qualifiedFileName = realfilename;
        else
            qualifiedFileName = dir.filePath(realfilename);
    }
    return (pHnd != 0);
}

bool QLibraryPrivate::unload_sys()
{
    if (!FreeLibrary(pHnd)) {
        errorString = QLibrary::tr("QLibrary::unload_sys: Cannot unload %1 (%2)").arg(fileName).arg(::qt_error_string());
        return false;
    }
    errorString.clear();
    return true;
}

void* QLibraryPrivate::resolve_sys(const char* symbol)
{
#ifdef Q_OS_TEMP
    void* address = (void*)GetProcAddress(pHnd, (const wchar_t*)QString(symbol).ucs2());
#else
    void* address = (void*)GetProcAddress(pHnd, symbol);
#endif
    if (!address) {
        errorString = QLibrary::tr("QLibrary::resolve_sys: Symbol \"%1\" undefined in %2 (%3)").arg(
            QString::fromAscii(symbol)).arg(fileName).arg(::qt_error_string());
    } else {
        errorString.clear();
    }
    return address;
}

