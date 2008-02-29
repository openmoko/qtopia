/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3mainwindow_plugin.h"
#include "q3mainwindow_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <Qt3Support/Q3MainWindow>

Q3MainWindowPlugin::Q3MainWindowPlugin(QObject *parent)
    : QObject(parent), m_initialized(false)
{}

QString Q3MainWindowPlugin::name() const
{ return QLatin1String("Q3MainWindow"); }

QString Q3MainWindowPlugin::group() const
{ return QLatin1String("[invisible]"); }

QString Q3MainWindowPlugin::toolTip() const
{ return QString(); }

QString Q3MainWindowPlugin::whatsThis() const
{ return QString(); }

QString Q3MainWindowPlugin::includeFile() const
{ return QLatin1String("q3mainwindow.h"); }

QIcon Q3MainWindowPlugin::icon() const
{ return QIcon(); }

bool Q3MainWindowPlugin::isContainer() const
{ return true; }

QWidget *Q3MainWindowPlugin::createWidget(QWidget *parent)
{ return new Q3MainWindow(parent); }

bool Q3MainWindowPlugin::isInitialized() const
{ return m_initialized; }

void Q3MainWindowPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    mgr->registerExtensions(new Q3MainWindowContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
}

QString Q3MainWindowPlugin::codeTemplate() const
{ return QString(); }

QString Q3MainWindowPlugin::domXml() const
{
    return QLatin1String("\
    <widget class=\"Q3MainWindow\" name=\"widgetStack\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" name=\"centralWidget\" />\
    </widget>\
    ");
}

