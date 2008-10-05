/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include <qd_common.h>
#include <qtopiadesktop>
#include "pluginmanager.h"
#include <QSignalSpy>
#include "outlooksync.h"
#include <desktopsettings.h>
#include "outlookthread.h"
#include <private/qdplugin_p.h>
#include <QProcess>

class DesktopWrapper : public CenterInterface
{
public:
    DesktopWrapper( QDPlugin *p, QObject *syncObject ) : mSyncObject( syncObject ) {}
    QDDevPlugin *currentDevice() { return 0; }
    const QDLinkPluginList linkPlugins() { return QDLinkPluginList(); }
    const QDDevPluginList devicePlugins() { return QDDevPluginList(); }
    QDPlugin *getPlugin( const QString &id ) { return 0; }
    QObject *syncObject() { return mSyncObject; }
    QObject *mSyncObject;
};

class QtopiaDesktopApplication
{
    friend class TestOutlookComms;
public:
    static void plugin_internal_init( QDPlugin *p ) { p->internal_init(); }
};

#define COMMON_CLASS_DATA\
    private:\
        QObject *syncObject;\
        PluginManager *pm;\
        OutlookSyncPlugin *thePlugin;\
        QList<QByteArray> slowRecords;\
        QList<QByteArray> todayRecords;\
        QMap<QString,QString> idMap;

#define INIT_TEST_CASE_BODY(selected_plugin,enable_trace,wait_for_debugger)\
    QD_COMMON_INIT_TEST_CASE_BODY(enable_trace,wait_for_debugger)\
\
    syncObject = new QObject();\
\
    /* get the requested plugin */\
    pm = new PluginManager( this );\
    pm->setupPlugins( false );\
    foreach ( QDPlugin *plugin, pm->plugins() ) {\
        /*qDebug() << "QDPlugin::init" << "for plugin" << plugin->displayName();*/\
        pm->pluginData(plugin)->center = new DesktopWrapper( plugin, syncObject );\
        plugin->init();\
        QtopiaDesktopApplication::plugin_internal_init( plugin );\
    }\
    QDPlugin *p = 0;\
    foreach ( QDPlugin *plugin, pm->plugins() ) {\
        if ( plugin->id() == selected_plugin ) {\
            p = plugin;\
            break;\
        }\
    }\
    thePlugin = qobject_cast<OutlookSyncPlugin*>(p);\
    QVERIFY(thePlugin);\
\
    /* Force Qt Extended Sync Agent to have no memory of any Outlook events */\
    DesktopSettings settings(thePlugin->id());\
    settings.setValue( "rememberedIds", QStringList() );\
\
    /* Clean out the mapping database */\
    QFile file( DesktopSettings::homePath()+"qtopia.sqlite" );\
    if ( file.exists() )\
        file.remove();\
    QVERIFY(!file.exists());

#define PREPARE_FOR_SYNC_BODY()\
    QSignalSpy spy( thePlugin, SIGNAL(readyForSync(bool)) );\
    thePlugin->prepareForSync();\
    /* Async reply... just idle the event loop while we wait */\
    while ( spy.count() == 0 )\
        qApp->processEvents();\
    QList<QVariant> args = spy.takeFirst();\
    bool ok = args.at(0).toBool();\
    QVERIFY(ok);

#define CLEANUP_TEST_CASE_BODY()\
    thePlugin->finishSync();\
    delete syncObject;\
    delete pm;\
    pm = 0;

#define SANITIZE(array) QString(array).replace("\n","").replace(QRegExp("> +<"), "><").replace(QRegExp("<\\?xml .*\\?>"),"").trimmed().toLocal8Bit()

#endif
