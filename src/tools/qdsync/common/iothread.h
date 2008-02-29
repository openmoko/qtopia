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
#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <qdglobal.h>
#include <QThread>
#include <QMutex>
#include <QByteArray>

typedef void* HANDLE;

class QD_EXPORT IOThread : public QThread
{
    Q_OBJECT
public:
    IOThread( QObject *parent = 0 );
    ~IOThread();

    void run();
    void write();
    void abort();
    void quit();

signals:
    void emitReadyRead();
    void ioThreadStopped();

public:
    HANDLE handle;
    QByteArray rb;
    QMutex rbm;
    QByteArray wb;
    QMutex wbm;
    HANDLE writeSem;
    HANDLE quitSem;
    enum State {
        Idle, Wait, Read, Write, Quit
    } currentState, nextState;
    bool brokenSerial;
};

#endif
