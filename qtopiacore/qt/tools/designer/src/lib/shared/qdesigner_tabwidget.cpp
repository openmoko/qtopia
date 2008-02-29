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

#include "qdesigner_tabwidget_p.h"
#include "qdesigner_command_p.h"

#include <QtDesigner/QDesignerFormWindowInterface>

#include <QtGui/QApplication>
#include <QtGui/QTabBar>
#include <QtGui/QAction>
#include <QtGui/QMouseEvent>

#include <QtCore/qdebug.h>

namespace qdesigner_internal {

class MyMimeData : public QMimeData
{
    Q_OBJECT
public:
    QDesignerTabWidget *tab;
};

} // namespace qdesigner_internal

using namespace qdesigner_internal;

QDesignerTabWidget::QDesignerTabWidget(QWidget *parent)
    : QTabWidget(parent), m_actionDeletePage(0)
{
    dropIndicator = 0;
    dragPage = 0;
    mousePressed = false;

    tabBar()->setAcceptDrops(true);
    tabBar()->installEventFilter(this);

    m_actionInsertPage = new QAction(this);
    m_actionInsertPage->setText(tr("Before Current Page"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(addPage()));

    m_actionInsertPageAfter = new QAction(this);
    m_actionInsertPageAfter->setText(tr("After Current Page"));
    connect(m_actionInsertPageAfter, SIGNAL(triggered()), this, SLOT(addPageAfter()));

    m_actionDeletePage = new QAction(this);
    m_actionDeletePage->setText(tr("Delete Page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(removeCurrentPage()));

    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)));
}

QDesignerTabWidget::~QDesignerTabWidget()
{
}

QString QDesignerTabWidget::currentTabName() const
{
    return currentWidget()
        ? currentWidget()->objectName()
        : QString();
}

void QDesignerTabWidget::setCurrentTabName(const QString &tabName)
{
    if (QWidget *w = currentWidget())
        w->setObjectName(tabName);
}

QString QDesignerTabWidget::currentTabText() const
{
    return tabText(currentIndex());
}

void QDesignerTabWidget::setCurrentTabText(const QString &tabText)
{
    setTabText(currentIndex(), tabText);
}

QString QDesignerTabWidget::currentTabToolTip() const
{
    return tabToolTip(currentIndex());
}

void QDesignerTabWidget::setCurrentTabToolTip(const QString &tabToolTip)
{
    setTabToolTip(currentIndex(), tabToolTip);
}

QIcon QDesignerTabWidget::currentTabIcon() const
{
    return tabIcon(currentIndex());
}

void QDesignerTabWidget::setCurrentTabIcon(const QIcon &tabIcon)
{
    setTabIcon(currentIndex(), tabIcon);
}

