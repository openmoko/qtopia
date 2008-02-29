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
TRANSLATOR qdesigner_internal::LabelTaskMenu
*/

#include "label_taskmenu.h"

#include <QtDesigner/QtDesigner>
#include <richtexteditor_p.h>

#include <QtGui/QAction>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

LabelTaskMenu::LabelTaskMenu(QLabel *label, QObject *parent)
    : QDesignerTaskMenu(label, parent),
      m_label(label)
{
    m_editTextAction= new QAction(this);
    m_editTextAction->setText(tr("Change text..."));
    connect(m_editTextAction, SIGNAL(triggered()), this, SLOT(editText()));
    m_taskActions.append(m_editTextAction);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    m_taskActions.append(sep);
}

LabelTaskMenu::~LabelTaskMenu()
{
}

QAction *LabelTaskMenu::preferredEditAction() const
{
    return m_editTextAction;
}

QList<QAction*> LabelTaskMenu::taskActions() const
{
    return m_taskActions + QDesignerTaskMenu::taskActions();
}

void LabelTaskMenu::editText()
{
    m_formWindow = QDesignerFormWindowInterface::findFormWindow(m_label);
    if (!m_formWindow.isNull()) {
        RichTextEditorDialog *dlg = new RichTextEditorDialog(m_formWindow);
        Q_ASSERT(m_label->parentWidget() != 0);
        RichTextEditor *editor = dlg->editor();

        editor->setDefaultFont(m_label->font());
        editor->setText(m_label->text());
        editor->selectAll();
        editor->setFocus();

        if (dlg->exec()) {
            QString text = editor->text(m_label->textFormat());
            m_formWindow->cursor()->setWidgetProperty(m_label, QLatin1String("text"), QVariant(text));
        }

        delete dlg;
    }
}

void LabelTaskMenu::editIcon()
{
}

LabelTaskMenuFactory::LabelTaskMenuFactory(QExtensionManager *extensionManager)
    : QExtensionFactory(extensionManager)
{
}

QObject *LabelTaskMenuFactory::createExtension(QObject *object, const QString &iid, QObject *parent) const
{
    if (QLabel *label = qobject_cast<QLabel*>(object)) {
        if (iid == Q_TYPEID(QDesignerTaskMenuExtension)) {
            return new LabelTaskMenu(label, parent);
        }
    }

    return 0;
}

void LabelTaskMenu::updateText(const QString &text)
{
    m_formWindow->cursor()->setWidgetProperty(m_label, QLatin1String("text"), QVariant(text));
}

