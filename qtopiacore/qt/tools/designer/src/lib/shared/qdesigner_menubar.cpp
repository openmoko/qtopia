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

#include "qdesigner_menubar_p.h"
#include "qdesigner_menu_p.h"
#include "qdesigner_toolbar_p.h"
#include "qdesigner_command_p.h"
#include "actionrepository_p.h"
#include "actionprovider_p.h"
#include "actioneditor_p.h"
#include "qdesigner_utils_p.h"

#include <QtDesigner/QtDesigner>

#include <QtCore/QTimer>
#include <QtCore/qdebug.h>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QToolButton>
#include <QtGui/QDrag>
#include <QtGui/QLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

using namespace qdesigner_internal;

namespace qdesigner_internal
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////
SpecialMenuAction::SpecialMenuAction(QObject *parent)
    : QAction(parent)
{
}

SpecialMenuAction::~SpecialMenuAction()
{
}


} // namespace qdesigner_internal


/////////////////////////////////////////////////////////////////////////////////////////////////////////
QDesignerMenuBar::QDesignerMenuBar(QWidget *parent)
    : QMenuBar(parent),
      m_interactive(true)
{
    m_currentIndex = 0;
    m_lastMenuActionIndex = -1;
    m_dragging = false;

    setContextMenuPolicy(Qt::DefaultContextMenu);

    setAcceptDrops(true); // ### fake

    m_addMenu = new SpecialMenuAction(this);
    m_addMenu->setText(tr("Type Here"));
    addAction(m_addMenu);

    QFont italic;
    italic.setItalic(true);
    m_addMenu->setFont(italic);

    m_editor = new QLineEdit(this);
    m_editor->setObjectName("__qt__passive_editor");
    m_editor->hide();
    m_editor->installEventFilter(this);

    qApp->installEventFilter(this);
}

QDesignerMenuBar::~QDesignerMenuBar()
{
}

void QDesignerMenuBar::paintEvent(QPaintEvent *event)
{
    QMenuBar::paintEvent(event);

    QPainter p(this);

    foreach (QAction *a, actions()) {
        if (qobject_cast<SpecialMenuAction*>(a)) {
            QRect g = actionGeometry(a);
            QLinearGradient lg(g.left(), g.top(), g.left(), g.bottom());
            lg.setColorAt(0.0, Qt::transparent);
            lg.setColorAt(0.7, QColor(0, 0, 0, 32));
            lg.setColorAt(1.0, Qt::transparent);

            p.fillRect(g, lg);
        }
    }

    QAction *action = currentAction();

    if (m_dragging || !action)
        return;

    if (hasFocus()) {
        QRect g = actionGeometry(action);
        QDesignerMenu::drawSelection(&p, g.adjusted(1, 1, -1, -1));
    } else if (action->menu() && action->menu()->isVisible()) {
        QRect g = actionGeometry(action);
        p.drawRect(g.adjusted(1, 1, -1, -1));
    }
}

bool QDesignerMenuBar::handleEvent(QWidget *widget, QEvent *event)
{
    if (!formWindow())
        return false;

    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut)
        update();

    switch (event->type()) {
        default: break;

        case QEvent::MouseButtonDblClick:
            return handleMouseDoubleClickEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonPress:
            return handleMousePressEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleMouseReleaseEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return handleMouseMoveEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::ContextMenu:
            return handleContextMenuEvent(widget, static_cast<QContextMenuEvent*>(event));
        case QEvent::KeyPress:
            return handleKeyPressEvent(widget, static_cast<QKeyEvent*>(event));
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            return widget != m_editor;
    }

    return true;
}

bool QDesignerMenuBar::handleMouseDoubleClickEvent(QWidget *, QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return true;

    if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
        return true;

    event->accept();

    m_startPosition = QPoint();

    m_currentIndex = actionAtPosition(event->pos());
    if (m_currentIndex != -1) {
        showLineEdit();
    }

    return true;
}

