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
#include "desktopwrapper.h"
#include "qtopiadesktopapplication.h"
#include "pluginmanager.h"
#include "connectionmanager.h"
#include "syncmanager.h"

#include <qdplugin.h>

DesktopWrapper::DesktopWrapper( QDPlugin *_plugin )
    : QObject( _plugin ),
    plugin( _plugin )
{
    q = qobject_cast<QtopiaDesktopApplication*>(qApp);
    Q_ASSERT( q );
}

DesktopWrapper::~DesktopWrapper()
{
}

QDDevPlugin *DesktopWrapper::currentDevice()
{
    return q->connectionManager?q->connectionManager->currentDevice():0;
}

const QDDevPluginList DesktopWrapper::devicePlugins()
{
    if ( qobject_cast<QDConPlugin*>(plugin) )
        return qdPluginManager()->devPlugins();
    return QDDevPluginList();
}

const QDLinkPluginList DesktopWrapper::linkPlugins()
{
    if ( qobject_cast<QDConPlugin*>(plugin) )
        return qdPluginManager()->linkPlugins();
    return QDLinkPluginList();
}

QDPlugin *DesktopWrapper::getPlugin( const QString &id )
{
    foreach ( QDPlugin *plugin, qdPluginManager()->plugins() )
        if ( plugin->id() == id )
            return plugin;
    return 0;
}

QObject *DesktopWrapper::syncObject()
{
    if ( qobject_cast<QDSyncPlugin*>(plugin) )
        return q->syncManager->syncObject();
    return 0;
}

