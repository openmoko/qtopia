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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <qdplugindefs.h>

#include <QObject>
#include <QFlags>
#include <QString>
#include <QMap>

class PluginManagerPrivate;
class QDPluginFilter;
class QDAppPluginFilter;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    PluginManager( QObject *parent = 0 );
    ~PluginManager();

    void setupPlugins( bool safeMode );
    void setupPlugin( QDPlugin *plugin );

    QDAppPluginList activePlugins();
    QDAppPlugin *currentPlugin();
    QDPluginData *pluginData( QDPlugin *plugin );
    QDAppPluginData *pluginData( QDAppPlugin *plugin );

    QDPluginList plugins( QDPluginFilter *filter = 0 );
    QDAppPluginList appPlugins( QDAppPluginFilter *filter = 0 );
    QDConPluginList conPlugins();
    QDDevPluginList devPlugins();
    QDLinkPluginList linkPlugins();
    QDSyncPluginList syncPlugins();

    QStringList detectedPluginFiles();
    QMap<QString,QString> detectedPluginIds();

private:
    void internal_setupPlugin( QDPlugin *plugin, bool safeMode, bool addToLists );

    PluginManagerPrivate *d;
};

extern PluginManager *qdPluginManager();


class QDPluginFilter
{
public:
    QDPluginFilter();
    virtual ~QDPluginFilter();

    enum PluginTypeFlags {
        App  = 0x01,
        Link = 0x02,
        Con  = 0x04,
        Dev  = 0x08,
        Sync = 0x10,
    };
    Q_DECLARE_FLAGS(PluginType,PluginTypeFlags)
    enum FilterOption { Either, Set, NotSet };

    virtual bool filter( QDPlugin *plugin );

    PluginType pluginType;
};

class QDAppPluginFilter : public QDPluginFilter
{
public:
    QDAppPluginFilter();
    virtual ~QDAppPluginFilter();

    virtual bool filter( QDPlugin *plugin );

    FilterOption appWidget;
    FilterOption settingsWidget;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDPluginFilter::PluginType)

#endif
