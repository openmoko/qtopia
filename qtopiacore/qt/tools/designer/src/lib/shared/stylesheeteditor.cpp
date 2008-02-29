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
TRANSLATOR qdesigner_internal::StyleSheetEditorDialog
*/

#include "stylesheeteditor_p.h"
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtDesigner/QtDesigner>

namespace qdesigner_internal {

StyleSheetEditor::StyleSheetEditor(QWidget *parent)
    : QTextEdit(parent)
{
}

StyleSheetEditorDialog::StyleSheetEditorDialog(QWidget *fw, QWidget *widget)
    : QDialog(fw), m_widget(widget)
{
    m_fw = qobject_cast<QDesignerFormWindowInterface *>(fw);
    Q_ASSERT(m_fw != 0);
    setWindowTitle(tr("Edit Style Sheet"));
    QVBoxLayout *layout = new QVBoxLayout;
    m_editor = new StyleSheetEditor;
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QPushButton *apply = buttonBox->button(QDialogButtonBox::Apply);
    QObject::connect((const QObject *)apply, SIGNAL(clicked()), this, SLOT(applyStyleSheet()));
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(applyStyleSheet()));

    layout->addWidget(m_editor);;
    layout->addWidget(buttonBox);
    setLayout(layout);

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_fw->core()->extensionManager(), m_widget);
    Q_ASSERT(sheet != 0);
    m_editor->setText(sheet->property(sheet->indexOf("styleSheet")).toString());

    m_editor->setFocus();
}

StyleSheetEditor *StyleSheetEditorDialog::editor() const
{
    return m_editor;
}

void StyleSheetEditorDialog::applyStyleSheet()
{
    QString text = m_editor->toPlainText();
    m_fw->cursor()->setWidgetProperty(m_widget, "styleSheet", QVariant(text));
}

} // namespace qdesigner_internal
