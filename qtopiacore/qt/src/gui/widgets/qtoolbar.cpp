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

#include "qtoolbar.h"

#ifndef QT_NO_TOOLBAR

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qsignalmapper.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>
#include <private/qwidgetaction_p.h>
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#endif

#include <private/qmainwindowlayout_p.h>

#include "qtoolbar_p.h"
#include "qtoolbarextension_p.h"
#include "qtoolbarhandle_p.h"
#include "qtoolbarseparator_p.h"

#include  "qdebug.h"
static int positionForArea(Qt::ToolBarArea area)
{
    switch (area) {
    case Qt::LeftToolBarArea: return 0;
    case Qt::RightToolBarArea: return 1;
    case Qt::TopToolBarArea: return 2;
    case Qt::BottomToolBarArea: return 3;
    default: return -1;
    }
}

static QStyleOptionToolBar getStyleOption(QToolBar *toolBar)
{
    QStyleOptionToolBar option;
    option.init(toolBar);
    if (toolBar->orientation() == Qt::Horizontal)
        option.state |= QStyle::State_Horizontal;
    option.lineWidth = toolBar->style()->pixelMetric(QStyle::PM_ToolBarFrameWidth);
    option.features = toolBar->isMovable() ? QStyleOptionToolBar::Movable : QStyleOptionToolBar::None;

    // Add more styleoptions if the toolbar has been added to a mainwindow.
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(toolBar->parent());

    if (!mainWindow)
        return option;

    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
    Q_ASSERT_X(layout != 0, "QToolBarPrivate::getStyleOption()",
               "QMainWindow->layout() != QMainWindowLayout");

    // Determine the toolbar area.
    int layoutIndex = layout->indexOf(toolBar);
    if (layoutIndex != -1)
        option.toolBarArea = layout->toolBarArea(toolBar);

    // Find the toolbar line, and position within the line.
    int toolBarTotalLineCount = 0;
    int toolBarLineCount = 0;


	foreach	(QMainWindowLayout::ToolBarLineInfo	lineInfo, layout->tb_layout_info) {

        if (lineInfo.pos !=	positionForArea(option.toolBarArea))
            continue;
        int	toolBarIndex = -1;
        bool lineVisible = false;

        for	(int i = 0;	i <	lineInfo.list.size(); ++i){
            QMainWindowLayout::ToolBarLayoutInfo layoutInfo	= lineInfo.list.at(i);

            if (layoutInfo.item->widget()->isVisible())
                lineVisible = true;

            if (layoutInfo.item->widget() == toolBar) {
                // This	is our toolbar,	so we now have the line	and	position.
                toolBarLineCount = toolBarTotalLineCount;

                // We have to determine how many visible toolbars there are in this line
                int visibleLines = 0;
                for	(int j = 0;	j <	lineInfo.list.size(); ++j){
                    QMainWindowLayout::ToolBarLayoutInfo info = lineInfo.list.at(j);
                    if (info.item->widget() == toolBar)
                        toolBarIndex = visibleLines;
                    if (info.item->widget()->isVisible())
                        visibleLines++;
                }

                // Determine the position within this toolbar line
                if (toolBarIndex ==	0) {
                    if (visibleLines ==	1)
                        option.positionWithinLine = QStyleOptionToolBar::OnlyOne;
                    else
                        option.positionWithinLine = QStyleOptionToolBar::Beginning;
                } else if (toolBarIndex	< visibleLines - 1)	{
                    option.positionWithinLine = QStyleOptionToolBar::Middle;
                } else if (toolBarIndex	== visibleLines	- 1) {
                    option.positionWithinLine = QStyleOptionToolBar::End;
                }
                break;
            }
        }
		if(lineVisible)
            ++toolBarTotalLineCount;
	}

	Q_ASSERT_X(toolBarLineCount	>= 0, "QToolBarPrivate::getStyleOption()",
			   "toolbar	not	found in layout");



    if (option.toolBarArea== Qt::BottomToolBarArea || option.toolBarArea==Qt::RightToolBarArea){
        toolBarLineCount = toolBarTotalLineCount-toolBarLineCount-1;
    }

	// Determine the line position of this toolbar
	if (toolBarLineCount ==	0) {
		if (toolBarTotalLineCount == 1)
            option.positionOfLine = QStyleOptionToolBar::OnlyOne;
		else
			option.positionOfLine = QStyleOptionToolBar::Beginning;
	} else if (toolBarLineCount	< toolBarTotalLineCount	- 1) {
		option.positionOfLine = QStyleOptionToolBar::Middle;
	} else if (toolBarLineCount	== toolBarTotalLineCount - 1) {
		option.positionOfLine = QStyleOptionToolBar::End;
	}

    return option;
}


