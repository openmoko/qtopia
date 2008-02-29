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

#include "applicationlauncher.h"
#include <Qtopia>
#include <QFile>
#include <QProcess>
#include <QTimer>
#include <qwindowsystem_qws.h>
#include <QValueSpaceObject>
#include "qcoprouter.h"
#include "qcopfile.h"
#include <QtopiaApplication>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaabstractservice.h>
#include <qtopialog.h>
#include <QWSServer>
#include <QContent>
#include <QContentFilter>
#include <QContentSet>

#ifndef QT_NO_SXE
#include "sandboxedprocess.h"
#endif

/*!
  \class ApplicationIpcRouter::RouteDestination
  \brief The ApplicationIpcRouter::RouteDestination class represents an IPC route destination.

  A route destination allows an ApplicationTypeLauncher to add a manual
  transport route to the ApplicationIpcRouter instance.  Manual transport routes
  are usefull to adapt the system's primary IPC transport to other transport
  models.

  Please refer to the  \l {Application Control Subsystem} documentation for
  more information on how the transport routes fit into the system.
 */

/*!
  \fn ApplicationIpcRouter::RouteDestination::~RouteDestination()

  \internal
*/

/*!
  \fn void ApplicationIpcRouter::RouteDestination::routeMessage(const QString &appName, const QString &message, const QByteArray &data)

  Invoked by the system's IpcRouter to deliver an IPC message.  \a appName is
  set to the name of the application the message should be delivered to (the
  "destination").  \a message and \a data represent the message itself.
 */

/*!
  \class ApplicationIpcRouter
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationIpcRouter class provides an interface through which
         ApplicationTypeLauncher instances to control IPC message routing.

  Generally only one task in the system implements the ApplicationIpcRouter task
  interface.  This task is known as the system's IPC Router and is broadly
  responsible for coupling the configured IPC system into Qtopia.  Currently
  Qtopia only supports the both QCop IPC system, but could conceivably
  support other transport systems in the future.

  Please refer to the  \l {Application Control Subsystem} documentation for
  more information on how the IpcRouter fits into the system.
 */

/*!
  \fn void ApplicationIpcRouter::addRoute(const QString &app, RouteDestination *dest)

  Add a manual transport route for \a app to \a dest to the router.  An IPC
  Router \bold {must} be able to deliver any pending messages to an installed
  route \bold {immediately}.
 */

/*!
  \fn void ApplicationIpcRouter::remRoute(const QString &app, RouteDestination *dest)
  Remove a manual transport route for \a app to \a dest from the router.
 */

/*!
  \class ApplicationTypeLauncher
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationTypeLauncher class provides an interface to control
         a particular application type in the system.

  Implementers of ApplicationTypeLauncher are consumed primarily by the
  ApplicationLauncher.
 */

/*!
  \enum ApplicationTypeLauncher::TerminationReason

  Represents the reason an application terminated.

  \value Normal The application terminated normally.
  \value Killed The application was killed by the system.
  \value FailedToStart The application failed to start.
  \value Crashed The application crashed.
  \value Unknown The application terminated for an unknown reason.
*/

/*!
  \enum ApplicationTypeLauncher::ApplicationState

  Represents the state of an application.

  \value NotRunning The application is not running.
  \value Starting The application is starting.
  \value Running The application is running.
 */

/*!
  \fn ApplicationTypeLauncher::ApplicationState ApplicationTypeLauncher::applicationState(const QString &application)

  Returns the current \a application state.
 */

/*!
  \fn bool ApplicationTypeLauncher::canLaunch(const QString &application)

  Returns true if the \a application can by launched by this launcher instance.
 */

/*!
  \fn void ApplicationTypeLauncher::launch(const QString &application)

  Requests that the \a application be launched.
 */

/*!
  \fn void ApplicationTypeLauncher::kill(const QString &application)

  Requests that the \a application be killed.
 */

/*!
  \fn void ApplicationTypeLauncher::applicationStateChanged(const QString &application, ApplicationTypeLauncher::ApplicationState state)

  Emitted whenever the \a application state changes.  \a state will be the
  application's new state.
*/

/*!
  \fn void ApplicationTypeLauncher::terminated(const QString &application, ApplicationTypeLauncher::TerminationReason reason)

  Emitted whenever the \a application terminates.  \a reason represents the
  cause of the termination.
*/

/*!
  \class ApplicationTerminationHandler
  \ingroup QtopiaServer::AppLaunch
  \ingroup QtopiaServer::Task::Interfaces
  \brief The ApplicationTerminationHandler class allows tasks to be notified,
         and possibly filter, when an application terminates.

  When an application terminates, the ApplicationLauncher notifies all tasks, in
  order, that implement the ApplicationTerminationHandler interface.  A task
  that implements the ApplicationTeminationHandler interface can filter the
  termination notification by returning true from its terminated() notification
  method.  Filtering a termination will prevent subsequently ordered tasks from
  being notified.

  Regardless of whether or not a task is filtered, the ApplicationLauncher
  class will still emit the global ApplicationLauncher::applicationTerminated()
  signal, albeit with the \bold filtered parameter set to true.

  Termination handlers can be used to hide the termination of "system"
  applications from the user, or to implement custom termination notices.  The
  QTerminationHandler class, for example, uses a server side termination handler
  to display a more descriptive crash message when an application abnormally
  exits.
 */

