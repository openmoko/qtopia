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

#include "qdesigner_menu_p.h"
#include "qdesigner_menubar_p.h"
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
#include <QtGui/QLineEdit>
#include <QtGui/QPainter>
#include <QtGui/QRubberBand>
#include <QtGui/QToolTip>
#include <QtGui/qevent.h>

Q_DECLARE_METATYPE(QAction*)
Q_DECLARE_METATYPE(QListWidgetItem*)

using namespace qdesigner_internal;

QDesignerMenu::QDesignerMenu(QWidget *parent)
    : QMenu(parent)
{
    m_interactive = true;
    m_dragging = false;
    m_currentIndex = 0;
    m_lastSubMenuIndex = -1;

    setContextMenuPolicy(Qt::DefaultContextMenu);
    setAcceptDrops(true); // ### fake
    setSeparatorsCollapsible(false);

    m_adjustSizeTimer = new QTimer(this);
    connect(m_adjustSizeTimer, SIGNAL(timeout()), this, SLOT(slotAdjustSizeNow()));

    m_addItem = new SpecialMenuAction(this);
    m_addItem->setText(tr("Type Here"));
    addAction(m_addItem);

    m_addSeparator = new SpecialMenuAction(this);
    m_addSeparator->setText(tr("Add Separator"));
    addAction(m_addSeparator);

    m_showSubMenuTimer = new QTimer(this);
    connect(m_showSubMenuTimer, SIGNAL(timeout()), this, SLOT(slotShowSubMenuNow()));

    m_deactivateWindowTimer = new QTimer(this);
    connect(m_deactivateWindowTimer, SIGNAL(timeout()), this, SLOT(slotDeactivateNow()));

    m_editor = new QLineEdit(this);
    m_editor->setObjectName("__qt__passive_editor");
    m_editor->hide();

    m_editor->installEventFilter(this);
    installEventFilter(this);
}

QDesignerMenu::~QDesignerMenu()
{
}

void QDesignerMenu::slotAdjustSizeNow()
{
    // Not using a single-shot, since we want to compress the timers if many items are being
    // adjusted
    m_adjustSizeTimer->stop();
    adjustSize();
}

bool QDesignerMenu::handleEvent(QWidget *widget, QEvent *event)
{
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
        update();

        if (widget == m_editor)
            return false;
    }

    switch (event->type()) {
        default: break;

        case QEvent::MouseButtonPress:
            return handleMousePressEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonRelease:
            return handleMouseReleaseEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseButtonDblClick:
            return handleMouseDoubleClickEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::MouseMove:
            return handleMouseMoveEvent(widget, static_cast<QMouseEvent*>(event));
        case QEvent::ContextMenu:
            return handleContextMenuEvent(widget, static_cast<QContextMenuEvent*>(event));
        case QEvent::KeyPress:
            return handleKeyPressEvent(widget, static_cast<QKeyEvent*>(event));
    }

    return true;
}

void QDesignerMenu::startDrag(const QPoint &pos)
{
    int index = findAction(pos);
    if (index >= realActionCount())
        return;

    QAction *action = safeActionAt(index);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, action, actions().at(index + 1));
    formWindow()->commandHistory()->push(cmd);

    QDrag *drag = new QDrag(this);
    drag->setPixmap(action->icon().pixmap(QSize(22, 22)));

    ActionRepositoryMimeData *data = new ActionRepositoryMimeData();
    data->items.append(action);
    drag->setMimeData(data);

    int old_index = m_currentIndex;
    m_currentIndex = -1;

    if (drag->start() == Qt::IgnoreAction) {
        QAction *previous = safeActionAt(index);

        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, previous);
        formWindow()->commandHistory()->push(cmd);

        m_currentIndex = old_index;
    }
}

