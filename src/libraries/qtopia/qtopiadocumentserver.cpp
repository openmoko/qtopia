/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include <QtopiaDocumentServer>
#include <qtopia/private/qcategorystoreserver_p.h>
#include <qtopia/private/qcontentstoreserver_p.h>
#include <qtopia/private/qdocumentselectorsocketserver_p.h>

class QtopiaDocumentServerPrivate
{
public:
    QContentStoreServerTask contentServer;
    QCategoryStoreServerTask categoryServer;
    QDocumentSelectorSocketServer documentSelectorServer;
};

/*!
    \class QtopiaDocumentServer
    \mainclass
    \brief The QtopiaDocumentServer class provides an instance of the Qtopia Document Server.

    The Qtopia Document Server provides applications with a mechanism to interact with the
    \l{Document System}{Qtopia Document System} without having direct access to the document databases or
    even the file system.  A single instance of the document server runs in a server process, and client
    applications connect to this server in order to gain access to the functionality provided by the document
    system API.

    The document server runs multiple threads and should be shut down before being destroyed in order to allow
    the threads to exit cleanly.  A shut down can be initiated with the shutdown() slot, and the shutdownComplete()
    signal will be emitted when this is complete.
*/

/*!
    Constructs a new Qtopia document server with the parent \a parent.

    Only one instance of the document server should ever exist at once across all Qtopia applications.
*/
QtopiaDocumentServer::QtopiaDocumentServer( QObject *parent )
    : QObject( parent )
{
    d = new QtopiaDocumentServerPrivate;

    connect( &d->contentServer , SIGNAL(finished()), this, SLOT(threadFinished()) );
    connect( &d->categoryServer, SIGNAL(finished()), this, SLOT(threadFinished()) );
}

/*!
    Destroys the document server object.
*/
QtopiaDocumentServer::~QtopiaDocumentServer()
{
    delete d;
}

/*!
    \fn QtopiaDocumentServer::shutdownComplete()

    Emitted by the server when the server has successfully been shutdown in response to a call to shutdown().

    \sa shutdown()
*/

/*!
    Initiates a shutdown of the document server.  The shutdownComplete() signal will be emitted when all the
    document server threads have exited.

    \sa shutdownComplete()
*/
void QtopiaDocumentServer::shutdown()
{
    d->contentServer.quit();
    d->categoryServer.quit();
}

/*!
    Called when one of the document server threads has exited.
*/
void QtopiaDocumentServer::threadFinished()
{
    if( d->contentServer.isFinished() && d->categoryServer.isFinished() )
        emit shutdownComplete();
}