/*
static QStyleOptionFrame getStyleOption(QToolBar *tb)
{
    QStyleOptionFrame opt;
    opt.init(tb);
    if (tb->orientation() == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    opt.lineWidth = tb->style()->pixelMetric(QStyle::PM_ToolBarFrameWidth);
    return opt;
}
*/

/*
    QToolBarPrivate
*/

void QToolBarPrivate::init()
{
    Q_Q(QToolBar);
    movable = true;
    q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
    q->setBackgroundRole(QPalette::Button);


    QStyleOptionToolBar opt = getStyleOption(q);

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, q);
    QStyle *style = q->style();

    int e = style->pixelMetric(QStyle::PM_ToolBarIconSize);
    iconSize = QSize(e, e);

    layout->setAlignment(Qt::AlignLeft);
    layout->setMargin(style->pixelMetric(QStyle::PM_ToolBarFrameWidth, &opt, q)
                      + style->pixelMetric(QStyle::PM_ToolBarItemMargin, &opt, q));
    layout->setSpacing(style->pixelMetric(QStyle::PM_ToolBarItemSpacing, &opt, q));

    handle = new QToolBarHandle(q);
    QObject::connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
                     handle, SLOT(setOrientation(Qt::Orientation)));
    layout->addWidget(handle);
    handle->setVisible(movable && (qobject_cast<QMainWindow *>(q->parentWidget()) != 0));

    extension = new QToolBarExtension(q);
    QObject::connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
                     extension, SLOT(setOrientation(Qt::Orientation)));
    extension->setFocusPolicy(Qt::NoFocus);
    extension->hide();

#ifdef Q_WS_MAC
    if (q->parentWidget() && q->parentWidget()->isWindow()) {
        // Make sure that the window has the "toolbar" button.
        reinterpret_cast<QToolBar *>(q->parentWidget())->d_func()->createWinId(); // Please let me create your winId...
        extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
        ChangeWindowAttributes(qt_mac_window_for(q->parentWidget()), kWindowToolbarButtonAttribute,
                               kWindowNoAttributes);
    }
#endif

    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(_q_toggleView(bool)));
}

void QToolBarPrivate::_q_toggleView(bool b)
{
    Q_Q(QToolBar);
    if (b == q->isHidden()) {
        if (b)
            q->show();
        else
            q->close();
    }
}

void QToolBarPrivate::_q_updateIconSize(const QSize &sz)
{
    Q_Q(QToolBar);
    if (!explicitIconSize) {
        // iconSize not explicitly set
        q->setIconSize(sz);
        explicitIconSize = false;
    }
}

void QToolBarPrivate::_q_updateToolButtonStyle(Qt::ToolButtonStyle style)
{
    Q_Q(QToolBar);
    if (!explicitToolButtonStyle) {
        q->setToolButtonStyle(style);
        explicitToolButtonStyle = false;
    }
}

QToolBarItem QToolBarPrivate::createItem(QAction *action)
{
    Q_Q(QToolBar);
    QToolBarItem item;
    item.action = action;
    item.hidden = false;
    item.hasCustomWidget = false;

    QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action);
    if (widgetAction) {
        item.widget = widgetAction->requestWidget(q);
        if (item.widget) {
            item.hasCustomWidget = true;
            return item;
        }
    }
    if (action->isSeparator()) {
        item.widget = new QToolBarSeparator(q);
        QObject::connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
                         item.widget, SLOT(setOrientation(Qt::Orientation)));
    } else {
        QToolButton *button = new QToolButton(q);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(iconSize);
        button->setToolButtonStyle(toolButtonStyle);
        QObject::connect(q, SIGNAL(iconSizeChanged(QSize)),
                         button, SLOT(setIconSize(QSize)));
        QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                         button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
        button->setDefaultAction(action);
        QObject::connect(button, SIGNAL(triggered(QAction*)), q, SIGNAL(actionTriggered(QAction*)));
        item.widget = button;
    }

    return item;
}

