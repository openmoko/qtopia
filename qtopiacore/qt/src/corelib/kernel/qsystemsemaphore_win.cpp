/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "qsystemsemaphore.h"
#include "qsystemsemaphore_p.h"
#include "qcoreapplication.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE
        
#ifndef QT_NO_SYSTEMSEMAPHORE

QSystemSemaphorePrivate::QSystemSemaphorePrivate() :
        semaphore(0), error(QSystemSemaphore::NoError)
{
}

void QSystemSemaphorePrivate::setErrorString(const QString &function)
{
    BOOL windowsError = GetLastError();
    if (windowsError == 0)
        return;

    switch (windowsError) {
    case ERROR_NO_SYSTEM_RESOURCES:
    case ERROR_NOT_ENOUGH_MEMORY:
        error = QSystemSemaphore::OutOfResources;
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: out of resources").arg(function);
        break;
    case ERROR_ACCESS_DENIED:
        error = QSystemSemaphore::PermissionDenied;
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: permission denied").arg(function);
        break;
    default:
        errorString = QCoreApplication::translate("QSystemSemaphore", "%1: unknown error %2").arg(function).arg(windowsError);
        error = QSystemSemaphore::UnknownError;
#if defined QSYSTEMSEMAPHORE_DEBUG
        qDebug() << errorString << "key" << key;
#endif
    }
}

HANDLE QSystemSemaphorePrivate::handle(QSystemSemaphore::AccessMode)
{
    // don't allow making handles on empty keys
    if (key.isEmpty())
        return 0;

    // Create it if it doesn't already exists.
    if (semaphore == 0) {
        QString safeName = makeKeyFileName();
        QT_WA({
            semaphore = CreateSemaphoreW(0, initialValue, MAXLONG, (TCHAR*)safeName.utf16());
        }, {
            semaphore = CreateSemaphoreA(0, initialValue, MAXLONG, safeName.toLocal8Bit().constData());
        });
        if (semaphore == NULL)
            setErrorString(QLatin1String("QSystemSemaphore::handle"));
    }

    return semaphore;
}

void QSystemSemaphorePrivate::cleanHandle()
{
    if (semaphore && !CloseHandle(semaphore)) {
#if defined QSYSTEMSEMAPHORE_DEBUG
        qDebug() << QLatin1String("QSystemSemaphorePrivate::CloseHandle: sem failed");
#endif
    }
    semaphore = 0;
}

bool QSystemSemaphorePrivate::modifySemaphore(int count)
{
    if (0 == handle())
        return false;

    if (count > 0) {
	if (0 == ReleaseSemaphore(semaphore, count, 0)) {
            setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
#if defined QSYSTEMSEMAPHORE_DEBUG
            qDebug() << QLatin1String("QSystemSemaphore::modifySemaphore ReleaseSemaphore failed");
#endif
            return false;
        }
    } else {
        if (WAIT_OBJECT_0 != WaitForSingleObject(semaphore, INFINITE)) {
            setErrorString(QLatin1String("QSystemSemaphore::modifySemaphore"));
#if defined QSYSTEMSEMAPHORE_DEBUG
            qDebug() << QLatin1String("QSystemSemaphore::modifySemaphore WaitForSingleObject failed");
#endif
            return false;
        }
    }

    return true;
}

#endif //QT_NO_SYSTEMSEMAPHORE

QT_END_NAMESPACE