/*!
  \fn ApplicationTerminationHandler::ApplicationTerminationHandler(QObject *parent = 0)

  Construct a new ApplicationTerminationHandler interface with the given
  \a parent.
 */

/*!
  \fn bool ApplicationTerminationHandler::terminated(const QString &appName,
                            ApplicationTypeLauncher::TerminationReason reason)

  Invoked when an application terminates.  \a appName is the name of the
  application that terminated and \a reason is the way in which it terminated.

  Returning false from this method allows the termination notification to
  continue to propagate to other ApplicationTerminationHandler instances.
  Returning true stops further propagation.  Implementors should only return
  true if they intend to notify the user of the termination, or if they have
  determined that notification is not required.
 */

class LegacyLauncherService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    LegacyLauncherService( QObject *parent )
        : QtopiaAbstractService( "Launcher", parent )
        { publishAll(); }

public:
    ~LegacyLauncherService();

public slots:
    void execute( const QString& app );
    void execute( const QString& app, const QString& document );
};

// declare QtopiaServerApplicationLauncher
class QtopiaServerApplicationLauncher : public ApplicationTypeLauncher,
                                        public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    QtopiaServerApplicationLauncher();

    // ApplicationTypeLauncher
    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);
    virtual void kill(const QString &app);
    virtual ApplicationState applicationState(const QString &app);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);
};
QTOPIA_TASK(QtopiaServerApplicationLauncher, QtopiaServerApplicationLauncher);
QTOPIA_TASK_PROVIDES(QtopiaServerApplicationLauncher, ApplicationTypeLauncher);

// define QtopiaServerApplicationLauncher
QtopiaServerApplicationLauncher::QtopiaServerApplicationLauncher()
{
}

bool QtopiaServerApplicationLauncher::canLaunch(const QString &app)
{
    return QtopiaApplication::applicationName() == app;
}

void QtopiaServerApplicationLauncher::launch(const QString &app)
{
    Q_ASSERT(canLaunch(app));
#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->addRoute(app, this);
#endif
    emit applicationStateChanged(app, Starting);
    emit applicationStateChanged(app, Running);
}

void QtopiaServerApplicationLauncher::kill(const QString &)
{
    // Cannot kill the application launcher
}

QtopiaServerApplicationLauncher::ApplicationState QtopiaServerApplicationLauncher::applicationState(const QString &app)
{
    Q_ASSERT(canLaunch(app));
    Q_UNUSED(app);
    return Running;
}

void QtopiaServerApplicationLauncher::routeMessage(const QString &app,
                                                   const QString &message,
                                                   const QByteArray &data)
{
#ifndef QTOPIA_DBUS_IPC
    QCopFile::writeQCopMessage(app, message, data);
    QtopiaChannel::send("QPE/pid/" + QString::number(::getpid()),
                        "QPEProcessQCop()");
#endif
}

// declare ExeApplicationLauncherPrivate
struct ExeApplicationLauncherPrivate {
    struct RunningProcess {
        RunningProcess()
            : proc(0) {}
        RunningProcess(const QString &_a, QProcess *_p,
                       ApplicationTypeLauncher::ApplicationState _s)
            : app(_a), proc(_p), state(_s), pidChannelOpen( false ) {}
        ~RunningProcess()
        {
            if(proc) {
                proc->disconnect();
                proc->deleteLater();
            }
        }

        QString app;
        QProcess *proc;
        ApplicationTypeLauncher::ApplicationState state;
        bool pidChannelOpen;
    };

    RunningProcess *runningProcess(const QString &);
    RunningProcess *runningProcess(int);
    RunningProcess *runningProcess(QProcess *);

    QMap<QString, RunningProcess *> m_runningProcesses;
};

/*!
  \class ExeApplicationLauncher
  \ingroup QtopiaServer::AppLaunch
  \brief The ExeApplicationLauncher class simplifies implementing ApplicationTypeLauncher for process based applications.

  Many application types are just different ways of starting a QtopiaApplication
  application.  For example, the SimpleExeApplicationLauncher and the
  QuickExeApplicationLauncher types both start external processes that
  instantiate a QtopiaApplication instance.  The ExeApplicationLauncher
  encapsulates the commonality between these types of application launchers.

  It is the responsibility of derived classes to respond to the canLaunch() and
  launch() methods of the ApplicationTypeLauncher interface.  Once the derived
  class has brung a process to the state where it is launching, it should call
  addStartingApplication() to pass ownership of the process to the
  ExeApplicationLauncher.  The ExeApplicationLauncher class operates under
  the following assumptions:
  \list 1
  \i The application will send an \c {available(QString,int)} message to the
     \c {QPE/QtopiaApplication} channel when it has completed startup.
  \i The process, and thus the QProcess instance, will terminate normally when
     it is done.
  \endlist
 */

// define ExeApplicationLauncher
/*!
  Construct a new ExeApplicationLauncher instance.
  */
ExeApplicationLauncher::ExeApplicationLauncher()
: d(new ExeApplicationLauncherPrivate)
{
    QtopiaChannel *channel = new QtopiaChannel("QPE/QtopiaApplication", this);
    connect(channel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(qtopiaApplicationChannel(const QString&,const QByteArray&)));

    connect( qwsServer,
             SIGNAL( newChannel( const QString& ) ),
             this,
             SLOT( newChannel( const QString& ) ) );
}

