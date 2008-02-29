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

#include "q3widgetstack_plugin.h"
#include "q3widgetstack_container.h"

#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include "qdesigner_q3widgetstack_p.h"

Q3WidgetStackPlugin::Q3WidgetStackPlugin(QObject *parent)
    : QObject(parent), m_initialized(false)
{}

QString Q3WidgetStackPlugin::name() const
{ return QLatin1String("Q3WidgetStack"); }

QString Q3WidgetStackPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3WidgetStackPlugin::toolTip() const
{ return QString(); }

QString Q3WidgetStackPlugin::whatsThis() const
{ return QString(); }

QString Q3WidgetStackPlugin::includeFile() const
{ return QLatin1String("q3widgetstack.h"); }

QIcon Q3WidgetStackPlugin::icon() const
{ return QIcon(); }

bool Q3WidgetStackPlugin::isContainer() const
{ return true; }

QWidget *Q3WidgetStackPlugin::createWidget(QWidget *parent)
{ return new QDesignerQ3WidgetStack(parent); }

bool Q3WidgetStackPlugin::isInitialized() const
{ return m_initialized; }

void Q3WidgetStackPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    m_initialized = true;
    QExtensionManager *mgr = core->extensionManager();
    mgr->registerExtensions(new Q3WidgetStackContainerFactory(mgr), Q_TYPEID(QDesignerContainerExtension));
}

QString Q3WidgetStackPlugin::codeTemplate() const
{ return QString(); }

QString Q3WidgetStackPlugin::domXml() const
{
    return QLatin1String("\
    <widget class=\"Q3WidgetStack\" name=\"widgetStack\">\
        <property name=\"geometry\">\
            <rect>\
                <x>0</x>\
                <y>0</y>\
                <width>100</width>\
                <height>80</height>\
            </rect>\
        </property>\
        <widget class=\"QWidget\" name=\"page\"/>\
        <widget class=\"QWidget\" name=\"page_2\"/>\
    </widget>\
    ");
}