bool QDesignerMenuBar::handleKeyPressEvent(QWidget *, QKeyEvent *e)
{
    if (m_editor->isHidden()) { // In navigation mode
        switch (e->key()) {

        case Qt::Key_Delete:
            if (m_currentIndex == -1 || m_currentIndex >= realActionCount())
                break;
            hideMenu();
            deleteMenu();
            break;

        case Qt::Key_Left:
            e->accept();
            if (QApplication::layoutDirection() == Qt::LeftToRight)
                moveLeft(e->modifiers() & Qt::ControlModifier);
            else
                moveRight(e->modifiers() & Qt::ControlModifier);
            return true;

        case Qt::Key_Right:
            e->accept();
            if (QApplication::layoutDirection() == Qt::LeftToRight)
                moveRight(e->modifiers() & Qt::ControlModifier);
            else
                moveLeft(e->modifiers() & Qt::ControlModifier);
            return true; // no update

        case Qt::Key_Up:
            e->accept();
            moveUp();
            return true;

        case Qt::Key_Down:
            e->accept();
            moveDown();
            return true;

        case Qt::Key_PageUp:
            m_currentIndex = 0;
            break;

        case Qt::Key_PageDown:
            m_currentIndex = actions().count() - 1;
            break;

        case Qt::Key_Enter:
        case Qt::Key_Return:
            e->accept();
            enterEditMode();
            return true; // no update

        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Escape:
            e->ignore();
            setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
            return true; // no update

        default:
            if (!e->text().isEmpty() && e->text().at(0).toLatin1() >= 32) {
                showLineEdit();
                QApplication::sendEvent(m_editor, e);
                e->accept();
            } else {
                e->ignore();
            }
            return true;
        }
    } else { // In edit mode
        switch (e->key()) {
        default:
            return false;

        case Qt::Key_Control:
            e->ignore();
            return true;

        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (!m_editor->text().isEmpty()) {
                leaveEditMode(ForceAccept);
                if (m_lastFocusWidget)
                    m_lastFocusWidget->setFocus();

                m_editor->hide();
                showMenu();
                break;
            }
            // fall through

        case Qt::Key_Escape:
            update();
            setFocus();
            break;
        }
    }

    e->accept();
    update();

    return true;
}

void QDesignerMenuBar::startDrag(const QPoint &pos)
{
    int index = findAction(pos);
    if (m_currentIndex == -1 || index >= realActionCount())
        return;

    QAction *action = safeActionAt(index);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, action, actions().at(index + 1));
    formWindow()->commandHistory()->push(cmd);

    adjustSize();

    hideMenu(index);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(action->icon().pixmap(QSize(22, 22)));

    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    data->items.append(action);
    drag->setMimeData(data);

    int old_index = m_currentIndex;
    m_currentIndex = -1;

    if (drag->start() == Qt::IgnoreAction) {
        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, safeActionAt(index));
        formWindow()->commandHistory()->push(cmd);

        m_currentIndex = old_index;
        adjustSize();
    }
}

bool QDesignerMenuBar::handleMousePressEvent(QWidget *, QMouseEvent *event)
{
    m_startPosition = QPoint();
    event->accept();

    if (event->button() != Qt::LeftButton)
        return true;

    m_startPosition = event->pos();
    m_currentIndex = actionAtPosition(m_startPosition);
    update();

    return true;
}

bool QDesignerMenuBar::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
    m_startPosition = QPoint();

    if (event->button() != Qt::LeftButton)
        return true;

    event->accept();
    m_currentIndex = actionAtPosition(event->pos());
    if (!m_editor->isVisible() && m_currentIndex != -1 && m_currentIndex < realActionCount())
        showMenu();

    return true;
}

bool QDesignerMenuBar::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
        return true;

    if (m_startPosition.isNull())
        return true;

    QPoint pos = mapFromGlobal(event->globalPos());

    if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance())
        return true;

    int index = actionAtPosition(m_startPosition);
    if (index < actions().count()) {
        hideMenu(index);
        update();
    }

    startDrag(m_startPosition);
    m_startPosition = QPoint();

    return true;
}

bool QDesignerMenuBar::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
{
    event->accept();

    m_currentIndex = actionAtPosition(mapFromGlobal(event->globalPos()));
    QAction *action = safeActionAt(m_currentIndex);

    update();

    QMenu menu(this);

    if (action && !qobject_cast<SpecialMenuAction*>(action)) {
        QVariant itemData;
        qVariantSetValue(itemData, action);

        QAction *remove_action = menu.addAction(tr("Remove Menu '%1'").arg(action->menu()->objectName()));
        remove_action->setData(itemData);
        connect(remove_action, SIGNAL(triggered()), this, SLOT(slotRemoveSelectedAction()));

        menu.addSeparator();
    }

    QAction *remove_menubar = menu.addAction(tr("Remove Menu Bar"));
    connect(remove_menubar, SIGNAL(triggered()), this, SLOT(slotRemoveMenuBar()));

    menu.exec(event->globalPos());

    return true;
}

