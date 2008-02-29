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

#include "q3listbox_plugin.h"
#include "q3listbox_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3ListBox>

Q3ListBoxPlugin::Q3ListBoxPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3ListBoxPlugin::name() const
{ return QLatin1String("Q3ListBox"); }

QString Q3ListBoxPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3ListBoxPlugin::toolTip() const
{ return QString(); }

QString Q3ListBoxPlugin::whatsThis() const
{ return QString(); }

QString Q3ListBoxPlugin::includeFile() const
{ return QLatin1String("q3listbox.h"); }

QIcon Q3ListBoxPlugin::icon() const
{ return QIcon(); }

bool Q3ListBoxPlugin::isContainer() const
{ return false; }

QWidget *Q3ListBoxPlugin::createWidget(QWidget *parent)
{ return new Q3ListBox(parent); }

bool Q3ListBoxPlugin::isInitialized() const
{ return m_initialized; }

void Q3ListBoxPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3ListBoxExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3ListBoxPlugin::codeTemplate() const
{ return QString(); }

QString Q3ListBoxPlugin::domXml() const
{ return QLatin1String("\
    <widget class=\"Q3ListBox\" name=\"listBox\">\
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


