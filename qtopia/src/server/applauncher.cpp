/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTOPIA_INTERNAL_PRELOADACCESS
#define QTOPIA_INTERNAL_PRELOADACCESS
#endif
#ifndef QTOPIA_INTERNAL_FILEOPERATIONS
#define QTOPIA_INTERNAL_FILEOPERATIONS
#endif
#ifndef QTOPIA_PROGRAM_MONITOR
#define QTOPIA_PROGRAM_MONITOR
#endif
#include <qtopia/qpeglobal.h>

#ifndef Q_OS_WIN32
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#else
#include <process.h>
#include <windows.h>
#include <winbase.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qwindowsystem_qws.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/applnk.h>
#include <qtopia/stringutil.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>

#ifndef Q_OS_WIN32
#include <qtopia/qprocess.h>
#endif

#include "applauncher.h"
#include "documentlist.h"

const int AppLauncher::RAISE_TIMEOUT_MS = 5000;

//---------------------------------------------------------------------------

AppLauncher *AppLauncher::appLauncherPtr = 0;

bool AppLauncher::wasCritMemKill = FALSE;

AppLauncher::AppLauncher(QObject *parent, const char *name)
    : QObject(parent, name), qlProc(0), qlPid(0), qlReady(FALSE),
      appKillerBox(0)
{
    connect(qwsServer, SIGNAL(newChannel(const QString&)), this, SLOT(newQcopChannel(const QString&)));
    connect(qwsServer, SIGNAL(removedChannel(const QString&)), this, SLOT(removedQcopChannel(const QString&)));
    QCopChannel* channel = new QCopChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(received(const QCString&,const QByteArray&)) );

    channel = new QCopChannel( "QPE/Server", this );
    connect( channel, SIGNAL(received(const QCString&,const QByteArray&)),
	     this, SLOT(received(const QCString&,const QByteArray&)) );
    
#ifdef Q_OS_WIN32
    runningAppsProc.setAutoDelete( TRUE );
#endif
    QString tmp = qApp->argv()[0];
    int pos = tmp.findRev('/');
    if ( pos > -1 )
	tmp = tmp.mid(++pos);
    RunningApp app;
    app.name = tmp;
    app.proc = 0;
    runningApps[::getpid()] = app;
    
    appLauncherPtr = this;

    QTimer::singleShot( 1000, this, SLOT(createQuickLauncher()) );
}

AppLauncher::~AppLauncher()
{
    appLauncherPtr = 0;
    if ( qlPid ) {
	int status;
	::kill( qlPid, SIGTERM );
	waitpid( qlPid, &status, 0 );
    }
}

/*  We use the QCopChannel of the app as an indicator of when it has been launched
    so that we can disable the busy indicators */
void AppLauncher::newQcopChannel(const QString& channelName) 
{
//  qDebug("channel %s added", channelName.data() );
    QString prefix("QPE/Application/");
    if (channelName.startsWith(prefix)) {
	{
	    QCopEnvelope e("QPE/System", "newChannel(QString)");
	    e << channelName;
	}
	QString appName = channelName.mid(prefix.length());
	if ( appName != "quicklauncher" ) {
	    emit connected( appName );
	    QCopEnvelope e("QPE/System", "notBusy(QString)");
	    e << appName;
	}
    } else if (channelName.startsWith("QPE/QuickLauncher-")) {
	qDebug("Registered %s", channelName.latin1());
	int pid = channelName.mid(18).toInt();
	if (pid == qlPid)
	    qlReady = TRUE;
    }
}

void AppLauncher::removedQcopChannel(const QString& channelName) 
{
    if (channelName.startsWith("QPE/Application/")) {
	QCopEnvelope e("QPE/System", "removedChannel(QString)");
	e << channelName;
    }
}


bool AppLauncher::mDelayMessages = FALSE;

void AppLauncher::delayMessages(bool b)
{
    if (mDelayMessages != b) {
	mDelayMessages = b;
	if (!b) {
	    appLauncherPtr->sendQueuedMessages();
	}
    }
}

