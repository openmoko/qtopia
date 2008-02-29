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

#include "qaccessiblemenu.h"

#include <qmenu.h>
#include <qmenubar.h>
#include <qstyle.h>

#ifndef QT_NO_ACCESSIBILITY
#ifndef QT_NO_MENU

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);
QString Q_GUI_EXPORT qt_accHotKey(const QString &text);

QAccessibleMenu::QAccessibleMenu(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(menu());
}

QMenu *QAccessibleMenu::menu() const
{
    return qobject_cast<QMenu*>(object());
}

int QAccessibleMenu::childCount() const
{
    return menu()->actions().count();
}

QRect QAccessibleMenu::rect(int child) const
{
    if (!child || child > childCount())
        return QAccessibleWidget::rect(child);

    QRect r = menu()->actionGeometry(menu()->actions()[child - 1]);
    QPoint tlp = menu()->mapToGlobal(QPoint(0,0));

    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessibleMenu::childAt(int x, int y) const
{
    QAction *act = menu()->actionAt(menu()->mapFromGlobal(QPoint(x,y)));
    if(act && act->isSeparator())
        act = 0;
    return menu()->actions().indexOf(act) + 1;
}

/*int QAccessibleMenu::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    switch (direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return childCount();
    case NavNext:
    case NavDown:
        return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavUp:
        return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
        break;
    }
    return -1;
}*/

QString QAccessibleMenu::text(Text t, int child) const
{
    QString tx = QAccessibleWidget::text(t, child);
    if (tx.size())
        return tx;

    switch (t) {
    case Name:
        if (!child)
            return menu()->windowTitle();
        return qt_accStripAmp(menu()->actions().at(child-1)->text());
    case Help:
        return child ? menu()->actions().at(child-1)->whatsThis() : tx;
#ifndef QT_NO_SHORTCUT
    case Accelerator:
        return child ? static_cast<QString>(menu()->actions().at(child-1)->shortcut()) : tx;
#endif
    default:
        break;
    }
    return tx;
}

QAccessible::Role QAccessibleMenu::role(int child) const
{
    if (!child)
        return PopupMenu;

    QAction *action = menu()->actions()[child-1];
    if (action && action->isSeparator())
        return Separator;
    return MenuItem;
}

QAccessible::State QAccessibleMenu::state(int child) const
{
    State s = QAccessibleWidget::state(child);
    if (!child)
        return s;

    QAction *action = menu()->actions()[child-1];
    if (!action)
        return s;

    if (menu()->style()->styleHint(QStyle::SH_Menu_MouseTracking))
        s |= HotTracked;
    if (action->isSeparator() || !action->isEnabled())
        s |= Unavailable;
    if (action->isChecked())
        s |= Checked;
    if (menu()->activeAction() == action)
        s |= Focused;

    return s;
}

QString QAccessibleMenu::actionText(int action, QAccessible::Text text, int child) const
{
    if (action == QAccessible::DefaultAction && child && text == QAccessible::Name) {
        QAction *a = menu()->actions().value(child-1, 0);
        if (!a || a->isSeparator())
            return QString();
        if (a->menu()) {
            if (a->menu()->isVisible())
                return QMenu::tr("Close");
            return QMenu::tr("Open");
        }
        return QMenu::tr("Execute");
     }

    return QAccessibleWidget::actionText(action, text, child);
}

bool QAccessibleMenu::doAction(int act, int child, const QVariantList &)
{
    if (!child || act != QAccessible::DefaultAction)
        return false;

    QAction *action = menu()->actions().value(child-1, 0);
    if (!action || !action->isEnabled())
        return false;

    if (action->menu() && action->menu()->isVisible())
        action->menu()->hide();
    else
        menu()->setActiveAction(action);
    return true;
}

/*bool QAccessibleMenu::setFocus(int child)
{
    if (!child)
        return false;

    int id = menu()->idAt(child -1);
    QMenuItem *item = menu()->findItem(id);
    if (!item || !item->isEnabled())
        return false;

    menu()->setActiveItem(child - 1);
    return true;
}
*/
#endif // QT_NO_MENU

#ifndef QT_NO_MENUBAR
QAccessibleMenuBar::QAccessibleMenuBar(QWidget *w)
: QAccessibleWidget(w)
{
    Q_ASSERT(menuBar());
}

QMenuBar *QAccessibleMenuBar::menuBar() const
{
    return qobject_cast<QMenuBar*>(object());
}

int QAccessibleMenuBar::childCount() const
{
    return menuBar()->actions().count();
}

QRect QAccessibleMenuBar::rect(int child) const
{
    if (!child)
        return QAccessibleWidget::rect(child);

    QRect r = menuBar()->actionGeometry(menuBar()->actions()[child - 1]);
    QPoint tlp = menuBar()->mapToGlobal(QPoint(0,0));
    return QRect(tlp.x() + r.x(), tlp.y() + r.y(), r.width(), r.height());
}

int QAccessibleMenuBar::childAt(int x, int y) const
{
    QPoint p = menuBar()->mapFromGlobal(QPoint(x,y));
    return menuBar()->actions().indexOf(menuBar()->actionAt(p)) + 1;
}

/*
int QAccessibleMenuBar::navigate(NavDirection direction, int startControl) const
{
    if (direction != NavFirstChild && direction != NavLastChild && direction != NavFocusChild && !startControl)
        return QAccessibleWidget::navigate(direction, startControl);

    switch (direction) {
    case NavFirstChild:
        return 1;
    case NavLastChild:
        return childCount();
    case NavNext:
    case NavRight:
        return (startControl + 1) > childCount() ? -1 : startControl + 1;
    case NavPrevious:
    case NavLeft:
        return (startControl - 1) < 1 ? -1 : startControl - 1;
    default:
        break;
    }

    return -1;
}
*/

QString QAccessibleMenuBar::text(Text t, int child) const
{
    QString str;

    if(child) {
        if (QAction *action = menuBar()->actions().value(child - 1, 0)) {
            switch (t) {
            case Name:
                return qt_accStripAmp(action->text());
            case Accelerator:
                str = qt_accHotKey(action->text());
                break;
            default:
                break;
            }
        }
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);
    return str;
}

QAccessible::Role QAccessibleMenuBar::role(int child) const
{
    if (!child)
        return MenuBar;

    QAction *action = menuBar()->actions()[child-1];
    if (action && action->isSeparator())
        return Separator;
    return MenuItem;
}

QAccessible::State QAccessibleMenuBar::state(int child) const
{
    State s = QAccessibleWidget::state(child);
    if (!child)
        return s;

    QAction *action = menuBar()->actions().value(child-1, 0);
    if (!action)
        return s;

    if (menuBar()->style()->styleHint(QStyle::SH_Menu_MouseTracking))
        s |= HotTracked;
    if (action->isSeparator() || !action->isEnabled())
        s |= Unavailable;
    if (menuBar()->activeAction() == action)
        s |= Focused;

    return s;
}

QString QAccessibleMenuBar::actionText(int action, QAccessible::Text text, int child) const
{
    if (action == QAccessible::DefaultAction && child && text == QAccessible::Name) {
        QAction *a = menuBar()->actions().value(child-1, 0);
        if (!a || a->isSeparator())
            return QString();
        if (a->menu()) {
            if (a->menu()->isVisible())
                return QMenu::tr("Close");
            return QMenu::tr("Open");
        }
        return QMenu::tr("Execute");
    }

    return QAccessibleWidget::actionText(action, text, child);
}

bool QAccessibleMenuBar::doAction(int act, int child, const QVariantList &)
{
    if (act != !child)
        return false;

    QAction *action = menuBar()->actions().value(child-1, 0);
    if (!action || !action->isEnabled())
        return false;
    if (action->menu() && action->menu()->isVisible())
        action->menu()->hide();
    else
        menuBar()->setActiveAction(action);
    return true;
}

/*
bool QAccessibleMenuBar::setFocus(int child)
{
    if (!child)
        return false;

    int id = menuBar()->idAt(child -1);
    QMenuItem *item = menuBar()->findItem(id);
    if (!item || !item->isEnabled())
        return false;

    return true;
}
*/
#endif // QT_NO_MENUBAR
#endif // QT_NO_ACCESSIBILITY
