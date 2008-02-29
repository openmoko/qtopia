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

#include "qterminationhandler.h"
#include "qterminationhandler_p.h"
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QtopiaApplication>

/*!
  \class QTerminationHandler
  \mainclass
  \brief The QTerminationHandler class allows an application to instruct the system on how to handle an abnormal termination.

  While the abnormal termination - ie. crash - of an application is
  undesirable, sometimes it is unavoidable.  By default, Qtopia displays a
  generic notification when this occurs that gives no information to the end
  user as to what caused the crash, and how they could potentially avoid it
  reoccuring.

  The QTerminationHandler class allows an application to register expanded
  information for display to the user if it crashes.  To use the
  QTerminationHandler class, an application only has to construct it prior to
  commencing the potentially dangerous operation and destroy it when the
  termination information is no longer valid.

  An application may register as many termination handlers as required,
  however only the most recently installed termination handler will be used in
  the case an abnormal termination.

  \ingroup environment
 */


struct QTerminationHandlerPrivate {
    QList<QTerminationHandlerData> stack;
};

QTerminationHandlerPrivate* QTerminationHandler::staticData()
{
    static QTerminationHandlerPrivate* d = 0;
    if( 0 == d )
        d = new QTerminationHandlerPrivate;
    return d;
}

/*!
  Constructs a QTerminationHandler.  If a crash occurs, \a text will be
  presented to the user in a theme dictated manner.  Additionally, if \a action
  is valid, and if either \a buttonText or \a buttonIcon are valid the user will
  have the option of activating a button that causes \a action to occur.

  The QTerminationHandler is a QObject with the given \a parent.

  For example,
  \code
  QTerminationHandler * myTermHandler = new QTerminationHandler(tr("The message you tried to read caused Acme EMail to crash.  Please try viewing the mail in text only mode.", "Restart", ":icon/acmeemail", QtopiaServiceRequest("AcmeEmail", "viewMail()"));

  // Open mail

  delete myTermHandler;
  \endcode
*/
QTerminationHandler::QTerminationHandler(const QString &text,
                                         const QString &buttonText,
                                         const QString &buttonIcon,
                                         const QtopiaServiceRequest &action,
                                         QObject *parent) : QObject(parent)
{
    QTerminationHandlerData data(QtopiaApplication::applicationName(), text, buttonText, buttonIcon, action);
    installHandler(data);
}

/*!
  Constructs a QTerminationHandler.  If a crash occurs, \a action will occur.

  For example,
  \code
  QTerminationHandler * myTermHandler = new QTerminationHandler(QtopiaServiceRequest("AcmeEmail", "showCrashScreen()"));

  // Open mail

  delete myTermHandler;
  \endcode
 */
QTerminationHandler::QTerminationHandler(const QtopiaServiceRequest &action,
                                         QObject *parent) : QObject(parent)
{
    QTerminationHandlerData data(QtopiaApplication::applicationName(), QString(), QString(), QString(), action);
    installHandler(data);
}

/*!
  Destroys the QTerminationHandler instance.  This automatically unregisters
  the handler from the system.
 */
QTerminationHandler::~QTerminationHandler()
{
    QTerminationHandlerPrivate*d = staticData();
    Q_ASSERT(d->stack.count() > 0);
    /* Remove myself */
    d->stack.takeLast();
    if( d->stack.count() > 0 ) {
        /* Install the next handler */
        installHandler(d->stack.last());
    } else {
        /* All handlers removed  */
        QDataStream ostream(new QBuffer);
        ostream.device()->open(QIODevice::WriteOnly);
        ostream << QtopiaApplication::applicationName();
        QByteArray data = ((QBuffer*)ostream.device())->buffer();
        QtopiaChannel::send("Qtopia/TerminationHandler", "removeHandler(QString)", data);
        QtopiaChannel::flush();
    }
}

void QTerminationHandler::installHandler( const QTerminationHandlerData& handler )
{
    /* Add another handler to the stack */
    staticData()->stack.append(handler);

    /* Send this handler */
    QDataStream ostream(new QBuffer);
    ostream.device()->open(QIODevice::WriteOnly);
    ostream << handler;
    QByteArray data = ((QBuffer*)ostream.device())->buffer();
    QtopiaChannel::send("Qtopia/TerminationHandler", "installHandler(QTerminationHandlerData)", data);
    QtopiaChannel::flush();
}
