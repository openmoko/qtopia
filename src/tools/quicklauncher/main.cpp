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

#ifdef QUICKLAUNCHER_FORKED
#include "quicklaunchforked.h"
#else
#include "quicklaunch.h"
#endif

#include <qpainter.h>
#include <qtimer.h>
#include <qpointer.h>
#include <qtopiachannel.h>
#include <QIcon>
#include <qtimezone.h>
#include <qtopiaapplication.h>
#include <qpluginmanager.h>
#include <qapplicationplugin.h>
#include <perftest.h>
#include <QSocketNotifier>
#include <qtopialog.h>
#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef SINGLE_EXEC
#define MAIN_FUNC main_quicklaunch
#else
#define MAIN_FUNC main
#ifndef QT_NO_SXE
QSXE_APP_KEY
#endif
#endif

#ifdef Q_OS_LINUX
extern char **environ;
#endif

int MAIN_FUNC( int argc, char** argv )
{
#ifndef QT_NO_SXE
    // This is just the key for the quicklauncher, launched processes get
    // their own key
    QTransportAuth::getInstance()->setProcessKey( _key, "quicklauncher" );
#endif

    QuickLauncher::app = new QtopiaApplication( argc, argv );
    QuickLauncher::app->registerRunningTask("QuickLaunch");

    ::unsetenv( "LD_BIND_NOW" );

#ifndef SINGLE_EXEC
    QuickLauncher::loader = new QPluginManager( "application" );
#endif

    QString arg0 = argv[0];
    int sep = arg0.lastIndexOf( '/' );
    if ( sep > 0 )
        arg0 = arg0.mid( sep+1 );
    if ( arg0 != "quicklauncher" ) {
        qLog(Quicklauncher) << "QuickLauncher invoked as:" << arg0.toLatin1();
        QuickLauncher::exec( argc, argv );
    } else {

#ifdef Q_OS_LINUX
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

        QuickLauncher::argv0 = argv;
        if (i > 0)
            QuickLauncher::argv_lth = envp[i-1] + strlen(envp[i-1]) - QuickLauncher::argv0[0];
        else
            QuickLauncher::argv_lth = QuickLauncher::argv0[argc-1] + strlen(QuickLauncher::argv0[argc-1]) - QuickLauncher::argv0[0];
#endif
        QuickLauncher::eventLoop = new QEventLoop();
#ifdef QUICKLAUNCHER_FORKED
        qLog(Quicklauncher) << "QuickLauncherForked running";
#else
        qLog(Quicklauncher) << "QuickLauncher running";
#endif

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
        QTimeZone::current().isValid(); // populate timezone cache
        QTimeString::currentAMPM(); // create internal structures
        QIcon(":icon/new"); // do internal init
#ifdef QTOPIA_PHONE
        QSoftMenuBar::menuKey(); // read config.
#endif

        // Create a widget to force initialization of title bar images, etc.
        QObject::disconnect(QuickLauncher::app, SIGNAL(lastWindowClosed()), QuickLauncher::app, SLOT(hideOrQuit()));
#if QT_VERSION < 0x040300
        // Avoid warning about Qt::Tool and Qt::FramelessWindowHint in Qt 4.2
        Qt::WindowFlags wFlags = Qt::WindowContextHelpButtonHint | Qt::Tool | Qt::FramelessWindowHint;
#else
        // We do want this behaviour.  Hopefully Qt 4.3 will have it.
        Qt::WindowFlags wFlags = Qt::WindowContextHelpButtonHint|Qt::Tool;
#endif
        QWidget *w = new QWidget(0,wFlags);
        w->setWindowTitle("_ignore_");  // no tr
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->setGeometry( -100, -100, 10, 10 );
        w->show();
        QTimer::singleShot( 0, w, SLOT(close()) );

#ifdef QUICKLAUNCHER_FORKED
        QuickLauncher* ql = new QuickLauncher();
#else
        (void)new QuickLauncher();
#endif
        while (!QuickLauncher::validExitLoop)
            if(-1 == QuickLauncher::eventLoop->exec())
                break;

#ifdef QUICKLAUNCHER_FORKED
        delete ql;
#endif
    }

    qLog(Performance) << QuickLauncher::app->applicationName().toLatin1().constData() << " : " << "Entering event loop : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    int rv = -1;
    if(QuickLauncher::app->willKeepRunning())
        QuickLauncher::app->exec();
    qLog(Performance) << QuickLauncher::app->applicationName().toLatin1().constData() << " : " << "Exited event loop : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    if ( QuickLauncher::mainWindow )
        delete (QWidget*)QuickLauncher::mainWindow;

/* Causes problems with undeleted TLWs
    if ( appIface )
        loader->releaseInterface( appIface );
    delete loader;
*/
    // For performance testing
    QString appName = QuickLauncher::app->applicationName();

    delete QuickLauncher::app;
    delete QuickLauncher::eventLoop;
#ifndef SINGLE_EXEC
    delete QuickLauncher::loader;
#endif

    qLog(Performance) << appName.toLatin1().constData() << " : " << "Exiting quicklauncher main : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    return rv;
}

