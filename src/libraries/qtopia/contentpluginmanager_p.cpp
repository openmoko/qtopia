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

#include "contentpluginmanager_p.h"
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include <qcategorymanager.h>
#include <qtranslatablesettings.h>
#include <QMimeType>

DotDesktopContentPlugin::DotDesktopContentPlugin()
{
}

DotDesktopContentPlugin::~DotDesktopContentPlugin()
{
}

QStringList DotDesktopContentPlugin::keys() const
{
    return QStringList() << QLatin1String( "desktop" );
}

QList< QDrmRights::Permission > extractPermissions( const QString &permissionList )
{
    QStringList permissionStrings = permissionList.toLower()
            .split(QLatin1Char(';'), QString::SkipEmptyParts);

    QList< QDrmRights::Permission > permissions;

    foreach( QString permission, permissionStrings )
    {
        if( permission.isEmpty() )
            continue;

        if( permission == QLatin1String( "play" ) )
            permissions.append( QDrmRights::Play );
        else if( permission == QLatin1String( "display" ) )
            permissions.append( QDrmRights::Display );
        else if( permission == QLatin1String( "execute" ) )
            permissions.append( QDrmRights::Execute );
        else if( permission == QLatin1String( "print" ) )
            permissions.append( QDrmRights::Print );
        else if( permission == QLatin1String( "export" ) )
            permissions.append( QDrmRights::Export );
        else if( permission == QLatin1String( "browsecontents" ) )
            permissions.append( QDrmRights::BrowseContents );
        else
            permissions.append( QDrmRights::Unrestricted );
    }

    return permissions;
}

bool DotDesktopContentPlugin::installContent( const QString &filePath, QContent *content )
{
    QSettings settings( filePath, QSettings::IniFormat );

    QDir dir = QFileInfo( filePath ).absoluteDir();

    settings.beginGroup( QLatin1String( "Desktop Entry" ) );

    QString name = settings.value( QLatin1String("Name[]") ).toString();

    QString type = settings.value( QLatin1String("Type") ).toString();

    QString role = settings.value( QLatin1String( "Role" ) ).toString();

    if( type == QLatin1String( "Application" ) || type == QLatin1String( "ConsoleApplication" ) )
    {
        content->setType( QLatin1String( "application/x-executable" ) );
        content->setFile( settings.value( QLatin1String("Exec") ).toString() );
        content->setRole( QContent::Application );
        if( type == QLatin1String( "ConsoleApplication" ) )
            content->setProperty( QLatin1String("ConsoleApplication"), "1");
    }
    else
    {
        content->setType( type );

        if( role == "Document" )
            content->setRole( QContent::Document );
        else if( role == "Application" )
            content->setRole( QContent::Application );
        else
            content->setRole( QContent::Data );

        QString target = settings.value( QLatin1String("File") ).toString();

        if( !target.isEmpty() && target[0] != QDir::separator() )
            content->setFile( dir.filePath( target ) );
        else
            content->setFile( target );
    }

    content->setName( !name.isEmpty() ? name : QFileInfo( filePath ).baseName() );

    content->setComment( settings.value( QLatin1String("Comment[]") ).toString() );

    content->setIcon( settings.value( QLatin1String("Icon") ).toString() );

    content->setProperty( QLatin1String("Rotation"), settings.value( QLatin1String("Rotation") ).toString() );
    content->setProperty( QLatin1String("CanFastLoad"), settings.value( QLatin1String("CanFastLoad") ).toString() );
    content->setProperty( QLatin1String("Builtin"), settings.value( QLatin1String("Builtin") ).toString() );

    QStringList mimeTypes;
    QStringList mimeIcons;
    QList< QDrmRights::Permission > mimePermissions;

    if (settings.contains(QLatin1String("MimeType"))) {
        mimeTypes = settings.value(QLatin1String("MimeType"))
                .toString().toLower().split(QLatin1Char(';'), QString::SkipEmptyParts);

        if (mimeTypes.count() == 1)
            mimeTypes = mimeTypes.first().split(QLatin1Char(','));
    }

    if (settings.contains(QLatin1String("MimeTypeIcons"))) {
        mimeIcons = settings.value(QLatin1String("MimeTypeIcons"))
                .toString().split(QLatin1Char(';'), QString::SkipEmptyParts);

        if (mimeIcons.count() == 1)
            mimeIcons = mimeIcons.first().split(QLatin1Char(','), QString::SkipEmptyParts);
    }

    settings.endGroup();
    settings.beginGroup( QLatin1String( "DRM" ) );

    if (settings.contains( QLatin1String("MimeTypePermissions")))
        mimePermissions = extractPermissions( settings.value( QLatin1String("MimeTypePermissions") ).toString() );

    settings.endGroup();
    settings.beginGroup( QLatin1String( "Desktop Entry" ) );

    if( mimeTypes.count() > 1 )
    {
        if( mimeIcons.count() == 1 )
            for( int i = 1; i < mimeTypes.count(); i++ )
                mimeIcons.append( mimeIcons.first() );
        if( mimePermissions.count() == 1 )
            for( int i = 1; i < mimeTypes.count(); i++ )
                mimePermissions.append( mimePermissions.first() );
    }

    content->setMimeTypes( mimeTypes, mimeIcons, mimePermissions);

    content->setLinkFile( filePath );

    QStringList categories;

    // Import categories from a Qtopia 2.x .desktop file
    QCategoryManager catMan( content->role() == QContent::Application ? QLatin1String("Applications") : QLatin1String("Documents") );

#ifndef QTOPIA_CONTENT_INSTALLER
    QString folder = dir.dirName();

    if( catMan.contains( folder ) )
        categories.append( folder );
#endif

    QStringList categoryIds = settings.value(QLatin1String("Categories"))
            .toString().split(QLatin1Char(';'), QString::SkipEmptyParts);

    if (categoryIds.count() == 1)
        categoryIds = categoryIds.first().split(QLatin1Char(','), QString::SkipEmptyParts);

    foreach (QString id, categoryIds) {
        if ( !catMan.exists(id) ) {
            if ( id.startsWith("_") ) {
                // It's a "system" category but we don't know about it. Add it anyway and hope for a translation.
                QString text = id;
                text.replace(QRegExp("^_"), "");
                // For new code a more unique id should be used instead of using the untranslated text
                // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
                catMan.ensureSystemCategory( id, text, QString() );
            } else {
                // It's a translated string. Do a (dodgy) check for the string
                if ( catMan.containsLabel(id) ) {
                    // Get the real id for this translated string
                    id = catMan.idForLabel(id);
                } else {
                    // It doesn't seem to exist. Add the translated string and get a real id back
                    id = catMan.add(id);
                }
            }
        }
        categories.append( id );
    }

#ifndef QTOPIA_CONTENT_INSTALLER
    if( categories.isEmpty() && content->role() == QContent::Application )
    {
        // No .directory file exists, just use the Applications category
        QString id = QLatin1String("Applications");
        // Ensure the category id exists
        // For new code a more unique id should be used instead of using the untranslated text
        // eg. ensureSystemCategory("com.mycompany.myapp.mycategory", "My Category");
        catMan.ensureSystemCategory( id, id, QLatin1String( "qpe/AppsIcon" ) );
        categories.append( id );
    }
#endif

    content->setCategories( categories );

    settings.endGroup();

    foreach( QString group, settings.childGroups() )
    {
        if( group == QLatin1String("Desktop Entry") )
            continue;

        settings.beginGroup( group );

        foreach( QString key, settings.childKeys() )
            content->setProperty( key, settings.value( key ).toString(), group );

        settings.endGroup();
    }

    return true;
}