/*
    Returns the position of \a action. This function returns -1 if \a
    action is not found.
*/
int QToolBarPrivate::indexOf(QAction *action) const
{
    for (int i = 0; i < items.size(); ++i) {
        const QToolBarItem &item = items.at(i);
        if (item.action == action)
            return i;
    }
    return -1;
}

/*!
    \class QToolBar

    \brief The QToolBar class provides a movable panel that contains a
    set of controls.

    \ingroup application
    \mainclass

    Toolbar buttons are added by adding \e actions, using addAction()
    or insertAction(). Groups of buttons can be separated using
    addSeparator() or insertSeparator(). If a toolbar button is not
    appropriate, a widget can be inserted instead using addWidget() or
    insertWidget(); examples of suitable widgets are QSpinBox,
    QDoubleSpinBox, and QComboBox. When a toolbar button is pressed it
    emits the actionTriggered() signal.

    A toolbar can be fixed in place in a particular area (e.g. at the
    top of the window), or it can be movable (isMovable()) between
    toolbar areas; see allowedAreas() and isAreaAllowed().

    When a toolbar is resized in such a way that it is too small to
    show all the items it contains, an extension button will appear as
    the last item in the toolbar. Pressing the extension button will
    pop up a menu containing the items that does not currently fit in
    the toolbar. Note that only action based items will be shown in
    the menu. If only non-action based items are to appear in the
    extension menu (e.g. a QSpinBox), the extension button will appear
    as usual, but it will be disabled to indicate that some items in
    the toolbar are currently not visible.

    \sa QToolButton, QMenu, QAction, {Application Example}
*/

/*!
    \fn bool QToolBar::isAreaAllowed(Qt::ToolBarArea area) const

    Returns true if this toolbar is dockable in the given \a area;
    otherwise returns false.
*/

/*!
    \fn void QToolBar::actionTriggered(QAction *action)

    This signal is emitted when a toolbar button is pressed. The
    parameter holds the toolbar button's associated \a action.
*/

/*!
    \fn void QToolBar::allowedAreasChanged(Qt::ToolBarAreas allowedAreas)

    This signal is emitted when the collection of allowed areas for the
    toolbar is changed. The new areas in which the toolbar can be positioned
    are specified by \a allowedAreas.

    \sa allowedAreas
*/

/*!
    \fn void QToolBar::iconSizeChanged(const QSize &iconSize)

    This signal is emitted when the icon size is changed.  The \a
    iconSize parameter holds the toolbar's new icon size.

    \sa iconSize QMainWindow::iconSize
*/

/*!
    \fn void QToolBar::movableChanged(bool movable)

    This signal is emitted when the toolbar becomes movable or fixed.
    If the toolbar can be moved, \a movable is true; otherwise it is
    false.

    \sa movable
*/

/*!
    \fn void QToolBar::orientationChanged(Qt::Orientation orientation)

    This signal is emitted when the orientation of the toolbar changes.
    The new orientation is specified by the \a orientation given.

    \sa orientation
*/

/*!
    \fn void QToolBar::toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle)

    This signal is emitted when the tool button style is changed. The
    \a toolButtonStyle parameter holds the toolbar's new tool button
    style.

    \sa toolButtonStyle QMainWindow::toolButtonStyle
*/

/*!
    Constructs a QToolBar with the given \a parent.
*/
QToolBar::QToolBar(QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
}

/*!
    Constructs a QToolBar with the given \a parent.

    The given window \a title identifies the toolbar and is shown in
    the context menu provided by QMainWindow.

    \sa setWindowTitle()
*/
QToolBar::QToolBar(const QString &title, QWidget *parent)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setWindowTitle(title);
}

#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs a QToolBar with the given \a parent and \a name.
*/
QToolBar::QToolBar(QWidget *parent, const char *name)
    : QWidget(*new QToolBarPrivate, parent, 0)
{
    Q_D(QToolBar);
    d->init();
    setObjectName(QString::fromAscii(name));
}
#endif

