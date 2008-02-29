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
#include "pluginmanager.h"
#include "mainwindow.h"
#include "qpluginmanager.h"

#include <qdplugin.h>
#include <private/qdplugin_p.h>

#include <qtopiadesktoplog.h>
#include <desktopsettings.h>

#include <QMap>
#include <QWidget>
#include <QStackedWidget>
#include <qdebug.h>

extern QList<qdPluginCreateFunc_t> *qdInternalPlugins();

PluginManager *qdPluginManager()
{
    static PluginManager *pm = 0;
    if ( !pm )
        pm = new PluginManager( 0 );
    return pm;
}

static bool qdpluginLessThan( QDPlugin *p1, QDPlugin *p2 )
{
    return ( p1->displayName() < p2->displayName() );
}

// ==========================================================================

class PluginManagerPrivate
{
public:
    // This is the Qtopia class that wraps up loading of plugins
    QPluginManager *pluginManager;
    // An evil(tm) array thinggy
    QMap<QDPlugin*,QDPluginData*> pluginData;
    QDPluginList allPlugins;
    QDAppPluginList appPlugins;
    QDConPluginList conPlugins;
    QDDevPluginList devPlugins;
    QDLinkPluginList linkPlugins;
    QDSyncPluginList syncPlugins;

    QStringList disabledPluginFiles;
    QStringList detectedPluginFiles;
    QStringList disabledPluginIds;
    QMap<QString,QString> detectedPluginIds;
};

// ==========================================================================

PluginManager::PluginManager( QObject *parent )
    : QObject( parent )
{
    TRACE(PM) << "PluginManager::PluginManager";
    d = new PluginManagerPrivate;

    DesktopSettings settings("settings");
    d->disabledPluginFiles = settings.value("DisabledPluginFiles").toStringList();
    d->disabledPluginIds = settings.value("DisabledPluginIds").toStringList();
    d->disabledPluginIds.removeAll("com.trolltech.plugin.app.infopage");
}

PluginManager::~PluginManager()
{
    TRACE(PM) << "PluginManager::~PluginManager";
    // Clean up the widgets nicely (if we don't do it here, they'll miss out)
    foreach ( QDAppPlugin *plugin, d->appPlugins ) {
        QDAppPluginData *dta = pluginData(plugin);
        if ( dta->appWidget )
            delete dta->appWidget;
        if ( dta->settingsWidget )
            delete dta->settingsWidget;
        if ( dta->mainWindow )
            delete dta->mainWindow;
    }
    foreach ( QDPlugin *plugin, d->allPlugins ) {
        QDPluginData *dta = pluginData( plugin );
        LOG() << "delete plugin";
        delete plugin;
        LOG() << "delete plugin data";
        delete dta;
    }
    delete d;
}

QDAppPluginList PluginManager::activePlugins()
{
    return QDAppPluginList();
}

QDAppPlugin *PluginManager::currentPlugin()
{
    return 0;
}

QDPluginData *PluginManager::pluginData( QDPlugin *plugin )
{
    return d->pluginData[plugin];
}

QDAppPluginData *PluginManager::pluginData( QDAppPlugin *plugin )
{
    return (QDAppPluginData*)d->pluginData[plugin];
}