void AppLauncher::queueMessage(const QCString &msg, const QByteArray &data)
{
    qDebug("store message");
    QueuedItem it;
    it.msg = msg;
    it.data = data;
    qmessages.append(it);
}

void AppLauncher::sendQueuedMessages() {
    //check for queued messages
    qDebug("send messages");

    QValueList<QueuedItem>::Iterator it = qmessages.begin();
    for(; it != qmessages.end(); ++it) {
	received((*it).msg, (*it).data);
    }

    qmessages.clear();
}


void AppLauncher::received(const QCString& msg, const QByteArray& data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "execute(QString)" ) { // from QPE/Server
	QString t;
	stream >> t;
	if ( !executeBuiltin( t, QString::null ) )
	    execute(t, QString::null);
    } else if ( msg == "execute(QString,QString)" ) { // from QPE/Server
	QString t,d;
	stream >> t >> d;
	if ( !executeBuiltin( t, d ) )
	    execute( t, d );
    } else if ( msg == "processQCop(QString)" ) { // from QPE/Server
	// will queue instead
	if (mDelayMessages) {
	    queueMessage(msg, data);
	    return;
	}
	QString t;
	stream >> t;
	if ( !executeBuiltin( t, QString::null ) )
	    execute( t, QString::null, TRUE);
    } else if ( msg == "raise(QString)" ) {
	QString appName;
	stream >> appName;

	/*
	** DUPLICATED from execute()
	*/
	if ( !executeBuiltin( appName, QString::null ) ) {
	    if ( !waitingHeartbeat.contains( appName ) && appKillerName != appName ) {
		//qDebug( "Raising: %s", appName.latin1() );
		QCString channel = "QPE/Application/";
		channel += appName.latin1();

		// Need to lock it to avoid race conditions with QPEApplication::processQCopFile
		QFile f( Global::tempDir() + "qcop-msg-" + appName);
		if ( f.open(IO_WriteOnly | IO_Append) ) {
#ifndef Q_OS_WIN32
		    flock(f.handle(), LOCK_EX);
#endif
		    QDataStream ds(&f);
		    QByteArray b;
		    QDataStream bstream(b, IO_WriteOnly);
		    ds << channel << QCString("raise()") << b;
		    f.flush();
#ifndef Q_OS_WIN32
		    flock(f.handle(), LOCK_UN);
#endif
		    f.close();
		}
		bool alreadyRunning = isRunning( appName );
		if ( execute(appName, QString::null) ) {
		    int id = startTimer(RAISE_TIMEOUT_MS + alreadyRunning?2000:0);
		    waitingHeartbeat.insert( appName, id );
		}
	    }
	}
    } else if ( msg == "sendRunningApps()" ) {
	QStringList apps;
	QMap<int,RunningApp>::Iterator it;
        for( it = runningApps.begin(); it != runningApps.end(); ++it )
	    apps.append( (*it).name );
	QCopEnvelope e( "QPE/Desktop", "runningApps(QStringList)" );
	e << apps;
    } else if ( msg == "appRaised(QString)" ) {
	QString appName;
	stream >> appName;
	qDebug("Got a heartbeat from %s", appName.latin1());
	QMap<QString,int>::Iterator it = waitingHeartbeat.find(appName);
	if ( it != waitingHeartbeat.end() ) {
	    killTimer( *it );
	    waitingHeartbeat.remove(it);
	}
	// Check to make sure we're not waiting on user input...
	if ( appKillerBox && appName == appKillerName ) {
	    // If we are, we kill the dialog box, and the code waiting on the result
	    // will clean us up (basically the user said "no").
	    delete appKillerBox;
	    appKillerBox = 0;
	    appKillerName = QString::null;
	}
	runningList.remove(appName);
	runningList.prepend(appName);
	emit raised(appName);
    }
}