/*!
  Destroy the ExeApplicationLauncher instance.
  */
ExeApplicationLauncher::~ExeApplicationLauncher()
{
    delete d;
}

/*!
  Add a new process, \a proc, for the applications \a app to the
  ExeApplicationLauncher.  The ExeApplicationLauncher will take ownership of the
  process.
  */
void ExeApplicationLauncher::addStartingApplication(const QString &app,
                                                    QProcess *proc)
{
    connect(proc, SIGNAL(finished(int)),
            this, SLOT(appExited(int)));
    connect(proc, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(appError(QProcess::ProcessError)));

    d->m_runningProcesses.insert(app,
                                 new ExeApplicationLauncherPrivate::RunningProcess(app, proc, Starting));
#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->addRoute(app, this);
#endif
    emit applicationStateChanged(app, Starting);
}

/*!
  Returns true if \a app is already being managed by the ExeApplicationLauncher
  (that is, addStartingApplication() has been called), otherwise returns false.
  */
bool ExeApplicationLauncher::isRunning(const QString &app)
{
    return d->m_runningProcesses.contains(app);
}

/*! \internal */
void ExeApplicationLauncher::kill(const QString &app)
{
    ExeApplicationLauncherPrivate::RunningProcess *rp = d->runningProcess(app);
    if(!rp) return;
    rp->proc->kill();
}

/*! \internal */
ExeApplicationLauncher::ApplicationState
ExeApplicationLauncher::applicationState(const QString &app)
{
    ExeApplicationLauncherPrivate::RunningProcess *rp = d->runningProcess(app);
    if(!rp) return NotRunning;
    else return rp->state;
}

/*! \internal */
void ExeApplicationLauncher::routeMessage(const QString &app,
                                          const QString &message,
                                          const QByteArray &data)
{
#ifndef QTOPIA_DBUS_IPC
    ExeApplicationLauncherPrivate::RunningProcess *rp = d->runningProcess(app);
    Q_ASSERT(rp);

    QCopFile::writeQCopMessage(app, message, data);
    QtopiaChannel::send("QPE/pid/" + QString::number(rp->proc->pid()),
                        "QPEProcessQCop()");
#endif
}

/*! \internal */
void ExeApplicationLauncher::appExited(int code)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ExeApplicationLauncherPrivate::RunningProcess *rp = d->runningProcess(proc);
    Q_ASSERT(rp);
    Q_ASSERT(NotRunning != rp->state);

#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->remRoute(rp->app, this);
#endif

    if(Starting == rp->state) {
        rp->state = NotRunning;
        emit terminated(rp->app, FailedToStart);
        emit applicationStateChanged(rp->app, NotRunning);
    } else {
        rp->state = NotRunning;
        emit terminated(rp->app, Normal);
        emit applicationStateChanged(rp->app, NotRunning);
    }

    d->m_runningProcesses.remove(rp->app);
    delete rp;
}

/*! \internal */
void ExeApplicationLauncher::appError(QProcess::ProcessError error)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ExeApplicationLauncherPrivate::RunningProcess *rp = d->runningProcess(proc);
    Q_ASSERT(rp);
    Q_ASSERT(NotRunning != rp->state);

    TerminationReason reason = Unknown;
    switch(error) {
        case QProcess::FailedToStart:
            reason = FailedToStart;
            break;
        case QProcess::Crashed:
            reason = Crashed;
            break;
        default:
            break;
    };

    rp->state = NotRunning;
    d->m_runningProcesses.remove(rp->app);

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("notBusy(QString)") );
        e << rp->app;
    }

#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->remRoute(rp->app, this);
#endif

    emit terminated(rp->app, reason);
    emit applicationStateChanged(rp->app, NotRunning);
    delete rp;
}

/*! \internal */
void ExeApplicationLauncher::qtopiaApplicationChannel(const QString &message,
                                                      const QByteArray &data)
{
    if(message == "available(QString,int)") {
        QDataStream ds(data);
        QString name;
        int pid;
        ds >> name >> pid;
        ExeApplicationLauncherPrivate::RunningProcess *rp =
            d->runningProcess(name);
        if(rp && rp->proc->pid() == pid) {
            rp->state = Running;
            emit applicationStateChanged(rp->app, Running);
            return;
        }
    }
}

/*! \internal */
void ExeApplicationLauncher::newChannel( const QString& ch )
{
    QString pidChannel = "QPE/pid/"; // No tr
    if ( ch.startsWith( pidChannel ) ) {
        int pid = ch.mid( pidChannel.count() ).toInt();
        ExeApplicationLauncherPrivate::RunningProcess *rp =
            d->runningProcess( pid );
        if ( rp != 0 ) {
            rp->pidChannelOpen = true;
        }
    }
}

ExeApplicationLauncherPrivate::RunningProcess *
ExeApplicationLauncherPrivate::runningProcess(const QString &app)
{
    QMap<QString, RunningProcess *>::Iterator iter =
        m_runningProcesses.find(app);
    if(iter == m_runningProcesses.end())
        return 0;
    else
        return *iter;
}

ExeApplicationLauncherPrivate::RunningProcess *
ExeApplicationLauncherPrivate::runningProcess(int pid)
{
    for(QMap<QString, RunningProcess *>::Iterator iter = m_runningProcesses.begin();
            iter != m_runningProcesses.end();
            ++iter) {
        if((*iter)->proc->pid() == pid)
            return *iter;
    }
    return 0;
}

