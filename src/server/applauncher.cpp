/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QTOPIA_INTERNAL_PRELOADACCESS
#define QTOPIA_PROGRAM_MONITOR

#ifndef Q_OS_WIN32
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#else
#include <process.h>
#include <windows.h>
#include <winbase.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qwindowsystem_qws.h>
#include <qmessagebox.h>
#include <qfile.h>

#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/applnk.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>

#include "applauncher.h"

static AppLauncher* appLauncherPtr;

const int appStopEventID = 1290;

class AppStoppedEvent : public QCustomEvent
{
public:
    AppStoppedEvent(int pid, int status)
	: QCustomEvent( appStopEventID ), mPid(pid), mStatus(status) { }
    
    int pid() { return mPid; }
    int status() { return mStatus; }

private:
    int mPid, mStatus;
};

AppLauncher::AppLauncher(const AppLnkSet *as, QObject *parent, const char *name)
    : QObject(parent, name), appLnkSet(as)
{
    connect(qwsServer, SIGNAL(newChannel(const QString&)), this, SLOT(newQcopChannel(const QString&)));
    QCopChannel* channel = new QCopChannel( "QPE/System", this );
    connect( channel, SIGNAL(received(const QCString&, const QByteArray&)),
	     this, SLOT(received(const QCString&, const QByteArray&)) );
    
    signal(SIGCHLD, signalHandler);

    appLauncherPtr = this;
}

AppLauncher::~AppLauncher()
{
    appLauncherPtr = 0;
    signal(SIGCHLD, SIG_DFL);
}

void AppLauncher::setAppLnkSet(const AppLnkSet* as)
{
    appLnkSet = as;
}

/*  We use the QCopChannel of the app as an indicator of when it has been launched
    so that we can disable the busy indicators */
void AppLauncher::newQcopChannel(const QString& channelName) 
{
//  qDebug("channel %s added", channelName.data() );
    QString prefix("QPE/Application/");
    if (channelName.startsWith(prefix)) {
	QString appName = channelName.mid(prefix.length());
	emit connected( appName );
	QCopEnvelope e("QPE/System", "notBusy(QString)");
	e << appName;
    }
}


void AppLauncher::received(const QCString& msg, const QByteArray& data)
{
    QDataStream stream( data, IO_ReadOnly );
    if ( msg == "execute(QString)" ) {
	QString t;
	stream >> t;
	if ( !executeBuiltin( t, "" ) )
	    execute(t, "");
    } else if ( msg == "execute(QString,QString)" ) {
	QString t,d;
	stream >> t >> d;
	if ( !executeBuiltin( t, d ) )
	    execute( t, d );
    } else if ( msg == "sendRunningApps()" ) {
	QStringList apps;
	QMap<int,QString>::Iterator it;
        for( it = runningApps.begin(); it != runningApps.end(); ++it )
	    apps.append( *it );
	QCopEnvelope e( "QPE/Desktop", "runningApps(QStringList)" );
	e << apps;
    }
}

void AppLauncher::signalHandler(int)
{
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
/*    if (pid == 0 || &status == 0 ) {
	qDebug("hmm, could not get return value from signal");
    }
*/
    QApplication::postEvent(appLauncherPtr, new AppStoppedEvent(pid, status) );
}

bool AppLauncher::event(QEvent *e)
{
    if ( e->type() == appStopEventID ) {
	AppStoppedEvent *ae = (AppStoppedEvent *) e;
	sigStopped(ae->pid(), ae->status() );
	return TRUE;
    }
    
    return QObject::event(e);
}