void AppLauncher::timerEvent( QTimerEvent *e )
{
    int id = e->timerId();
    QMap<QString,int>::Iterator it;
    for ( it = waitingHeartbeat.begin(); it != waitingHeartbeat.end(); ++it ) {
	if ( *it == id ) {
	    if ( appKillerBox ) // we're already dealing with one
		return;

	    appKillerName = it.key();
	    killTimer( id );
	    waitingHeartbeat.remove( it );

	    // qDebug("Checking in on %s", appKillerName.latin1());

	    // We store this in case the application responds while we're
	    // waiting for user input so we know not to delete ourselves.
	    appKillerBox = new QMessageBox(tr("Application Problem"),
		    tr("<qt>%1 is not responding. Would you like to "
		       "force the application to exit?</qt>").arg(appKillerName),
		    QMessageBox::Warning, QMessageBox::Yes, 
		    QMessageBox::No | QMessageBox::Default, 
		    QMessageBox::NoButton);
	    if (appKillerBox->exec() == QMessageBox::Yes) {    
		// qDebug("Killing the app!!! Bwuhahahaha!");
		int pid = pidForName(appKillerName);
		if ( pid > 0 )
		    kill( pid );
	    }
	    appKillerName = QString::null;
	    delete appKillerBox;
	    appKillerBox = 0;
	    return;
	}
    }

    QObject::timerEvent( e );
}

bool AppLauncher::isRunning(const QString &app)
{
    for (QMap<int,RunningApp>::ConstIterator it = runningApps.begin(); it != runningApps.end(); ++it) {
	if ( (*it).name == app ) {
#ifdef Q_OS_UNIX
	    pid_t t = ::__getpgid( it.key() );
	    if ( t == -1 ) {
		qDebug("appLauncher bug, %s believed running, but pid %d is not existing", app.data(), it.key() );
		runningApps.remove( it.key() );
		return FALSE;
	    }
#endif
	    return TRUE;
	}
    }

    return FALSE;
}

bool AppLauncher::executeBuiltin(const QString &c, const QString &document)
{
    Global::Command* builtin = Global::builtinCommands();
    QGuardedPtr<QWidget> *running = Global::builtinRunning();
    
    // Attempt to execute the app using a built-in class for the app
    if (builtin) {
	for (int i = 0; builtin[i].file; i++) {
	    if ( builtin[i].file == c ) {
		if ( running[i] ) {
		    if ( !document.isNull() && builtin[i].documentary )
			Global::setDocument(running[i], document);
		    running[i]->raise();
		    running[i]->show();
		    running[i]->setActiveWindow();
		} else {
		    running[i] = builtin[i].func( builtin[i].maximized );
		}
#ifndef QT_NO_COP
		QCopEnvelope e("QPE/System", "notBusy(QString)" );
		e << c; // that was quick ;-)
#endif
		return TRUE;
	    }
	}
    }

    // Convert the command line into a list of arguments
    QStringList list = QStringList::split(QRegExp("  *"),c);
    QString ap=list[0];

    if ( ap == "suspend" ) { // No tr
	QWSServer::processKeyEvent( 0xffff, Qt::Key_F34, FALSE, TRUE, FALSE );
	return TRUE;
    }

    return FALSE;
}

