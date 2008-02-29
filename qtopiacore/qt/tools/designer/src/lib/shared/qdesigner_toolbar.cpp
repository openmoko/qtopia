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
TRANSLATOR qdesigner_internal::Sentinel
*/

#include "qdesigner_toolbar_p.h"
#include "qdesigner_command_p.h"
#include "actionrepository_p.h"
#include "actionprovider_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QtGui/QMainWindow>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

using namespace qdesigner_internal;

QDesignerToolBar::QDesignerToolBar(QWidget *parent)
    : QToolBar(parent),
      m_interactive(true)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setAcceptDrops(true); // ### fake

    QWidget *w = new QWidget(this);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_sentinel = addWidget(w);
    addAction(m_sentinel);

    qApp->installEventFilter(this);
}

QDesignerToolBar::~QDesignerToolBar()
{
}

bool QDesignerToolBar::handleEvent(QWidget *widget, QEvent *event)
{
    if (!formWindow() || isPassiveWidget(widget))
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::MouseButtonPress:
            return handleMousePressEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleMouseReleaseEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return handleMouseMoveEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::ContextMenu:
            return handleContextMenuEvent(widget, static_cast<QContextMenuEvent*>(event));
    }

    return true;
}

void QDesignerToolBar::startDrag(const QPoint &pos)
{
    int index = findAction(pos);
    if (index == actions().count() - 1)
        return;

    QAction *action = actions().at(index);
    removeAction(action);
    adjustSize();

    adjustIndicator(pos);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(action->icon().pixmap(QSize(22, 22)));

    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    data->items.append(action);
    drag->setMimeData(data);

    if (drag->start() == Qt::IgnoreAction) {
        QAction *previous = actions().at(index);

        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, previous);
        formWindow()->commandHistory()->push(cmd);
    }
}

bool QDesignerToolBar::handleMousePressEvent(QWidget *, QMouseEvent *event)
{
    event->accept();
    m_startPosition = QPoint();

    if (event->button() != Qt::LeftButton)
        return true;

    if (QDesignerFormWindowInterface *fw = formWindow()) {
        if (QDesignerPropertyEditorInterface *pe = fw->core()->propertyEditor()) {
            pe->setObject(this);
        }
    }

    m_startPosition = mapFromGlobal(event->globalPos());

    return true;
}

bool QDesignerToolBar::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    m_startPosition = QPoint();

    return true;
}

bool QDesignerToolBar::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
    event->accept();

    if (m_startPosition.isNull())
        return true;

    QPoint pos = mapFromGlobal(event->globalPos());

    if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance())
        return true;

    startDrag(m_startPosition);
    m_startPosition = QPoint();

    return true;
}

bool QDesignerToolBar::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
{
    event->accept();

    int index = findAction(mapFromGlobal(event->globalPos()));

    QAction *action = actions().at(index);
    QVariant itemData;
    qVariantSetValue(itemData, action);

    QMenu menu(0);

    QAction *newSeperatorAct = menu.addAction(tr("Insert Separator"));
    newSeperatorAct->setData(itemData);
    connect(newSeperatorAct, SIGNAL(triggered()), this, SLOT(slotInsertSeparator()));
    menu.addSeparator();

    if (action && action != m_sentinel) {
        QAction *a = menu.addAction(tr("Remove action '%1'").arg(action->objectName()));
        a->setData(itemData);

        connect(a, SIGNAL(triggered()), this, SLOT(slotRemoveSelectedAction()));

    }

    QAction *remove_toolbar = menu.addAction(tr("Remove Toolbar '%1'").arg(objectName()));
    connect(remove_toolbar, SIGNAL(triggered()), this, SLOT(slotRemoveToolBar()));

    menu.exec(event->globalPos());

    return true;
}

void QDesignerToolBar::slotRemoveToolBar()
{
    Q_ASSERT(formWindow() != 0);

    QDesignerFormWindowInterface *fw = formWindow();

    DeleteToolBarCommand *cmd = new DeleteToolBarCommand(fw);
    cmd->init(this);
    fw->commandHistory()->push(cmd);
}

void QDesignerToolBar::slotRemoveSelectedAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    QAction *a = qvariant_cast<QAction*>(action->data());
    Q_ASSERT(a != 0);

    int pos = actions().indexOf(a);
    QAction *action_before = 0;
    if (pos != -1 && actions().count() > pos + 1)
        action_before = actions().at(pos + 1);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, a, action_before);
    formWindow()->commandHistory()->push(cmd);
}

