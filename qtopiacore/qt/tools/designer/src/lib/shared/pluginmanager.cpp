/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "pluginmanager_p.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QPluginLoader>
#include <QtCore/QLibrary>
#include <QtCore/QLibraryInfo>
#include <QtCore/qdebug.h>
#include <QtCore/QCoreApplication>

static QStringList unique(const QStringList &lst)
{
    QSet<QString> s = QSet<QString>::fromList(lst);
    return s.toList();
}

QStringList QDesignerPluginManager::defaultPluginPaths() const
{
    QStringList result;

    QStringList path_list = QCoreApplication::libraryPaths();
    foreach (const QString &path, path_list)
        result.append(path + QDir::separator() + QLatin1String("designer"));

    result.append(QDir::homePath()
                    + QDir::separator()
                    + QLatin1String(".designer")
                    + QDir::separator()
                    + QLatin1String("plugins"));

    return result;
}

QDesignerPluginManager::QDesignerPluginManager(QDesignerFormEditorInterface *core)
    : QObject(core), m_core(core)
{
    QSettings settings;

    settings.beginGroup(QLatin1String("PluginManager"));

    m_pluginPaths = defaultPluginPaths();
    m_disabledPlugins
        = unique(settings.value(QLatin1String("DisabledPlugins")).toStringList());
    updateRegisteredPlugins();

    settings.endGroup();
}

QDesignerPluginManager::~QDesignerPluginManager()
{
    syncSettings();
}

QDesignerFormEditorInterface *QDesignerPluginManager::core() const
{
    return m_core;
}

QStringList QDesignerPluginManager::findPlugins(const QString &path)
{
    QStringList result;

    QDir dir(path);
    if (!dir.exists())
        return result;
    QStringList candidates = dir.entryList(QDir::Files | QDir::NoSymLinks);
    foreach (QString plugin, candidates) {
        if (!QLibrary::isLibrary(plugin))
            continue;
        result.append(dir.absoluteFilePath(plugin));
    }

    return result;
}

void QDesignerPluginManager::setDisabledPlugins(const QStringList &disabled_plugins)
{
    m_disabledPlugins = disabled_plugins;
    updateRegisteredPlugins();
}

void QDesignerPluginManager::setPluginPaths(const QStringList &plugin_paths)
{
    m_pluginPaths = plugin_paths;
    updateRegisteredPlugins();
}

QStringList QDesignerPluginManager::disabledPlugins() const
{
    return m_disabledPlugins;
}

QStringList QDesignerPluginManager::registeredPlugins() const
{
    return m_registeredPlugins;
}

QStringList QDesignerPluginManager::pluginPaths() const
{
    return m_pluginPaths;
}

QObject *QDesignerPluginManager::instance(const QString &plugin) const
{
    if (m_disabledPlugins.contains(plugin))
        return 0;

    QPluginLoader loader(plugin);
    return loader.instance();
}

void QDesignerPluginManager::updateRegisteredPlugins()
{
    m_registeredPlugins.clear();
    foreach (QString path,  m_pluginPaths)
        registerPath(path);
}

void QDesignerPluginManager::registerPath(const QString &path)
{
    QStringList candidates = findPlugins(path);

    foreach (QString plugin, candidates)
        registerPlugin(plugin);
}

void QDesignerPluginManager::registerPlugin(const QString &plugin)
{
    if (m_disabledPlugins.contains(plugin))
        return;
    if (m_registeredPlugins.contains(plugin))
        return;

    QPluginLoader loader(plugin);
    if (loader.load())
        m_registeredPlugins += plugin;
    if (!loader.isLoaded()) {
        qWarning("QDesignerPluginManager: failed to load plugin\n"
                 " - pluginName='%s'\n"
                 " - error='%s'\n",
                 qPrintable(plugin),
                 qPrintable(loader.errorString()));
    }
}

bool QDesignerPluginManager::syncSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("PluginManager"));
    settings.setValue(QLatin1String("DisabledPlugins"), m_disabledPlugins);
    settings.endGroup();
    return settings.status() == QSettings::NoError;
}

void QDesignerPluginManager::ensureInitialized()
{
    QStringList plugins = registeredPlugins();

    m_customWidgets.clear();
    foreach (QString plugin, plugins) {
        QObject *o = instance(plugin);

        if (QDesignerCustomWidgetInterface *c = qobject_cast<QDesignerCustomWidgetInterface*>(o)) {
            m_customWidgets.append(c);
        } else if (QDesignerCustomWidgetCollectionInterface *coll = qobject_cast<QDesignerCustomWidgetCollectionInterface*>(o)) {
            m_customWidgets += coll->customWidgets();
        }
    }

    foreach (QDesignerCustomWidgetInterface *c, m_customWidgets) {
        if (!c->isInitialized()) {
            c->initialize(core());
        }
    }
}

QList<QDesignerCustomWidgetInterface*> QDesignerPluginManager::registeredCustomWidgets() const
{
    const_cast<QDesignerPluginManager*>(this)->ensureInitialized();
    return m_customWidgets;
}

QList<QObject*> QDesignerPluginManager::instances() const
{
    QStringList plugins = registeredPlugins();

    QList<QObject*> lst;
    foreach (QString plugin, plugins) {
        if (QObject *o = instance(plugin))
            lst.append(o);
    }

    return lst;
}

