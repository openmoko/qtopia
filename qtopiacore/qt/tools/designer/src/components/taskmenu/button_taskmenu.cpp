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
TRANSLATOR qdesigner_internal::ButtonTaskMenu
*/

#include "button_taskmenu.h"
#include "inplace_editor.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

ButtonTaskMenu::ButtonTaskMenu(QAbstractButton *button, QObject *parent)
    : QDesignerTaskMenu(button, parent),
      m_button(button)
{
    m_preferredEditAction = new QAction(this);
    m_preferredEditAction->setText(tr("Change text..."));
    connect(m_preferredEditAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_preferredEditAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

ButtonTaskMenu::~ButtonTaskMenu()
{
}

QAction *ButtonTaskMenu::preferredEditAction() const
{
    return m_preferredEditAction;
}

QList<QAction*> ButtonTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void ButtonTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_button);
    if (!m_formWindow.isNull()) {
        connect(m_formWindow, SIGNAL(selectionChanged()), this, SLOT(updateSelection()));
        Q_ASSERT(m_button->parentWidget() != 0);

        m_editor = new InPlaceEditor(m_button, m_formWindow);
        m_editor->setObjectName(QLatin1String("__qt__passive_m_editor"));

        m_editor->setFrame(false);
        m_editor->setText(m_button->text());
        m_editor->selectAll();
        m_editor->setBackgroundRole(m_button->backgroundRole());
        connect(m_editor, SIGNAL(editingFinished()), m_editor, SLOT(close()));
        connect(m_editor, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));

        QStyleOptionButton opt;
        opt.init(m_button);
        QRect r = m_button->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, m_button);

        m_editor->setGeometry(QRect(m_button->mapTo(m_button->window(), r.topLeft()), r.size()));
        m_editor->setFocus();
        m_editor->show();
    }
}

void ButtonTaskMenu::editIcon()
{
}

ButtonTaskMenuFactory::ButtonTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *ButtonTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QAbstractButton *button = qobject_cast<QAbstractButton*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new ButtonTaskMenu(button, parent);
        }
    }

    return 0;
}

void ButtonTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_button, QLatin1String("text"), QVariant(text));
}

void ButtonTaskMenu::updateSelection()
{
    if (m_editor)
        m_editor->deleteLater();
}

