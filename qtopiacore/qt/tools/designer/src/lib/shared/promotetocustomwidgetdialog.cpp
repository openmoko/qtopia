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
TRANSLATOR qdesigner_internal::PromoteToCustomWidgetDialog
*/

#include "promotetocustomwidgetdialog_p.h"
#include "ui_promotetocustomwidgetdialog.h"

#include <QtDesigner/QtDesigner>

#include <QtGui/QRegExpValidator>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include <QtCore/qdebug.h>

namespace qdesigner_internal {

PromoteToCustomWidgetDialog::PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                                        const QString &base_class_name,
                                                        QWidget *parent)
    : QDialog(parent)
{
    setModal(true);

    ui = new Ui::PromoteToCustomWidgetDialog;
    ui->setupUi(this);

    m_db = db;
    m_base_class_name = base_class_name;


    ui->m_class_name_input->addItem(QString());
    for (int i = 0; i < db->count(); ++i) {
        QDesignerWidgetDataBaseItemInterface *item = db->item(i);
        if (!item->isPromoted())
            continue;
        if (item->extends() != base_class_name)
            continue;
        m_promoted_list.append(qMakePair(item->name(), item->includeFile()));
        ui->m_class_name_input->addItem(item->name());
    }

    ui->m_class_name_input->setValidator(new QRegExpValidator(QRegExp(QLatin1String("[_a-zA-Z:][:_a-zA-Z0-9]*")), ui->m_class_name_input));
    ui->m_base_class_name_label->setText(base_class_name);

    connect(ui->m_class_name_input->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(checkInputs()));
    connect(ui->m_class_name_input, SIGNAL(activated(QString)),
            this, SLOT(setIncludeForClass(QString)));
    connect(ui->m_header_file_input, SIGNAL(textChanged(QString)), this, SLOT(checkInputs()));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    m_automatic_include = true;

    ui->m_class_name_input->setFocus();
}

PromoteToCustomWidgetDialog::~PromoteToCustomWidgetDialog()
{
    delete ui;
    ui = 0;
}

void PromoteToCustomWidgetDialog::checkInputs()
{
    bool blocked = ui->m_header_file_input->blockSignals(true);
    if (sender() == ui->m_class_name_input->lineEdit()) {
        if (m_automatic_include) {
            QString class_name = customClassName();
            if (class_name.isEmpty())
                ui->m_header_file_input->clear();
            else
                ui->m_header_file_input->setText(class_name.toLower().replace(QLatin1String("::"), QLatin1String("_")) + QLatin1String(".h"));
        }
    } else if (sender() == ui->m_header_file_input) {
        m_automatic_include = false;
    }
    ui->m_header_file_input->blockSignals(blocked);

    bool enabled = !customClassName().isEmpty() && !includeFile().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(enabled);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(!enabled);
}

void PromoteToCustomWidgetDialog::setIncludeForClass(const QString &name)
{
    bool blocked = ui->m_header_file_input->blockSignals(true);
    ui->m_header_file_input->clear();
    foreach (PromotedWidgetInfo info, m_promoted_list) {
        if (info.first == name) {
            ui->m_header_file_input->setText(info.second);
            break;
        }
    }
    ui->m_header_file_input->blockSignals(blocked);
    m_automatic_include = true;
}

void PromoteToCustomWidgetDialog::accept()
{
    QString custom_class_name = customClassName();
    QString include_file = includeFile();

    QDesignerWidgetDataBaseItemInterface *item = m_db->item(m_db->indexOfClassName(custom_class_name));
    if (item != 0) {
        if (!item->isPromoted()) {
            QMessageBox::warning(0, tr("Conflicting class name"),
                                    tr("<b>%1</b> cannot be used as the class of the promoted"
                                        " widget, as a class of that name already exists"
                                        " and is not a promoted widget.")
                                            .arg(custom_class_name),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
        if (item->extends() != m_base_class_name) {
            QMessageBox::warning(0, tr("Conflicting class name"),
                                    tr("<b>%1</b> cannot be used as the class of the promoted"
                                        " widget, as a class of that name already exists"
                                        " and extends <b>%2</b>.")
                                            .arg(custom_class_name)
                                            .arg(item->extends()),
                                    QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
    }

    foreach (PromotedWidgetInfo info, m_promoted_list) {
        if (info.first == custom_class_name) {
            if (info.second != include_file) {
                int result
                    = QMessageBox::warning(0, tr("Conflicting include file"),
                            tr("<b>%1</b> has been previously specified as the"
                            " include file for <b>%2</b>. Do you want to"
                            " change all instances to use <b>%3</b> instead?")
                                    .arg(info.second)
                                    .arg(custom_class_name)
                                    .arg(include_file),
                            QMessageBox::Yes, QMessageBox::No);
                if (result == QMessageBox::No)
                    return;
            }
            break;
        }
    }

    QDialog::accept();
}

QString PromoteToCustomWidgetDialog::includeFile() const
{
    return ui->m_header_file_input->text();
}

QString PromoteToCustomWidgetDialog::customClassName() const
{
    return ui->m_class_name_input->lineEdit()->text();
}

} // namespace qdesigner_internal