bool QDesignerMenu::handleKeyPressEvent(QWidget * /*widget*/, QKeyEvent *e)
{
    m_showSubMenuTimer->stop();

    if (m_editor->isHidden() && hasFocus()) { // In navigation mode
        switch (e->key()) {

        case Qt::Key_Delete:
            if (m_currentIndex == -1 || m_currentIndex >= realActionCount())
                break;
            hideSubMenu();
            deleteAction();
            break;

        case Qt::Key_Left:
            e->accept();
            moveLeft();
            return true;

        case Qt::Key_Right:
            e->accept();
            moveRight();
            return true; // no update

        case Qt::Key_Up:
            e->accept();
            moveUp(e->modifiers() & Qt::ControlModifier);
            return true;

        case Qt::Key_Down:
            e->accept();
            moveDown(e->modifiers() & Qt::ControlModifier);
            return true;

        case Qt::Key_PageUp:
            m_currentIndex = 0;
            break;

        case Qt::Key_PageDown:
            m_currentIndex = actions().count() - 1;
            break;

        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_F2:
            e->accept();
            enterEditMode();
            return true; // no update

        case Qt::Key_Escape:
            e->ignore();
            setFocus();
            hide();
            closeMenuChain();
            return true;

        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Control:
            e->ignore();
            setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
            return true; // no update

        default:
            if (!currentAction() || currentAction()->isSeparator() || currentAction() == m_addSeparator) {
                e->ignore();
                return true;
            } else if (!e->text().isEmpty() && e->text().at(0).toLatin1() >= 32) {
                showLineEdit();
                QApplication::sendEvent(m_editor, e);
                e->accept();
            } else {
                e->ignore();
            }
            return true;
        }
    } else if (m_editor->hasFocus()) { // In edit mode
        switch (e->key()) {
        default:
            e->ignore();
            return false;

        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (!m_editor->text().isEmpty()) {
                leaveEditMode(ForceAccept);
                m_editor->hide();
                setFocus();
                moveDown(false);
                break;
            }
            // fall through

        case Qt::Key_Escape:
            m_editor->hide();
            setFocus();
            break;
        }
    }

    e->accept();
    update();

    return true;
}

bool QDesignerMenu::handleMouseDoubleClickEvent(QWidget *, QMouseEvent *event)
{
    event->accept();
    m_startPosition = QPoint();

    if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
        return true;

    if (!rect().contains(event->pos())) {
        // special case for menubar
        QWidget *target = QApplication::widgetAt(event->globalPos());
        QMenuBar *mb = qobject_cast<QMenuBar*>(target);
        QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(target);
        if (mb != 0 || menu != 0) {
            QPoint pt = target->mapFromGlobal(event->globalPos());

            QAction *action = mb == 0 ? menu->actionAt(pt) : mb->actionAt(pt);
            if (action) {
                QMouseEvent e(event->type(), pt, event->globalPos(), event->button(), event->buttons(), event->modifiers());
                QApplication::sendEvent(target, &e);
            }
        }
        return true;
    }

    m_currentIndex = findAction(event->pos());
    QAction *action = safeActionAt(m_currentIndex);

    QRect pm_rect;
    if (action->menu() || hasSubMenuPixmap(action)) {
        pm_rect = subMenuPixmapRect(action);
        pm_rect.setLeft(pm_rect.left() - 20); // give the user a little more
                                              // space to click
    }

    if (!pm_rect.contains(event->pos()) && m_currentIndex != -1)
        enterEditMode();

    return true;
}

bool QDesignerMenu::handleMousePressEvent(QWidget * /*widget*/, QMouseEvent *event)
{
    if (!rect().contains(event->pos())) {
        if (QMenuBar *mb = qobject_cast<QMenuBar*>(QApplication::widgetAt(event->globalPos()))) {
            QPoint pt = mb->mapFromGlobal(event->globalPos());
            QAction *action = mb->actionAt(pt);

            if (action && action->menu() == findRootMenu()) {
                // propagate the mouse press event (but don't close the popup)
                QMouseEvent e(event->type(), pt, event->globalPos(), event->button(), event->buttons(), event->modifiers());
                QApplication::sendEvent(mb, &e);
                return true;
            }
        }

        // hide the popup Qt will replay the event
        slotDeactivateNow();
        return true;
    }

    m_showSubMenuTimer->stop();
    m_startPosition = QPoint();
    event->accept();

    if (event->button() != Qt::LeftButton)
        return true;

    m_startPosition = mapFromGlobal(event->globalPos());

    int index = findAction(m_startPosition);

    QAction *action = safeActionAt(index);
    QRect pm_rect = subMenuPixmapRect(action);
    pm_rect.setLeft(pm_rect.left() - 20); // give the user a little more space to click

    int old_index = m_currentIndex;
    m_currentIndex = index;

    if ((hasSubMenuPixmap(action) || action->menu() != 0)
        && pm_rect.contains(m_startPosition)) {
        if (m_currentIndex == m_lastSubMenuIndex) {
            hideSubMenu();
        } else
            slotShowSubMenuNow();
    } else {
        if (index == old_index) {
            if (m_currentIndex == m_lastSubMenuIndex) {
                hideSubMenu();
            } else
                slotShowSubMenuNow();
        } else {
            hideSubMenu();
        }
    }

    updateCurrentAction();

    return true;
}

