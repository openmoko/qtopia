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

#include "q3listview_plugin.h"
#include "q3listview_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3ListView>

Q3ListViewPlugin::Q3ListViewPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3ListViewPlugin::name() const
{ return QLatin1String("Q3ListView"); }

QString Q3ListViewPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3ListViewPlugin::toolTip() const
{ return QString(); }

QString Q3ListViewPlugin::whatsThis() const
{ return QString(); }

QString Q3ListViewPlugin::includeFile() const
{ return QLatin1String("q3listview.h"); }

QIcon Q3ListViewPlugin::icon() const
{ return QIcon(); }

bool Q3ListViewPlugin::isContainer() const
{ return false; }

QWidget *Q3ListViewPlugin::createWidget(QWidget *parent)
{ return new Q3ListView(parent); }

bool Q3ListViewPlugin::isInitialized() const
{ return m_initialized; }

void Q3ListViewPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3ListViewExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3ListViewPlugin::codeTemplate() const
{ return QString(); }

QString Q3ListViewPlugin::domXml() const
{ return QLatin1String("\
    <widget class=\"Q3ListView\" name=\"listView\">\
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


