/****************************************************************************
**
** Copyright (C) 2005-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