bool QDesignerMenu::handleMouseReleaseEvent(QWidget *, QMouseEvent *event)
{
    event->accept();
    m_startPosition = QPoint();

    return true;
}

bool QDesignerMenu::handleMouseMoveEvent(QWidget *, QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) != Qt::LeftButton)
        return true;

    if (!rect().contains(event->pos())) {

        if (QMenuBar *mb = qobject_cast<QMenuBar*>(QApplication::widgetAt(event->globalPos()))) {
            QPoint pt = mb->mapFromGlobal(event->globalPos());
            QAction *action = mb->actionAt(pt);

            if (action && action->menu() == findRootMenu()) {
                // propagate the mouse press event (but don't close the popup)
                QMouseEvent e(event->type(), pt, event->globalPos(), event->button(), event->buttons(), event->modifiers());
                QApplication::sendEvent(mb, &e);
                return true;
            }
            // hide the popup Qt will replay the event
            slotDeactivateNow();
        }
        return true;
    }

    if (m_startPosition.isNull())
        return true;

    event->accept();

    QPoint pos = mapFromGlobal(event->globalPos());

    if ((pos - m_startPosition).manhattanLength() < qApp->startDragDistance())
        return true;

    startDrag(m_startPosition);
    m_startPosition = QPoint();

    return true;
}

bool QDesignerMenu::handleContextMenuEvent(QWidget *, QContextMenuEvent *event)
{
    event->accept();

    int index = findAction(mapFromGlobal(event->globalPos()));
    QAction *action = safeActionAt(index);
    if (qobject_cast<SpecialMenuAction*>(action))
        return true;

    QMenu menu(this);
    QAction *a = menu.addAction(tr("Remove action '%1'").arg(action->objectName()));
    QVariant itemData;
    qVariantSetValue(itemData, action);
    a->setData(itemData);

    connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(slotRemoveSelectedAction(QAction*)));
    menu.exec(event->globalPos());

    return true;
}

void QDesignerMenu::slotRemoveSelectedAction(QAction *action)
{
    QAction *a = qvariant_cast<QAction*>(action->data());
    Q_ASSERT(a != 0);

    int pos = actions().indexOf(a);
    QAction *action_before = 0;
    if (pos != -1)
        action_before = safeActionAt(pos + 1);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, a, action_before);
    formWindow()->commandHistory()->push(cmd);
}

QRect QDesignerMenu::subMenuPixmapRect(QAction *action) const
{
    static const QPixmap pm(":/trolltech/formeditor/images/submenu.png");
    QRect g = actionGeometry(action);
    int x = g.right() - pm.width() - 2;
    int y = g.top() + (g.height() - pm.height())/2 + 1;
    return QRect(x, y, pm.width(), pm.height());
}

bool QDesignerMenu::hasSubMenuPixmap(QAction *action) const
{
    return action != 0
            && qobject_cast<SpecialMenuAction*>(action) == 0
            && !action->isSeparator()
            && !action->menu()
            && canCreateSubMenu(action);
}

void QDesignerMenu::paintEvent(QPaintEvent *event)
{
    QMenu::paintEvent(event);

    QPainter p(this);

    QAction *current = currentAction();

    foreach (QAction *a, actions()) {
        QRect g = actionGeometry(a);

        if (qobject_cast<SpecialMenuAction*>(a)) {
            QLinearGradient lg(g.left(), g.top(), g.left(), g.bottom());
            lg.setColorAt(0.0, Qt::transparent);
            lg.setColorAt(0.7, QColor(0, 0, 0, 32));
            lg.setColorAt(1.0, Qt::transparent);

            p.fillRect(g, lg);
        } else if (hasSubMenuPixmap(a)) {
            static const QPixmap pm(":/trolltech/formeditor/images/submenu.png");
            p.drawPixmap(subMenuPixmapRect(a).topLeft(), pm);
        }
    }

    if (!hasFocus() || !current || m_dragging)
        return;

    if (QDesignerMenu *menu = parentMenu()) {
        if (menu->dragging())
            return;
    }

    if (QDesignerMenuBar *menubar = qobject_cast<QDesignerMenuBar*>(parentWidget())) {
        if (menubar->dragging())
            return;
    }

    QRect g = actionGeometry(current);
    drawSelection(&p, g.adjusted(1, 1, -3, -3));
}

