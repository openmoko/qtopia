/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3progressbar_plugin.h"

#include <QtCore/qplugin.h>
#include <QtGui/QIcon>
#include <Qt3Support/Q3ProgressBar>

Q3ProgressBarPlugin::Q3ProgressBarPlugin(QObject *parent)
        : QObject(parent), m_initialized(false)
{}

QString Q3ProgressBarPlugin::name() const
{ return QLatin1String("Q3ProgressBar"); }

QString Q3ProgressBarPlugin::group() const
{ return QLatin1String("Qt 3 Support"); }

QString Q3ProgressBarPlugin::toolTip() const
{ return QString(); }

QString Q3ProgressBarPlugin::whatsThis() const
{ return QString(); }

QString Q3ProgressBarPlugin::includeFile() const
{ return QLatin1String("Qt3Support/Q3ProgressBar"); }

QIcon Q3ProgressBarPlugin::icon() const
{ return QIcon(); }

bool Q3ProgressBarPlugin::isContainer() const
{ return false; }

QWidget *Q3ProgressBarPlugin::createWidget(QWidget *parent)
{ return new Q3ProgressBar(parent); }

bool Q3ProgressBarPlugin::isInitialized() const
{ return m_initialized; }

void Q3ProgressBarPlugin::initialize(QDesignerFormEditorInterface *core)
{
    Q_UNUSED(core);
    m_initialized = true;
}
