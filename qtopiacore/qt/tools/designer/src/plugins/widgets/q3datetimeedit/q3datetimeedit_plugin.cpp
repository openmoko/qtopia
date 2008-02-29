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

#include "q3datetimeedit_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3DateTimeEdit>

Q3DateTimeEditPlugin::Q3DateTimeEditPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3DateTimeEditPlugin::name() const
{ return QLatin1String("Q3DateTimeEdit"); }

QString Q3DateTimeEditPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3DateTimeEditPlugin::toolTip() const
{ return QString(); }

QString Q3DateTimeEditPlugin::whatsThis() const
{ return QString(); }

QString Q3DateTimeEditPlugin::includeFile() const
{ return QLatin1String("Qt3Support/Q3DateTimeEdit"); }

QIcon Q3DateTimeEditPlugin::icon() const
{ return QIcon(); }

bool Q3DateTimeEditPlugin::isContainer() const
{ return false; }

QWidget *Q3DateTimeEditPlugin::createWidget(QWidget *parent)
{ return new Q3DateTimeEdit(parent); }

bool Q3DateTimeEditPlugin::isInitialized() const
{ return m_initialized; }

void Q3DateTimeEditPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