bool QDesignerMenu::dragging() const
{
    return m_dragging;
}

QDesignerMenu *QDesignerMenu::findRootMenu() const
{
    if (parentMenu())
        return parentMenu()->findRootMenu();

    return const_cast<QDesignerMenu*>(this);
}

QDesignerMenu *QDesignerMenu::findActivatedMenu() const
{
    QList<QDesignerMenu*> candidates;
    candidates.append(const_cast<QDesignerMenu*>(this));
    candidates += qFindChildren<QDesignerMenu*>(this);

    foreach (QDesignerMenu *m, candidates) {
        if (m == qApp->activeWindow())
            return m;
    }

    return 0;
}

bool QDesignerMenu::eventFilter(QObject *object, QEvent *event)
{
    if (object != this && object != m_editor) {
        return false;
    }

    if (!m_editor->isHidden() && object == m_editor && event->type() == QEvent::FocusOut) {
        leaveEditMode(Default);
        m_editor->hide();
        update();
        return false;
    }

    bool dispatch = true;

    switch (event->type()) {
        default: break;

        case QEvent::WindowDeactivate:
            deactivateMenu();
            break;
        case QEvent::ContextMenu:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:

            while (QApplication::activePopupWidget() && !qobject_cast<QDesignerMenu*>(QApplication::activePopupWidget())) {
                QApplication::activePopupWidget()->close();
            }

        // fall through
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseMove:
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
    }

    return false;
};

int QDesignerMenu::actionAtPosition(const QPoint &pos) const
{
    for (int i = 0; i<actions().count(); ++i) {
        QRect g = actionGeometry(safeActionAt(i));
        g.setTopLeft(QPoint(0, 0));

        if (g.contains(pos))
            return i;
    }

    return -1;
}

int QDesignerMenu::findAction(const QPoint &pos) const
{
    int index = actionAtPosition(pos);
    if (index == -1)
        return realActionCount();

    return index;
}

void QDesignerMenu::adjustIndicator(const QPoint &pos)
{
    if (QDesignerActionProviderExtension *a = actionProvider()) {
        a->adjustIndicator(pos);
    }
}

QAction *QDesignerMenu::actionMimeData(const QMimeData *mimeData) const
{
    if (const ActionRepositoryMimeData *d = qobject_cast<const ActionRepositoryMimeData*>(mimeData)) {
        Q_ASSERT(!d->items.isEmpty());

        return d->items.first();
    }

    return 0;
}

bool QDesignerMenu::checkAction(QAction *action) const
{
    if (!action || (action->menu() && action->menu()->parentWidget() != const_cast<QDesignerMenu*>(this)))
        return false; // menu action!! nothing to do

    if (actions().contains(action))
        return false; // we already have the action in the menu

    if (!Utils::isObjectAncestorOf(formWindow()->mainContainer(), action))
        return false; // the action belongs to another form window

    return true;
}

void QDesignerMenu::dragEnterEvent(QDragEnterEvent *event)
{
    QAction *action = actionMimeData(event->mimeData());
    if (!action)
        return;

    m_dragging = true;
    event->acceptProposedAction();

    if (checkAction(action)) {
        adjustIndicator(event->pos());
    }
}

void QDesignerMenu::dragMoveEvent(QDragMoveEvent *event)
{
    if (actionGeometry(m_addSeparator).contains(event->pos())) {
        event->ignore();
        adjustIndicator(QPoint(-1, -1));
        return;
    }

    QAction *action = actionMimeData(event->mimeData());
    if (action == 0 || !checkAction(action)) {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
    adjustIndicator(event->pos());
    m_currentIndex = findAction(event->pos());

    if (m_lastSubMenuIndex != m_currentIndex)
        m_showSubMenuTimer->start(300);
}

void QDesignerMenu::dragLeaveEvent(QDragLeaveEvent *)
{
    m_dragging = false;
    adjustIndicator(QPoint(-1, -1));
    m_showSubMenuTimer->stop();
}

void QDesignerMenu::dropEvent(QDropEvent *event)
{
    m_showSubMenuTimer->stop();
    hideSubMenu();
    m_dragging = false;

    QAction *action = actionMimeData(event->mimeData());
    if (action && checkAction(action)) {
        event->acceptProposedAction();
        int index = findAction(event->pos());
        index = qMin(index, actions().count() - 1);

        formWindow()->beginCommand(tr("Insert action"));
        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, safeActionAt(index));
        formWindow()->commandHistory()->push(cmd);

        m_currentIndex = index;

        if (parentMenu()) {
            QAction *parent_action = parentMenu()->currentAction();
            if (parent_action->menu() == 0) {
                CreateSubmenuCommand *cmd = new CreateSubmenuCommand(formWindow());
                cmd->init(parentMenu(), parentMenu()->currentAction());
                formWindow()->commandHistory()->push(cmd);
            }
        }
        updateCurrentAction();
        formWindow()->endCommand();
    } else {
        event->ignore();
    }
    adjustIndicator(QPoint(-1, -1));
}

