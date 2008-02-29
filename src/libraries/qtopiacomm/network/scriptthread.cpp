/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "scriptthread.h"

#include <QDebug>
#include <QProcess>

#include <qtopialog.h>

ScriptThread::ScriptThread( QObject * parent )
    : QThread( parent ), quit( false )
{

}

ScriptThread::~ScriptThread()
{
    quit = true;
    waitCond.wakeOne();
    wait();
}

void ScriptThread::addScriptToRun( const QString& scriptPath, const QStringList& parameter )
{
    QMutexLocker lock(&mutex);
    qLog(Network) << "Adding new network script:" << scriptPath << parameter;
    scripts.append( scriptPath );
    params.append( parameter );

    if ( !isRunning() )
        start();
    else
        waitCond.wakeOne();
}

int ScriptThread::remainingTasks() const
{
    QMutexLocker lock( &mutex );
    return scripts.count();
}

void ScriptThread::run()
{
    mutex.lock();
    QString script = scripts.first();
    QStringList parameter = params.first();
    mutex.unlock();

    while ( !quit ) {
        qLog(Network) << "Executing network script" << script << parameter;
        QProcess::execute( script, parameter );

        mutex.lock();
        scripts.removeFirst();
        params.removeFirst();
        emit scriptDone();
        if ( scripts.isEmpty() ) {
            qLog(Network) << "All network scripts executed -> blocking";
            waitCond.wait( &mutex );
        }

        if ( !scripts.isEmpty() ) {
            script = scripts.first();
            parameter = params.first();
        } else if ( !quit ) {
            qLog(Network) << "Invalid state: No network scripts left to execute";
        }
        mutex.unlock();
    }

}