void QDesignerMenuBar::slotRemoveMenuBar()
{
    Q_ASSERT(formWindow() != 0);

    QDesignerFormWindowInterface *fw = formWindow();

    DeleteMenuBarCommand *cmd = new DeleteMenuBarCommand(fw);
    cmd->init(this);
    fw->commandHistory()->push(cmd);
}

void QDesignerMenuBar::slotRemoveSelectedAction()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    QAction *a = qvariant_cast<QAction*>(action->data());
    if (qobject_cast<SpecialMenuAction*>(a))
        return; // nothing to do

    int pos = actions().indexOf(a);
    QAction *action_before = 0;
    if (pos != -1)
        action_before = safeActionAt(pos + 1);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, a, action_before);
    formWindow()->commandHistory()->push(cmd);
}

void QDesignerMenuBar::focusOutEvent(QFocusEvent *event)
{
    QMenuBar::focusOutEvent(event);
}

QAction *QDesignerMenuBar::createAction(const QString &name)
{
    Q_ASSERT(formWindow() != 0);

    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = fw->core();
    QMenu *menu = qobject_cast<QMenu*>(core->widgetFactory()->createWidget("QMenu", this));
    core->widgetFactory()->initialize(menu);
    menu->setObjectName(name);
    menu->setTitle(tr("Menu"));
    fw->ensureUniqueObjectName(menu);

    QAction *menuAction = menu->menuAction();

    AddMenuActionCommand *cmd = new AddMenuActionCommand(formWindow());
    cmd->init(menuAction, this);
    formWindow()->commandHistory()->push(cmd);

    return menuAction;
}


void QDesignerMenuBar::enterEditMode()
{
    if (m_currentIndex >= 0 && m_currentIndex <= realActionCount()) {
        showLineEdit();
    }
}

void QDesignerMenuBar::leaveEditMode(LeaveEditMode mode)
{
    m_editor->releaseKeyboard();

    if (mode == Default)
        return;

    if (m_editor->text().isEmpty())
        return;

    QAction *action = 0;

    if (m_currentIndex >= 0 && m_currentIndex < realActionCount()) {
        action = safeActionAt(m_currentIndex);
        formWindow()->beginCommand(QLatin1String("Change Title"));
    } else {
        Q_ASSERT(formWindow() != 0);
        formWindow()->beginCommand(QLatin1String("Insert Menu"));

        QString niceObjectName = ActionEditor::actionTextToName(m_editor->text());
        if (niceObjectName.startsWith("action"))
            niceObjectName.replace(0, 6, QLatin1String("menu"));

        action = createAction(niceObjectName);
        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, m_addMenu);
        formWindow()->commandHistory()->push(cmd);
    }

    SetPropertyCommand *cmd = new SetPropertyCommand(formWindow());
    cmd->init(action, QLatin1String("text"), m_editor->text());
    formWindow()->commandHistory()->push(cmd);

    formWindow()->endCommand();
    //moveDown();
}

void QDesignerMenuBar::showLineEdit()
{
    QAction *action = 0;

    if (m_currentIndex >= 0 && m_currentIndex < realActionCount())
        action = safeActionAt(m_currentIndex);
    else
        action = m_addMenu;

    if (action->isSeparator())
        return;

    // hideMenu();

    m_lastFocusWidget = qApp->focusWidget();

    // open edit field for item name
    QString text = action != m_addMenu ? action->text() : QString();

    m_editor->setText(text);
    m_editor->selectAll();
    m_editor->setGeometry(actionGeometry(action));
    m_editor->show();
    qApp->setActiveWindow(m_editor);
    m_editor->setFocus();
    m_editor->grabKeyboard();
}

bool QDesignerMenuBar::eventFilter(QObject *object, QEvent *event)
{
    if (object != this && object != m_editor)
        return false;

    if (!m_editor->isHidden() && object == m_editor && event->type() == QEvent::FocusOut) {
        leaveEditMode(Default);
        m_editor->hide();
        update();
        return true;
    }

    bool dispatch = true;

    switch (event->type()) {
        default: break;

        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::ContextMenu:
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
            dispatch = (object != m_editor);
            // no break

        case QEvent::Enter:
        case QEvent::Leave:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        {
            QWidget *widget = qobject_cast<QWidget*>(object);

            if (dispatch && widget && (widget == this || isAncestorOf(widget)))
                return handleEvent(widget, event);
        } break;

        case QEvent::Shortcut:
            event->accept();
            return true;
    }

    return false;
};