void QDesignerMenu::actionEvent(QActionEvent *event)
{
    QMenu::actionEvent(event);
    m_adjustSizeTimer->start(0);
}

QDesignerFormWindowInterface *QDesignerMenu::formWindow() const
{
    if (parentMenu())
        return parentMenu()->formWindow();

    return QDesignerFormWindowInterface::findFormWindow(parentWidget());
}

QDesignerActionProviderExtension *QDesignerMenu::actionProvider()
{
    if (QDesignerFormWindowInterface *fw = formWindow()) {
        QDesignerFormEditorInterface *core = fw->core();
        return qt_extension<QDesignerActionProviderExtension*>(core->extensionManager(), this);
    }

    return 0;
}

void QDesignerMenu::closeMenuChain()
{
    m_showSubMenuTimer->stop();

    QWidget *w = this;
    while (w && qobject_cast<QMenu*>(w))
        w = w->parentWidget();

    if (w) {
        foreach (QMenu *subMenu, qFindChildren<QMenu*>(w)) {
            subMenu->hide();
        }
    }

    m_lastSubMenuIndex = -1;
}

void QDesignerMenu::moveLeft()
{
    if (parentMenu()) {
        hide();
    } else {
        closeMenuChain();
        if (QDesignerMenuBar *mb = parentMenuBar()) {
            if (QApplication::layoutDirection() == Qt::LeftToRight)
                mb->moveLeft();
            else
                mb->moveRight();
        }
    }
    updateCurrentAction();
}

void QDesignerMenu::moveRight()
{
    QAction *action = currentAction();

    if (qobject_cast<SpecialMenuAction*>(action) || action->isSeparator()) {
        closeMenuChain();
        if (QDesignerMenuBar *mb = parentMenuBar()) {
            if (QApplication::layoutDirection() == Qt::LeftToRight)
                mb->moveRight();
            else
                mb->moveLeft();
        }
    } else {
        m_lastSubMenuIndex = -1; // force a refresh
        slotShowSubMenuNow();
    }
}

void QDesignerMenu::moveUp(bool ctrl)
{
    if (m_currentIndex == 0) {
        hide();
        return;
    }

    if (ctrl)
        (void) swap(m_currentIndex, m_currentIndex - 1);

    m_currentIndex = qMax(0, --m_currentIndex);
    updateCurrentAction();
}

void QDesignerMenu::moveDown(bool ctrl)
{
    if (m_currentIndex == actions().count() - 1) {
        return;
    }

    if (ctrl)
        (void) swap(m_currentIndex + 1, m_currentIndex);

    m_currentIndex = qMin(actions().count() - 1, ++m_currentIndex);
    updateCurrentAction();
}

QAction *QDesignerMenu::currentAction() const
{
    if (m_currentIndex < 0 || m_currentIndex >= actions().count())
        return 0;

    return safeActionAt(m_currentIndex);
}

int QDesignerMenu::realActionCount() const
{
    return actions().count() - 2; // 2 fake actions
}

void QDesignerMenu::updateCurrentAction()
{
    update();
}

void QDesignerMenu::createRealMenuAction(QAction *action)
{
    if (action->menu())
        return; // nothing to do

    QDesignerFormWindowInterface *fw = formWindow();
    QDesignerFormEditorInterface *core = formWindow()->core();

    QDesignerMenu *menu = findOrCreateSubMenu(action);
    m_subMenus.remove(action);

    action->setMenu(menu);
    menu->setTitle(action->text());

    Q_ASSERT(formWindow() != 0);

    core->widgetFactory()->initialize(menu);

    QString niceObjectName = ActionEditor::actionTextToName(menu->title());
    if (niceObjectName.startsWith("action"))
        niceObjectName.replace(0, 6, QLatin1String("menu"));
    menu->setObjectName(niceObjectName);

    core->metaDataBase()->add(menu);
    fw->ensureUniqueObjectName(menu);

    QAction *menuAction = menu->menuAction();
    core->metaDataBase()->add(menuAction);
}