ExeApplicationLauncherPrivate::RunningProcess *
ExeApplicationLauncherPrivate::runningProcess(QProcess *proc)
{
    for(QMap<QString, RunningProcess *>::Iterator iter = m_runningProcesses.begin();
            iter != m_runningProcesses.end();
            ++iter) {
        if((*iter)->proc == proc)
            return *iter;
    }
    return 0;
}

// define SimpleExeApplicationLauncher
/*!
  \class SimpleExeApplicationLauncher
  \ingroup QtopiaServer::AppLaunch
  \brief The SimpleExeApplicationLauncher class supports launching regular
         QtopiaApplication executables.

  The SimpleExeApplicationLauncher class provides the ApplicationLauncherType
  implementation for simple, executable based applications.  It also
  (implicitly) doubles as the fallback for the QuickExeApplicationLauncher.

  If the application requested is an absolute path, it is run as is.  Otherwise,
  the paths returned by the Qtopia::installPaths() method are searched for
  \c {bin/<application name>}.

  The SimpleExeApplicationLauncher class provides the
  \c {SimpleExeApplicationLauncher} task.
*/
QTOPIA_TASK(SimpleExeApplicationLauncher, SimpleExeApplicationLauncher);
QTOPIA_TASK_PROVIDES(SimpleExeApplicationLauncher, ApplicationTypeLauncher);

/*!
  Constructs a new SimpleExeApplicationLauncher instance.
 */
SimpleExeApplicationLauncher::SimpleExeApplicationLauncher()
: d(0)
{
}

/*!
  Destroys the SimpleExeApplicationLauncher instance.
 */
SimpleExeApplicationLauncher::~SimpleExeApplicationLauncher()
{
}

/*! \internal */
bool SimpleExeApplicationLauncher::canLaunch(const QString &app)
{

    // Check whether the executable exists
    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii)
        if(QFile::exists(exes.at(ii)))
            return true;

    return false;
}

/*! \internal */
void SimpleExeApplicationLauncher::launch(const QString &app)
{
    if(isRunning(app))
        return; // We're already launching/have launched this guy

    Q_ASSERT(canLaunch(app));

    // We need to launch it
    QProcess *proc = new QProcess(this);
    proc->setReadChannelMode(QProcess::ForwardedChannels);
    proc->closeWriteChannel();

    QStringList args;
    args.append("-noshow");

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {
            proc->start(exes.at(ii), args);
            addStartingApplication(app, proc);
            return; // Found and done
        }
    }
}

/*! \internal */
QStringList
SimpleExeApplicationLauncher::applicationExecutable(const QString &app)
{
    if ( app.startsWith( "/" )) // app is a path, not just in standard location
        return QStringList() << app;

    QStringList rv;
    QStringList paths = Qtopia::installPaths();
    for(int ii = 0; ii < paths.count(); ++ii)
        rv.append(paths.at(ii) + "bin/" + app);

    return rv;
}

#ifndef QT_NO_SXE
/*!
  \class SandboxedExeApplicationLauncher
  \ingroup QtopiaServer::AppLaunch
  \brief The SandboxedExeApplicationLauncher class supports launching untrusted
         installed application executables.

  \bold {Note:} This class is only relevant if SXE is enabled

  The SanboxedExeApplicationLauncher class provides the ApplicationLauncherType
  implementation for simple but untrusted executable based applications (which
  have been installed via packagemanager).  The executable is run under
  sandboxed conditions so as to minimize any potential damage the application
  may cause.

  If the application requested is an absolute path containing Qtopia::packagePath(),
  it is run with restrictions;  otherwise Qtopia::packagePath()/bin is searched.

  The SandboxedExeApplicationLauncher class provides the
  \c {SanboxedExeApplicationLauncher} task.
*/

QTOPIA_TASK(SandboxedExeApplicationLauncher, SandboxedExeApplicationLauncher);
QTOPIA_TASK_PROVIDES(SandboxedExeApplicationLauncher, ApplicationTypeLauncher);

// define SandboxedExeApplicationLauncher
/*!
  Constructs a new SandboxedExeApplicationLauncher instance.
 */
SandboxedExeApplicationLauncher::SandboxedExeApplicationLauncher()
: d(0)
{
}

/*!
  Destroys the SandboxedExeApplicationLauncher instance.
 */
SandboxedExeApplicationLauncher::~SandboxedExeApplicationLauncher()
{
}

/*! \internal */
bool SandboxedExeApplicationLauncher::canLaunch(const QString &app)
{
    // Check whether the executable exists
    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if( QFile::symLinkTarget(exes.at(ii)).contains("__DISABLED") ) {
            QMessageBox::warning( 0,tr("Security Alert"), tr("<qt>Program has been <font color=\"#FF0000\">disabled</font></qt>"));
            return false;
        }
        if(QFile::exists(exes.at(ii)))
            return true;
    }
    return false;
}


/*! \internal */
void SandboxedExeApplicationLauncher::launch(const QString &app)
{
    if(isRunning(app))
        return; // We're already launching/have launched this guy

    Q_ASSERT(canLaunch(app));

    QStringList args;
    args.append("-noshow");

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {
            // We need to launch it
            QProcess *proc = new SandboxedProcess(this);
            proc->setReadChannelMode(QProcess::ForwardedChannels);
            proc->closeWriteChannel();

            proc->start(exes.at(ii), args);
            addStartingApplication(app, proc);
            return; // Found and done
        }
    }
}

