/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef DRMCONTENT_H
#define DRMCONTENT_H

#include <QObject>
#include <QDateTime>
#include <qtimestring.h>
#include <qcontent.h>
#include <QAbstractFileEngineHandler>
#include <QFSFileEngine>
#include <QTimerEvent>
#include <qdrmcontent.h>
#include "thumbnailview_p.h"
#include "singleview_p.h"
#include "contentlnk_p.h"
#include <qdrmcontentplugin.h>
#include <qpluginmanager.h>

class DrmContentPrivate
{
public:
    static bool isProtected( const QString &filePath );

    static QDrmRights::Permissions permissions( const QString &filePath );

    static QDrmRights getRights( const QString &filePath, QDrmRights::Permission permission );

    static QDrmContentLicense *requestContentLicense( const QContent &content, QDrmRights::Permission permission, QDrmContent::LicenseOptions options );

    static QIODevice *createDecoder( const QString &filePath, QDrmRights::Permission permission );

    static bool canActivate( const QString &filePath );

    static bool activate( const QContent &content, QDrmRights::Permission permission, QWidget *focus = 0 );

    static void activate( const QContent &content, QWidget *focus = 0 );

    static void reactivate( const QContent &content, QDrmRights::Permission permission, QWidget *focus = 0 );

    static bool deleteFile( const QString &filePath );

    static qint64 unencryptedSize( const QString &filePath );

    static bool installContent( const QString &filePath, QContent *content );

    static QDrmContentPlugin *plugin( const QString &filePath );

    static QIcon createIcon( const QIcon &baseIcon, int smallSize, int bigSize, bool validRights = true );

    static QPixmap thumbnail( const QString &filePath, const QSize &size, Qt::AspectRatioMode mode = Qt::KeepAspectRatio );

    static QPixmap drmIcon( int size );

    static QPixmap compositeDrmIcon( const QPixmap &base, int size, bool validRights = true );

};

class DrmContentPluginManager
{
    public:
    DrmContentPluginManager();
    ~DrmContentPluginManager();

    QDrmContentPlugin *plugin( const QString &filePath ) const;

    QList< QDrmContentPlugin * > plugins() const;

    static DrmContentPluginManager *instance();
private:
    QPluginManager m_manager;

    QMap< QString, QDrmContentPlugin * > m_pluginMap;
    QList< QDrmContentPlugin * > m_plugins;
};


class DRMThumbnailLoader : public ThumbnailLoader
{
public:
    DRMThumbnailLoader( ThumbnailCache* cache, QObject* parent );

protected:
    virtual QPixmap loadThumbnail( const QString &filename, const QSize &size );
};

#endif