int QDesignerMenuBar::actionAtPosition(const QPoint &pos) const
{
    QList<QAction*> lst = actions();
    int index = 0;
    for (; index<lst.count(); ++index) {
        QRect g = actionGeometry(lst.at(index));
        if (QApplication::layoutDirection() == Qt::LeftToRight)
            g.setTopLeft(QPoint(0, 0));
        else
            g.setTopRight(QPoint(rect().width(), 0));

        if (g.contains(pos))
            return index;
    }

    return -1;
}


int QDesignerMenuBar::findAction(const QPoint &pos) const
{
    int index = actionAtPosition(pos);
    if (index == -1)
        return realActionCount();

    return index;
}

void QDesignerMenuBar::adjustIndicator(const QPoint &pos)
{
    int index = findAction(pos);
    QAction *action = safeActionAt(index);
    Q_ASSERT(action != 0);

    if (pos != QPoint(-1, -1)) {
        QDesignerMenu *m = qobject_cast<QDesignerMenu*>(action->menu());
        if (!m || m->parentMenu()) {
            m_currentIndex = index;
            showMenu(index);
        }
    }

    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(pos);
    }
}

QAction *QDesignerMenuBar::actionMimeData(const QMimeData *mimeData) const
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(mimeData)) {
        Q_ASSERT(!d->items.isEmpty());

        return d->items.first();
    }

    return 0;
}

bool QDesignerMenuBar::checkAction(QAction *action) const
{
    if (!action || !action->menu())
        return false; // no menu action!! nothing to do

    QDesignerMenu *m = qobject_cast<QDesignerMenu*>(action->menu());
    if (m && m->parentMenu())
        return false; // it looks like a submenu

    if (actions().contains(action))
        return false; // we already have the action in the menubar

    if (!Utils::isObjectAncestorOf(formWindow()->mainContainer(), action))
        return false; // action belongs to another form

    return true;
}

void QDesignerMenuBar::dragEnterEvent(QDragEnterEvent *event)
{
    QAction *action = actionMimeData(event->mimeData());
    if (!action || !Utils::isObjectAncestorOf(formWindow()->mainContainer(), action))
        return;

    m_dragging = true;
    event->acceptProposedAction();

    if (checkAction(action)) {
        adjustIndicator(event->pos());
    }
}

void QDesignerMenuBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (checkAction(actionMimeData(event->mimeData()))) {
        event->acceptProposedAction();
        adjustIndicator(event->pos());
    } else {
        event->ignore();
        int index = findAction(event->pos());
        showMenu(index);
    }
}

void QDesignerMenuBar::dragLeaveEvent(QDragLeaveEvent *)
{
    m_dragging = false;

    adjustIndicator(QPoint(-1, -1));
}

void QDesignerMenuBar::dropEvent(QDropEvent *event)
{
    m_dragging = false;

    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(event->mimeData())) {

        QAction *action = d->items.first();
        if (checkAction(action)) {
            event->acceptProposedAction();
            int index = findAction(event->pos());
            index = qMin(index, actions().count() - 1);

            InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
            cmd->init(this, action, safeActionAt(index));
            formWindow()->commandHistory()->push(cmd);

            m_currentIndex = index;
            update();
            adjustIndicator(QPoint(-1, -1));
            return;
        }
    }
    event->ignore();
}

void QDesignerMenuBar::actionEvent(QActionEvent *event)
{
    QMenuBar::actionEvent(event);
}

QDesignerFormWindowInterface *QDesignerMenuBar::formWindow() const
{
    return QDesignerFormWindowInterface::findFormWindow(const_cast<QDesignerMenuBar*>(this));
}

QDesignerActionProviderExtension *QDesignerMenuBar::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}

QAction *QDesignerMenuBar::currentAction() const
{
    if (m_currentIndex < 0 || m_currentIndex >= actions().count())
        return 0;

    return safeActionAt(m_currentIndex);
}

int QDesignerMenuBar::realActionCount() const
{
    return actions().count() - 1; // 1 fake actions
}

void QDesignerMenuBar::moveLeft(bool ctrl)
{
    if (ctrl)
        (void) swap(m_currentIndex, m_currentIndex - 1);

    m_currentIndex = qMax(0, --m_currentIndex);

    update();
}

bool QDesignerMenuBar::dragging() const
{
    return m_dragging;
}

