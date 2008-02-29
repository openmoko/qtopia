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

#include <QtDesigner/QtDesigner>

#include "qdesigner_toolbox_p.h"
#include "qdesigner_command_p.h"
#include "orderdialog_p.h"

#include <QtGui/QAction>

using namespace qdesigner_internal;

QDesignerToolBox::QDesignerToolBox(QWidget *parent)
    : QToolBox(parent)
{
    m_actionDeletePage = new QAction(this);
    m_actionDeletePage->setText(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(this);
    m_actionInsertPage->setText(tr("Before Current Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));

    m_actionInsertPageAfter = new QAction(this);
    m_actionInsertPageAfter->setText(tr("After Current Page"));
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));

    m_actionChangePageOrder = new QAction(this);
    m_actionChangePageOrder->setText(tr("Change Page Order..."));
    connect(m_actionChangePageOrder, SIGNAL(triggered()), this, SLOT(changeOrder()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

QString QDesignerToolBox::currentItemText() const
{
    return itemText(currentIndex());
}

void QDesignerToolBox::setCurrentItemText(const QString &itemText)
{
    setItemText(currentIndex(), itemText);
}

QString QDesignerToolBox::currentItemName() const
{
    if (currentIndex() == -1)
        return QString();

    return widget(currentIndex())->objectName();
}

void QDesignerToolBox::setCurrentItemName(const QString &itemName)
{
    if (currentIndex() == -1)
        return;

    widget(currentIndex())->setObjectName(itemName);
}

QIcon QDesignerToolBox::currentItemIcon() const
{
    return itemIcon(currentIndex());
}

void QDesignerToolBox::setCurrentItemIcon(const QIcon &itemIcon)
{
    setItemIcon(currentIndex(), itemIcon);
}

QString QDesignerToolBox::currentItemToolTip() const
{
    return itemToolTip(currentIndex());
}

void QDesignerToolBox::setCurrentItemToolTip(const QString &itemToolTip)
{
    setItemToolTip(currentIndex(), itemToolTip);
}

void QDesignerToolBox::removeCurrentPage()
{
    if (currentIndex() == -1 || !widget(currentIndex()))
        return;

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        DeleteToolBoxPageCommand *cmd = new DeleteToolBoxPageCommand(fw);
        cmd->init(this);

        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::addPage()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        AddToolBoxPageCommand *cmd = new AddToolBoxPageCommand(fw);
        cmd->init(this, AddToolBoxPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::changeOrder()
{
    QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this);
    
    if (!fw)
        return;

    OrderDialog *dlg = new OrderDialog(fw, this);

    QList<QWidget*> wList;
    for(int i=0; i<count(); ++i) {
        wList.append(widget(i));
    }
    dlg->setPageList(&wList);

    if (dlg->exec() == QDialog::Accepted)
    {
        fw->beginCommand(tr("Change Page Order"));

        for(int i=0; i<wList.count(); ++i) {
            if (wList.at(i) == widget(i))
                continue;
            MoveToolBoxPageCommand *cmd = new MoveToolBoxPageCommand(fw);
            cmd->init(this, wList.at(i), i);
            fw->commandHistory()->push(cmd);
        }
        fw->endCommand();
    }
}

void QDesignerToolBox::addPageAfter()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        AddToolBoxPageCommand *cmd = new AddToolBoxPageCommand(fw);
        cmd->init(this, AddToolBoxPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBox::itemInserted(int index)
{
    if (count() > 1 && widget(index))
        widget(index)->setBackgroundRole(widget(index>0?0:1)->backgroundRole());
}


QPalette::ColorRole QDesignerToolBox::currentItemBackgroundRole() const
{
    return widget(0) ? widget(0)->backgroundRole() : QPalette::Window;
}

void QDesignerToolBox::setCurrentItemBackgroundRole(QPalette::ColorRole role)
{
    for (int i = 0; i < count(); ++i) {
        QWidget *w = widget(i);
        w->setBackgroundRole(role);
        w->update();
    }
}

void QDesignerToolBox::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}