/*! \internal */
QStringList SandboxedExeApplicationLauncher::applicationExecutable(const QString &app)
{
    if ( app.startsWith( "/" )  && app.contains(Qtopia::packagePath()) )
        return QStringList() << app;

    QStringList rv(Qtopia::packagePath() + "bin/" + app);

    return rv;
}
#endif

// declare BuiltinApplicationLauncherPrivate
struct BuiltinApplicationLauncherPrivate
{
    QMap<QString, QWidget *> runningApplications;
    QMap<QByteArray, BuiltinApplicationLauncher::BuiltinFunc> builtins;
};
Q_GLOBAL_STATIC(BuiltinApplicationLauncherPrivate, bat);

/*!
  \class BuiltinApplicationLauncher
  \ingroup QtopiaServer::AppLaunch
  \brief The BuiltinApplicationLauncher class supports launching simple
         applications that run inside the Qtopia Server process.

  The BuiltinApplicationLauncher class provides the ApplicationLauncherType
  implementation for simple applications compiled into the Qtopia Server -
  known as builtin applications.

  A builtin application must consist of a single, toplevel widget.  Generally
  builtins do not implement services, but they may.  If multiple builtins
  implement the same service, or the Qtopia Server and a builtin implements the
  same service, the behaviour is undefined.

  Adding a builtin application is a simple case of linking it into the Qtopia
  Server, and adding a QTOPIA_SIMPLE_BUILTIN() macro to your code.  The
  QTOPIA_SIMPLE_BUILTIN() macro registers the builtin application with the
  BuiltinApplicationLauncher class.  The macro takes the application name,
  and a simple static function that "launches" the application and returns a
  QWidget pointer as paramters.

  XXX - builtins may not currently implement services.

  The BuiltinApplicationLauncher class provides the
  \c {BuiltinApplicationLauncher} task.
 */

/*!
  \typedef BuiltinApplicationLauncher::BuiltinFunc

  The BuiltinFunc provides a type definition that the static function used to
  create a builtin application must conform to.  The exact specification is:
  \code
  typdef QWidget *(*BuiltinFunc)()
  \endcode
 */

/*!
  \macro QTOPIA_SIMPLE_BUILTIN(ApplicationName, createFunc)
  \relates BuiltinApplicationLauncher

  Add a new builtin application to the server.  \a ApplicationName must be the
  name of the application and \a createFunc a static function, of type
  BuiltinApplicationLauncher::BuiltinFunc, that creates the applications.  For
  example, the following code adds a simple builtin named "TestBuiltin" to the
  server.
  \code
  class TestBuiltinWindow : public QWidget
  {
     // ...
  };
  static QWidget *testBuiltinFunc()
  {
      return new TestBuiltinWindow(0);
  }
  QTOPIA_SIMPLE_BUILTIN(TestBuiltin, testBultinFunc);
  \endcode

 */
/*!
  Constructs a new BuiltinApplicationLauncher instance.
 */
BuiltinApplicationLauncher::BuiltinApplicationLauncher()
: d(0)
{
}

/*!
  Destroys the BuiltinExeApplicationLauncher instance.
 */
BuiltinApplicationLauncher::~BuiltinApplicationLauncher()
{
}

/*! \internal */
void BuiltinApplicationLauncher::routeMessage(const QString &app,
                                          const QString &msg,
                                          const QByteArray &data)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    QMap<QString, QWidget *>::Iterator iter = p->runningApplications.find(app);
    if(iter == p->runningApplications.end()) return; // No app

    QWidget *w = *iter;
    if(!w) return; // No messages we can interpret

    if("raise()" == msg) {
        w->showMaximized();
        w->raise();
    } else if("close()" == msg) {
        w->close();
    } else if("setDocument(QString)" == msg && w->metaObject()->indexOfMethod("setDocument(QString)") != -1) {
        QString document;
        QDataStream stream( data );
        stream >> document;
        QMetaObject::invokeMethod(w, "setDocument", Q_ARG(QString, document));
        w->showMaximized();
        w->raise();
    }
}

/*! \internal */
void BuiltinApplicationLauncher::appDestroyed(QObject *obj)
{
    Q_ASSERT(obj->isWidgetType());
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;
    QWidget *wid = static_cast<QWidget *>(obj);

    for(QMap<QString, QWidget *>::Iterator iter = p->runningApplications.begin();
            iter != p->runningApplications.end();
            ++iter) {
        if(*iter == wid) {
            // Found!
            emit terminated(iter.key(), Normal);
            emit applicationStateChanged(iter.key(), NotRunning);
            ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
            if(r) r->remRoute(iter.key(), this);
            p->runningApplications.erase(iter);
            return;
        }
    }
    Q_ASSERT(!"Couldn't find destroyed application.");
}

/*! \internal */
BuiltinApplicationLauncher::ApplicationState
BuiltinApplicationLauncher::applicationState(const QString &app)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return NotRunning;

    if(p->runningApplications.contains(app))
        return Running;
    else
        return NotRunning;
}

/*! \internal */
bool BuiltinApplicationLauncher::canLaunch(const QString &app)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return false;

    return p->builtins.contains(app.toAscii());
}

