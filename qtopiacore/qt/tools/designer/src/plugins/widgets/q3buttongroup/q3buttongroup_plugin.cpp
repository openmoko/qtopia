/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
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

#include "q3buttongroup_plugin.h"

#include <Qt3Support/Q3ButtonGroup>
#include <QtGui/QLayout>
#include <QtCore/qplugin.h>

Q3ButtonGroupPlugin::Q3ButtonGroupPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

Q3ButtonGroupPlugin::~Q3ButtonGroupPlugin()
{
}


QString Q3ButtonGroupPlugin::name() const
{
    return QLatin1String("Q3ButtonGroup");
}

QString Q3ButtonGroupPlugin::group() const
{
    return QLatin1String("Qt 3 Support");
}

QString Q3ButtonGroupPlugin::toolTip() const
{
    return QString();
}

QString Q3ButtonGroupPlugin::whatsThis() const
{
    return QString();
}

QString Q3ButtonGroupPlugin::includeFile() const
{
    return QLatin1String("Qt3Support/Q3ButtonGroup");
}

QIcon Q3ButtonGroupPlugin::icon() const
{
    return QIcon();
}

bool Q3ButtonGroupPlugin::isContainer() const
{
    return true;
}

QWidget *Q3ButtonGroupPlugin::createWidget(QWidget *parent)
{
    Q3ButtonGroup *g = new Q3ButtonGroup(parent);
    g->setColumnLayout(0, Qt::Vertical);
    return g;
}

bool Q3ButtonGroupPlugin::isInitialized() const
{
    return m_initialized;
}

void Q3ButtonGroupPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
