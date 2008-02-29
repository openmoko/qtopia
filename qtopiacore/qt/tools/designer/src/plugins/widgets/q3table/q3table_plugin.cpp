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

#include "q3table_plugin.h"
#include "q3table_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3Table>

Q3TablePlugin::Q3TablePlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3TablePlugin::name() const
{ return QLatin1String("Q3Table"); }

QString Q3TablePlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3TablePlugin::toolTip() const
{ return QString(); }

QString Q3TablePlugin::whatsThis() const
{ return QString(); }

QString Q3TablePlugin::includeFile() const
{ return QLatin1String("q3table.h"); }

QIcon Q3TablePlugin::icon() const
{ return QIcon(); }

bool Q3TablePlugin::isContainer() const
{ return false; }

QWidget *Q3TablePlugin::createWidget(QWidget *parent)
{ return new Q3Table(parent); }

bool Q3TablePlugin::isInitialized() const
{ return m_initialized; }

void Q3TablePlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3TableExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3TablePlugin::codeTemplate() const
{ return QString(); }

QString Q3TablePlugin::domXml() const
{ return QLatin1String("\
    <widget class=\"Q3Table\" name=\"table\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
    </widget>\
    ");
}


