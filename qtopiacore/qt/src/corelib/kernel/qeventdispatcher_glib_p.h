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

#ifndef QEVENTDISPATCHER_GLIB_P_H
#define QEVENTDISPATCHER_GLIB_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "qabstracteventdispatcher.h"
#include "qabstracteventdispatcher_p.h"

#include <QtCore/qhash.h>

class QEventDispatcherGlibPrivate;

class Q_CORE_EXPORT QEventDispatcherGlib : public QAbstractEventDispatcher
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QEventDispatcherGlib)

public:
    explicit QEventDispatcherGlib(QObject *parent = 0);
    ~QEventDispatcherGlib();

    bool processEvents(QEventLoop::ProcessEventsFlags flags);
    bool hasPendingEvents();

    void registerSocketNotifier(QSocketNotifier *socketNotifier);
    void unregisterSocketNotifier(QSocketNotifier *socketNotifier);

    void registerTimer(int timerId, int interval, QObject *object);
    bool unregisterTimer(int timerId);
    bool unregisterTimers(QObject *object);
    QList<TimerInfo> registeredTimers(QObject *object) const;

    void wakeUp();
    void interrupt();
    void flush();

protected:
    QEventDispatcherGlib(QEventDispatcherGlibPrivate &dd, QObject *parent);
};

struct GPostEventSource;
struct GSocketNotifierSource;
struct GTimerSource;
typedef struct _GMainContext GMainContext;

class Q_CORE_EXPORT QEventDispatcherGlibPrivate : public QAbstractEventDispatcherPrivate
{

public:
    QEventDispatcherGlibPrivate();
    GMainContext *mainContext;
    GPostEventSource *postEventSource;
    GSocketNotifierSource *socketNotifierSource;
    GTimerSource *timerSource;
};

#endif // QEVENTDISPATCHER_GLIB_P_H
