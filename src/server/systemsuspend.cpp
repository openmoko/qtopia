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

#include "systemsuspend.h"
#include <QList>

/*!
  \class SystemSuspend
  \brief The SystemSuspend class manages entering and leaving system suspend.
  \ingroup QtopiaServer::Task

  The system suspend state is expected to be a very low, but non-destructive,
  power saving state.

  Some hardware devices may need to be shutdown before or reinitialized after
  entry into the suspend state.  Integrators can provide objects that implement
  the SystemSuspendHandler interface that will be called before the system
  enters suspend and after the system leaves it.

  While the SystemSuspend class is an abstract interfaces - that conceivably
  allows replacement of the suspend sub-system - a default implementation is
  provided under the task name \c SystemSuspend which should generally be
  adequate.

  \sa SystemSuspendHandler, SuspendService
 */

/*!
  \fn SystemSuspend::SystemSuspend()
  \internal
  */

/*!
  \fn bool SystemSuspend::suspendSystem()

  Request that the system be suspended.  The system will block in this method
  until execution is resumed.  The method returns true if the suspension was
  successful, and false otherwise.

  When called, all tasks in the system that implement the SystemSuspendHandler
  are instantiated.  If any of these tasks returns false from the
  SystemSuspendHandler::canSuspend() method, the suspend is canceled and the
  systemSuspendCanceled() signal is emitted.  Otherwise, the
  SystemSuspendHandler::suspend() methods are called on the tasks in reverse
  order.  That is, the task with the highest interface priority is called last.
  It is assumed that this last task will actually perform the hardware suspend -
  possibly using the standard "apm --suspend" system call.

  Once all the tasks have successfully completed the suspend, the
  SystemSuspendHandler::wake() method is invoked on each in-order.  This is done
  immediately after the last invokation of the SystemSuspendHandler::suspend()
  method, so it is important that this last handler actually suspends the device
  or, from the users perspective, the device will suspend and immediately
  resume.
 */

/*!
  \fn void SystemSuspend::systemSuspending()

  Emitted whenever the system begins suspending.  This is emitted after the
  SystemSuspendHandlers have all reported that the system is in a state to
  suspend.

  Following the systemSuspending() signal, a systemWaking() and systemActive()
  signals are guarenteed.
 */

/*!
  \fn void SystemSuspend::systemWaking()

  Emitted whenever the system is in the process of resuming from suspend.
 */

/*!
  \fn void SystemSuspend::systemActive()

  Emitted whenever the system has completed resuming from suspend.
 */

/*!
  \fn void SystemSuspend::systemSuspendCanceled()

  Emitted whenever a system suspend has been requested but a
  SystemSuspendHandler reported that it was not in a state to suspend.
  The suspendSystem() call that requested the suspend will return false.
 */

/*!
  \class SystemSuspendHandler
  \brief The SystemSuspendHandler class provides an interface for tasks that
         provide system suspension or resumption functionality.
  \ingroup QtopiaServer::Task::Interfaces

  Tasks that provide the SystemSuspendHandler interface will be called whenever
  a system suspend is requested through the SystemSuspend class.  More
  information on how a system suspend proceeds is available in the documentation
  for that class.
 */

/*!
  \fn SystemSuspendHandler::SystemSuspendHandler(QObject *parent = 0)

  Construct the handler with the given \a parent.
 */

/*!
  \fn bool SystemSuspendHandler::canSuspend() const

  Returns true if the handler is capable of suspending at the time it is called.
  Suspend handlers can return false from this call to prevent the system from
  entering a suspended state.  This might be useful to prevent a suspend during
  a device synchronization or other scenarios where it may be advantageous
  not to enter such a state.
 */

/*!
  \fn bool SystemSuspendHandler::suspend()

  Perform the handler's suspend action.  If this method returns false, the
  suspend process will pause - entring the Qt event loop - until the handler
  emits the operationCompleted() signal.  This can be used to perform
  asynchronous actions in the handler.

  Normally this method will return true.
 */

/*!
  \fn bool SystemSuspendHandler::wake()

  Perform the handler's wake action.  If this method returns false, the
  wake process will pause - entering the Qt event loop - until the handler
  emits the operationCompleted() signal.  This can be used to perform
  asynchronous actions in the handler.

  Normally this method will return true.
 */

/*!
  \fn void SystemSuspendHandler::operationCompleted()

  Emitted to indicate that the suspend or wake process can continue.  This
  should only be emitted after previously returning false from suspend() or
  wake().
 */

// declare SystemSuspendPrivate
class SystemSuspendPrivate : public SystemSuspend
{
Q_OBJECT
public:
    SystemSuspendPrivate(QObject *parent = 0);

    virtual bool suspendSystem();

private:
    bool handlersValid;
    QList<SystemSuspendHandler *> handlers;

    SystemSuspendHandler *waitingOn;

private slots:
    void operationCompleted();
};
QTOPIA_TASK(SystemSuspend, SystemSuspendPrivate);
QTOPIA_TASK_PROVIDES(SystemSuspend, SystemSuspend);

// define SystemSuspendPrivate
SystemSuspendPrivate::SystemSuspendPrivate(QObject *parent)
: SystemSuspend(parent), handlersValid(false), waitingOn(0)
{
    SuspendService *s = new SuspendService(this);
    QObject::connect(s, SIGNAL(doSuspend()), this, SLOT(suspendSystem()));
}

void SystemSuspendPrivate::operationCompleted()
{
    if(sender() == waitingOn)
        waitingOn = 0;
}

bool SystemSuspendPrivate::suspendSystem()
{
    if(!handlersValid) {
        handlers = qtopiaTasks<SystemSuspendHandler>();
        handlersValid = true;
        for(int ii = 0; ii < handlers.count(); ++ii) {
            QObject::connect(handlers.at(ii), SIGNAL(operationCompleted()),
                             this, SLOT(operationCompleted()));
        }
    }

    // Check can suspend
    for(int ii = handlers.count(); ii > 0; --ii) {
        if(!handlers.at(ii - 1)->canSuspend()) {
            emit systemSuspendCanceled();
            return false;
        }
    }

    // Do suspend
    emit systemSuspending();
    for(int ii = handlers.count(); ii > 0; --ii) {
        waitingOn = handlers.at(ii - 1);
        if(!waitingOn->suspend()) {
            while(waitingOn)
                QApplication::instance()->processEvents();
        } else {
            waitingOn = 0;
        }
    }

    // Do wakeup
    emit systemWaking();

    for(int ii = 0; ii < handlers.count(); ++ii) {
        waitingOn = handlers.at(ii);
        if(!waitingOn->wake()) {
            while(waitingOn)
                QApplication::instance()->processEvents();
        } else {
            waitingOn = 0;
        }
    }

    // Done
    emit systemActive();

    return true;
}

// define SuspendService
/*!
  \service SuspendService Suspend
  \brief Provides the Suspend service.

  The \i Suspend service allows applications request that the device enter its
  suspend state.  The suspend state is likely to be a very low, but
  non-destructive, power state.
 */

/*!
  \internal
 */
SuspendService::SuspendService(QObject *parent)
: QtopiaAbstractService("Suspend", parent)
{
    publishAll();
}

/*!
  Enter the suspend state.

  This slot corresponds to the QCop service message \c {Suspend::suspend()}.
 */
void SuspendService::suspend()
{
    emit doSuspend();
}

/*!
  \fn void SuspendService::doSuspend()
  \internal
 */

#include "systemsuspend.moc"