/*!
    Destroys the toolbar.
*/
QToolBar::~QToolBar()
{
    // Remove the toolbar button if there is nothing left.
    QMainWindow *mainwindow = qobject_cast<QMainWindow *>(parentWidget());
    if (mainwindow) {
        QMainWindowLayout *mainwin_layout = qobject_cast<QMainWindowLayout *>(mainwindow->layout());
        mainwin_layout->removeToolBarInfo(this);
        mainwin_layout->relayout();
#ifdef Q_WS_MAC
        if (mainwin_layout && mainwin_layout->tb_layout_info.isEmpty()
                && mainwindow->testAttribute(Qt::WA_WState_Created))
            ChangeWindowAttributes(qt_mac_window_for(mainwindow), kWindowNoAttributes,
                                   kWindowToolbarButtonAttribute);
#endif
    }
    Q_D(QToolBar);
    while (!d->items.isEmpty()) {
        const QToolBarItem item = d->items.takeFirst();
        QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(item.action);
        if (item.hasCustomWidget && widgetAction) {
            widgetAction->releaseWidget(item.widget);
        }
    }
}

/*! \property QToolBar::movable
    \brief whether the user can move the toolbar within the toolbar area,
    or between toolbar areas

    By default, this property is true.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa allowedAreas
*/

void QToolBar::setMovable(bool movable)
{
    Q_D(QToolBar);
    if (!movable == !d->movable)
        return;
    d->movable = movable;
    d->handle->setVisible(d->movable && (qobject_cast<QMainWindow *>(parentWidget()) != 0));
    emit movableChanged(d->movable);
}

bool QToolBar::isMovable() const
{ Q_D(const QToolBar); return d->movable; }

/*!
    \property QToolBar::allowedAreas
    \brief areas where the toolbar may be placed

    The default is Qt::AllToolBarAreas.

    This property only makes sense if the toolbar is in a
    QMainWindow.

    \sa movable
*/

void QToolBar::setAllowedAreas(Qt::ToolBarAreas areas)
{
    Q_D(QToolBar);
    areas &= Qt::ToolBarArea_Mask;
    if (areas == d->allowedAreas)
        return;
    d->allowedAreas = areas;
    emit allowedAreasChanged(d->allowedAreas);
}

Qt::ToolBarAreas QToolBar::allowedAreas() const
{ Q_D(const QToolBar); return d->allowedAreas; }

/*! \property QToolBar::orientation
    \brief orientation of the toolbar

    The default is Qt::Horizontal.

    The orientation is updated automatically when the toolbar is
    managed by QMainWindow.
*/

void QToolBar::setOrientation(Qt::Orientation orientation)
{
    Q_D(QToolBar);
    if (orientation == d->orientation)
        return;

    d->orientation = orientation;

    QBoxLayout *box = qobject_cast<QBoxLayout *>(layout());
    Q_ASSERT_X(box != 0, "QToolBar::setOrientation", "internal error");

    switch (d->orientation) {
    case Qt::Vertical:
	box->setDirection(QBoxLayout::TopToBottom);
        box->setAlignment(Qt::AlignTop);
 	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	break;

    case Qt::Horizontal:
	box->setDirection(QBoxLayout::LeftToRight);
        box->setAlignment(Qt::AlignLeft);
 	setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
	break;
    }

    emit orientationChanged(d->orientation);
}

Qt::Orientation QToolBar::orientation() const
{ Q_D(const QToolBar); return d->orientation; }

/*! \property QToolBar::iconSize
    \brief size of icons in the toolbar.

    The default is Qt::AutomaticIconSize.
*/

QSize QToolBar::iconSize() const
{ Q_D(const QToolBar); return d->iconSize; }

void QToolBar::setIconSize(const QSize &iconSize)
{
    Q_D(QToolBar);
    QSize sz = iconSize;
    if (!sz.isValid()) {
        QMainWindow *mw = qobject_cast<QMainWindow *>(parentWidget());
        if (mw && mw->layout()) {
            QLayout *layout = mw->layout();
            int i = 0;
            QLayoutItem *item = 0;
            do {
                item = layout->itemAt(i++);
                if (item && (item->widget() == this))
                    sz = mw->iconSize();
            } while (!sz.isValid() && item != 0);
        }
    }
    if (!sz.isValid()) {
        const int metric = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        sz = QSize(metric, metric);
    }
    if (d->iconSize != sz) {
        d->iconSize = sz;
        setMinimumSize(0, 0);
        emit iconSizeChanged(d->iconSize);
    }
    d->explicitIconSize = iconSize.isValid();
}

/*!
    \property QToolBar::toolButtonStyle
    \brief the style of toolbar buttons

    The default is Qt::ToolButtonIconOnly.
*/

