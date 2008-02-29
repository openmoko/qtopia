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

#include <qpainter.h>
#include <qstrlist.h>
#include <qtimer.h>
#include <qguardedptr.h>
#include <qwidgetlist.h>
#include <qcopchannel_qws.h>
#define QTOPIA_INTERNAL_INITAPP
#include <qtopia/timezone.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/pluginloader.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static QPEApplication *app = 0;
static PluginLoader *loader = 0;
static ApplicationInterface *appIface = 0;
static QGuardedPtr<QWidget> mainWindow;
static bool validExitLoop = FALSE;

#ifdef _OS_LINUX_
static char **argv0 = 0;
static int argv_lth;
extern char **environ;
#ifndef SPT_BUFSIZE
#define SPT_BUFSIZE     2048
#endif
#include <stdarg.h>
void setproctitle (const char *fmt,...) {
    int        i;
    char       buf[SPT_BUFSIZE];
    va_list    ap;

    if (!argv0)
	return;

    va_start(ap, fmt);
    (void) vsnprintf(buf, SPT_BUFSIZE, fmt, ap);
    va_end(ap);

    i = strlen (buf);
    if (i > argv_lth - 2) {
	i = argv_lth - 2;
	buf[i] = '\0';
    }
    memset(argv0[0], '\0', argv_lth);       /* clear the memory area */
    (void) strcpy (argv0[0], buf);

    argv0[1] = NULL;
}
#endif

class QuickLauncher : public QObject
{
    Q_OBJECT
public:
    QuickLauncher() : QObject()
    {
	QCString ch("QPE/QuickLauncher-");
	ch += QString::number(getpid());
	qlChannel = new QCopChannel( ch, this);
	connect( qlChannel, SIGNAL(received(const QCString&,const QByteArray&)),
		 this, SLOT(message(const QCString&,const QByteArray&)) );
    }

    static void exec( int /*argc*/, char **argv )
    {
	QString appName = argv[0];
	int sep = appName.findRev( '/' );
	if ( sep > 0 )
	    appName = appName.mid( sep+1 );

	appIface = 0;
	if ( loader->queryInterface(appName, IID_QtopiaApplication, (QUnknownInterface**)&appIface) == QS_OK ) {
	    mainWindow = appIface->createMainWindow( appName );
	}
	if ( mainWindow ) {
	    if ( mainWindow->metaObject()->slotNames(true).contains("setDocument(const QString&)") ) {
		app->showMainDocumentWidget( mainWindow );
	    } else {
		app->showMainWidget( mainWindow );
	    }
	} else {
	    qWarning( "Could not create application main window" );
	    exit(-1);
	}
    }

private slots:
    void message(const QCString &msg, const QByteArray & data)
    {
	QStrList argList;

	if ( msg == "execute(QStrList)" ) {
	    delete qlChannel;
	    QDataStream stream( data, IO_ReadOnly );
	    QStrList argList;
	    stream >> argList;
	    qDebug( "QuickLauncher execute: %s", argList.at(0) );
	    doQuickLaunch( argList );
	    delete this;
	} else if ( msg == "execute(QString)" ) {
	    delete qlChannel;
	    QDataStream stream( data, IO_ReadOnly );
	    QString arg;
	    stream >> arg;
	    qDebug( "QuickLauncher execute: %s", arg.latin1() );
	    QStrList argList;
	    argList.append( arg.utf8() );
	    doQuickLaunch( argList );
	    delete this;
	}
    }

private:
    void doQuickLaunch( QStrList &argList )
    {
	static int myargc = argList.count();
	static char **myargv = new char *[myargc + 1];
	for ( int j = 0; j < myargc; j++ ) {
	    myargv[j] = new char [strlen(argList.at(j))+1];
	    strcpy( myargv[j], argList.at(j) );
	}
	myargv[myargc] = NULL;
#ifdef _OS_LINUX_
	// Change name of process
	setproctitle(myargv[0]);
#endif

	connect(app, SIGNAL(lastWindowClosed()), app, SLOT(hideOrQuit()));
	validExitLoop = TRUE;
	app->exit_loop();
	app->initApp( myargc, myargv );
	exec( myargc, myargv );
    }

private:
    QCopChannel *qlChannel;
};

int main( int argc, char** argv )
{
    app = new QPEApplication( argc, argv );

    loader = new PluginLoader( "application" );

    unsetenv( "LD_BIND_NOW" );

    QCString arg0 = argv[0];
    int sep = arg0.findRev( '/' );
    if ( sep > 0 )
	arg0 = arg0.mid( sep+1 );
    if ( arg0 != "quicklauncher" ) {
	qDebug( "QuickLauncher invoked as: %s", arg0.data() );
	QuickLauncher::exec( argc, argv );
    } else {
#ifdef _OS_LINUX_
	// Setup to change proc title
	int i;
	char **envp = environ;
	/* Move the environment so we can reuse the memory.
	 * (Code borrowed from sendmail.) */
	for (i = 0; envp[i] != NULL; i++)
	    continue;
	environ = (char **) malloc(sizeof(char *) * (i + 1));
	if (environ == NULL)
	    return -1;
	for (i = 0; envp[i] != NULL; i++)
	    if ((environ[i] = strdup(envp[i])) == NULL)
		return -1;
	environ[i] = NULL;

	argv0 = argv;
	if (i > 0)
	    argv_lth = envp[i-1] + strlen(envp[i-1]) - argv0[0];
	else
	    argv_lth = argv0[argc-1] + strlen(argv0[argc-1]) - argv0[0];
#endif
	(void)new QuickLauncher();
	qDebug( "QuickLauncher running" );
	// Pre-load default fonts
	QFontMetrics fm( QApplication::font() );
	fm.ascent(); // causes font load.
	QFont f( QApplication::font() );
	f.setWeight( QFont::Bold );
	QFontMetrics fmb( f );
	fmb.ascent(); // causes font load.

	// Each of the following force internal structures/internal
	// initialization to be performed.  This may mean allocating
	// memory that is not needed by all applications.
	TimeZone::current().isValid(); // populate timezone cache
	TimeString::currentDateFormat(); // create internal structures
	TimeString::currentAMPM();
	Resource::loadIconSet("new"); // do internal init

	// Create a widget to force initialization of title bar images, etc.
	QObject::disconnect(app, SIGNAL(lastWindowClosed()), app, SLOT(hideOrQuit()));
	QWidget *w = new QWidget(0,0,Qt::WDestructiveClose|Qt::WStyle_ContextHelp|Qt::WStyle_Tool);
	w->setGeometry( -100, -100, 10, 10 );
	w->show();
	QTimer::singleShot( 0, w, SLOT(close()) );

	while (!validExitLoop)
	    app->enter_loop();
    }

    int rv = app->exec();

    if ( mainWindow )
	delete (QWidget*)mainWindow;

/* Causes problems with undeleted TLWs
    if ( appIface )
	loader->releaseInterface( appIface );
    delete loader;
*/
    delete app;

    return rv;
}

#include "main.moc"
