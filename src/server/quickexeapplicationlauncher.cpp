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

// Local includes
#include "quickexeapplicationlauncher.h"
#include "applicationlauncher.h"
#include "qcopfile.h"

// Qtopia includes
#include <Qtopia>
#include <QtopiaChannel>
#include <QtopiaIpcEnvelope>

// Qt includes
#include <QProcess>
#include <QFile>
#include <QTimer>

// Systme includes
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
// Constants
static const int NUM_POOLED_QLPROCESSES = 2;

// ============================================================================
//
// QlProcessInfo
//
// ============================================================================

class QlProcessInfo
{
public:
    QlProcessInfo( QProcess* process )
    : mProcess( process ), mReady( false ) {};

    QProcess* mProcess;
    bool mReady;
};

// ============================================================================
//
// QuickExeApplicationLauncherPrivate
//
// ============================================================================

class QuickExeApplicationLauncherPrivate
{
public:
    QuickExeApplicationLauncherPrivate() : mQlProcesses() {}

    void addProcess( QProcess* process );
    QProcess* removeProcess( const int pid );
    void processAvailable( const int pid );
    bool ready() const;
    QProcess* nextProcess();
    bool createProcess() const;

private:
    QList<QlProcessInfo*> mQlProcesses;
};

void QuickExeApplicationLauncherPrivate::addProcess( QProcess* process )
{
    mQlProcesses.append( new QlProcessInfo( process ) );
}

void QuickExeApplicationLauncherPrivate::processAvailable( const int pid )
{
    foreach( QlProcessInfo* info, mQlProcesses ) {
        if ( info->mProcess->pid() == pid ) {
            info->mReady = true;
            break;
        }
    }
}

QProcess* QuickExeApplicationLauncherPrivate::removeProcess( const int pid )
{
    for ( int i=0; i<mQlProcesses.count(); ++i ) {
        if ( mQlProcesses[i]->mProcess->pid() == pid ) {
            QlProcessInfo* info = mQlProcesses.takeAt( i );
            QProcess* process = info->mProcess;
            delete info;
            return process;
        }
    }

    return 0;
}

bool QuickExeApplicationLauncherPrivate::ready() const
{
    foreach( QlProcessInfo* info, mQlProcesses ) {
        if ( info->mReady )
            return true;
    }

    return false;
}

QProcess* QuickExeApplicationLauncherPrivate::nextProcess()
{
    for ( int i=0; i<mQlProcesses.count(); ++i ) {
        if ( mQlProcesses[i]->mReady ) {
            QlProcessInfo* info = mQlProcesses.takeAt( i );
            QProcess* process = info->mProcess;
            delete info;
            return process;
        }
    }

    return 0;
}

bool QuickExeApplicationLauncherPrivate::createProcess() const
{
    if ( mQlProcesses.count() < NUM_POOLED_QLPROCESSES )
        return true;

    return false;
}

/*!
  \class QuickExeApplicationLauncher
  \ingroup QtopiaServer::AppLaunch
  \brief The QuickExeApplicationLauncher class supports launching "quicklaunched" QtopiaApplication applications.

  The QuickExeApplicationLauncher class provides the ApplicationLauncherType
  implementation that works with the \c {quicklauncher} executable to accelerate
  application launching.

  When Qtopia applications start, there are many non-application-specific tasks
  they must perform during startup.  For example, every Qtopia application
  dynamically links against the Qtopia libraries and instantiates a
  QtopiaApplication instance.  These tasks are independent of any specifics
  regarding the application.

  The Quicklauncher process attempts to minimize the inefficiencies of these
  predictable tasks by performing them before any specific application is
  launched.  The system consists of two main parts - the \c {quicklauncher}
  binary and the application launcher interface provided by this class.

  When the QuickExeApplicationLauncher is first instantiated, it spawns a
  copy of the \c {quicklauncher} executable.  The \c {quicklauncher} executable,
  like every other Qtopia application, links against the Qtopia libraries,
  instantiates a QtopiaApplication instance and other performs all the other
  common, non-application-specific tasks.  Once these tasks are completed,
  however, the \c {quicklauncher} stub enters an idle loop awaiting instruction
  from the QuickExeApplicationLauncher.

  When the user attempts to launch an application with the \c {quicklauncher}
  running, the QuickExeApplicationLauncher class checks to see whether the
  application was compiled to support Quicklaunching.  An application that
  supports Quicklaunching is compiled as a shared library, not an executable.
  This shared library has a well known entry point that can be called to
  run application specific code.  The build system documentation contains more
  information on compiling an application to support Quicklaunching.

  If the application does support Quicklaunching, a message is sent to the idle
  \c {quicklauncher} instance.  This instance then loads the application
  shared library and immediately begins executing application specific code,
  bypassing the non-application-specific portions that it executed earlier in
  anticipation of an application launch.

  Once a \c {quicklauncher} instance has transformed itself into a running
  application, the QuickExeApplicationLauncher class starts another.

  Both the QuickExeApplicationLauncher and the \c {quicklauncher} itself search
  the paths returned by the Qtopia::installPaths() method for the
  \c {plugins/application/lib<application name>.so} application plugin.

  XXX - \c {quicklauncher} itself does not search correctly.

  The QuickExeApplicationLauncher class provides the
  \c {QuickExeApplicationLauncher} task.
 */
