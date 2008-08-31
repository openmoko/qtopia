/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
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
