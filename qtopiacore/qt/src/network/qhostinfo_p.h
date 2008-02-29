/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
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

#ifndef QHOSTINFO_P_H
#define QHOSTINFO_P_H

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

#include "QtCore/qcoreapplication.h"
#include "private/qcoreapplication_p.h"
#include "QtNetwork/qhostinfo.h"
#include "QtCore/qmutex.h"
#include "QtCore/qwaitcondition.h"
#include "QtCore/qobject.h"
#include "QtCore/qpointer.h"

#if !defined QT_NO_THREAD
#include "QtCore/qthread.h"
#    define QHostInfoAgentBase QThread
#else
#    define QHostInfoAgentBase QObject
#endif

class QHostInfoResult : public QObject
{
    Q_OBJECT
public:
    inline void emitResultsReady(const QHostInfo &info)
    {
        emit resultsReady(info);
    }

    int lookupId;
Q_SIGNALS:
    void resultsReady(const QHostInfo &info);
};

struct QHostInfoQuery
{
    inline QHostInfoQuery() : object(0) {}
    inline ~QHostInfoQuery() { delete object; }
    inline QHostInfoQuery(const QString &name, QHostInfoResult *result)
        : hostName(name), object(result) {}

    QString hostName;
    QHostInfoResult *object;
};

class QHostInfoAgent : public QHostInfoAgentBase
{
    Q_OBJECT
public:
    inline QHostInfoAgent()
    {
        connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
        connect(QCoreApplication::instance(), SIGNAL(destroyed(QObject *)), this, SLOT(cleanup()));
        moveToThread(QCoreApplicationPrivate::mainThread());
        quit = false;
        pendingQueryId = -1;
    }
    inline ~QHostInfoAgent()
    { cleanup(); }

    void run();
    static QHostInfo fromName(const QString &hostName);

    inline void addHostName(const QString &name, QHostInfoResult *result)
    {
        QMutexLocker locker(&mutex);
        queries << new QHostInfoQuery(name, result);
        cond.wakeOne();
    }

    inline void abortLookup(int id)
    {
        QMutexLocker locker(&mutex);
        for (int i = 0; i < queries.size(); ++i) {
            QHostInfoResult *result = queries.at(i)->object;
            if (result->lookupId == id) {
                result->disconnect();
                delete queries.takeAt(i);
                return;
            }
        }
        if (pendingQueryId == id)
            pendingQueryId = -1;
    }

public Q_SLOTS:
    inline void cleanup()
    {
        {
            QMutexLocker locker(&mutex);
            qDeleteAll(queries);
            queries.clear();
            quit = true;
            cond.wakeOne();
        }
#ifndef QT_NO_THREAD
        wait();
#endif
    }

private:
    QList<QHostInfoQuery *> queries;
    QMutex mutex;
    QWaitCondition cond;
    bool quit;
    int pendingQueryId;
};

class QHostInfoPrivate
{
public:
    inline QHostInfoPrivate()
        : err(QHostInfo::NoError),
          errorStr(QLatin1String(QT_TRANSLATE_NOOP("QHostInfo", "Unknown error")))
    {
    }

    QHostInfo::HostInfoError err;
    QString errorStr;
    QList<QHostAddress> addrs;
    QString hostName;
    int lookupId;
};

#endif // QHOSTINFO_P_H
