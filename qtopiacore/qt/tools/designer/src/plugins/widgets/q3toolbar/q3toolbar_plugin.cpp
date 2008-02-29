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

#include "q3toolbar_plugin.h"
#include "q3toolbar_extrainfo.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>

#include <Qt3Support/Q3MainWindow>
#include <Qt3Support/Q3ToolBar>

Q3ToolBarPlugin::Q3ToolBarPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3ToolBarPlugin::name() const
{ return QLatin1String("Q3ToolBar"); }

QString Q3ToolBarPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3ToolBarPlugin::toolTip() const
{ return QString(); }

QString Q3ToolBarPlugin::whatsThis() const
{ return QString(); }

QString Q3ToolBarPlugin::includeFile() const
{ return QLatin1String("q3listview.h"); }

QIcon Q3ToolBarPlugin::icon() const
{ return QIcon(); }

bool Q3ToolBarPlugin::isContainer() const
{ return false; }

QWidget *Q3ToolBarPlugin::createWidget(QWidget *parent)
{ return new Q3ToolBar(qobject_cast<Q3MainWindow*>(parent)); }

bool Q3ToolBarPlugin::isInitialized() const
{ return m_initialized; }

void Q3ToolBarPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);

    if (m_initialized)
        return;

    QExtensionManager *mgr = core->extensionManager();
    Q_ASSERT(mgr != 0);

    mgr->registerExtensions(new Q3ToolBarExtraInfoFactory(core, mgr), Q_TYPEID(QDesignerExtraInfoExtension));

    m_initialized = true;
}

QString Q3ToolBarPlugin::codeTemplate() const
{ return QString(); }

QString Q3ToolBarPlugin::domXml() const
{ return QString(); }


