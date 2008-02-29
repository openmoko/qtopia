/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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