bool AppLauncher::execute(const QString &c, const QString &docParam, bool noRaise)
{
/*
    // debug info
    for (QMap<int,QString>::Iterator it = runningApps.begin(); it != runningApps.end(); ++it) {
	qDebug("execute: running according to internal list: %s, with pid %d", (*it).data(), it.key() );
    }
*/
    // Convert the command line in to a list of arguments
    QStringList list = QStringList::split(QRegExp("  *"),c);
    if ( !docParam.isEmpty() )
	list.append( docParam );

    QString appName = list[0];
    if ( isRunning(appName) ) {
	QCString channel = "QPE/Application/";
	channel += appName.latin1();
	
	// Need to lock it to avoid race conditions with QPEApplication::processQCopFile
	QFile f(Global::tempDir() + "qcop-msg-" + appName);
	if ( !noRaise && f.open(IO_WriteOnly | IO_Append) ) {
#ifndef Q_OS_WIN32
	    flock(f.handle(), LOCK_EX);
#endif
	    
	    QDataStream ds(&f);
	    QByteArray b;
	    QDataStream bstream(b, IO_WriteOnly);
	    if ( !f.size() ) {
		ds << channel << QCString("raise()") << b;
		if ( !waitingHeartbeat.contains( appName ) && appKillerName != appName ) {
		    int id = startTimer(RAISE_TIMEOUT_MS);
		    waitingHeartbeat.insert( appName, id );
		}
	    }
	    if ( !docParam.isEmpty() ) {
		bstream << docParam;
		ds << channel << QCString("setDocument(QString)") << b;
	    }

	    f.flush();
#ifndef Q_OS_WIN32
	    flock(f.handle(), LOCK_UN);
#endif
	    f.close();
	}
	if ( QCopChannel::isRegistered(channel) ) // avoid unnecessary warnings
	    QCopChannel::send(channel,"QPEProcessQCop()");
	
	return TRUE;
    }

#ifdef QT_NO_QWS_MULTIPROCESS
    QMessageBox::warning( 0, tr("Error"), tr("<qt>Could not find the application %1</qt>").arg(c),
	tr("OK"), 0, 0, 0, 1 );
#else

    QStrList slist;
    unsigned j;
    for ( j = 0; j < list.count(); j++ )
	slist.append( list[j].utf8() );

    const char **args = new const char *[slist.count() + 1];
    for ( j = 0; j < slist.count(); j++ )
	args[j] = slist.at(j);
    args[j] = NULL;

#ifndef Q_OS_WIN32
#ifdef SINGLE_EXEC
    QPEAppMap *qpeAppMap();
#endif
    if ( qlPid && qlReady && 
#ifndef SINGLE_EXEC
        QFile::exists( QPEApplication::qpeDir()+"plugins/application/lib"+args[0] + ".so" )
#else
	qpeAppMap()->contains(args[0] )
#endif
	    ) {
	qDebug( "Quick launching: %s", args[0] );
	if ( getuid() == 0 )
	    setpriority( PRIO_PROCESS, qlPid, 0 );
	QCString qlch("QPE/QuickLauncher-");
	qlch += QString::number(qlPid);
	QCopEnvelope env( qlch, "execute(QStrList)" );
	env << slist;
	QTOPIA_PROFILE("sending qcop to quicklauncher");
	appLaunched(qlProc, args[0]);
	qlProc = 0;
	qlPid = 0;
	qlReady = FALSE;
	QTimer::singleShot( getuid() == 0 ? 800 : 1500, this, SLOT(createQuickLauncher()) );
	delete [] args;
	return TRUE;
    }
#endif // Q_OS_WIN32

    QProcess *proc = new QProcess(this);
    // Try bindir first, so that foo/bar works too
    proc->setCommunication(0);
    proc->addArgument(QPEApplication::qpeDir()+"bin/"+args[0]);
    for (int i=1; i < (int)slist.count(); i++)
	proc->addArgument(args[i]);
    connect(proc, SIGNAL(processExited()), this, SLOT(processExited()));
    bool started = proc->start();
    if (!started) {
	proc->clearArguments();
	for (int i=0; i < (int)slist.count(); i++)
	    proc->addArgument(args[i]);
	started = proc->start();
    }

    if (!started) {
	qDebug("Unable to start application %s", args[0]);
	delete proc;
    } else {
# ifndef Q_OS_WIN32
	appLaunched(proc, args[0]);
# else
	PROCESS_INFORMATION *procInfo = (PROCESS_INFORMATION *)proc->processIdentifier();
	if (procInfo) {
	    DWORD pid = procInfo->dwProcessId;
	    runningAppsProc.append(proc);
	    appLaunched(proc, args[0]);
	} else {
	    qDebug("Unable to read process inforation #1 for %s", args[0]);
	}
# endif
    }
#endif //QT_NO_QWS_MULTIPROCESS

    delete [] args;
    return started;
}

void AppLauncher::appLaunched(QProcess *proc, const QString &appName)
{
    RunningApp app;
    app.name = appName;
    app.proc = proc;
    int pid = (int)proc->processIdentifier();
    runningApps[pid] = app;
    runningList.remove(appName);
    runningList.prepend(appName);
    emit launched(pid, appName);
    QCopEnvelope e("QPE/System", "busy()");
}