void QDesignerMenu::removeRealMenu(QAction *action)
{
    QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(action->menu());
    if (menu == 0)
        return;
    action->setMenu(0);
    m_subMenus.insert(action, menu);
    QDesignerFormEditorInterface *core = formWindow()->core();
    core->metaDataBase()->remove(menu);
}

QDesignerMenu *QDesignerMenu::findOrCreateSubMenu(QAction *action)
{
    if (action->menu())
        return qobject_cast<QDesignerMenu*>(action->menu());

    QDesignerMenu *menu = m_subMenus.value(action);
    if (!menu) {
        menu = new QDesignerMenu(this);
        m_subMenus.insert(action, menu);
    }

    return menu;
}

bool QDesignerMenu::canCreateSubMenu(QAction *action) const // ### improve it's a bit too slow
{
    QWidget *topLevel = formWindow()->mainContainer();
    QList<QMenu*> menus = qFindChildren<QMenu*>(topLevel);
    QList<QToolBar*> toolBars = qFindChildren<QToolBar*>(topLevel);

    foreach (const QMenu *m, menus) {
        if (m != this && m->actions().contains(action)) {
            return false; // sorry
        }
    }

    foreach (QToolBar *tb, toolBars) {
        if (tb->actions().contains(action)) {
            return false; // sorry
        }
    }

    return true;
}

void QDesignerMenu::slotShowSubMenuNow()
{
    m_showSubMenuTimer->stop();

    if (m_lastSubMenuIndex == m_currentIndex)
        return;

    if (m_lastSubMenuIndex != -1)
        hideSubMenu();

    if (m_currentIndex >= realActionCount())
        return;

    QAction *action = currentAction();

    if (action->isSeparator() || !canCreateSubMenu(action))
        return;

    if (QMenu *menu = findOrCreateSubMenu(action)) {
        if (!menu->isVisible()) {
            if ((menu->windowFlags() & Qt::Popup) != Qt::Popup)
                menu->setWindowFlags(Qt::Popup);
            QRect g = actionGeometry(action);
            menu->move(mapToGlobal(g.topRight()));
            menu->show();
            menu->setFocus();
        } else {
            menu->raise();
        }
        menu->setFocus();

        m_lastSubMenuIndex = m_currentIndex;
    }
}

void QDesignerMenu::showSubMenu(QAction *action)
{
    m_showSubMenuTimer->stop();

    if (m_editor->isVisible() || !action || qobject_cast<SpecialMenuAction*>(action)
            || action->isSeparator() || !isVisible())
        return;

    m_showSubMenuTimer->start(300);
}

QDesignerMenu *QDesignerMenu::parentMenu() const
{
    return qobject_cast<QDesignerMenu*>(parentWidget());
}

QDesignerMenuBar *QDesignerMenu::parentMenuBar() const
{
    if (QDesignerMenuBar *mb = qobject_cast<QDesignerMenuBar*>(parentWidget())) {
        return mb;
    } else if (QDesignerMenu *m = parentMenu()) {
        return m->parentMenuBar();
    }

    return 0;
}

void QDesignerMenu::setVisible(bool visible)
{
    if (visible)
        m_currentIndex = 0;
    else
        m_lastSubMenuIndex = -1;

    QMenu::setVisible(visible);

}

void QDesignerMenu::adjustSpecialActions()
{
    removeAction(m_addItem);
    removeAction(m_addSeparator);
    addAction(m_addItem);
    addAction(m_addSeparator);
}

bool QDesignerMenu::interactive(bool i)
{
    bool old = m_interactive;
    m_interactive = i;
    return old;
}

void QDesignerMenu::enterEditMode()
{
    if (m_currentIndex >= 0 && m_currentIndex <= realActionCount()) {
        showLineEdit();
    } else {
        hideSubMenu();
        formWindow()->beginCommand(tr("Add separator"));
        QAction *sep = createAction(QString(), true);

        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, sep, safeActionAt(realActionCount()));
        formWindow()->commandHistory()->push(cmd);

        if (parentMenu()) {
            QAction *parent_action = parentMenu()->currentAction();
            if (parent_action->menu() == 0) {
                CreateSubmenuCommand *cmd = new CreateSubmenuCommand(formWindow());
                cmd->init(parentMenu(), parentMenu()->currentAction());
                formWindow()->commandHistory()->push(cmd);
            }
        }

        formWindow()->endCommand();

        m_currentIndex = actions().indexOf(m_addItem);
        update();
    }
}