void PluginManager::setupPlugins( bool safeMode )
{
    TRACE(PM) << "PluginManager::setupPlugins" << "safeMode" << safeMode;

    DesktopSettings settings("settings");
    // Grab a list of the instances we can find
    QList<QObject*> instances;

    // Builtin plugins
    QList<qdPluginCreateFunc_t> internalPlugins( *qdInternalPlugins() );
    foreach( qdPluginCreateFunc_t createFunc, internalPlugins ) {
        QDPlugin *plugin = createFunc( this );
        LOG() << "Internal plugin" << plugin->displayName();
        instances.append( plugin );
    }

    // External factories
    QStringList loadedPluginFiles;
    d->pluginManager = new QPluginManager("qtopiadesktop");
    foreach ( const QString &name, d->pluginManager->list() ) {
        d->detectedPluginFiles << name;
        if ( safeMode || d->disabledPluginFiles.contains(name) )
            continue;
        loadedPluginFiles << name;
        QObject *instance = d->pluginManager->instance(name);
        if ( !instance ) {
            WARNING() << name << "is not a Qt plugin!";
            continue;
        }
        LOG() << "External plugin file" << name;
        instance->setObjectName( name.toLatin1().constData() );
        instances.append( instance );
    }

    // Extract the QDPlugins from the factories
    foreach ( QObject *instance, instances ) {
        if ( QDPluginFactory *pf = qobject_cast<QDPluginFactory*>(instance) ) {
            foreach ( const QString &key, pf->keys() ) {
                QDPlugin *plugin = pf->create( key );
                if ( plugin )
                    setupPlugin( plugin, safeMode );
                else
                    WARNING() << "plugin factory" << instance << "did not create advertised plugin" << key;
            }
        } else if ( QDPlugin *plugin = qobject_cast<QDPlugin*>(instance) ) {
            setupPlugin( plugin, safeMode );
        } else {
            WARNING() << instance->objectName() << "is not a plug-in type that Qtopia Sync Agent recognizes";
            delete instance;
        }
    }

    // Sort the plugins in a consistent way
#define SORT(list)\
    qSort(d-> list ## Plugins .begin(), d-> list ## Plugins .end(), qdpluginLessThan)
    SORT(all);
    SORT(app);
    SORT(con);
    SORT(dev);
    SORT(link);
    SORT(sync);
#undef SORT

    QStringList loadedPlugins;
    foreach ( QDPlugin *p, d->allPlugins ) {
        loadedPlugins << p->id();
    }
    // This sorted because d->allPlugins is sorted
    settings.setValue("loadedPlugins", loadedPlugins);
    loadedPluginFiles.sort();
    settings.setValue("loadedPluginFiles", loadedPluginFiles);
}

void PluginManager::setupPlugin( QDPlugin *plugin, bool safeMode )
{
    d->detectedPluginIds[plugin->id()] = plugin->displayName();
    if ( ( safeMode && plugin->id() != "com.trolltech.plugin.app.infopage" ) ||
         d->disabledPluginIds.contains(plugin->id()) ) {
        delete plugin;
        return;
    }

    QDPluginData *dta = 0;
    d->allPlugins.append( plugin );
    if ( QDAppPlugin *p = qobject_cast<QDAppPlugin*>(plugin) ) {
        // Application plugins
        d->appPlugins.append(p);
        dta = new QDAppPluginData;
    } else if ( QDConPlugin *p = qobject_cast<QDConPlugin*>(plugin) ) {
        // Connection plugins
        d->conPlugins.append(p);
        dta = new QDPluginData;
    } else if ( QDDevPlugin *p = qobject_cast<QDDevPlugin*>(plugin) ) {
        // Device plugins
        d->devPlugins.append(p);
        dta = new QDPluginData;
    } else if ( QDLinkPlugin *p = qobject_cast<QDLinkPlugin*>(plugin) ) {
        // Link plugins (eg. USB, Serial, LAN, QVfb)
        d->linkPlugins.append(p);
        dta = new QDLinkPluginData;
    } else if ( QDSyncPlugin *p = qobject_cast<QDSyncPlugin*>(plugin) ) {
        // Sync plugins (eg. Outlook, Qtopia)
        d->syncPlugins.append(p);
        dta = new QDPluginData;
    }
    d->pluginData[plugin] = dta;
    plugin->d = dta;
}

QDPluginList PluginManager::plugins( QDPluginFilter *filter )
{
    if ( filter ) {
        QDPluginList list;
        foreach ( QDPlugin *plugin, d->appPlugins )
            if ( filter->filter( plugin ) )
                list << plugin;
        return list;
    } else {
        return d->allPlugins;
    }
}

QDAppPluginList PluginManager::appPlugins( QDAppPluginFilter *filter )
{
    if ( filter ) {
        QDAppPluginList list;
        foreach ( QDAppPlugin *plugin, d->appPlugins )
            if ( filter->filter( plugin ) )
                list << plugin;
        return list;
    } else {
        return d->appPlugins;
    }
}

QDConPluginList PluginManager::conPlugins( /*QDConPluginFilter *filter*/ )
{
    return d->conPlugins;
}

QDDevPluginList PluginManager::devPlugins( /*QDDevPluginFilter *filter*/ )
{
    return d->devPlugins;
}

QDLinkPluginList PluginManager::linkPlugins( /*QDLinkPluginFilter *filter*/ )
{
    return d->linkPlugins;
}

QDSyncPluginList PluginManager::syncPlugins( /*QDSyncPluginFilter *filter*/ )
{
    return d->syncPlugins;
}

QStringList PluginManager::detectedPluginFiles()
{
    return d->detectedPluginFiles;
}

QMap<QString,QString> PluginManager::detectedPluginIds()
{
    return d->detectedPluginIds;
}

// ==========================================================================

QDPluginFilter::QDPluginFilter()
    : pluginType(0)
{
}

QDPluginFilter::~QDPluginFilter()
{
}

bool QDPluginFilter::filter( QDPlugin *plugin )
{
    if ( (pluginType & QDPluginFilter::App) && qobject_cast<QDAppPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Link) && qobject_cast<QDLinkPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Con) && qobject_cast<QDConPlugin*>(plugin) == 0 ) 
        return false;
    if ( (pluginType & QDPluginFilter::Dev) && qobject_cast<QDDevPlugin*>(plugin) == 0 )
        return false;
    if ( (pluginType & QDPluginFilter::Sync) && qobject_cast<QDSyncPlugin*>(plugin) == 0 )
        return false;
    return true;
}

// ==========================================================================

QDAppPluginFilter::QDAppPluginFilter()
    : appWidget(Either), settingsWidget(Either)
{
    pluginType = QDPluginFilter::App;
}

QDAppPluginFilter::~QDAppPluginFilter()
{
}

bool QDAppPluginFilter::filter( QDPlugin *plugin )
{
    if ( ! QDPluginFilter::filter(plugin) )
        return false;
    QDAppPluginData *data = qdPluginManager()->pluginData((QDAppPlugin*)plugin);
    if ( (appWidget == Set && data->appWidget == 0 ) ||
         (appWidget == NotSet && data->appWidget != 0 ) )
        return false;
    if ( (settingsWidget == Set && data->settingsWidget == 0 ) ||
         (settingsWidget == NotSet && data->settingsWidget != 0 ) )
        return false;
    return true;
}

