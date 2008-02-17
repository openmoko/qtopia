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

#include <qtopia/private/qdrmcontentengine_p.h>
#include <qtopia/private/drmcontent_p.h>
#include <qtopiabase/qtopialog.h>
#include <qtopiabase/qtopianamespace.h>

/*!
    \class QDrmContentEngine
    \mainclass
    \brief QDrmContentEngine is the default content engine for DRM protected content.

    \internal
*/

/*!
    Constructs a new unpopulated QDrmContentEngine.
*/
QDrmContentEngine::QDrmContentEngine()
    : QFSContentEngine( QLatin1String( "*/*" ) )
{
}

/*!
    Constructs a new QDrmContentPrivate where \a engineType is the engine mime type of an inheriting engine.
*/
QDrmContentEngine::QDrmContentEngine( const QString &engineType )
    : QFSContentEngine( engineType )
{
}


/*!
    Destroys a QDrmContentEngine.
*/
QDrmContentEngine::~QDrmContentEngine()
{
}

/*!
    \reimp
*/
QContentEngine *QDrmContentEngine::createCopy() const
{
    QDrmContentEngine *engine = new QDrmContentEngine;

    engine->copy( *this );
    engine->setId( id() );

    return engine;
}

/*!
    \reimp
*/
QIODevice *QDrmContentEngine::open( QIODevice::OpenMode mode )
{
    QDrmContentPlugin *plugin = DrmContentPrivate::plugin( fileName() );

    QDrmContentLicense *license = plugin ? plugin->license( fileName() ) : 0;

    QIODevice *io = license ? plugin->createDecoder( fileName(), license->permission() ) : 0;

    if( !(io && io->open( mode )) )
    {
        delete io;

        io = 0;
    }

    return io;
}

/*!
    \reimp
*/
bool QDrmContentEngine::execute( const QStringList &arguments ) const
{
    if( role() == QContent::Application )
    {
        qLog(DocAPI) << "QDrmContentEngine::execute" << fileName() << arguments;

        Qtopia::execute( fileName(), arguments.count() ? arguments[0] : QString() );

        return true;
    }
    else
    {
        QContent app = mimeType().application();

        if( app.isValid() && const_cast< QDrmContentEngine * >( this )->activate( mimeType().permission(), 0 ) )
        {
            app.execute( QStringList() << arguments << fileName() );

            return true;
        }
    }

    return false;
}

/*!
    \reimp
*/
QDrmRights::Permissions QDrmContentEngine::queryPermissions()
{
    return DrmContentPrivate::permissions( fileName() );
}

/*!
    \reimp
*/
QDrmRights QDrmContentEngine::rights( QDrmRights::Permission permission ) const
{
    return DrmContentPrivate::getRights( fileName(), permission );
}

/*!
    \reimp
*/
bool QDrmContentEngine::canActivate() const
{
    return DrmContentPrivate::canActivate( fileName() );
}

/*!
    \reimp
*/
bool QDrmContentEngine::activate( QDrmRights::Permission permission, QWidget *parent )
{
    return DrmContentPrivate::activate( fileName(), permission, parent );
}

/*!
    \reimp
*/
bool QDrmContentEngine::reactivate( QDrmRights::Permission permission, QWidget *parent )
{
    DrmContentPrivate::reactivate( fileName(), permission, parent );

    return true;
}

/*!
    \reimp
*/
QDrmContentLicense *QDrmContentEngine::requestLicense( QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    return DrmContentPrivate::requestContentLicense( QContent( this ), permission, options );
}
