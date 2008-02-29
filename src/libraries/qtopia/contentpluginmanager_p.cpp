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

#include "contentpluginmanager_p.h"
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include <qcategorymanager.h>
#include <qtranslatablesettings.h>

DotDirectoryContentPlugin::DotDirectoryContentPlugin()
{
}

DotDirectoryContentPlugin::~DotDirectoryContentPlugin()
{
}

QStringList DotDirectoryContentPlugin::keys() const
{
    return QStringList() << QLatin1String( "directory" );
}

bool DotDirectoryContentPlugin::installContent( const QString &filePath, QContent *content )
{
    QSettings settings( filePath, QSettings::IniFormat );

    settings.beginGroup( QLatin1String( "Desktop Entry" ) );

    QString folder = QFileInfo( filePath ).absoluteDir().path();

    if( folder.endsWith( QDir::separator() ) )
        folder.chop( 1 );

    folder = folder.section( QDir::separator(), -1 );

    QString name = settings.value( QLatin1String("Name[]") ).toString();

    content->setName( !name.isEmpty() ? name : folder );

    content->setComment( settings.value( QLatin1String("Comment[]") ).toString() );

    content->setIcon( settings.value( QLatin1String("Icon") ).toString() );

    content->setType( QLatin1String( "Folder/" ) + folder );

    content->setFile( folder );

    content->setLinkFile( filePath );

    content->setRole( QContent::Data );

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

bool DotDirectoryContentPlugin::updateContent( QContent *content )
{
    return content->linkFileKnown() ? installContent( content->linkFile(), content ) : false;
}

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

bool DotDesktopContentPlugin::installContent( const QString &filePath, QContent *content )
{
    QSettings settings( filePath, QSettings::IniFormat );

    QFileInfo info( filePath );

    QDir dir = info.absoluteDir();

    QString folder = dir.absolutePath();

    if( folder.endsWith( QDir::separator() ) )
        folder.chop( 1 );

    QDir parentDir( dir );
    parentDir.cdUp();

    folder = folder.section( QDir::separator(), -1 );

    settings.beginGroup( QLatin1String( "Desktop Entry" ) );

    QString name = settings.value( QLatin1String("Name[]") ).toString();

    QString type = settings.value( QLatin1String("Type") ).toString();

    QString role = settings.value( QLatin1String( "Role" ) ).toString();

    QStringList categories;

    if( type == QLatin1String( "Application" ) || type == QLatin1String( "ConsoleApplication" ) )
    {
        content->setType( QLatin1String( "application/x-executable" ) );
        content->setFile( settings.value( QLatin1String("Exec") ).toString() );
        content->setRole( QContent::Application );
        if( type == QLatin1String( "ConsoleApplication" ) )
            content->setProperty( QLatin1String("ConsoleApplication"), "1");

        QCategoryManager catMan( QLatin1String( "Applications" ) );

        if ( dir.exists( QLatin1String( ".directory" )) ||
                parentDir.exists( QLatin1String( ".directory" )))
        {
            // Extract the category id from the .directory file
            QString dotDir = dir.exists( QLatin1String( ".directory" )) ?
                dir.filePath(".directory") : parentDir.filePath(".directory");
            QSettings ts( dotDir, QSettings::IniFormat);
            ts.beginGroup("Desktop Entry");
            //QString id = QString("_apps_%1").arg(ts.value("Name[]").toString());
            QString id = ts.value("Name[]").toString();  // Don't format the id.
            // Ensure the category id exists
            if( !catMan.exists( id ) )
            {
                catMan.addCategory( id, ts.value("Name[]").toString(), ts.value("Icon").toString(), false, true );
                if(!catMan.isSystem(id))
                    catMan.setSystem(id);
            }
            categories.append( id );
        }
        else
        {
            // No .directory file exists, just use the Applications category
            QString id = QLatin1String("Applications");
            // Ensure the category id exists
            if( !catMan.exists( id ) )
            {
                catMan.addCategory( id, id, QLatin1String( "qpe/AppsIcon" ), false, true );
                if(!catMan.isSystem(id))
                    catMan.setSystem(id);
            }
            categories.append( id );
        }
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

    if( settings.contains( QLatin1String("MimeType") ))
        content->setMimeTypes( settings.value( QLatin1String("MimeType") ).toString().toLower().split( ';' ) );

    if( settings.contains( QLatin1String("MimeTypeIcons") ))
        content->setProperty( QLatin1String("MimeTypeIcons"), settings.value( QLatin1String("MimeTypeIcons") ).toString() );

    content->setLinkFile( filePath );

    // Import categories from a Qtopia 2.x .desktop file
    QCategoryManager catMan("Documents");

    foreach( QString id, settings.value( QLatin1String("Categories") ).toString().split(';') ) {
        if( id.isEmpty() )
            continue;
        if ( !catMan.exists(id) ) {
            if ( id.startsWith("_") ) {
                // It's a "system" category but we don't know about it. Add it anyway and hope for a translation.
                QString text = id;
                text.replace(QRegExp("^_"), "");
                catMan.addCategory( id, text, QString(), false, true );
                if(!catMan.isSystem( id ))
                    catMan.setSystem( id );
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
    \internal

    Loader of QContentPlugins.
*/
ContentPluginManager::ContentPluginManager()
    : manager( QLatin1String("content") )
{
    QStringList pluginNames = manager.list();

    static DotDesktopContentPlugin dotDesktopPlugin;
    static DotDirectoryContentPlugin dotDirectoryPlugin;

    foreach( QString key, dotDesktopPlugin.keys() )
        typePluginMap.insert( key, &dotDesktopPlugin );

    foreach( QString key, dotDirectoryPlugin.keys() )
        typePluginMap.insert( key, &dotDirectoryPlugin );

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
