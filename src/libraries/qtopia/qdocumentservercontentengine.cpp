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
#include <qtopia/private/qdocumentservercontentengine_p.h>
#include <qtopia/private/qdocumentservercontentstore_p.h>
#include <qtopia/private/drmcontent_p.h>

QDocumentServerContentEngine::QDocumentServerContentEngine( const QString &engineType )
    : QContentEngine( engineType )
{
}

QDrmRights QDocumentServerContentEngine::rights( QDrmRights::Permission permission ) const
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentRights( id(), permission );
}

QContentEngine *QDocumentServerContentEngine::copyTo( const QString &newPath )
{
    Q_UNUSED( newPath );

    return 0;
}

bool QDocumentServerContentEngine::moveTo( const QString &newPath )
{
    Q_UNUSED( newPath );

    return false;
}

bool QDocumentServerContentEngine::execute( const QStringList &arguments ) const
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->executeContent( id(), arguments );
}

bool QDocumentServerContentEngine::canActivate() const
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::canActivate( fileName() );
    }
    else
    {
        return false;
    }
}

bool QDocumentServerContentEngine::activate( QDrmRights::Permission permission, QWidget *parent )
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::activate( fileName(), permission, parent );
    }
    else
    {
        return true;
    }
}

bool QDocumentServerContentEngine::reactivate( QDrmRights::Permission permission, QWidget *parent )
{
    if( drmState() == QContent::Protected )
    {
        DrmContentPrivate::reactivate( fileName(), permission, parent );
    }

    return true;
}

QDrmContentLicense *QDocumentServerContentEngine::requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    if( drmState() == QContent::Protected )
    {
        return DrmContentPrivate::requestContentLicense( QContent( this ), permission, options );
    }
    else
        return 0;
}

bool QDocumentServerContentEngine::remove()
{
    return false;
}

QIODevice *QDocumentServerContentEngine::open( QIODevice::OpenMode mode )
{
    QFile *file = new QFile( fileName() );

    if( file->open( mode ) )
    {
        return file;
    }
    else
    {
        delete file;

        return 0;
    }
}

QContentEngine *QDocumentServerContentEngine::createCopy() const
{
    QDocumentServerContentEngine *copy = new QDocumentServerContentEngine( engineType() );

    copy->setId( id() );
    copy->copy( *this );

    return copy;
}

bool QDocumentServerContentEngine::isOutOfDate() const
{
    return false;
}

QDrmRights::Permissions QDocumentServerContentEngine::queryPermissions()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentPermissions( id() );
}

qint64 QDocumentServerContentEngine::querySize()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentSize( id() );
}

bool QDocumentServerContentEngine::queryValidity()
{
    return static_cast< QDocumentServerContentStore * >( QContentStore::instance() )->contentValid( id() );
}