/*! \internal */
void BuiltinApplicationLauncher::launch(const QString &app)
{
    Q_ASSERT(canLaunch(app));

    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    if(p->runningApplications.contains(app)) return; // Already running

    QMap<QByteArray, BuiltinApplicationLauncher::BuiltinFunc>::Iterator iter =
        p->builtins.find(app.toAscii());
    Q_ASSERT(iter != p->builtins.end());

    QWidget * wid = (*iter)();
    if(!wid) {
        // Non UI builtin
        // Starts then stops
        p->runningApplications.insert(app, 0);
        emit applicationStateChanged(app, Starting);
        emit applicationStateChanged(app, Running);
        emit terminated(app, Normal);
        emit applicationStateChanged(app, NotRunning);
        p->runningApplications.remove(app);
    } else {
        // UI builtin
        p->runningApplications.insert(app, wid);
        QObject::connect(wid, SIGNAL(destroyed(QObject *)),
                         this, SLOT(appDestroyed(QObject *)));
        emit applicationStateChanged(app, Starting);
        emit applicationStateChanged(app, Running);
        ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
        if(r) r->addRoute(app, this);
        if(wid->isHidden())
            wid->deleteLater(); // Shutdown
    }
}

/*! \internal */
void BuiltinApplicationLauncher::kill(const QString &app)
{
    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    QMap<QString, QWidget *>::Iterator iter = p->runningApplications.find(app);
    if(iter != p->runningApplications.end())
        (*iter)->deleteLater();
}

/*! \internal */
void BuiltinApplicationLauncher::install(const char *_name, BuiltinFunc func)
{
    QByteArray name(_name);
    if(name.isEmpty()) return;

    BuiltinApplicationLauncherPrivate *p = bat();
    if(!p) return;

    p->builtins.insert(name, func);
}

QTOPIA_TASK(BuiltinApplicationLauncher, BuiltinApplicationLauncher);
QTOPIA_TASK_PROVIDES(BuiltinApplicationLauncher, ApplicationTypeLauncher);

// declare ConsoleApplicationLauncherPrivate
struct ConsoleApplicationLauncherPrivate
{
    ConsoleApplicationLauncherPrivate()
        : set(0) { 
            QContentFilter filter(QContent::Application);
            set = new QContentSet(filter);
        }
    
    ~ConsoleApplicationLauncherPrivate()
    {
        delete set;
        set = 0;
    }


    struct App {
        App() : process(0) { process = new QProcess(0); }
        ~App()  { process->disconnect(); process->deleteLater(); }
        QString app;
        QProcess *process;
        ConsoleApplicationLauncher::ApplicationState state;
    };

    QMap<QString, App *> apps;
    QContentSet *set;
    
    App *processToApp(QProcess *proc)
    {
        for(QMap<QString, App *>::ConstIterator iter = apps.begin();
            iter != apps.end();
            ++iter) {
            if((*iter)->process == proc)
                return *iter;
        }
        return 0;
    }
};

// define ConsoleApplicationLauncher
/*!
  \class ConsoleApplicationLauncher
  \ingroup QtopiaServer
  \brief The ConsoleApplicationLauncher class supports launching console 
         applications.
*/

ConsoleApplicationLauncher::ConsoleApplicationLauncher()
: d(new ConsoleApplicationLauncherPrivate)
{
}

ConsoleApplicationLauncher::~ConsoleApplicationLauncher()
{
    delete d;
    d = 0;
}

ConsoleApplicationLauncher::ApplicationState 
ConsoleApplicationLauncher::applicationState(const QString &app)
{
    QMap<QString, ConsoleApplicationLauncherPrivate::App *>::Iterator iter = 
        d->apps.find(app);
    if(iter == d->apps.end())
        return NotRunning;
    else
        return (*iter)->state;
}

bool ConsoleApplicationLauncher::canLaunch(const QString &app)
{
    QContent capp = d->set->findExecutable(app);
    if(!capp.isValid())
        return false;
    return capp.property("ConsoleApplication") == QLatin1String("1");
}

void ConsoleApplicationLauncher::launch(const QString &app)
{
    if(d->apps.find(app) != d->apps.end())
        return;

    Q_ASSERT(canLaunch(app));

    QStringList exes = applicationExecutable(app);
    for(int ii = 0; ii < exes.count(); ++ii) {
        if(QFile::exists(exes.at(ii))) {

            ConsoleApplicationLauncherPrivate::App *capp = new
                ConsoleApplicationLauncherPrivate::App();
            capp->app = app;
            capp->state = Starting;
            capp->process->setReadChannelMode(QProcess::ForwardedChannels);
            capp->process->closeWriteChannel();
            qLog(QtopiaServer) << "ConsoleApplicationLauncher: Starting"
                               << exes.at(ii);
            capp->process->start(exes.at(ii));

            QObject::connect(capp->process, SIGNAL(started()),
                             this, SLOT(appStarted()));
            QObject::connect(capp->process, SIGNAL(finished(int)),
                             this, SLOT(appExited(int)));
            QObject::connect(capp->process, SIGNAL(error(QProcess::ProcessError)),
                             this, SLOT(appError(QProcess::ProcessError)));

            d->apps.insert(app, capp);

#ifndef QTOPIA_DBUS_IPC
            ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
            if(r) r->addRoute(app, this);
#endif

            emit applicationStateChanged(app, Starting);
            return;
        }
    }
}