void QDesignerMenuBar::moveRight(bool ctrl)
{
    if (ctrl)
        (void) swap(m_currentIndex + 1, m_currentIndex);

    m_currentIndex = qMin(actions().count() - 1, ++m_currentIndex);
    update();
}

void QDesignerMenuBar::moveUp()
{
    update();
}

void QDesignerMenuBar::moveDown()
{
    showMenu();
}

void QDesignerMenuBar::adjustSpecialActions()
{
    removeAction(m_addMenu);
    addAction(m_addMenu);
}

bool QDesignerMenuBar::interactive(bool i)
{
    bool old = m_interactive;
    m_interactive = i;
    return old;
}

void QDesignerMenuBar::hideMenu(int index)
{
    if (index < 0 && m_currentIndex >= 0)
        index = m_currentIndex;

    if (index < 0 || index >= realActionCount())
        return;

    QAction *action = safeActionAt(index);

    if (action && action->menu()) {
        action->menu()->hide();

        if (QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(action->menu())) {
            menu->closeMenuChain();
        }
    }
}

void QDesignerMenuBar::deleteMenu()
{
    QAction *action = currentAction();

    if (action && !qobject_cast<SpecialMenuAction*>(action)) {
        int pos = actions().indexOf(action);
        QAction *action_before = 0;
        if (pos != -1)
            action_before = safeActionAt(pos + 1);

        formWindow()->beginCommand("Remove menu");
        RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
        cmd->init(this, action, action_before);
        formWindow()->commandHistory()->push(cmd);

        RemoveMenuActionCommand *cmd2 = new RemoveMenuActionCommand(formWindow());
        cmd2->init(action, this);
        formWindow()->commandHistory()->push(cmd2);
        formWindow()->endCommand();
    }
}

void QDesignerMenuBar::showMenu(int index)
{
    if (index < 0 && m_currentIndex >= 0)
        index = m_currentIndex;

    if (index < 0 || index >= realActionCount())
        return;

    m_currentIndex = index;
    QAction *action = currentAction();

    if (action && action->menu()) {
        if (m_lastMenuActionIndex != -1 && m_lastMenuActionIndex != index) {
            hideMenu(m_lastMenuActionIndex);
        }

        m_lastMenuActionIndex = index;
        QMenu *menu = action->menu();
        QRect g = actionGeometry(action);

        if (!menu->isVisible()) {
            if ((menu->windowFlags() & Qt::Popup) != Qt::Popup)
                menu->setWindowFlags(Qt::Popup);
            menu->adjustSize();
            menu->move(mapToGlobal(g.bottomLeft()));
            menu->setFocus(Qt::MouseFocusReason);
            menu->raise();
            menu->show();
        } else {
            menu->raise();
        }
    }
}

QAction *QDesignerMenuBar::safeActionAt(int index) const
{
    if (index < 0 || index >= actions().count())
        return 0;

    return actions().at(index);
}

bool QDesignerMenuBar::swap(int a, int b)
{
    int left = qMin(a, b);
    int right = qMax(a, b);

    QAction *action_a = safeActionAt(left);
    QAction *action_b = safeActionAt(right);

    if (action_a == action_b
            || !action_a
            || !action_b
            || qobject_cast<SpecialMenuAction*>(action_a)
            || qobject_cast<SpecialMenuAction*>(action_b))
        return false; // nothing to do

    right = qMin(right, realActionCount());
    if (right < 0)
        return false; // nothing to do

    formWindow()->beginCommand(QLatin1String("Move action"));

    QAction *action_b_before = safeActionAt(right + 1);

    RemoveActionFromCommand *cmd1 = new RemoveActionFromCommand(formWindow());
    cmd1->init(this, action_b, action_b_before, false);
    formWindow()->commandHistory()->push(cmd1);

    QAction *action_a_before = safeActionAt(left + 1);

    InsertActionIntoCommand *cmd2 = new InsertActionIntoCommand(formWindow());
    cmd2->init(this, action_b, action_a_before, false);
    formWindow()->commandHistory()->push(cmd2);

    RemoveActionFromCommand *cmd3 = new RemoveActionFromCommand(formWindow());
    cmd3->init(this, action_a, action_b, false);
    formWindow()->commandHistory()->push(cmd3);

    InsertActionIntoCommand *cmd4 = new InsertActionIntoCommand(formWindow());
    cmd4->init(this, action_a, action_b_before, true);
    formWindow()->commandHistory()->push(cmd4);

    formWindow()->endCommand();

    return true;
}

void QDesignerMenuBar::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

void QDesignerMenuBar::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}


