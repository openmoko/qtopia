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

#include "drmcontent_p.h"
#include <qtopialog.h>
#include <qthumbnail.h>
#include <qcontentfilter.h>
#include <QPainter>
#include <QThread>
#include <QPixmapCache>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif

/*!
    \internal
    \class DRMThumbnailLoader
    A specialisation of the ThumbnailLoader class that can load thumbnails of
    DRM protected images.
 */

/*!
    Creates a new DRMThumbnailLoader object.
 */
DRMThumbnailLoader::DRMThumbnailLoader( ThumbnailCache* cache, QObject* parent )
    : ThumbnailLoader( cache, parent )
{
}

/*!
    Loads a thumbnail of an image, if the image is not DRM protected use the default implemetation
    in ThumbnailLoader.  If the image is DRM protected use the loadDRMThumbnail function.
*/
QPixmap DRMThumbnailLoader::loadThumbnail( const QString &filename, const QSize &size )
{
    foreach( QDrmContentPlugin *plugin, DrmContentPluginManager::instance()->plugins() )
    {
        foreach( QString key, plugin->keys() )
            if( filename.endsWith( '.' + key ) || filename.contains( '.' + key + QDir::separator() ) )
                return DrmContentPrivate::thumbnail( filename, size );
    }
    return ThumbnailLoader::loadThumbnail( filename, size );
}

DrmContentPluginManager::DrmContentPluginManager()
     : m_manager( QLatin1String( "drmagent" ) )
{
    QStringList pluginNames = m_manager.list();

    foreach( QString pluginName, pluginNames )
    {
        qLog(DRMAgent) << "Load Plugin" << pluginName;

        QObject *object = m_manager.instance( pluginName );

        QDrmAgentPlugin *agent = qobject_cast< QDrmAgentPlugin * >( object );

        if( agent )
        {
            QDrmContentPlugin *plugin = agent->createDrmContentPlugin();

            QStringList keys = plugin->keys();

            foreach( QString key, keys )
            {
                qLog(DRMAgent) << "Key" << key;

                if( !m_pluginMap.contains( key ) )
                    m_pluginMap[ key ] = plugin;
                else
                    qWarning() << "Multiple DRM plugins with a common key";
            }

            m_plugins.append( plugin );
        }
    }
}

DrmContentPluginManager::~DrmContentPluginManager()
{
    qDeleteAll( m_plugins );
}

QDrmContentPlugin *DrmContentPluginManager::plugin( const QString &filePath ) const
{
    foreach( QDrmContentPlugin *plugin, m_plugins )
        foreach( QString key, plugin->keys() )
        if( filePath.endsWith( '.' + key ) || filePath.contains( '.' + key + QDir::separator() ) )
            return plugin;

    return 0;
}

QList< QDrmContentPlugin * > DrmContentPluginManager::plugins() const
{
    return m_plugins;
}

DrmContentPluginManager *DrmContentPluginManager::instance()
{
    static DrmContentPluginManager *instance = 0;

    if( !instance )
        instance = new DrmContentPluginManager;

    return instance;
}

/*!
    \class DrmContentPrivate
    \internal
*/

QIcon DrmContentPrivate::createIcon( const QIcon &baseIcon, int smallSize, int bigSize, bool validRights )
{
    QIcon icon;

    icon.addPixmap( compositeDrmIcon( baseIcon.pixmap(smallSize, smallSize), smallSize, validRights ) );
    icon.addPixmap( compositeDrmIcon( baseIcon.pixmap(bigSize, bigSize), bigSize, validRights ) );

    return icon;
}

QPixmap DrmContentPrivate::drmIcon( int size )
{
    QPixmap overlay;
#ifndef QTOPIA_NO_CACHE_PIXMAPS
    QString id = QLatin1String("_QPE_Global_Drm_lock_") + QString::number( size );
    if( !QPixmapCache::find( id, overlay ) )
    {
#endif
        QString path = size <= 16 ? QLatin1String(":image/drm/Drm_lock_16") : QLatin1String(":image/drm/Drm_lock");

        overlay = QPixmap( path ).scaled( size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

#ifndef QTOPIA_NO_CACHE_PIXMAPS
        QPixmapCache::insert( id, overlay );
    }
#endif
    return overlay;
}

QPixmap DrmContentPrivate::compositeDrmIcon( const QPixmap &base, int size, bool validRights )
{
    QPixmap pm = base;
    QPixmap overlay = drmIcon( size );

    if( !pm.isNull() && ! overlay.isNull() )
    {
        QPainter painter( &pm );

        if( !validRights )
        {
            painter.setPen( Qt::NoPen );
            painter.setBrush( QBrush( QColor( 127, 127, 127, 127 ) ) );

            painter.drawRect( pm.rect() );
        }

        QSize diff = base.size() - overlay.size();

        painter.drawPixmap( diff.width(), diff.height(), overlay );
    }
    return pm;
}



bool DrmContentPrivate::isProtected( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->isProtected( filePath ) : false;
}

QDrmRights::Permissions DrmContentPrivate::permissions( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->permissions( filePath ) : QDrmRights::Permissions();
}

QDrmRights DrmContentPrivate::getRights( const QString &filePath, QDrmRights::Permission permission )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->getRights( filePath, permission ) : QDrmRights();
}

QDrmContentLicense *DrmContentPrivate::requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options )
{
    QDrmContentPlugin *p = plugin( content.file() );

    return p ? p->requestContentLicense( content, permission, options ) : 0;
}

bool DrmContentPrivate::activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    if( content.permissions() == QDrmRights::Unrestricted )
        return true;
    else if( content.permissions() & permission )
        return true;
    else
    {
        QDrmContentPlugin *p = plugin( content.file() );

        return p ? p->activate( content, permission, focus ) : false;
    }
}

void DrmContentPrivate::reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus )
{
    QDrmContentPlugin *p = plugin( content.file() );

    if( p )
        p->reactivate( content, permission, focus );
}

QIODevice *DrmContentPrivate::createDecoder( const QString &filePath, QDrmRights::Permission permission )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->createDecoder( filePath, permission ) : 0;
}

bool DrmContentPrivate::canActivate( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->canActivate( filePath ) : false;
}

void DrmContentPrivate::activate( const QContent &content, QWidget *focus )
{
    QDrmContentPlugin *p = plugin( content.file() );

    if( p )
        p->activate( content, focus );
}

bool DrmContentPrivate::deleteFile( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->deleteFile( filePath ) : false;
}

qint64 DrmContentPrivate::unencryptedSize( const QString &filePath )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->unencryptedSize( filePath ) : 0;
}

bool DrmContentPrivate::installContent( const QString &filePath, QContent *content )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->installContent( filePath, content ) : false;
}

bool DrmContentPrivate::updateContent( QContent *content )
{
    QDrmContentPlugin *p = plugin( content->file() );

    return p ? p->updateContent( content ) : false;
}

QPixmap DrmContentPrivate::thumbnail( const QString &filePath, const QSize &size, Qt::AspectRatioMode mode )
{
    QDrmContentPlugin *p = plugin( filePath );

    return p ? p->thumbnail( filePath, size, mode ) : QPixmap();
}


QDrmContentPlugin *DrmContentPrivate::plugin( const QString &filePath )
{
    return DrmContentPluginManager::instance()->plugin( filePath );
}


