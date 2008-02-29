/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "scriptthread.h"

#include <QDebug>
#include <QProcess>

#include <qtopialog.h>

/*!
  \class ScriptThread
  \brief The ScriptThread class provides serialized execution of applications/scripts.
  \internal
  \mainclass

  The ScriptThread class is used by the Qtopia network plug-ins. It allows the serialized execution 
  of scripts without blocking the caller. New scripts can be added via addScriptToRun() and
  are added to queue. The excution is based on the FIFO principle. Each time when 
  */

//This class is used by the Qtopia network plug-ins and is not a public class.
//In the future this might be replaced by Qt's improved concurrency API.

/*!
  Constructs a ScriptThread instance with the given \a parent.
  */
ScriptThread::ScriptThread( QObject * parent )
    : QThread( parent ), quit( false )
{

}

/*!
  Destroys the ScriptThread instance
  */
ScriptThread::~ScriptThread()
{
    quit = true;
    waitCond.wakeOne();
    wait();
}

/*!
  \internal
  \threadsafe
  Adds the script with the give \a scriptPath to the queue of executable scripts. \a parameter
  is the list of parameters that should be passed to the script.
  */
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

/*!
  \internal
  \threadsafe

  Returns the number of remaining jobs (including the currently running one) to be executed.
  */
int ScriptThread::remainingTasks() const
{
    QMutexLocker lock( &mutex );
    return scripts.count();
}

/*!
  \internal

  Executes the scripts queued up for this thread instance.
  */
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
