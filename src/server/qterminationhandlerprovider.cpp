/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qterminationhandlerprovider.h"

#include <QtopiaChannel>
#include <QtopiaServiceRequest>
#include <qtopia/private/qterminationhandler_p.h>
#include "applicationmonitor.h"

#include <QString>

class QtopiaChannel;
class TerminationHandlerPrivate;
class TerminationHandler : public ApplicationTerminationHandler
{
    Q_OBJECT
public:
    TerminationHandler();
    virtual ~TerminationHandler();

    virtual bool terminated(const QString &,
                            ApplicationTypeLauncher::TerminationReason);

    void addHandler() { ++handlers; }
    void remHandler() { Q_ASSERT(handlers); --handlers; }

signals:
    void applicationTerminated(const QString &name, const QString &text,
                               const QPixmap &icon, const QString &buttonText,
                               QtopiaServiceRequest &buttonAction);

protected slots:
    void messageReceived(const QString&, const QByteArray&);

private:
    int handlers;
    TerminationHandlerPrivate* d;
};
static TerminationHandler *termInstance = 0;

struct TerminationHandlerPrivate
{
    QMap<QString,QTerminationHandlerData> installedHandlers;
};

TerminationHandler::TerminationHandler()
: handlers(0)
{
    Q_ASSERT(!termInstance);
    termInstance = this;
    d = new TerminationHandlerPrivate;
    QtopiaChannel* channel = new QtopiaChannel("Qtopia/TerminationHandler", this);
    QObject::connect(channel, SIGNAL(received(QString,QByteArray)),
            this, SLOT(messageReceived(QString,QByteArray)));
}

TerminationHandler::~TerminationHandler()
{
    delete d;
}

void TerminationHandler::messageReceived(const QString& msg, const QByteArray& data)
{
    QDataStream istream(data);
    if(msg == "installHandler(QTerminationHandlerData)") {
        // name text buttonText buttonIcon action
        QTerminationHandlerData data;
        istream >> data;
        if(d->installedHandlers.contains(data.name))
            d->installedHandlers.remove(data.name);
        d->installedHandlers[data.name] = data;
    } else if(msg == "removeHandler(QString)") {
        // name
        QString name;
        istream >> name;
        if(d->installedHandlers.contains(name))
            d->installedHandlers.remove(name);
    }
}

bool TerminationHandler::terminated(const QString& app,
                        ApplicationTypeLauncher::TerminationReason reason)
{
    if(d->installedHandlers.contains(app)) {
        QTerminationHandlerData data = d->installedHandlers[app];
        d->installedHandlers.remove(app);
        if(reason == ApplicationTypeLauncher::Normal)
            return false;

        if(handlers) {
            emit applicationTerminated(app, data.text, QPixmap(data.buttonIcon),
                                       data.buttonText, data.action);
            return true;
        } else {
            return false;
        }

    } else {
        return false;
    }
}

QTOPIA_TASK(TerminationHandler, TerminationHandler);
QTOPIA_TASK_PROVIDES(TerminationHandler, ApplicationTerminationHandler);

/*!
  \class QTerminationHandlerProvider
  \ingroup QtopiaServer
  \brief The QTerminationHandlerProvider class provides the backend for the QTerminationHandler class.

  For termination handling to be tracked successfully, the
  \c {TerminationHandler} task must be running prior to starting any
  applications.

  While it is legal to have more than one concurrent instance of
  QTerminationHandlerProvider, it is generally discorouged.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  Creates a new QTerminationHandlerProvider instance with the specified
  \a parent.
 */
QTerminationHandlerProvider::QTerminationHandlerProvider(QObject *parent)
: QObject(parent)
{
    if(!termInstance) {
        QtopiaServerApplication::qtopiaTask("TerminationHandler");
    }

    if(!termInstance) {
        qWarning("QTerminationHandlerProvider: TerminationHandler task not running.  QTerminationHandlerProvider will not function.");
    } else {
        termInstance->addHandler();
        QObject::connect(termInstance, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)), this, SIGNAL(applicationTerminated(QString,QString,QPixmap,QString,QtopiaServiceRequest&)));
    }
}

/*!
  Destroys the QTerminationHandlerProvider instance.
 */
QTerminationHandlerProvider::~QTerminationHandlerProvider()
{
    if(termInstance)
        termInstance->remHandler();
}

/*!
  \fn void QTerminationHandlerProvider::applicationTerminated(const QString &name, const QString &text, const QPixmap &icon, const QString &buttonText, QtopiaServiceRequest &buttonAction)

  Emitted whenever an application that has installed a termination handler
  through the QTerminationHandler class abnormally terminates.

  \a name is set to the name or the terminating application, and \a text,
  \a icon, \a buttonText and \a buttonAction are set to those passed to the
  QTerminationHandler class by the terminating application.
 */

#include "qterminationhandlerprovider.moc"