Qt::ToolButtonStyle QToolBar::toolButtonStyle() const
{ Q_D(const QToolBar); return d->toolButtonStyle; }

void QToolBar::setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
    Q_D(QToolBar);
    d->explicitToolButtonStyle = true;
    if (d->toolButtonStyle == toolButtonStyle)
        return;
    d->toolButtonStyle = toolButtonStyle;
    setMinimumSize(0, 0);
    emit toolButtonStyleChanged(d->toolButtonStyle);
}

/*!
    Removes all actions from the toolbar.

    \sa removeAction()
*/
void QToolBar::clear()
{
    QList<QAction *> actions = this->actions();
    for(int i = 0; i < actions.size(); i++)
        removeAction(actions.at(i));
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar.
*/
QAction *QToolBar::addAction(const QString &text)
{
    QAction *action = new QAction(text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a icon and \a text. This
    action is added to the end of the toolbar.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text)
{
    QAction *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the given \a text. This action is added to
    the end of the toolbar. The action's \link QAction::triggered()
    triggered()\endlink signal is connected to \a member in \a
    receiver.
*/
QAction *QToolBar::addAction(const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*!
    \overload

    Creates a new action with the icon \a icon and text \a text. This
    action is added to the end of the toolbar. The action's \link
    QAction::triggered() triggered()\endlink signal is connected to \a
    member in \a receiver.
*/
QAction *QToolBar::addAction(const QIcon &icon, const QString &text,
                             const QObject *receiver, const char* member)
{
    QAction *action = new QAction(icon, text, this);
    QObject::connect(action, SIGNAL(triggered()), receiver, member);
    addAction(action);
    return action;
}

/*!
     Adds a separator to the end of the toolbar.

     \sa insertSeparator()
*/
QAction *QToolBar::addSeparator()
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

/*!
    Inserts a separator into the toolbar in front of the toolbar
    item associated with the \a before action.

    \sa addSeparator()
*/
QAction *QToolBar::insertSeparator(QAction *before)
{
    QAction *action = new QAction(this);
    action->setSeparator(true);
    insertAction(before, action);
    return action;
}

/*!
    Adds the given \a widget to the toolbar as the toolbar's last
    item.

    Note: You should use QAction::setVisible() to change the
    visibility of the widget. Using QWidget::setVisible(),
    QWidget::show() and QWidget::hide() does not work.

    \sa insertWidget()
*/
QAction *QToolBar::addWidget(QWidget *widget)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);
    action->d_func()->autoCreated = true;
    addAction(action);
    return action;
}

/*!
    Inserts the given \a widget in front of the toolbar item
    associated with the \a before action.

    Note: You should use QAction::setVisible() to change the
    visibility of the widget. Using QWidget::setVisible(),
    QWidget::show() and QWidget::hide() does not work.

    \sa addWidget()
*/
QAction *QToolBar::insertWidget(QAction *before, QWidget *widget)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);
    action->d_func()->autoCreated = true;
    insertAction(before, action);
    return action;
}

/*!
    \internal

    Returns the geometry of the toolbar item associated with the given
    \a action, or an invalid QRect if no matching item is found.
*/
QRect QToolBar::actionGeometry(QAction *action) const
{
    Q_D(const QToolBar);
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.action == action)
            return item.widget->geometry();
    }
    return QRect();
}

/*!
    Returns the action at point \a p. This function returns zero if no
    action was found.

    \sa QWidget::childAt()
*/
QAction *QToolBar::actionAt(const QPoint &p) const
{
    Q_D(const QToolBar);
    QWidget *widget = childAt(p);
    for (int i = 0; i < d->items.size(); ++i) {
        const QToolBarItem &item = d->items.at(i);
        if (item.widget == widget)
            return item.action;
    }
    return 0;
}

/*! \fn QAction *QToolBar::actionAt(int x, int y) const
    \overload

    Returns the action at the point \a x, \a y. This function returns
    zero if no action was found.
*/