void ConsoleApplicationLauncher::appStarted()
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);

    app->state = Running;
    emit applicationStateChanged(app->app, Running);
}

void ConsoleApplicationLauncher::appExited(int)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);

    Q_ASSERT(Running == app->state);

#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->remRoute(app->app, this);
#endif

    app->state = NotRunning;
    emit terminated(app->app, Normal);
    emit applicationStateChanged(app->app, NotRunning);
    
    d->apps.remove(app->app);
    delete app;
}

void ConsoleApplicationLauncher::appError(QProcess::ProcessError error)
{
    QProcess *proc = qobject_cast<QProcess *>(sender());
    Q_ASSERT(proc);

    ConsoleApplicationLauncherPrivate::App *app = d->processToApp(proc);
    Q_ASSERT(proc);
    Q_ASSERT(NotRunning != app->state);

    TerminationReason reason = Unknown;
    switch(error) {
        case QProcess::FailedToStart:
            reason = FailedToStart;
            break;
        case QProcess::Crashed:
            reason = Crashed;
            break;
        default:
            break;
    };

    app->state = NotRunning;

#ifndef QTOPIA_DBUS_IPC
    ApplicationIpcRouter *r = qtopiaTask<ApplicationIpcRouter>();
    if(r) r->remRoute(app->app, this);
#endif

    emit terminated(app->app, reason);
    emit applicationStateChanged(app->app, NotRunning);
    
    d->apps.remove(app->app);
    delete app;
}

/*! \internal */
QStringList
ConsoleApplicationLauncher::applicationExecutable(const QString &app)
{
    if ( app.startsWith( "/" )) // app is a path, not just in standard location
        return QStringList() << app;

    QStringList rv;
    QStringList paths = Qtopia::installPaths();
    for(int ii = 0; ii < paths.count(); ++ii)
        rv.append(paths.at(ii) + "bin/" + app);

    return rv;
}

void ConsoleApplicationLauncher::kill(const QString &app)
{
}

void ConsoleApplicationLauncher::routeMessage(const QString &, const QString &, const QByteArray &)
{
    // Do nothing
}

QTOPIA_TASK(ConsoleApplicationLauncher, ConsoleApplicationLauncher);
QTOPIA_TASK_PROVIDES(ConsoleApplicationLauncher, ApplicationTypeLauncher);

/*!
  \class ApplicationLauncher
  \ingroup QtopiaServer
  \brief The ApplicationLauncher class is responsible for bringing an
         application to a point at which it can understand IPC messages.
 */

// define ApplicationLauncher
/*!
  \internal */
ApplicationLauncher::ApplicationLauncher()
: m_chan(0), m_vso(0)
{
    m_launchers = qtopiaTasks<ApplicationTypeLauncher>();
    for(int ii = 0; ii < m_launchers.count(); ++ii) {
        QObject::connect(m_launchers.at(ii),
                         SIGNAL(applicationStateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)),
                         this,
                         SLOT(stateChanged(const QString &, ApplicationTypeLauncher::ApplicationState)));
        QObject::connect(m_launchers.at(ii),
                         SIGNAL(terminated(const QString &, ApplicationTypeLauncher::TerminationReason)),
                         this,
                         SLOT(terminated(const QString &, ApplicationTypeLauncher::TerminationReason)));
    }


    m_vso = new QValueSpaceObject("/System/Applications", this);

    new LegacyLauncherService(this);
}

void ApplicationLauncher::stateChanged(const QString &app,
                                       ApplicationTypeLauncher::ApplicationState state)
{
    if(!m_runningApps.contains(app))
        return; // I don't know about this app

    if(ApplicationTypeLauncher::NotRunning == state) {
        m_vso->removeAttribute(app + "/Info");
        m_runningApps.remove(app);
        m_orderedApps.removeAll(app);
        qLog(QtopiaServer) << "ApplicationLauncher::stateChanged(" << app << ", NotRunning )";
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("notBusy(QString)") );
        e << app;
    } else {
        if(ApplicationTypeLauncher::Starting == state)
            m_vso->setAttribute(app + "/Info/LaunchTime",
                                QDateTime::currentDateTime());

        QByteArray stateText("Unknown");
        switch(state) {
            case ApplicationTypeLauncher::NotRunning:
                stateText = "NotRunning";
                break;
            case ApplicationTypeLauncher::Starting:
                stateText = "Starting";
                break;
            case ApplicationTypeLauncher::Running:
                stateText = "Running";
                break;
        }

        if(state != ApplicationTypeLauncher::Starting)
        {
            QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("notBusy(QString)") );
            e << app;
        }

        m_vso->setAttribute(app + "/Info/State", stateText);
        qLog(QtopiaServer) << "ApplicationLauncher::stateChanged(" << app << ", " << stateText << ")";
        qLog(Performance) << "QtopiaServer : " << "ApplicationLauncher::stateChanged(" << app << ", "
                          << stateText << ") : " << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    }

    emit applicationStateChanged(app, state);
}

/*!
  Return the list of starting and running applications.  The list order
  represents the order in which the applications were launched.
  */
QStringList ApplicationLauncher::applications() const
{
    return m_orderedApps;
}

/*!
  Returns the current state of the application, \a app.
 */
