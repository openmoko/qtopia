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
TRANSLATOR qdesigner_internal::OrderDialog
*/

#include "orderdialog_p.h"
#include <QtDesigner/QtDesigner>

using namespace qdesigner_internal;

OrderDialog::OrderDialog(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent), m_pages(0)
{
    ui.setupUi(this);
    m_form = form;

    ui.upButton->setEnabled(false);
    ui.downButton->setEnabled(false);
}

OrderDialog::~OrderDialog()
{
}

void OrderDialog::setPageList(QList<QWidget*> *pages)
{
    m_pages = pages;
    for (int i=0; i<pages->count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(tr("Index %1 (%2)").arg(i).arg(pages->at(i)->objectName()));
        QVariant v;
        qVariantSetValue<QWidget*>(v, pages->at(i));
        item->setData(Qt::UserRole, v);
        ui.pageList->addItem(item);
    }

    if (ui.pageList->count() > 0)
        ui.pageList->setCurrentRow(0);
}

void OrderDialog::accept()
{
    m_pages->clear();
    for (int i=0; i<ui.pageList->count(); ++i)
    {
        if (QWidget *w = qvariant_cast<QWidget*>(ui.pageList->item(i)->data(Qt::UserRole)))
            m_pages->append(w);
    }
    QDialog::accept();
}

void OrderDialog::on_upButton_clicked()
{
    int row = ui.pageList->currentRow();
    if (row <= 0)
        return;

    ui.pageList->insertItem(row - 1, ui.pageList->takeItem(row));
    ui.pageList->setCurrentRow(row - 1);
}

void OrderDialog::on_downButton_clicked()
{
    int row = ui.pageList->currentRow();
    if (row == -1 || row == ui.pageList->count() - 1)
        return;

    ui.pageList->insertItem(row + 1, ui.pageList->takeItem(row));
    ui.pageList->setCurrentRow(row + 1);
}

void OrderDialog::on_pageList_currentRowChanged(int)
{
    ui.upButton->setEnabled(true);
    ui.downButton->setEnabled(true);
}
