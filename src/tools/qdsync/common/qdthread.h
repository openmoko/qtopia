/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QDTHREAD_H
#define QDTHREAD_H

#include <qdglobal.h>
#include <QThread>
#include <QMutex>
#include <QSemaphore>

class QD_EXPORT QDThread : public QThread
{
public:
    QDThread( QObject *parent = 0 );
    virtual ~QDThread();

    void init();
    void start();
    virtual void t_init();
    virtual void t_run();
    virtual void t_quit();

private:
    void run();
    bool inited;
    QMutex waitForStartMutex; 
    QSemaphore waitForRunSemaphore;
};

#endif

