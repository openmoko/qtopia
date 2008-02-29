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
#include <qtopia/private/qcategorystoreserver_p.h>
#include <qtopia/private/qcategorystore_p.h>
#include <qtopiabase/qtopianamespace.h>
#include <QtDebug>

QCategoryStoreServer::QCategoryStoreServer( QObject *parent )
    : QDocumentServerHost( "QCategoryStoreServer", parent )
{
    QObject::connect( QCategoryStore::instance(), SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()) );
    QObject::connect( this, SIGNAL(disconnected()), this, SLOT(deleteLater()) );
}

QDocumentServerMessage QCategoryStoreServer::invokeMethod( const QDocumentServerMessage &message )
{
    const QByteArray signature = message.signature();
    const QVariantList arguments = message.arguments();

    if( signature == "addCategory(QString,QString,QString,QString,bool)" )
    {
        QVariantList arguments = message.arguments();

        Q_ASSERT( arguments.count() == 5 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->addCategory(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ),
                qvariant_cast< QString >( arguments[ 2 ] ),
                qvariant_cast< QString >( arguments[ 3 ] ),
                qvariant_cast< bool    >( arguments[ 4 ] ) ) ) );
    }
    else if( signature == "categoryExists(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->categoryExists(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "categoryFromId(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->categoryFromId(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "scopeCategories(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->scopeCategories(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "removeCategory(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->removeCategory(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else if( signature == "setCategoryScope(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryScope(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setCategoryIcon(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryIcon(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if ( signature == "setCategoryRingTone(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );
        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryRingTone(
                qvariant_cast< QString >( arguments[ 0 ] ),
                qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setCategoryLabel(QString,QString)" )
    {
        Q_ASSERT( arguments.count() == 2 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setCategoryLabel(
                qvariant_cast< QString >( arguments[ 0 ] ),
        qvariant_cast< QString >( arguments[ 1 ] ) ) ) );
    }
    else if( signature == "setSystemCategory(QString)" )
    {
        Q_ASSERT( arguments.count() == 1 );

        return message.createReply( QVariant::fromValue( QCategoryStore::instance()->setSystemCategory(
                qvariant_cast< QString >( arguments[ 0 ] ) ) ) );
    }
    else
    {
        qWarning() << "Tried to invoke unknown message";

        Q_ASSERT( false );

        return message.createError( "Unknown method" );
    }
}

void QCategoryStoreServer::categoriesChanged()
{
    emitSignalWithArgumentList( "categoriesChanged()", QVariantList() );
}

QCategoryStoreSocketServer::QCategoryStoreSocketServer( QObject *parent )
    : QUnixSocketServer( parent )
{
    QByteArray socketPath = (Qtopia::tempDir() + QLatin1String( "QCategoryStoreServer" )).toLocal8Bit();

    listen( socketPath );
}

void QCategoryStoreSocketServer::incomingConnection( int socketDescriptor )
{
    QCategoryStoreServer *server = new QCategoryStoreServer( this );

    server->setSocketDescriptor( socketDescriptor );
}

QCategoryStoreServerTask::QCategoryStoreServerTask( QObject *parent )
    : QThread( parent )
{
    start();
}

void QCategoryStoreServerTask::run()
{
    QCategoryStoreSocketServer socketServer;

    exec();
}