void AppLauncher::kill( int pid )
{
#ifndef Q_OS_WIN32
    ::kill( pid, SIGTERM );
#else
    for ( QProcess *proc = runningAppsProc.first(); proc; proc = runningAppsProc.next() ) {
	if ( proc->processIdentifier() == pid ) {
	    proc->kill();
	    break;
	}
    }
#endif
}

int AppLauncher::pidForName( const QString &appName )
{
    int pid = -1;

    QMap<int, RunningApp>::Iterator it;
    for (it = runningApps.begin(); it!= runningApps.end(); ++it) {
	if ((*it).name == appName) {
	    pid = it.key();
	    break;
	}
    }

    return pid;
}

void AppLauncher::createQuickLauncher()
{
    qlReady = FALSE;
    qlProc = new QProcess(this);
    qlProc->setCommunication(0);
    connect(qlProc, SIGNAL(processExited()), this, SLOT(processExited()));
    qlProc->addArgument(QPEApplication::qpeDir()+"bin/quicklauncher");
    static int unset_bind = -1;
    if ( unset_bind == -1 ) {
        const char *bind_now = getenv("LD_BIND_NOW");
        if ( bind_now && ::strlen(bind_now) != 0 ) {
            unset_bind = 0;
        } else {
            unset_bind = 1;
        }
    }
    setenv( "LD_BIND_NOW", "1", 1 );
    bool started = qlProc->start();
    if (!started) {
	qlProc->clearArguments();
	qlProc->addArgument("quicklauncher");
	started = qlProc->start();
    }
    if (started) {
	qlPid = (int)qlProc->processIdentifier();
	if (getuid() == 0)
	    setpriority( PRIO_PROCESS, qlPid, 19 );
    } else {
	delete qlProc;
	qlProc = 0;
	qlPid = 0;
    }
    if ( unset_bind )
        unsetenv( "LD_BIND_NOW" );
}

