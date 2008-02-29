/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "quicklaunch.h"

#include <qpainter.h>
#include <qtimer.h>
#include <qpointer.h>
#include <qtopiachannel.h>
#include <QIcon>
#include <qtopialog.h>

#include <qtimezone.h>
#include <qtopiaapplication.h>
#include <qpluginmanager.h>
#include <qapplicationplugin.h>
#include <perftest.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <qtopiaipcenvelope.h>

#ifndef QT_NO_SXE
#include <private/qtransportauth_qws_p.h>
#endif

#ifndef SINGLE_EXEC
QPluginManager *QuickLauncher::loader = 0;
QObject *QuickLauncher::appInstance = 0;
QApplicationFactoryInterface *QuickLauncher::appIface = 0;
#endif

QtopiaApplication *QuickLauncher::app = 0;
QPointer<QWidget> QuickLauncher::mainWindow;
bool QuickLauncher::validExitLoop = false;
bool QuickLauncher::needsInit = false;
QEventLoop *QuickLauncher::eventLoop = 0;

char **QuickLauncher::argv0 = 0;
int QuickLauncher::argv_lth;

#if defined(QTOPIA_DBUS_IPC)
// For quicklaunched apps
class WaitForRaiseTask : public QObject
{
public:
    static const int timeout = 1000;

    WaitForRaiseTask(QObject * parent);
};

WaitForRaiseTask::WaitForRaiseTask(QObject *parent) : QObject(parent)
{
    QTimer::singleShot(timeout, this, SLOT(deleteLater()));
}
#endif

#ifdef Q_OS_LINUX
extern char **environ;
#ifndef SPT_BUFSIZE
#define SPT_BUFSIZE     2048
#endif
#include <stdarg.h>
void setproctitle (const char *fmt,...) {
    int        i;
    char       buf[SPT_BUFSIZE];
    va_list    ap;

    if (!QuickLauncher::argv0)
        return;

    va_start(ap, fmt);
    (void) vsnprintf(buf, SPT_BUFSIZE, fmt, ap);
    va_end(ap);

    i = strlen (buf);
    if (i > QuickLauncher::argv_lth - 2) {
        i = QuickLauncher::argv_lth - 2;
        buf[i] = '\0';
    }
    memset(QuickLauncher::argv0[0], '\0', QuickLauncher::argv_lth);       /* clear the memory area */
    (void) strcpy (QuickLauncher::argv0[0], buf);

    QuickLauncher::argv0[1] = NULL;
}
#endif

// ====================================================================

#ifdef SINGLE_EXEC
QPEAppMap *qpeAppMap() {
    static QPEAppMap *am = 0;
    if ( !am ) am = new QPEAppMap();
    return am;
}
void qtopia_registerApp(const char *name, qpeAppCreateFunc createFunc) {
    if ( name ) {
        QPEAppMap *am = qpeAppMap();
        am->insert(name, createFunc);
    }
}
QPEMainMap *qpeMainMap() {
    static QPEMainMap *am = 0;
    if ( !am ) am = new QPEMainMap();
    return am;
}
void qtopia_registerMain(const char *name, qpeMainFunc mainFunc) {
    if ( name ) {
        QPEMainMap *am = qpeMainMap();
        am->insert(name, mainFunc);
    }
}
#endif

// ====================================================================

QuickLauncher::QuickLauncher()
    : QObject()
{
    QString ch("QPE/QuickLauncher-");
    ch += QString::number(::getpid());
    qlChannel = new QtopiaChannel( ch, this);
    connect( qlChannel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(message(const QString&,const QByteArray&)) );
    QtopiaIpcEnvelope env("QPE/QuickLauncher", "available(int)");
    env << ::getpid();
}

void QuickLauncher::exec( int argc, char **argv )
{
    QuickLauncher::app->unregisterRunningTask("QuickLaunch");
    QString appName = argv[0];
    int sep = appName.lastIndexOf( '/' );
    if ( sep > 0 )
        appName = appName.mid( sep+1 );

    qLog(Performance) << appName.toLatin1().constData() << " : " << "Starting quicklauncher exec : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );

    if ( needsInit ) {
        needsInit = false;
        app->initApp( argc, argv );
    }

#if defined(QTOPIA_DBUS_IPC)
    app->registerRunningTask("WaitForDBUSRaise", new WaitForRaiseTask(0));
#endif

#ifndef SINGLE_EXEC
    qLog(Performance) << appName.toLatin1().constData() << " : " << "Before loading libraries : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
#ifndef QT_NO_SXE
    // loader invokes the constructor - need to clear the key before this
    guaranteed_memset( _key, 0, QSXE_KEY_LEN );
#endif
    appInstance = loader->instance(appName);
    qLog(Performance) << appName.toLatin1().constData() << " : " << "After loading libraries : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    appIface = qobject_cast<QApplicationFactoryInterface*>(appInstance);
    if ( !appIface ) {
        qWarning("%s: cannot load application: %s", (const char*) QFile::encodeName(appName),
        qPrintable(qt_error_string(errno)));
        exit(-1);
    }
#ifndef QT_NO_SXE
    appIface->setProcessKey( appName );
#endif
    qLog(Performance) << appName.toLatin1().constData() << " : " << "Before creating main window : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    mainWindow = appIface->createMainWindow( appName );
    qLog(Performance) << appName.toLatin1().constData() << " : " << "After creating main window : "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
#else
    if ( qpeAppMap()->contains(appName) ) {
        mainWindow = (*qpeAppMap())[appName](0, 0);
    } else {
        qFatal("Could not find application: %s", appName.toLatin1().constData());
    }
#endif
    app->setMainWidget(mainWindow);
    if ( mainWindow ) {
        if ( mainWindow->metaObject()->indexOfSlot("setDocument(QString)") != -1 ) {
            app->showMainDocumentWidget();
        } else {
            app->showMainWidget();
        }
    } else {
        qWarning( "Could not create application main window" );
        exit(-1);
    }
}

void QuickLauncher::message(const QString &msg, const QByteArray & data)
{
    QStringList argList;

    if ( msg == "execute(QStringList)" ) {
        delete qlChannel;
        QDataStream stream( data );
        QStringList argList;
        stream >> argList;
        qLog(Quicklauncher) << "QuickLauncher execute:" << argList;
        doQuickLaunch( argList );
        delete this;
    } else if ( msg == "execute(QString)" ) {
        delete qlChannel;
        QDataStream stream( data );
        QString arg;
        stream >> arg;
        qLog(Quicklauncher) << "QuickLauncher execute:" << arg;
        QStringList argList;
        argList.append( arg.toUtf8() );
        doQuickLaunch( argList );
        delete this;
    } else if ( msg == "quit()" ) {
        validExitLoop = true;
        eventLoop->exit();
        QTimer::singleShot( 5, app, SLOT(quit()) );
        qLog(Quicklauncher) << "Quicklauncher received quit: exiting...";
    }
}

void QuickLauncher::doQuickLaunch( QStringList &argList )
{
    static int myargc = argList.count();
    static char **myargv = new char *[myargc + 1];
    for ( int j = 0; j < myargc; j++ ) {
        myargv[j] = new char [strlen(argList.at(j).toLocal8Bit().constData())+1];
        strcpy( myargv[j], argList.at(j).toLocal8Bit().constData() );
    }
    myargv[myargc] = NULL;
#ifdef Q_OS_LINUX
    // Change name of process
    setproctitle(myargv[0]);
#endif

    validExitLoop = true;
    needsInit = true;
    eventLoop->exit();
    exec( myargc, myargv );
}