QTOPIA_TASK(QuickExeApplicationLauncher, QuickExeApplicationLauncher);
QTOPIA_TASK_PROVIDES(QuickExeApplicationLauncher, ApplicationTypeLauncher);

/*!
  Constructs a new QuickExeApplicationLauncher instance.
 */
QuickExeApplicationLauncher::QuickExeApplicationLauncher()
: d( new QuickExeApplicationLauncherPrivate )
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/QuickLauncher", this);
    connect( channel,
             SIGNAL( received( const QString&, const QByteArray&) ),
             this,
             SLOT( quickLauncherChannel( const QString&, const QByteArray& ) ) );

    respawnQuicklauncher(false);
}

/*!
  Destroys the QuickExeApplicationLauncher instance.
 */
QuickExeApplicationLauncher::~QuickExeApplicationLauncher()
{
    delete d;
}

/*! \internal */
bool QuickExeApplicationLauncher::canLaunch( const QString &app )
{
    if ( !d->ready() )
        return false;

    QStringList plugins = quicklaunchPlugin(app);
    for ( int ii = 0; ii < plugins.count(); ++ii )
        if ( QFile::exists( plugins.at(ii) ) )
            return true;

    return false;
}

/*! \internal */
void QuickExeApplicationLauncher::launch(const QString &app)
{
    if ( isRunning( app ) )
        return;

    Q_ASSERT( canLaunch( app ) );

    QProcess* process = d->nextProcess();
    Q_ASSERT( process );

    QString qlch("QPE/QuickLauncher-");
    qlch += QString::number( process->pid() );

    QtopiaIpcEnvelope env( qlch, "execute(QStringList)" );
    QStringList args;
    args << app << "-noshow";
    env << args;

    process->disconnect(); // We don't want error signals anymore
    addStartingApplication( app, process );

    if ( d->createProcess() )
        respawnQuicklauncher( false );
}

/*! \internal */
void QuickExeApplicationLauncher::quickLauncherChannel(const QString &message,
                                                       const QByteArray &data)
{
    if ( "available(int)" == message ) {
        QDataStream ds(data);
        int pid;
        ds >> pid;
        d->processAvailable( pid );
    }

    if ( d->createProcess() )
        respawnQuicklauncher( true );
}

/*! \internal */
void QuickExeApplicationLauncher::startNewQuicklauncher()
{
    // Create the new quicklauncher process
    QProcess* process = new QProcess( this );

    process->setReadChannelMode( QProcess::ForwardedChannels );
    process->closeWriteChannel();
    connect( process, SIGNAL( finished( int ) ),
             this, SLOT( qlProcessExited( int ) ) );
    connect( process, SIGNAL( error( QProcess::ProcessError ) ),
             this, SLOT( qlProcessError( QProcess::ProcessError ) ) );
    d->addProcess( process );

    // Determine if LD_BIND_NOW was in the environment before the server started
    static int unset_bind = -1;
    if ( unset_bind == -1 ) {
        const char *bind_now = getenv("LD_BIND_NOW");
        if ( bind_now && ::strlen(bind_now) != 0 ) {
            unset_bind = 0;
        } else {
            unset_bind = 1;
        }
    }

    // Ensure LD_BIND_NOW is set when launching Quicklauncher. If this isn't done,
    // quicklauncher is slower and i18n issues can appear.
    ::setenv( "LD_BIND_NOW", "1", 1 );

    process->start( quicklaunchExecutable() );
    if ( ::getuid() == 0 )
        ::setpriority( PRIO_PROCESS, process->pid(), 19 );

    // Unset LD_BIND_NOW if it wasn't already set (to make non-quicklaunched app launching faster)
    if ( unset_bind )
        unsetenv( "LD_BIND_NOW" );
}

/*! \internal */
void QuickExeApplicationLauncher::qlProcessExited( int pid )
{
    QProcess* process = d->removeProcess( pid );
    if ( process != 0 ) {
        process->disconnect();
        process->deleteLater();

        if ( d->createProcess() )
            respawnQuicklauncher( false );
    }
}

/*! \internal */
void QuickExeApplicationLauncher::qlProcessError( QProcess::ProcessError )
{
    if ( sender() != 0 ) {
        QProcess* process
            = d->removeProcess( qobject_cast<QProcess*>( sender() )->pid() );
        process->disconnect();
        process->deleteLater();

        if ( d->createProcess() )
            respawnQuicklauncher( false );
    }
}

/*! \internal */
void QuickExeApplicationLauncher::respawnQuicklauncher( bool fast )
{
    QTimer::singleShot( fast ? 500 : 5000,
                        this,
                        SLOT( startNewQuicklauncher() ) );
}

/*! \internal */
QStringList QuickExeApplicationLauncher::quicklaunchPlugin( const QString &app )
{
    QStringList rv;
    QStringList paths = Qtopia::installPaths();
    for( int ii = 0; ii < paths.count(); ++ii ) {
        rv.append(paths.at(ii) + "plugins/application/lib" + app + ".so");
    }
    return rv;
}

/*! \internal */
QString QuickExeApplicationLauncher::quicklaunchExecutable()
{
    return Qtopia::qtopiaDir() + "bin/quicklauncher";
}

