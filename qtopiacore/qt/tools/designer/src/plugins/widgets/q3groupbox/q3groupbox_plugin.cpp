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

#include "q3groupbox_plugin.h"

#include <Qt3Support/Q3GroupBox>
#include <QtGui/QLayout>
#include <QtCore/qplugin.h>

Q3GroupBoxPlugin::Q3GroupBoxPlugin(QObject *parent)
    : QObject(parent),
      m_initialized(false)
{
}

Q3GroupBoxPlugin::~Q3GroupBoxPlugin()
{
}


QString Q3GroupBoxPlugin::name() const
{
    return QLatin1String("Q3GroupBox");
}

QString Q3GroupBoxPlugin::group() const
{
    return QLatin1String("Qt 3 Support");
}

QString Q3GroupBoxPlugin::toolTip() const
{
    return QString();
}

QString Q3GroupBoxPlugin::whatsThis() const
{
    return QString();
}

QString Q3GroupBoxPlugin::includeFile() const
{
    return QLatin1String("Qt3Support/Q3GroupBox");
}

QIcon Q3GroupBoxPlugin::icon() const
{
    return QIcon();
}

bool Q3GroupBoxPlugin::isContainer() const
{
    return true;
}

QWidget *Q3GroupBoxPlugin::createWidget(QWidget *parent)
{
    Q3GroupBox *g = new Q3GroupBox(parent);
    g->setColumnLayout(0, Qt::Vertical);
    return g;
}

bool Q3GroupBoxPlugin::isInitialized() const
{
    return m_initialized;
}

void Q3GroupBoxPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}

