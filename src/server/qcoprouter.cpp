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
#if !defined(QTOPIA_DBUS_IPC) && !defined(QT_NO_COP)
#include "qcoprouter.h"
#include <qtopianamespace.h>
#include <qtopiaservices.h>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include "applicationlauncher.h"

#include <errno.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#include <sys/file.h>
#else
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "qcopfile.h"

/*!
  \class QCopRouter
  \ingroup QtopiaServer
  \brief The QCopRouter class takes care of routing low-level QCop messages
         that were sent to QPE/Application/foo or QPE/Service/foo to the actual
         applications that handle them.

  The QCopRouter class provides the \c {IpcRouter} task.
*/

QTOPIA_TASK(IpcRouter, QCopRouter);
QTOPIA_TASK_PROVIDES(IpcRouter, ApplicationIpcRouter);

/*!  \internal */
QCopRouter::QCopRouter()
{
    // cleanup old messages
    QDir dir( Qtopia::tempDir(), "qcop-msg-*" );

    if (!dir.exists())
        return;
    QStringList stale = dir.entryList();
    QStringList::Iterator it;
    for ( it = stale.begin(); it != stale.end(); ++it ) {
        dir.remove( *it );
    }

    QCopChannel *channel;

    channel = new QCopChannel( "QPE/Application/*", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(applicationMessage(const QString&,const QByteArray&)) );

    channel = new QCopChannel( "QPE/Service/*", this );
    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(serviceMessage(const QString&,const QByteArray&)) );
}

/*!  \internal */
QCopRouter::~QCopRouter()
{
    // Nothing to do here at present.
}

// Process all messages to QPE/Application/*.
/*!  \internal */
void QCopRouter::applicationMessage
            ( const QString& msg, const QByteArray& data )
{
    if ( msg == QLatin1String("forwardedMessage(QString,QString,QByteArray)") ) {

        QDataStream stream( data );
        QString channel, message;
        QByteArray newData;
        stream >> channel;
        stream >> message;
        stream >> newData;

        QString app = channel.mid(16 /* ::strlen("QPE/Application/") */);
        routeMessage(app, message, newData);
    }
}

// Process all messages to QPE/Service/*.
/*!  \internal */
void QCopRouter::serviceMessage( const QString& msg, const QByteArray& data )
{
    if ( msg == QLatin1String("forwardedMessage(QString,QString,QByteArray)") ) {

        QDataStream stream( data );
        QString channel, message;
        QByteArray newData;
        stream >> channel;
        stream >> message;
        stream >> newData;

        // Bail out if it doesn't look like a valid service request.
        if ( !channel.startsWith( QLatin1String( "QPE/Service/" ) ) )
            return;

        // Look up the application channel that handles the service.
        QString appChannel = QtopiaService::channel( channel.mid(12) );
        if ( appChannel.isEmpty() ) {
            qWarning( "No service mapping for %s, cannot forward %s",
                      channel.toLatin1().constData(),
                      message.toLatin1().constData() );
            return;
        }

        QString app = appChannel.mid(16 /* ::strlen("QPE/Application/") */);
        routeMessage(app, message, newData);
    }
}

/*!  \internal */
void QCopRouter::routeMessage(const QString &dest,
                              const QString &message,
                              const QByteArray &data)
{
    if(dest.isEmpty())
        return;

    // Launch route
    ApplicationLauncher *l = qtopiaTask<ApplicationLauncher>();
    Q_ASSERT(m_cDest.isEmpty());

    m_cDest = dest;
    m_cMessage = message;
    m_cData = data;

    if(l)
        l->launch(dest);

    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(dest);
    while(iter != m_routes.end() && iter.key() == dest) {
        if(!m_cRouted.contains(*iter))
            (*iter)->routeMessage(dest, message, data);
        ++iter;
    }

    m_cDest.clear();
    m_cMessage.clear();
    m_cData.clear();
    m_cRouted.clear();
}

/*!  \internal */
void QCopRouter::addRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    Q_ASSERT(!app.isEmpty());
    m_routes.insert(app, dest);
    if(app == m_cDest) {
        dest->routeMessage(m_cDest, m_cMessage, m_cData);
        m_cRouted.insert(dest);
    }
}

/*!  \internal */
void QCopRouter::remRoute(const QString &app, RouteDestination *dest)
{
    Q_ASSERT(dest);
    QMultiMap<QString, RouteDestination *>::Iterator iter = m_routes.find(app);
    while(iter != m_routes.end() && iter.key() == app) {
        if(iter.value() == dest) {
            m_routes.erase(iter);
            return;
        }
        ++iter;
    }
}
#endif