/*! \reimp */
void QToolBar::actionEvent(QActionEvent *event)
{
    Q_D(QToolBar);
    QAction *action = event->action();
    QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action);

    switch (event->type()) {
    case QEvent::ActionAdded:
        {
            Q_ASSERT_X(!widgetAction || d->indexOf(widgetAction) == -1,
                       "QToolBar", "widgets cannot be inserted multiple times");

            QToolBarItem item = d->createItem(action);
            bool visible = item.action->isVisible();
            // reparent the action to this toolbar if it has been created
            // using the addAction(text) etc. convenience functions, to
            // preserve Qt 4.1.x behavior. The widget is already
            // reparented to us due to the createWidget call inside
            // createItem()
            if (widgetAction && widgetAction->d_func()->autoCreated)
                    widgetAction->setParent(this);
            // make sure the layout doesn't show() the widget too soon
            item.widget->hide();
            if (event->before()) {
                int index = d->indexOf(event->before());
                Q_ASSERT_X(index >= 0 && index < d->items.size(), "QToolBar::insertAction",
                           "internal error");
                d->items.insert(index, item);
                qobject_cast<QBoxLayout *>(layout())->insertWidget(index + 1, item.widget);
            } else {
                d->items.append(item);
                qobject_cast<QBoxLayout *>(layout())->insertWidget(d->items.size(), item.widget);
            }
            item.widget->setVisible(visible);
            if (isVisible())
                QApplication::postEvent(this, new QResizeEvent(size(), size()));
            break;
        }

    case QEvent::ActionChanged:
        {
            int index = d->indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::actionEvent", "internal error");
            const QToolBarItem &item = d->items.at(index);
            if (!item.hidden) {
                item.widget->setVisible(item.action->isVisible());
            } else {
                // more elephant shaving
                if (isVisible())
                    QApplication::postEvent(this, new QResizeEvent(size(), size()));
            }

            break;
        }

    case QEvent::ActionRemoved:
        {
            int index = d->indexOf(action);
            Q_ASSERT_X(index >= 0 && index < d->items.size(),
                       "QToolBar::removeAction", "internal error");
            QToolBarItem item = d->items.takeAt(index);
            layout()->removeWidget(item.widget);
            if (widgetAction && item.hasCustomWidget) {
                widgetAction->releaseWidget(item.widget);
            } else {
                // destroy the QToolButton/QToolBarSeparator
                item.widget->hide();
                item.widget->deleteLater();
            }
            // show all widgets that might be hidden in the extension
            // menu, so that QLayout can recalculate their positions
            for (int i = 0; i< d->items.size(); ++i)
                d->items[i].widget->show();

            if (isVisible())
                QApplication::postEvent(this, new QResizeEvent(size(), size()));
            break;
        }

    default:
	Q_ASSERT_X(false, "QToolBar::actionEvent", "internal error");
    }
}

/*! \reimp */
void QToolBar::changeEvent(QEvent *event)
{
    Q_D(QToolBar);
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        d->toggleViewAction->setText(windowTitle());
        break;
    case QEvent::StyleChange:
        {
            QStyleOptionToolBar opt = getStyleOption(this);
            d->layout->setMargin(style()->pixelMetric(QStyle::PM_ToolBarFrameWidth, &opt, this)
                                 + style()->pixelMetric(QStyle::PM_ToolBarItemMargin, &opt, this));
            d->layout->setSpacing(style()->pixelMetric(QStyle::PM_ToolBarItemSpacing, &opt, this));
            break;
        }
    default:
        break;
    }
    QWidget::changeEvent(event);
}

/*! \reimp */
void QToolBar::childEvent(QChildEvent *event)
{
    Q_D(QToolBar);
    QWidget *widget = qobject_cast<QWidget *>(event->child());
    if (widget && event->type() == QEvent::ChildRemoved) {
        for (int i = 0; i < d->items.size(); ++i) {
            const QToolBarItem &item = d->items.at(i);
            QWidgetAction *widgetAction = 0;
            if (item.widget == widget
                && (widgetAction = qobject_cast<QWidgetAction *>(item.action))) {
                removeAction(widgetAction);
                // ### should we delete the action, or is it the programmer's responsibility?
                // delete widgetAction;
            }
        }
    }
    QWidget::childEvent(event);
}

/*! \reimp */
void QToolBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QStyleOptionToolBar opt = getStyleOption(this);
    style()->drawControl(QStyle::CE_ToolBar, &opt, &p, this);
}