bool DotDesktopContentPlugin::updateContent( QContent *content )
{
    return content->linkFileKnown() ? installContent( content->linkFile(), content ) : false;
}

/*!
    \class ContentPluginManager
    \mainclass
    \internal

    Loader of QContentPlugins.
*/
ContentPluginManager::ContentPluginManager()
    : manager( QLatin1String("content") )
{
    QStringList pluginNames = manager.list();

    static DotDesktopContentPlugin dotDesktopPlugin;

    foreach( QString key, dotDesktopPlugin.keys() )
        typePluginMap.insert( key, &dotDesktopPlugin );

    foreach( QString pluginName, pluginNames )
    {
        QContentPlugin *plugin = qobject_cast< QContentPlugin * >( manager.instance( pluginName ) );

        if( plugin )
        {
            QStringList keys = plugin->keys();

            foreach( QString key, keys )
                typePluginMap.insert( key, plugin );

            plugins.append( plugin );
        }
    }
}

ContentPluginManager::~ContentPluginManager()
{
}

QList< QContentPlugin * > ContentPluginManager::findPlugins( const QString &filePath )
{
    return typePluginMap.values( QFileInfo( filePath ).suffix().toLower() );
}

/*!
    \class QContentFactory
    \brief The QContentFactory class manages instances of QContentPlugin.

    \internal
*/


Q_GLOBAL_STATIC( ContentPluginManager, pluginManager );

/*!
    Populates \a content with data from the file with the file name \a fileName.  Returns true if the content
    plug-in successfully populated the QContent.

    Installation is only performed when the content is first identified by the content system, if the file changes
    after installation updateContent() will be called to ensure the content data is up to date; otherwise returns
    false
*/
bool QContentFactory::installContent( const QString &fileName, QContent *content )
{
    QList< QContentPlugin * > plugins = pluginManager()->findPlugins( fileName );

    foreach( QContentPlugin *p, plugins )
        if( p->installContent( fileName, content ) )
            return true;

    return false;
}

/*!
    Refreshes the content data of \a content following a change to the file it references.

    Returns true if the content data has been ensured to be up to date; otherwise returns false.
*/
bool QContentFactory::updateContent( QContent *content )
{
    QList< QContentPlugin * > plugins = pluginManager()->findPlugins( content->file() );

    foreach( QContentPlugin *p, plugins )
        if( p->updateContent( content ) )
            return true;

    return false;
}


