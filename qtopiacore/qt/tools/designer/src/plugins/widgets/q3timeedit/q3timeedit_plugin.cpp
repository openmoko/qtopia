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

#include "q3timeedit_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3TimeEdit>

Q3TimeEditPlugin::Q3TimeEditPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3TimeEditPlugin::name() const
{ return QLatin1String("Q3TimeEdit"); }

QString Q3TimeEditPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3TimeEditPlugin::toolTip() const
{ return QString(); }

QString Q3TimeEditPlugin::whatsThis() const
{ return QString(); }

QString Q3TimeEditPlugin::includeFile() const
{ return QLatin1String("Qt3Support/Q3TimeEdit"); }

QIcon Q3TimeEditPlugin::icon() const
{ return QIcon(); }

bool Q3TimeEditPlugin::isContainer() const
{ return false; }

QWidget *Q3TimeEditPlugin::createWidget(QWidget *parent)
{ return new Q3TimeEdit(parent); }

bool Q3TimeEditPlugin::isInitialized() const
{ return m_initialized; }

void Q3TimeEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