bool QDesignerToolBar::eventFilter(QObject *object, QEvent *event)
{
    if (object == qApp->activePopupWidget())
        return false;

    switch (event->type()) {
        default: break;

        case QEvent::ContextMenu:
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            QWidget *widget = qobject_cast<QWidget*>(object);

            if (widget && (widget == this || isAncestorOf(widget)))
                return handleEvent(widget, event);
        } break;
    }

    return false;
};

int QDesignerToolBar::findAction(const QPoint &pos) const
{
    QList<QAction*> lst = actions();
    int index = 0;
    for (; index<lst.size() - 1; ++index) {
        QRect g = actionGeometry(lst.at(index));
        g.setTopLeft(QPoint(0, 0));

        if (g.contains(pos))
            break;
    }

    return index;
}

QDesignerActionProviderExtension *QDesignerToolBar::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}

void QDesignerToolBar::adjustIndicator(const QPoint &pos)
{
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(pos);
    }
}

void QDesignerToolBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !action->menu() && !actions().contains(action) &&
            Utils::isObjectAncestorOf(formWindow()->mainContainer(), action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    }
}

void QDesignerToolBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        Q_ASSERT(!d->items.isEmpty());

        QAction *action = d->items.first();
        if (action && !action->menu() && !actions().contains(action)) {
            event->acceptProposedAction();
            adjustIndicator(event->pos());
        }
    }
}

void QDesignerToolBar::dragLeaveEvent(QDragLeaveEvent *)
{
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(QPoint(-1,-1));
    }
}

void QDesignerToolBar::dropEvent(QDropEvent *event)
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {
        event->acceptProposedAction();

        QAction *action = d->items.first();
        if (action && !actions().contains(action)) {
            int index = findAction(event->pos());
            index = qMin(index, actions().count() - 1);

            InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
            cmd->init(this, action, actions().at(index));
            formWindow()->commandHistory()->push(cmd);
        }
    }

    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(QPoint(-1,-1));
    }
}

void QDesignerToolBar::actionEvent(QActionEvent *event)
{
    QToolBar::actionEvent(event);
}

QDesignerFormWindowInterface *QDesignerToolBar::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerToolBar*>(this));
}

bool QDesignerToolBar::isPassiveWidget(QWidget *widget) const
{
    if (qobject_cast<Sentinel*>(widget))
        return true;
    else if (!qstrcmp(widget->metaObject()->className(), "QToolBarHandle"))
        return true;

    return false;
}

void QDesignerToolBar::slotNewToolBar()
{
    QDesignerFormWindowInterface *fw = formWindow();
    if (fw && qobject_cast<QMainWindow*>(fw->mainContainer())) {
        QMainWindow *mw = qobject_cast<QMainWindow*>(fw->mainContainer());

        AddToolBarCommand *cmd = new AddToolBarCommand(fw);
        cmd->init(mw);
        fw->commandHistory()->push(cmd);
    }
}

void QDesignerToolBar::adjustSpecialActions()
{
    removeAction(m_sentinel);
    addAction(m_sentinel);
}

bool QDesignerToolBar::interactive(bool i)
{
    bool old = m_interactive;
    m_interactive = i;
    return old;
}

// ### Share me with QDesignerMenu (a.k.a. I'm a copy of it)
QAction *QDesignerToolBar::createAction(const QString &objectName, bool separator)
{
    Q_ASSERT(formWindow() != 0);
    QDesignerFormWindowInterface *fw = formWindow();

    QAction *action = new QAction(fw);
    fw->core()->widgetFactory()->initialize(action);
    if (separator)
        action->setSeparator(true);

    action->setObjectName(objectName);
    fw->ensureUniqueObjectName(action);

    AddActionCommand *cmd = new AddActionCommand(fw);
    cmd->init(action);
    fw->commandHistory()->push(cmd);

    return action;
}

void QDesignerToolBar::slotInsertSeparator()
{
    QAction *theSender = qobject_cast<QAction*>(sender());
    QAction *previous = qvariant_cast<QAction *>(theSender->data());
    formWindow()->beginCommand(tr("Insert Separator"));
    QAction *action = createAction("separator", true);
    InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
    cmd->init(this, action, previous);
    formWindow()->commandHistory()->push(cmd);
    formWindow()->endCommand();
}

////////////////////////////////////////////////////////////////////////////////
SentinelAction::SentinelAction(QWidget *widget)
    : QAction(widget)
{
    setSeparator(true);
}

SentinelAction::~SentinelAction()
{
}

Sentinel::Sentinel(QWidget *widget)
    : QToolButton(widget)
{
    setObjectName(QString::fromUtf8("__qt__passive_new"));
    setText(">>");
    setToolButtonStyle(Qt::ToolButtonTextOnly);
    setToolTip(tr("New Tool Bar"));
}

Sentinel::~Sentinel()
{
}

