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
** Exception version 1.1, which can be found at
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

/*
TRANSLATOR qdesigner_internal::ToolBarTaskMenu
*/

#include "toolbar_taskmenu.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ToolBarTaskMenu::ToolBarTaskMenu(QToolBar *toolbar, QObject *parent)
    : QDesignerTaskMenu(toolbar, parent),
      m_toolbar(toolbar),
      m_editTextAction(new QAction(tr("Customize..."), this))
{
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editToolBar()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ToolBarTaskMenu::~ToolBarTaskMenu()
{
}

QAction *ToolBarTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> ToolBarTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ToolBarTaskMenu::editToolBar()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_toolbar);
    if (!m_formWindow.isNull()) {
    }
    Q_ASSERT(0);
}

ToolBarTaskMenuFactory::ToolBarTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *ToolBarTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QToolBar *toolbar = qobject_cast<QToolBar*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new ToolBarTaskMenu(toolbar, parent);
        }
    }

    return 0;
}
