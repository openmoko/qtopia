/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_stackedbox_p.h"
#include "abstractformwindow.h"
#include "qdesigner_command_p.h"
#include <QtGui/QUndoCommand>
#include "orderdialog_p.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QToolButton>
#include <QtGui/QAction>
#include <QtGui/qevent.h>
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

QDesignerStackedWidget::QDesignerStackedWidget(QWidget *parent)
    : QStackedWidget(parent), m_actionDeletePage(0)
{
    prev = new QToolButton();
    prev->setAttribute(Qt::WA_NoChildEventsForParent, true);
    prev->setParent(this);

    prev->setObjectName(QLatin1String("__qt__passive_prev"));
    prev->setArrowType(Qt::LeftArrow);
    prev->setAutoRaise(true);
    prev->setAutoRepeat(true);
    prev->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(prev, SIGNAL(clicked()), this, SLOT(prevPage()));

    next = new QToolButton();
    next->setAttribute(Qt::WA_NoChildEventsForParent, true);
    next->setParent(this);
    next->setObjectName(QLatin1String("__qt__passive_next"));
    next->setArrowType(Qt::RightArrow);
    next->setAutoRaise(true);
    next->setAutoRepeat(true);
    next->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
    connect(next, SIGNAL(clicked()), this, SLOT(nextPage()));

    updateButtons();

    m_actionPreviousPage = new QAction(tr("Previous Page"), this);
    connect(m_actionPreviousPage, SIGNAL(triggered()), this, SLOT(prevPage()));

    m_actionNextPage = new QAction(tr("Next Page"), this);
    connect(m_actionNextPage, SIGNAL(triggered()), this, SLOT(nextPage()));

    m_actionDeletePage = new QAction(tr("Delete Page"), this);
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    m_actionInsertPage = new QAction(tr("Before Current Page"), this);
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));

    m_actionInsertPageAfter = new QAction(tr("After Current Page"), this);
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));

    m_actionChangePageOrder = new QAction(tr("Change Page Order..."), this);
    connect(m_actionChangePageOrder, SIGNAL(triggered()), this, SLOT(changeOrder()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

void QDesignerStackedWidget::removeCurrentPage()
{
    if (currentIndex() == -1)
        return;

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        DeleteStackedWidgetPageCommand *cmd = new DeleteStackedWidgetPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::changeOrder()
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
            MoveStackedWidgetCommand *cmd = new MoveStackedWidgetCommand(fw);
            cmd->init(this, wList.at(i), i);
            fw->commandHistory()->push(cmd);
        }
        fw->endCommand();
    }
}

void QDesignerStackedWidget::addPage()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        AddStackedWidgetPageCommand *cmd = new AddStackedWidgetPageCommand(fw);
        cmd->init(this, AddStackedWidgetPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::addPageAfter()
{
    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        AddStackedWidgetPageCommand *cmd = new AddStackedWidgetPageCommand(fw);
        cmd->init(this, AddStackedWidgetPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerStackedWidget::updateButtons()
{
    if (prev) {
        prev->setGeometry(width() - 31, 1, 15, 15);
        prev->show();
        prev->raise();
    }

    if (next) {
        next->setGeometry(width() - 16, 1, 15, 15);
        next->show();
        next->raise();
    }
}

void QDesignerStackedWidget::prevPage()
{
    if (count() == 0) {
        // nothing to do
        return;
    }

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        int newIndex = currentIndex() - 1;
        if (newIndex < 0)
            newIndex = count() - 1;

        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, QLatin1String("currentIndex"), newIndex);
        fw->commandHistory()->push(cmd);
        updateButtons();
        fw->emitSelectionChanged();
    } else {
        setCurrentIndex(qMax(0, currentIndex() - 1));
        updateButtons();
    }
}

void QDesignerStackedWidget::nextPage()
{
    if (count() == 0) {
        // nothing to do
        return;
    }

    if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
        SetPropertyCommand *cmd = new SetPropertyCommand(fw);
        cmd->init(this, QLatin1String("currentIndex"), (currentIndex() + 1) % count());
        fw->commandHistory()->push(cmd);
        updateButtons();
        fw->emitSelectionChanged();
    } else {
        setCurrentIndex((currentIndex() + 1) % count());
        updateButtons();
    }
}

bool QDesignerStackedWidget::event(QEvent *e)
{
    if (e->type() == QEvent::LayoutRequest) {
        if (m_actionDeletePage)
            m_actionDeletePage->setEnabled(count() > 1);
        updateButtons();
    }

    return QStackedWidget::event(e);
}

void QDesignerStackedWidget::childEvent(QChildEvent *e)
{
    QStackedWidget::childEvent(e);
    updateButtons();
}

void QDesignerStackedWidget::resizeEvent(QResizeEvent *e)
{
    QStackedWidget::resizeEvent(e);
    updateButtons();
}

void QDesignerStackedWidget::showEvent(QShowEvent *e)
{
    QStackedWidget::showEvent(e);
    updateButtons();
}

QString QDesignerStackedWidget::currentPageName() const
{
    if (currentIndex() == -1)
        return QString();

    return widget(currentIndex())->objectName();
}

void QDesignerStackedWidget::setCurrentPageName(const QString &pageName)
{
    if (currentIndex() == -1)
        return;

    if (QWidget *w = widget(currentIndex())) {
        w->setObjectName(pageName);
    }
}

void QDesignerStackedWidget::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = QDesignerFormWindowInterface::findFormWindow(this)) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}