bool QDesignerTabWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o != tabBar())
        return false;

    if (formWindow() == 0)
        return false;

    switch (e->type()) {
    case QEvent::MouseButtonDblClick: break;

    case QEvent::MouseButtonPress: {
        QMouseEvent *mev = static_cast<QMouseEvent*>(e);
        if (mev->button() & Qt::LeftButton) {
            mousePressed = true;
            pressPoint = mev->pos();

            for (int i = 0; i < tabBar()->count(); ++i) {
                if (tabBar()->tabRect(i).contains(pressPoint)) {
                    if (i != tabBar()->currentIndex()) {
                        SetPropertyCommand *cmd = new SetPropertyCommand(formWindow());
                        cmd->init(this, "currentIndex", i);
                        formWindow()->commandHistory()->push(cmd);
                    }
                    break;
                }
            }
        }
    } break;

    case QEvent::MouseButtonRelease: {
        mousePressed = false;
    } break;

    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        if (mousePressed && canMove(mouseEvent)) {
            mousePressed = false;
            QDrag *drg = new QDrag(this);
            qdesigner_internal::MyMimeData *mimeData = new qdesigner_internal::MyMimeData();
            mimeData->tab = this;
            drg->setMimeData(mimeData);

            dragIndex = currentIndex();
            dragPage = currentWidget();
            dragLabel = currentTabText();
            dragIcon = currentTabIcon();

            removeTab(dragIndex);

            Qt::DropActions dropAction = drg->start(Qt::MoveAction);

            if (dropAction == 0) {
                // abort
                insertTab(dragIndex, dragPage, dragIcon, dragLabel);
                setCurrentIndex(dragIndex);
            }

            if (dropIndicator)
                dropIndicator->hide();
        }
    } break;

    case QEvent::DragLeave: {
        if (dropIndicator)
            dropIndicator->hide();
    } break;

    case QEvent::DragEnter:
    case QEvent::DragMove: {
        QDragMoveEvent *de = static_cast<QDragMoveEvent*>(e);

        bool accept = false;
        if (const QMimeData *mimeData = de->mimeData()) {
            const qdesigner_internal::MyMimeData *m = qobject_cast<const qdesigner_internal::MyMimeData *>(mimeData);
            if (m && m->tab == this)
                accept = true;
        }

        if (!accept)
            return false;

        de->accept();
        de->acceptProposedAction();

        int index = 0;
        QRect rect;
        for (; index < count(); index++) {
            QRect rc = tabBar()->tabRect(index);
            if (rc.contains(de->pos())) {
                rect = rc;
                break;
            }
        }

        if (index == count() -1) {
            QRect rect2 = rect;
            rect2.setLeft(rect2.left() + rect2.width() / 2);
            if (rect2.contains(de->pos()))
                index++;
        }

        if (!dropIndicator) {
            dropIndicator = new QWidget(this);
            QPalette p = dropIndicator->palette();
            p.setColor(backgroundRole(), Qt::red);
            dropIndicator->setPalette(p);
        }

        QPoint pos;
        if (index == count())
            pos = tabBar()->mapToParent(QPoint(rect.x() + rect.width(), rect.y()));
        else
            pos = tabBar()->mapToParent(QPoint(rect.x(), rect.y()));

        dropIndicator->setGeometry(pos.x(), pos.y() , 3, rect.height());
        dropIndicator->show();
    } break;

    case QEvent::Drop: {
        QDropEvent *de = static_cast<QDropEvent*>(e);

        bool accept = false;
        if (const QMimeData *mimeData = de->mimeData()) {
            const qdesigner_internal::MyMimeData *m = qobject_cast<const qdesigner_internal::MyMimeData *>(mimeData);
            if (m && m->tab == this)
                accept = true;
        }

        if (!accept)
            return false;
        de->acceptProposedAction();
        de->accept();

        int newIndex = 0;
        for (; newIndex < count(); newIndex++) {
            QRect rc = tabBar()->tabRect(newIndex);
            if (rc.contains(de->pos()))
                break;
        }

        if (newIndex == count() -1) {
            QRect rect2 = tabBar()->tabRect(newIndex);
            rect2.setLeft(rect2.left() + rect2.width() / 2);
            if (rect2.contains(de->pos()))
                newIndex++;
        }

        if (QDesignerFormWindowInterface *fw = formWindow()) {
            MoveTabPageCommand *cmd = new MoveTabPageCommand(fw);
            insertTab(dragIndex, dragPage, dragIcon, dragLabel);
            cmd->init(this, dragPage, dragIcon, dragLabel, dragIndex, newIndex);
            fw->commandHistory()->push(cmd);
        }
    } break;

    default:
        break;
    }

    return false;
}

void QDesignerTabWidget::removeCurrentPage()
{
    if (!currentWidget())
        return;

    if (QDesignerFormWindowInterface *fw = formWindow()) {
        DeleteTabPageCommand *cmd = new DeleteTabPageCommand(fw);
        cmd->init(this);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTabWidget::addPage()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        AddTabPageCommand *cmd = new AddTabPageCommand(fw);
        cmd->init(this, AddTabPageCommand::InsertBefore);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerTabWidget::addPageAfter()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        AddTabPageCommand *cmd = new AddTabPageCommand(fw);
        cmd->init(this, AddTabPageCommand::InsertAfter);
        fw->commandHistory()->push(cmd);
    }
}

bool QDesignerTabWidget::canMove(QMouseEvent *e) const
{
    QPoint pt = pressPoint - e->pos();
    return pt.manhattanLength() > QApplication::startDragDistance();
}

void QDesignerTabWidget::slotCurrentChanged(int index)
{
    if (widget(index)) {
        if (QDesignerFormWindowInterface *fw = formWindow()) {
            fw->clearSelection();
            fw->selectWidget(this, true);
        }
    }
}

QDesignerFormWindowInterface *QDesignerTabWidget::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerTabWidget*>(this));
}

void QDesignerTabWidget::tabInserted(int index)
{
    QTabWidget::tabInserted(index);

    if (m_actionDeletePage)
        m_actionDeletePage->setEnabled(count() > 1);
}

void QDesignerTabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);

    if (m_actionDeletePage)
        m_actionDeletePage->setEnabled(count() > 1);
}


#include "qdesigner_tabwidget.moc"