ApplicationTypeLauncher::ApplicationState
ApplicationLauncher::state(const QString &app) const
{
    QMap<QString, ApplicationTypeLauncher *>::ConstIterator iter =
        m_runningApps.find(app);
    if(iter != m_runningApps.end())
        return (*iter)->applicationState(app);
    else
        return ApplicationTypeLauncher::NotRunning;
}

/*!
  \fn void ApplicationLauncher::applicationTerminated(const QString &application, ApplicationTypeLauncher::TerminationReason reason, bool filtered)

  Emitted whenever the \a application terminates.  \a reason will be set to the
  termination reason.  If \a filtered is true, an ApplicationTerminationHandler
  instance has filtered the termination.
 */

/*!
  \fn void ApplicationLauncher::applicationStateChanged(const QString &application, ApplicationTypeLauncher::ApplicationState state)

  Emitted whenever the \a application state changes to \a state.
 */

/*!
  \fn void ApplicationLauncher::applicationNotFound(const QString &application)

  Emitted whenever a non-existant \a application is requested to launch.  The
  corresponding call to launch() will also return false.
 */

void ApplicationLauncher::terminated(const QString &app,
                                     ApplicationTypeLauncher::TerminationReason reason)
{
    m_vso->removeAttribute(app + "/Info");
    m_runningApps.remove(app);
    m_orderedApps.removeAll(app);

    bool filtered = false;
    QList<ApplicationTerminationHandler *> termHandlers =
        qtopiaTasks<ApplicationTerminationHandler>();
    for(int ii = 0; !filtered && ii < termHandlers.count(); ++ii)
        filtered = termHandlers.at(ii)->terminated(app, reason);

    if(qLogEnabled(QtopiaServer)) {
        QString reasonText("Unknown");
        switch(reason) {
            case ApplicationTypeLauncher::FailedToStart:
                reasonText = "FailedToStart";
                break;
            case ApplicationTypeLauncher::Crashed:
                reasonText = "Crashed";
                break;
            case ApplicationTypeLauncher::Unknown: 
                reasonText = "Unknown"; 
                break;
            case ApplicationTypeLauncher::Normal: 
                reasonText = "Normal";
                break;
            case ApplicationTypeLauncher::Killed:
                reasonText = "Killed";
                break;
        }

        qLog(QtopiaServer) << "ApplicationLauncher::applicationTerminated(" << app << ", " << reasonText << ", " << (filtered?"true":"false") << ")";
    }

    emit applicationTerminated(app, reason, filtered);

    qLog(QtopiaServer) << "ApplicationLauncher::stateChanged(" << app << ", NotRunning)";
    emit applicationStateChanged(app, ApplicationTypeLauncher::NotRunning);
}

/*!
  Returns true if the application, \a app, can be launched otherwise false.
  If this method returns true it indicates that an ApplicationLauncherType
  instance in the system has claimed responsibility for launching the
  application.  The actual launching process may still fail.
 */
bool ApplicationLauncher::canLaunch(const QString &app)
{
    if(m_runningApps.contains(app))
        return true;
    for(int ii = 0; ii < m_launchers.count(); ++ii) {
        if(m_launchers[ii]->canLaunch(app)) {
            return true;
        }
    }
    return false;
}

/*!
  Attempt to launch the application, \a app.  This method returns true if the
  application was launched and false otherwise.
  */
bool ApplicationLauncher::launch(const QString &app)
{
    if(m_runningApps.contains(app))
        return true;

    for(int ii = 0; ii < m_launchers.count(); ++ii) {
        if(m_launchers[ii]->canLaunch(app)) {
            m_runningApps.insert(app, m_launchers[ii]);
            m_orderedApps.append(app);
            m_launchers[ii]->launch(app);
            return true;
        }
    }

    emit applicationNotFound(app);
    return false;
}

/*!
  \internal

  Kill the application, \a app.  This method is currently not supported.
  */
bool ApplicationLauncher::kill(const QString &app)
{
    Q_UNUSED( app );

    qFatal("Not supported");

    return false;
}

QTOPIA_TASK(ApplicationLauncher, ApplicationLauncher);
QTOPIA_TASK_PROVIDES(ApplicationLauncher, ApplicationLauncher);

/*
  The LegacyLauncherService exists to provide the "Launcher" service used by
  the Qtopia::execute() method.  In Qtopia 4.1 it was an "external" service,
  for use by any application.
 */
LegacyLauncherService::~LegacyLauncherService()
{
}

void LegacyLauncherService::execute( const QString& app )
{
    qLog(ApplicationLauncher) << "LegacyLauncherService: Request for execute("
                              << app << ")";
    qLog(Performance) << "ApplicationLauncher : " << "LegacyLauncherService: Request for execute("
                      << app << ") : " << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    QtopiaIpcEnvelope env("QPE/Application/" + app, "raise()");
}

void LegacyLauncherService::execute(const QString& app,
                                    const QString& document )
{
    qLog(ApplicationLauncher) << "LegacyLauncherService: Request for execute("
                              << app << ", " << document << ")";
    qLog(Performance) << "ApplicationLauncher : " << "LegacyLauncherService: Request for execute("
                      << app << ", " << document << ") : " << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    QtopiaIpcEnvelope env("QPE/Application/" + app, "setDocument(QString)");
    env << document;
}

#include "applicationlauncher.moc"
