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

#ifndef QWAITCONDITION_H
#define QWAITCONDITION_H

#include <QtCore/qglobal.h>

#include <limits.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Core)

#ifndef QT_NO_THREAD

class QWaitConditionPrivate;
class QMutex;
class QReadWriteLock;

class Q_CORE_EXPORT QWaitCondition
{
public:
    QWaitCondition();
    ~QWaitCondition();

    bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);
    bool wait(QReadWriteLock *readWriteLock, unsigned long time = ULONG_MAX);

    void wakeOne();
    void wakeAll();

private:
    Q_DISABLE_COPY(QWaitCondition)

    QWaitConditionPrivate * d;
};

#else

class QMutex;
class Q_CORE_EXPORT QWaitCondition
{
public:
    QWaitCondition() {}
    ~QWaitCondition() {}

    bool wait(QMutex *mutex, unsigned long time = ULONG_MAX)
    {
	Q_UNUSED(mutex);
	Q_UNUSED(time);
	return true;
    }

    void wakeOne() {}
    void wakeAll() {}
};

#endif // QT_NO_THREAD

QT_END_NAMESPACE

QT_END_HEADER

#endif // QWAITCONDITION_H