void AppLauncher::sigStopped(int sigPid, int sigStatus)
{
    int exitStatus = 0;
    
    bool crashed = WIFSIGNALED(sigStatus);
    if ( !crashed ) {
	if ( WIFEXITED(sigStatus) )
	    exitStatus = WEXITSTATUS(sigStatus);
    } else {
	exitStatus  = WTERMSIG(sigStatus);
    }

    QMap<int,QString>::Iterator it = runningApps.find( sigPid );
    if ( it == runningApps.end() ) {
/*
	if ( sigPid == -1 )
	    qDebug("non-qtopia application exited (disregarded)");
	else
	    qDebug("==== no pid matching %d in list, definite bug", sigPid);
*/
	return;
    }
    QString appName = *it;
    runningApps.remove(it);

    // Remove any unprocessed QCop input, as that would otherwise
    // prevernt QCopEnvelope from starting the app.
    QFile::remove("/tmp/qcop-msg-" + appName);

    /* we must disable preload for an app that crashes as the system logic relies on preloaded apps
       actually being loaded.  If eg. the crash happened in the constructor, we can't automatically reload
       the app (withouth some timeout value for eg. 3 tries (which I think is a bad solution)
    */
    bool preloadDisabled = FALSE;
    if ( !appLnkSet ) return;
    const AppLnk* app = appLnkSet->findExec( appName );
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
    for (it = runningApps.begin(); it != runningApps.end(); ++it) {
	qDebug("running according to internal list: %s, with pid %d", (*it).data(), it.key() );
    }
*/

#ifdef QTOPIA_PROGRAM_MONITOR
    if ( crashed ) {
	QString sig;
	switch( exitStatus ) {
	    case SIGABRT: sig = "SIGABRT"; break;
	    case SIGALRM: sig = "SIGALRM"; break;
	    case SIGBUS: sig = "SIGBUS"; break;
	    case SIGFPE: sig = "SIGFPE"; break;
	    case SIGHUP: sig = "SIGHUP"; break;
	    case SIGILL: sig = "SIGILL"; break;
	    case SIGKILL: sig = "SIGKILL"; break;
	    case SIGPIPE: sig = "SIGPIPE"; break;
	    case SIGQUIT: sig = "SIGQUIT"; break;
	    case SIGSEGV: sig = "SIGSEGV"; break;
	    case SIGTERM: sig = "SIGTERM"; break;
	    case SIGTRAP: sig = "SIGTRAP"; break;
	    default: sig = QString("Unkown %1").arg(exitStatus);
	}
	if ( preloadDisabled )
	    sig += tr("<qt><p>Fast loading has been disabled for this application.  Tap and hold the application icon to reenable it.</qt>");
	
	QString str = tr("<qt><b>%1</b> was terminated due to signal code %2</qt>").arg( app->name() ).arg( sig );
	QMessageBox::information(0, tr("Application terminated"), str );
    } else {
	if ( exitStatus == 255 ) {  //could not find app (because global returns -1)
	    QMessageBox::information(0, tr("Application not found"), tr("<qt>Could not locate application <b>%1</b></qt>").arg( app->exec() ) );
	} else if ( exitStatus > 0 ) {
	    qDebug("interrupted signal for closed process %s (pid %d) with exit status %d", app->exec().data(), sigPid, exitStatus);
	}
    }

#endif
    
    emit terminated(sigPid, appName);
}

bool AppLauncher::isRunning(const QString &app)
{
    for (QMap<int,QString>::ConstIterator it = runningApps.begin(); it != runningApps.end(); ++it) {
	if ( *it == app ) {
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
    
    // Attempt to execute the app using a builtin class for the app
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

    // Convert the command line in to a list of arguments
    QStringList list = QStringList::split(QRegExp("  *"),c);
    QString ap=list[0];

    if ( ap == "suspend" ) { // No tr
	QWSServer::processKeyEvent( 0xffff, Qt::Key_F34, FALSE, TRUE, FALSE );
	return TRUE;
    }

    return FALSE;
}

void AppLauncher::execute(const QString &c, const QString &docParam)
{
    // Convert the command line in to a list of arguments
    QStringList list = QStringList::split(QRegExp("  *"),c);
    if ( !docParam.isEmpty() )
	list.append( docParam );

    QString ap=list[0];
    
    if ( isRunning(ap) ) {
#ifndef QT_NO_COP
	{ QCopEnvelope env( ("QPE/Application/" + ap).latin1(), "raise()" ); }
	if ( !docParam.isEmpty() ) {
	    QCopEnvelope env( ("QPE/Application/" + ap).latin1(), "setDocument(QString)" );
	    env << docParam;
	} 
#endif
	return;
    }

#ifdef QT_NO_QWS_MULTIPROCESS
    QMessageBox::warning( 0, tr("Error"), tr("Could not find the application %1").arg(c),
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
#ifdef HAVE_QUICKEXEC
    QString libexe = QPEApplication::qpeDir()+"binlib/lib"+args[0] + ".so";
    qDebug("libfile = %s", libexe.latin1() );
    if ( QFile::exists( libexe ) ) {
	qDebug("calling quickexec %s", libexe.latin1() );
	quickexecv( libexe.utf8().data(), (const char **)args );
    } else
#endif
    {
	int pid = ::vfork();
	if ( !pid ) {
	    for ( int fd = 3; fd < 100; fd++ )
		::close( fd );
	    ::setpgid( ::getpid(), ::getppid() );
	    // Try bindir first, so that foo/bar works too
	    ::execv( QPEApplication::qpeDir()+"bin/"+args[0], (char * const *)args );
	    ::execvp( args[0], (char * const *)args );
	    _exit( -1 );
	}

	runningApps[pid] = QString(args[0]);
	emit launched(pid, QString(args[0]));
    }
#else
    qDebug("Doing spawn %s: args %s", args[0], args[1]);
    spawnvp(_P_NOWAIT,args[0], args);
#endif
#endif //QT_NO_QWS_MULTIPROCESS

    delete [] args;
}


