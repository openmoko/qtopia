/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