void QDesignerMenu::leaveEditMode(LeaveEditMode mode)
{
    if (mode == Default)
        return;

    QAction *action = 0;

    if (m_currentIndex < realActionCount()) {
        action = safeActionAt(m_currentIndex);
        formWindow()->beginCommand(QLatin1String("Set action text"));
    } else {
        Q_ASSERT(formWindow() != 0);
        formWindow()->beginCommand(QLatin1String("Insert action"));
        action = createAction(ActionEditor::actionTextToName(m_editor->text()));
        InsertActionIntoCommand *cmd = new InsertActionIntoCommand(formWindow());
        cmd->init(this, action, currentAction());
        formWindow()->commandHistory()->push(cmd);
    }

    SetPropertyCommand *cmd = new SetPropertyCommand(formWindow());
    cmd->init(action, QLatin1String("text"), m_editor->text());
    formWindow()->commandHistory()->push(cmd);

    if (parentMenu()) {
        QAction *parent_action = parentMenu()->currentAction();
        if (parent_action->menu() == 0) {
            CreateSubmenuCommand *cmd = new CreateSubmenuCommand(formWindow());
            cmd->init(parentMenu(), parentMenu()->currentAction());
            formWindow()->commandHistory()->push(cmd);
        }
    }

    updateCurrentAction();
    formWindow()->endCommand();
}

QAction *QDesignerMenu::safeMenuAction(QDesignerMenu *menu) const
{
    QAction *action = menu->menuAction();

    if (!action)
        action = m_subMenus.key(menu);

    return action;
}

void QDesignerMenu::showLineEdit()
{
    m_showSubMenuTimer->stop();

    QAction *action = 0;

    if (m_currentIndex < realActionCount())
        action = safeActionAt(m_currentIndex);
    else
        action = m_addItem;

    if (action->isSeparator())
        return;

    hideSubMenu();

    // open edit field for item name
    setFocus();

    QString text = action != m_addItem ? action->text() : QString();
    m_editor->setText(text);
    m_editor->selectAll();
    m_editor->setGeometry(actionGeometry(action).adjusted(1, 1, -2, -2));
    m_editor->show();
    m_editor->setFocus();
}

// ### Share me with QDesignerToolBar (a.k.a. he's a copy of me)
QAction *QDesignerMenu::createAction(const QString &objectName, bool separator)
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

// ### share with QDesignerMenu::swap
bool QDesignerMenu::swap(int a, int b)
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

QAction *QDesignerMenu::safeActionAt(int index) const
{
    if (index < 0 || index >= actions().count())
        return 0;

    return actions().at(index);
}

void QDesignerMenu::hideSubMenu()
{
    m_lastSubMenuIndex = -1;
    foreach (QMenu *subMenu, qFindChildren<QMenu*>(this)) {
        subMenu->hide();
    }
}

void QDesignerMenu::deleteAction()
{
    QAction *action = currentAction();
    int pos = actions().indexOf(action);
    QAction *action_before = 0;
    if (pos != -1)
        action_before = safeActionAt(pos + 1);

    RemoveActionFromCommand *cmd = new RemoveActionFromCommand(formWindow());
    cmd->init(this, action, action_before);
    formWindow()->commandHistory()->push(cmd);

    update();
}

void QDesignerMenu::deactivateMenu()
{
    m_deactivateWindowTimer->start(10);
}

void QDesignerMenu::slotDeactivateNow()
{
    m_deactivateWindowTimer->stop();

    if (m_dragging)
        return;

    QDesignerMenu *root = findRootMenu();

    if (! root->findActivatedMenu()) {
        root->hide();
        root->hideSubMenu();
    }
}

void QDesignerMenu::drawSelection(QPainter *p, const QRect &r)
{
    p->save();

    QColor c = Qt::blue;
    p->setPen(QPen(c, 1));
    c.setAlpha(32);
    p->setBrush(c);
    p->drawRect(r);

    p->restore();
}

void QDesignerMenu::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

void QDesignerMenu::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}


