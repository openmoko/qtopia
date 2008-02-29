/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qwssignalhandler_p.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo  *__buf;
};

class QWSSignalHandlerPrivate
{
public:
    Q_GLOBAL_STATIC(QWSSignalHandler, instance);
};


QWSSignalHandler* QWSSignalHandler::instance()
{
    return QWSSignalHandlerPrivate::instance();
}

QWSSignalHandler::QWSSignalHandler()
{
    const int signums[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE,
                            SIGSEGV, SIGTERM, SIGBUS };
    const int n = sizeof(signums)/sizeof(int);

    for (int i = 0; i < n; ++i) {
        const int signum = signums[i];
        sighandler_t old = signal(signum, handleSignal);
        oldHandlers[signum] = (old == SIG_ERR ? SIG_DFL : old);
    }
}

QWSSignalHandler::~QWSSignalHandler()
{
    while (!semaphores.isEmpty())
        removeSemaphore(semaphores.last());
}

void QWSSignalHandler::removeSemaphore(int semno)
{
    const int index = semaphores.lastIndexOf(semno);
    if (index != -1) {
        semun semval;
        semval.val = 0;
        semctl(semaphores.at(index), 0, IPC_RMID, semval);
        semaphores.remove(index);
    }
}

void QWSSignalHandler::handleSignal(int signum)
{
    QWSSignalHandler *h = instance();

    signal(signum, h->oldHandlers[signum]);

    semun semval;
    semval.val = 0;
    for (int i = 0; i < h->semaphores.size(); ++i)
        semctl(h->semaphores.at(i), 0, IPC_RMID, semval);
    raise(signum);
}
