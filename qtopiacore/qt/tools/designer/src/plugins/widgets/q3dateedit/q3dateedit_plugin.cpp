/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3dateedit_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3DateEdit>

Q3DateEditPlugin::Q3DateEditPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3DateEditPlugin::name() const
{ return QLatin1String("Q3DateEdit"); }

QString Q3DateEditPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3DateEditPlugin::toolTip() const
{ return QString(); }

QString Q3DateEditPlugin::whatsThis() const
{ return QString(); }

QString Q3DateEditPlugin::includeFile() const
{ return QLatin1String("Qt3Support/Q3DateEdit"); }

QIcon Q3DateEditPlugin::icon() const
{ return QIcon(); }

bool Q3DateEditPlugin::isContainer() const
{ return false; }

QWidget *Q3DateEditPlugin::createWidget(QWidget *parent)
{ return new Q3DateEdit(parent); }

bool Q3DateEditPlugin::isInitialized() const
{ return m_initialized; }

void Q3DateEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
