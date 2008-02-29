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
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::GroupBoxTaskMenu
*/

#include "groupbox_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

GroupBoxTaskMenu::GroupBoxTaskMenu(QGroupBox *groupbox, QObject *parent)
    : QDesignerTaskMenu(groupbox, parent),
      m_groupbox(groupbox)
{
    m_editTitleAction = new QAction(tr("Change title..."), this);
    connect(m_editTitleAction, SIGNAL(triggered()), this, SLOT(editTitle()));
    m_taskActions.append(m_editTitleAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

GroupBoxTaskMenu::~GroupBoxTaskMenu()
{
}

QList<QAction*> GroupBoxTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void GroupBoxTaskMenu::editTitle()
{
    QDesignerFormWindowInterface *fw = formWindow();

    if (fw != 0) {
        connect(fw, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_groupbox->parentWidget() != 0);

        m_editor = new InPlaceEditor(m_groupbox, fw);
        m_editor->setFrame(false);
        m_editor->setText(m_groupbox->title());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_groupbox->backgroundRole());
        m_editor->setObjectName(QLatin1String("__qt__passive_m_editor"));
        connect(m_editor, SIGNAL(returnPressed()), m_editor, SLOT(deleteLater()));
        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));
        m_editor->installEventFilter(this); // ### we need this??
        QStyleOption opt; // ## QStyleOptionGroupBox
        opt.init(m_groupbox);
        QRect r = QRect(QPoint(), m_groupbox->size());
        // ### m_groupbox->style()->subRect(QStyle::SR_GroupBoxTitle, &opt, m_groupbox);
        r.setHeight(20);

        m_editor->setGeometry(QRect(m_groupbox->mapTo(m_groupbox->window(), r.topLeft()), r.size()));

        m_editor->setFocus();
        m_editor->show();
    }
}

void GroupBoxTaskMenu::editIcon()
{
}

GroupBoxTaskMenuFactory::GroupBoxTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *GroupBoxTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QGroupBox *groupbox = qobject_cast<QGroupBox*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new GroupBoxTaskMenu(groupbox, parent);
        }
    }

    return 0;
}

void GroupBoxTaskMenu::updateText(const QString &text)
{
    formWindow()->cursor()->setWidgetProperty(m_groupbox,
                                QLatin1String("title"), QVariant(text));
}

void GroupBoxTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

QAction *GroupBoxTaskMenu::preferredEditAction() const
{
    return m_editTitleAction;
}

