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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::TreeWidgetTaskMenu
*/

#include "treewidget_taskmenu.h"
#include "inplace_editor.h"
#include "treewidgeteditor.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

TreeWidgetTaskMenu::TreeWidgetTaskMenu(QTreeWidget *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_treeWidget(button),
      m_editItemsAction(new QAction(tr("Edit Items..."), this))
{
    connect(m_editItemsAction, SIGNAL(triggered()), this, SLOT(editItems()));
    m_taskActions.append(m_editItemsAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}


TreeWidgetTaskMenu::~TreeWidgetTaskMenu()
{
}

QAction *TreeWidgetTaskMenu::preferredEditAction() const
{
    return m_editItemsAction;
}

QList<QAction*> TreeWidgetTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void TreeWidgetTaskMenu::editItems()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_treeWidget);
    if (m_formWindow.isNull())
        return;

    Q_ASSERT(m_treeWidget != 0);

    TreeWidgetEditor dlg(m_formWindow, m_treeWidget->window());
    dlg.fillContentsFromTreeWidget(m_treeWidget);
    if (dlg.exec() == QDialog::Accepted)
        dlg.fillTreeWidgetFromContents(m_treeWidget);
}

TreeWidgetTaskMenuFactory::TreeWidgetTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *TreeWidgetTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QTreeWidget *button = qobject_cast<QTreeWidget*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new TreeWidgetTaskMenu(button, parent);
        }
    }

    return 0;
}

void TreeWidgetTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

