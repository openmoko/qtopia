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