void AppLauncher::processExited()
{
    // get the process that exited.
    QProcess *proc = (QProcess *) sender();
    if (!proc){
	qDebug("Internal error NULL proc");
	return;
    }

#ifndef Q_OS_WIN32
    if ( proc == qlProc ) {
	// Unexpected quicklauncher exit.
	qDebug( "quicklauncher stopped" );
	delete qlProc;
	qlProc = 0;
	qlPid = 0;
	qlReady = FALSE;
	QFile::remove(Global::tempDir() + "qcop-msg-quicklauncher" );
	QTimer::singleShot( 2000, this, SLOT(createQuickLauncher()) );
	return;
    }

    int pid = 0;
    QString appName;
    QMap<int,RunningApp>::Iterator it;
    for (it = runningApps.begin(); it != runningApps.end(); ++it ) {
	if ((*it).proc == proc) {
	    appName = (*it).name;
	    pid = it.key();
	    break;
	}
    }

    if (!pid) {
	qWarning("Unknown process exited");
	delete proc;
	return;
    }
    
    int status = proc->exitStatus();
#ifndef W_EXITCODE
    #ifdef __W_EXITCODE
	#define	W_EXITCODE(ret,sig)	__W_EXITCODE((ret), (sig))
    #else
	#define	W_EXITCODE(ret,sig)	((ret) << 8 | (sig))
    #endif
#endif
    if ( !proc->normalExit() )
	status = W_EXITCODE(status, SIGSEGV);   // XXX
    else
	status = W_EXITCODE(status, 0);

    int exitStatus = 0;
    
    bool crashed = WIFSIGNALED(status);
    if ( !crashed ) {
	if ( WIFEXITED(status) )
	    exitStatus = WEXITSTATUS(status);
    } else {
	exitStatus  = WTERMSIG(status);
    }

    delete proc;
    runningApps.remove(it);
    runningList.remove(appName);

    QMap<QString,int>::Iterator hbit = waitingHeartbeat.find(appName);
    if ( hbit != waitingHeartbeat.end() ) {
	killTimer( *hbit );
	waitingHeartbeat.remove( hbit );
    }
    if ( appName == appKillerName ) {
	appKillerName = QString::null;
	delete appKillerBox;
	appKillerBox = 0;
    }

    /* we must disable preload for an app that crashes as the system logic relies on preloaded apps
       actually being loaded.  If eg. the crash happened in the constructor, we can't automatically reload
       the app (without some timeout value for eg. 3 tries -which I think is a bad solution).
    */
    bool preloadDisabled = FALSE;
    if ( !DocumentList::appLnkSet ) return;
    const AppLnk* app = DocumentList::appLnkSet->findExec( appName );
    if ( !app ) return; // QCop messages processed to slow?
    if ( crashed && app->isPreloaded() ) {
	Config cfg("Launcher");
	cfg.setGroup("Preload");
	QStringList apps = cfg.readListEntry("Apps",',');
	QString exe = app->exec();
	apps.remove(exe);
	cfg.writeEntry("Apps",apps,',');
	preloadDisabled = TRUE;
    }

    // clean up 
    if ( exitStatus ) {
	QCopEnvelope e("QPE/System", "notBusy(QString)");
	e << app->exec();
    }

/*
    // debug info
    for (QMap<int,QString>::Iterator it = runningApps.begin(); it != runningApps.end(); ++it) {
	qDebug("running according to internal list: %s, with pid %d", (*it).data(), it.key() );
    }
*/

# ifdef QTOPIA_PROGRAM_MONITOR
    if ( crashed ) {
	QString appname = Qtopia::dehyphenate(app->name());
        QString str;
        if (wasCritMemKill) {
            str = tr("<qt><b>%1</b> was terminated due to insufficient memory.</qt>").arg(appname);
            wasCritMemKill = FALSE;
        } else 
	    str = tr("<qt><b>%1</b> was terminated due to application error (%2).</qt>").arg(appname).arg( exitStatus );
	if ( preloadDisabled )
	    str += tr(" (Fast loading has been disabled for this application. "
		      "Tap and hold the application icon to reenable it.)");
	QMessageBox::information(0, tr("Application terminated"), str );
    } else {
	if ( exitStatus == 255 ) {  //could not find app (because global returns -1)
	    QMessageBox::information(0, tr("Application not found"),
		    tr("<qt>Unable to locate application <b>%1</b></qt>").arg( app->exec() ) );
	} else  {
	    QFileInfo fi(Global::tempDir() + "qcop-msg-" + appName);
	    if ( fi.exists() && fi.size() ) {
		emit terminated(pid, appName);
		execute( appName, QString::null );
		return;
	    }
	}
    }

# endif
    
    emit terminated(pid, appName);

#else //Q_OS_WIN32
    bool found = FALSE;

    QString appName = proc->arguments()[0];
    qDebug("Removing application %s", appName.latin1());
    runningAppsProc.remove(proc);	

    QMap<QString,int>::Iterator hbit = waitingHeartbeat.find(appName);
    if ( hbit != waitingHeartbeat.end() ) {
	killTimer( *hbit );
	waitingHeartbeat.remove( hbit );
    }
    if ( appName == appKillerName ) {
	appKillerName = QString::null;
	delete appKillerBox;
	appKillerBox = 0;
    }

    // Search for the app to find its PID
    QMap<int, QString>::Iterator it;
    for (it = runningApps.begin(); it!= runningApps.end(); ++it){
	if (it.data() == appName){
	    found = TRUE;
	    break;
	}
    }

    if (found){
	runningList.remove(it.key());
	emit terminated(it.key(), it.data());
	runningApps.remove(it.key());
    }else{
	qDebug("Internal error application %s not listed as running", appName.latin1());
    }
    
#endif
}

void AppLauncher::criticalKill(const QString &app) 
{   
    if (!wasCritMemKill) {
        wasCritMemKill = TRUE;
        int pid = pidForName(app);
        kill(pid);
        emit terminated(pid, app);
    }
}
