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

#ifndef _APPLICATIONLAUNCHER_H_
#define _APPLICATIONLAUNCHER_H_

#include <QObject>
#include <QMap>
#include <QList>
#include <QProcess>
#include <qcopchannel_qws.h>
#include "qtopiaserverapplication.h"
class QValueSpaceObject;

// XXX - QString's used for application names should become QContent's

class ApplicationIpcRouter : public QObject
{
Q_OBJECT
public:
    class RouteDestination {
    public:
        virtual void routeMessage(const QString &, const QString &, const QByteArray &) = 0;
        virtual ~RouteDestination() {;}
    };
    virtual void addRoute(const QString &app, RouteDestination *) = 0;
    virtual void remRoute(const QString &app, RouteDestination *) = 0;
};
QTOPIA_TASK_INTERFACE(ApplicationIpcRouter);

class ApplicationTypeLauncher : public QObject
{
Q_OBJECT
public:
    enum TerminationReason { FailedToStart, Crashed, Unknown, Normal, Killed };
    enum ApplicationState { NotRunning, Starting, Running };

    virtual ApplicationState applicationState(const QString &) = 0;
    virtual bool canLaunch(const QString &) = 0;
    virtual void launch(const QString &) = 0;
    virtual void kill(const QString &) = 0;
signals:
    void applicationStateChanged(const QString &,
                                 ApplicationTypeLauncher::ApplicationState);
    void terminated(const QString &,
                    ApplicationTypeLauncher::TerminationReason);
};
QTOPIA_TASK_INTERFACE(ApplicationTypeLauncher);

class ApplicationTerminationHandler : public QObject
{
Q_OBJECT
public:
    ApplicationTerminationHandler(QObject *parent = 0)
        : QObject(parent) {}

    virtual bool terminated(const QString &,
                            ApplicationTypeLauncher::TerminationReason) = 0;
};
QTOPIA_TASK_INTERFACE(ApplicationTerminationHandler);

class ApplicationLauncher : public QObject
{
Q_OBJECT
public:
    ApplicationLauncher();

    bool canLaunch(const QString &);
    bool launch(const QString &); // Bring to a state where it can receive QCop
    bool kill(const QString &);   // For memory restoration

    // Order of starting
    QStringList applications() const;
    ApplicationTypeLauncher::ApplicationState state(const QString &) const;

signals:
    void applicationTerminated(const QString &,
                               ApplicationTypeLauncher::TerminationReason,
                               bool filtered = false);
    void applicationStateChanged(const QString &,
                                 ApplicationTypeLauncher::ApplicationState);
    void applicationNotFound(const QString &);


private slots:
    void stateChanged(const QString &,
                      ApplicationTypeLauncher::ApplicationState);
    void terminated(const QString &,
                    ApplicationTypeLauncher::TerminationReason);

private:
    QList<QString> m_orderedApps;
    QMap<QString, ApplicationTypeLauncher *> m_runningApps;
    QList<ApplicationTypeLauncher *> m_launchers;
    QCopChannel *m_chan;
    QValueSpaceObject *m_vso;
};
QTOPIA_TASK_INTERFACE(ApplicationLauncher);

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

class ExeApplicationLauncherPrivate;
class ExeApplicationLauncher : public ApplicationTypeLauncher,
                               public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    ExeApplicationLauncher();
    virtual ~ExeApplicationLauncher();

    // ApplicationTypeLauncher
    virtual void kill(const QString &app);
    virtual ApplicationState applicationState(const QString &app);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);

protected:
    void addStartingApplication(const QString &, QProcess *);
    bool isRunning(const QString &);

private slots:
    void appExited(int);
    void appError(QProcess::ProcessError);
    void qtopiaApplicationChannel(const QString &,const QByteArray &);
    void newChannel( const QString& ch );

private:
    ExeApplicationLauncherPrivate *d;
};

class SimpleExeApplicationLauncherPrivate;
class SimpleExeApplicationLauncher : public ExeApplicationLauncher
{
Q_OBJECT
public:
    SimpleExeApplicationLauncher();
    virtual ~SimpleExeApplicationLauncher();

    // ApplicationTypeLauncher
    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);

private:
    static QStringList applicationExecutable(const QString &app);

    SimpleExeApplicationLauncherPrivate *d;
};

#ifndef QT_NO_SXE
class SandboxedExeApplicationLauncherPrivate;
class SandboxedExeApplicationLauncher: public ExeApplicationLauncher
{
Q_OBJECT
public:
    SandboxedExeApplicationLauncher();
    virtual ~SandboxedExeApplicationLauncher();

    virtual bool canLaunch(const QString &app);
    virtual void launch(const QString &app);

private:
    static QStringList applicationExecutable(const QString &app);

    SandboxedExeApplicationLauncherPrivate *d;
};
#endif

class ConsoleApplicationLauncherPrivate;
class ConsoleApplicationLauncher : public ApplicationTypeLauncher,
                                   public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    ConsoleApplicationLauncher();
    virtual ~ConsoleApplicationLauncher();

    // ApplicationTypeLauncher
    virtual ApplicationState applicationState(const QString &);
    virtual bool canLaunch(const QString &);
    virtual void launch(const QString &);
    virtual void kill(const QString &);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);

private slots:
    void appStarted();
    void appExited(int);
    void appError(QProcess::ProcessError error);

private: 
    QStringList applicationExecutable(const QString &app);
    ConsoleApplicationLauncherPrivate *d;
};

class BuiltinApplicationLauncherPrivate;
class BuiltinApplicationLauncher : public ApplicationTypeLauncher,
                                   public ApplicationIpcRouter::RouteDestination
{
Q_OBJECT
public:
    BuiltinApplicationLauncher();
    virtual ~BuiltinApplicationLauncher();

    // ApplicationTypeLauncher
    virtual ApplicationState applicationState(const QString &);
    virtual bool canLaunch(const QString &);
    virtual void launch(const QString &);
    virtual void kill(const QString &);

    // QCopRouter::RouteDestination
    virtual void routeMessage(const QString &, const QString &,
                              const QByteArray &);

    typedef QWidget *(*BuiltinFunc)();
    static void install(const char *, BuiltinFunc);

private slots:
    void appDestroyed(QObject *);

private:
    friend class _BuiltinInstaller;
    BuiltinApplicationLauncherPrivate *d;
};

class _BuiltinInstaller
{
public:
    _BuiltinInstaller(const char *n, BuiltinApplicationLauncher::BuiltinFunc f)
    { BuiltinApplicationLauncher::install(n, f); }
};

#define QTOPIA_SIMPLE_BUILTIN(ApplicationName, createFunc) _BuiltinInstaller _builtin_install_ ## ApplicationName(# ApplicationName, createFunc)


#endif // _APPLICATIONLAUNCHER_H_
