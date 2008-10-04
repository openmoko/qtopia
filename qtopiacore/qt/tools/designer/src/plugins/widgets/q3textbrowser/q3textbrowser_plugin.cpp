/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "q3textbrowser_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3TextBrowser>

QT_BEGIN_NAMESPACE

Q3TextBrowserPlugin::Q3TextBrowserPlugin(const QIcon &icon, QObject *parent)
        : QObject(parent), m_initialized(false), m_icon(icon)
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
{ return m_icon; }

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

QT_END_NAMESPACE