/*! \reimp */
void QToolBar::resizeEvent(QResizeEvent *event)
{
    Q_D(QToolBar);
    if (d->inResizeEvent)
        return;
    d->inResizeEvent = true;

    QBoxLayout *box = qobject_cast<QBoxLayout *>(layout());
    Qt::Orientation orientation = (box->direction() == QBoxLayout::LeftToRight
                                   || box->direction() == QBoxLayout::RightToLeft)
                                  ? Qt::Horizontal
                                  : Qt::Vertical;
    const int margin = box->margin();
    int i = 0;
    int extension_size = 0;
    int hidden_count = 0;
    int max_item_extent = 0;
    i = d->items.size(); // note: the toolbar handle is not counted
    while (i > 0) {
	QWidget *w = box->itemAt(i)->widget();
	bool hide = false;
	if (QApplication::layoutDirection() == Qt::RightToLeft && orientation == Qt::Horizontal) {
            if (w->isHidden()) {
                if (box->itemAt(i-1) && !box->itemAt(i-1)->widget()->isHidden()) {
                    QWidget *pw = box->itemAt(i-1)->widget();
                    hide = pw->pos().x() < (extension_size + w->size().width() + margin + box->spacing());
                } else {
                    // calculate the pos of the hidden item
                    int pos = 0;
                    for (int k = 1; k < i; ++k) { // idx 0 == handle
                        QWidget * pw = box->itemAt(k)->widget();
                        if (pw == w)
                            break;
                        pos = pw->isHidden()
                              ? pos - pw->size().width() - box->spacing()
                              : pw->pos().x();
                    }
                    pos = pos - w->size().width() - box->spacing();
                    hide = pos < extension_size + margin;
                }
            } else {
                hide = w->pos().x() < extension_size + margin;
            }
	} else {
	    hide = pick(orientation, w->pos()) + pick(orientation, w->size())
		   >= pick(orientation, size()) - extension_size;
	}
	if (hide && i > 1) { // never hide the first item in the tb
	    w->hide();
            if (d->items[i - 1].action->isVisible()) {
                d->items[i - 1].hidden = true;
                ++hidden_count;
            }
	    // the size of the extension menu button needs to be
	    // considered when buttons in the toolbar are hidden
	    extension_size = pick(orientation, d->extension->sizeHint());
	} else {
            w->setVisible(d->items[i - 1].action->isVisible());
            d->items[i - 1].hidden = false;
            if (orientation == Qt::Horizontal)
                max_item_extent = qMax(max_item_extent, w->sizeHint().height());
            else
                max_item_extent = qMax(max_item_extent, w->sizeHint().width());
	}
	--i;
    }

    int box_spacing = (d->items.size() > 1) ? box->spacing()*2 : box->spacing();
    int item_min_size = (orientation == Qt::Horizontal) ? d->iconSize.width() : d->iconSize.height();
    if (!d->items.isEmpty()) {
        QSize sz(d->items[0].widget->sizeHint());
#ifndef QT_NO_MENUBAR
        if (qobject_cast<QMenuBar *>(d->items[0].widget))
            sz = d->items[0].widget->minimumSizeHint();
#endif
        item_min_size = (orientation == Qt::Horizontal) ? sz.width() : sz.height();
    }
    if (orientation == Qt::Horizontal) {
        setMinimumSize(d->handle->sizeHint().width() + box_spacing + extension_size + margin*2
                       + item_min_size, max_item_extent + margin*2);
    } else {
        setMinimumSize(max_item_extent + margin*2,
                       d->handle->sizeHint().height() + box_spacing + extension_size
                       + margin*2 + item_min_size);
    }

    if (hidden_count > 0) {
	if (orientation == Qt::Horizontal) {
	    int x = QApplication::layoutDirection() == Qt::RightToLeft
		    ? margin
		    : width() - d->extension->sizeHint().width() - margin;
	    d->extension->setGeometry(x, margin, d->extension->sizeHint().width(), max_item_extent);
        } else {
	    d->extension->setGeometry(margin,
				      height() - d->extension->sizeHint().height() - margin,
				      max_item_extent,
				      d->extension->sizeHint().height());
        }

#ifndef QT_NO_MENU
	QMenu *pop = d->extension->menu();
	if (!pop) {
	    pop = new QMenu(this);
	    d->extension->setMenu(pop);
	}
	pop->clear();
	for(int i = 0; i < d->items.size(); ++i) {
            const QToolBarItem &item = d->items.at(i);
            if (!item.hidden) continue;

            if (!qobject_cast<QWidgetAction *>(item.action)) {
                pop->addAction(item.action);
            } else {
#if !defined(QT_NO_SIGNALMAPPER) && !defined(QT_NO_COMBOBOX)
                if (QComboBox *cb = qobject_cast<QComboBox *>(item.widget)) {
                    QMenu *cb_menu = new QMenu(cb->windowTitle(), pop);
                    QSignalMapper *cb_mapper = new QSignalMapper(cb_menu);
                    pop->addMenu(cb_menu);
                    QActionGroup *ag = new QActionGroup(cb_menu);
                    ag->setExclusive(true);
                    for (int i=0; i<cb->count(); ++i) {
                        QAction *ac = cb_menu->addAction(cb->itemIcon(i), cb->itemText(i));
                        ac->setActionGroup(ag);
                        ac->setCheckable(true);
                        ac->setChecked(cb->currentIndex() == i);
                        connect(ac, SIGNAL(triggered(bool)), cb_mapper, SLOT(map()));
                        cb_mapper->setMapping(ac, i);
                        cb_mapper->setMapping(ac, cb->itemText(i));
                    }
                    connect(cb_mapper, SIGNAL(mapped(int)), cb, SIGNAL(activated(int)));
                    connect(cb_mapper, SIGNAL(mapped(int)), cb, SLOT(setCurrentIndex(int)));
                    connect(cb_mapper, SIGNAL(mapped(QString)), cb, SIGNAL(activated(QString)));
                } else
#endif // QT_NO_SIGNALMAPPER
                    if (QToolButton *tb = qobject_cast<QToolButton *>(item.widget)) {
                        QAction *ac = pop->addAction(tb->icon(), tb->text());
                        connect(ac, SIGNAL(triggered()), tb, SLOT(click()));
                    } else {
                        QList<QAbstractButton *> children = qFindChildren<QAbstractButton *>(item.widget);
                        QList<QAbstractButton *>::const_iterator it = children.constBegin();
                        while (it != children.constEnd()) {
                            QAction *ac = pop->addAction((*it)->icon(), (*it)->text());
                            ac->setCheckable((*it)->isCheckable());
                            ac->setChecked((*it)->isChecked());
                            ac->setEnabled((*it)->isEnabled());
                            connect(ac, SIGNAL(triggered()), *it, SLOT(click()));
                            ++it;
                        }
                    }
            }
        }
        if (pop->actions().size() > 0) {
            d->extension->show();
            d->extension->setEnabled(true);
        } else {
            // show a disabled ext btn in the case where widgets in
            // the toolbar are hidden but not put into the ext menu -
            // this indicates that some items in the tb is hidden
            d->extension->show();
            d->extension->setEnabled(false);
        }
#endif // QT_NO_MENU
    } else if (!d->extension->isHidden()) {
	if (d->extension->menu())
	    d->extension->menu()->clear();
	d->extension->hide();
    }
    QWidget::resizeEvent(event);

    d->inResizeEvent = false;
}

