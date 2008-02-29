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

#include "q3textbrowser_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3TextBrowser>

Q3TextBrowserPlugin::Q3TextBrowserPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3TextBrowserPlugin::name() const
{ return QLatin1String("Q3TextBrowser"); }

QString Q3TextBrowserPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3TextBrowserPlugin::toolTip() const
{ return QString(); }

QString Q3TextBrowserPlugin::whatsThis() const
{ return QString(); }

QString Q3TextBrowserPlugin::includeFile() const
{ return QLatin1String("Qt3Support/Q3TextBrowser"); }

QIcon Q3TextBrowserPlugin::icon() const
{ return QIcon(); }

bool Q3TextBrowserPlugin::isContainer() const
{ return false; }

QWidget *Q3TextBrowserPlugin::createWidget(QWidget *parent)
{ return new Q3TextBrowser(parent); }

bool Q3TextBrowserPlugin::isInitialized() const
{ return m_initialized; }

void Q3TextBrowserPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
