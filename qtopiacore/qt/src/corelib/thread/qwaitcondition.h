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

#ifndef QWAITCONDITION_H
#define QWAITCONDITION_H

#include <QtCore/qglobal.h>

#include <limits.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#ifndef QT_NO_THREAD

class QWaitConditionPrivate;
class QMutex;

class Q_CORE_EXPORT QWaitCondition
{
public:
    QWaitCondition();
    ~QWaitCondition();

    bool wait(QMutex *mutex, unsigned long time = ULONG_MAX);

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

QT_END_HEADER

#endif // QWAITCONDITION_H