/*! \reimp */
bool QToolBar::event(QEvent *event)
{
    Q_D(QToolBar);

    switch (event->type()) {
    case QEvent::Hide:
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
    case QEvent::ParentChange:
        d->handle->setVisible(d->movable && (qobject_cast<QMainWindow *>(parentWidget()) != 0));
        break;
    case QEvent::StyleChange:
        if (!d->explicitIconSize)
            setIconSize(QSize());
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

/*!
    Returns a checkable action that can be used to show or hide this
    toolbar.

    The action's text is set to the toolbar's window title.

    \sa QAction::text QWidget::windowTitle
*/
QAction *QToolBar::toggleViewAction() const
{ Q_D(const QToolBar); return d->toggleViewAction; }

/*!
    \fn void QToolBar::setLabel(const QString &label)

    Use setWindowTitle() instead.
*/

/*!
    \fn QString QToolBar::label() const

    Use windowTitle() instead.
*/

/*!
    \since 4.2

    Returns the widget associated with the specified \a action.

    \sa addWidget()
*/
QWidget *QToolBar::widgetForAction(QAction *action) const
{
    Q_D(const QToolBar);

    int index = d->indexOf(action);
    if (index < 0 || index >= d->items.size())
        return 0;

    return d->items.at(index).widget;
}

#include "moc_qtoolbar.cpp"
#endif // QT_NO_TOOLBAR
