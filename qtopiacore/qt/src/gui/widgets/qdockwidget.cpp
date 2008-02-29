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

#include "qdockwidget.h"

#ifndef QT_NO_DOCKWIDGET
#include <qaction.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfontmetrics.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbutton.h>
#include <qdebug.h>

#include <private/qwidgetresizehandler_p.h>

#include "qdockwidget_p.h"
#include "qdockwidgetlayout_p.h"
#include "qmainwindowlayout_p.h"
#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif


static inline bool hasFeature(QDockWidget *dockwidget, QDockWidget::DockWidgetFeature feature)
{ return (dockwidget->features() & feature) == feature; }


/*
    A Dock Window:

    [+] is the float button
    [X] is the close button

    +-------------------------------+
    | Dock Window Title       [+][X]|
    +-------------------------------+
    |                               |
    | place to put the single       |
    | QDockWidget child (this space |
    | does not yet have a name)     |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    |                               |
    +-------------------------------+

*/




/*
  Tool window title
*/

class QDockWidgetTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    QDockWidgetTitleButton(QDockWidget *dockWidget);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};


QDockWidgetTitleButton::QDockWidgetTitleButton(QDockWidget *dockWidget)
    : QAbstractButton(dockWidget)
{ setFocusPolicy(Qt::NoFocus); }

QSize QDockWidgetTitleButton::sizeHint() const
{
    ensurePolished();

    int dim = 0;
    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal);
        dim = qMax(pm.width(), pm.height());
    }

    return QSize(dim + 4, dim + 4);
}

void QDockWidgetTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void QDockWidgetTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void QDockWidgetTitleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    int shiftHorizontal = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this) : 0;
    int shiftVertical = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, this) : 0;

    r.adjust(2, 2, -2, -2);
    r.translate(shiftHorizontal, shiftVertical);

    QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), isEnabled() ?
                                underMouse() ? QIcon::Active : QIcon::Normal
                                    : QIcon::Disabled,
                                isDown() ? QIcon::On : QIcon::Off);
    style()->drawItemPixmap(&p, r, Qt::AlignCenter, pm);
}


/*
  Private class
*/

void QDockWidgetPrivate::init() {
    Q_Q(QDockWidget);

    box = new QDockWidgetBoxLayout(q);
    box->setMargin(0);
    box->setSpacing(0);

    topSpacer = new QSpacerItem(0, 20, QSizePolicy::Ignored, QSizePolicy::Fixed);
    box->addItem(topSpacer);

    resizer = new QWidgetResizeHandler(q);
    resizer->setMovingEnabled(false);
    resizer->setActive(false);

#ifndef QT_NO_ACTION
    toggleViewAction = new QAction(q);
    toggleViewAction->setCheckable(true);
    toggleViewAction->setText(q->windowTitle());
    QObject::connect(toggleViewAction, SIGNAL(triggered(bool)), q, SLOT(_q_toggleView(bool)));
#endif

    updateButtons();
}

QStyleOptionDockWidget QDockWidgetPrivate::getStyleOption()
{
    Q_Q(QDockWidget);
    QStyleOptionDockWidget opt;
    opt.init(q);
    opt.rect = titleArea;
    opt.title = q->windowTitle();
    opt.closable = hasFeature(q, QDockWidget::DockWidgetClosable);
    opt.movable = hasFeature(q, QDockWidget::DockWidgetMovable);
    opt.floatable = hasFeature(q, QDockWidget::DockWidgetFloatable);
    return opt;
}

void QDockWidgetPrivate::_q_toggleView(bool b)
{
    Q_Q(QDockWidget);
    if (b == q->isHidden()) {
        if (b)
            q->show();
        else
            q->close();
    }
}

void QDockWidgetPrivate::updateButtons()
{
    Q_Q(QDockWidget);
    if (hasFeature(q, QDockWidget::DockWidgetFloatable)) {
        if (!floatButton) {
            floatButton = new QDockWidgetTitleButton(q);
            QObject::connect(floatButton, SIGNAL(clicked()), q, SLOT(_q_toggleTopLevel()));

            if (!q->isHidden())
                floatButton->show();
        }
    } else {
        delete floatButton;
        floatButton = 0;
    }

    if (hasFeature(q, QDockWidget::DockWidgetClosable)) {
        if (!closeButton) {
            closeButton = new QDockWidgetTitleButton(q);
            QObject::connect(closeButton, SIGNAL(clicked()), q, SLOT(close()));

            if (!q->isHidden())
                closeButton->show();
        }
    } else {
        delete closeButton;
        closeButton = 0;
    }

    bool anyButton = (floatButton || closeButton);
    if (anyButton) {
        QStyleOptionDockWidget opt = getStyleOption();
        if (floatButton)
            floatButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarNormalButton, &opt, q));
        if (closeButton)
            closeButton->setIcon(q->style()->standardIcon(QStyle::SP_TitleBarCloseButton, &opt, q));
    }

    q->setAttribute(Qt::WA_ContentsPropagated, anyButton);
    relayout();
}

// ### Qt 4.3: Add subrects to style API, this will cover our styles for now
//             Also, add posibilty to get standardIcons
void QDockWidgetPrivate::relayout()
{
    Q_Q(QDockWidget);
    int fw = q->isFloating() ? q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q) : 0;
    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);
    QSize closeSize = closeButton ? closeButton->sizeHint() : QSize(0,0);
    QSize floatSize = floatButton ? floatButton->sizeHint() : QSize(0,0);

    int minWidth  = q->fontMetrics().width(q->windowTitle()) + 2 * fw + 2 * mw;
    int minHeight = qMax(closeSize.width(), closeSize.height());
    minHeight = qMax(minHeight, qMax(floatSize.width(), floatSize.height()));
#ifdef Q_WS_MAC
    if (qobject_cast<QMacStyle *>(q->style())) {
        extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
        QFont font = qt_app_fonts_hash()->value("QToolButton", q->font());
        QFontMetrics fm(font);
        minHeight = qMax(minHeight, fm.lineSpacing()); //Ensure 2 px margin around font
    } else
#endif
    {
        minHeight = qMax(minHeight, q->fontMetrics().lineSpacing()); //Ensure 2 px margin around font
    }
    minHeight += 2 + 2*mw - fw; // Allow 1px frame around title area with buttons inside
    titleArea = QRect(QPoint(fw, fw),
                      QSize(q->rect().width() - (fw * 2), minHeight));

#ifdef Q_OS_MAC
    titleArea.adjust(0, 0, -fw, fw);
#endif

    int posX = titleArea.right();

    QPoint buttonOffset(0, 0);
#ifdef Q_OS_WIN
    // ### Qt 4.3: Fix this properly
    if (q->style()->inherits("QWindowsXPStyle")) {
        if(q->isFloating())
            buttonOffset = QPoint(2, -1);
        else
            buttonOffset = QPoint(0, 1);
    }
#endif
    if (closeButton) {
        // ### Qt 4.3: Fix this properly
        closeButton->setGeometry(QStyle::visualRect(
				    q->layoutDirection(),
                                    titleArea, QRect(posX - closeSize.width() - mw + buttonOffset.x(),
                                    titleArea.center().y() - closeSize.height() / 2 + + buttonOffset.y(),
                                    closeSize.width(), closeSize.height())));
        posX -= closeSize.width() + 1;
    }

    if (floatButton) {
        // ### Qt 4.3: Fix this properly
        floatButton->setGeometry(QStyle::visualRect(
				    q->layoutDirection(),
                                    titleArea, QRect(posX - floatSize.width() - mw + buttonOffset.x(),
                                    titleArea.center().y() - floatSize.height() / 2 + buttonOffset.y(),
                                    floatSize.width(), floatSize.height())));
        posX -= floatSize.width() + 1;
    }

    topSpacer->changeSize(minWidth, 0 + titleArea.height(), QSizePolicy::Expanding, QSizePolicy::Fixed);
    box->setMargin(fw);
    box->invalidate();
}

void QDockWidgetPrivate::_q_toggleTopLevel()
{
    Q_Q(QDockWidget);
    QPoint p = q->mapToGlobal(QPoint(titleArea.height(), titleArea.height()));
    bool visible = q->isVisible();
    if (visible)
        q->hide();
    q->setFloating(!q->isFloating());
    if (q->isWindow())
        q->move(p);
    if (visible)
        q->show();
}

QMainWindow *QDockWidgetPrivate::findMainWindow(QWidget *widget) const
{
    Q_Q(const QDockWidget);
    QMainWindow *mainwindow = 0;
    // look for our QMainWindow
    while (widget && !mainwindow) {
        if (widget == q->parentWidget()) {
            // found our parent widget, has to be the mainwindow we're looking for
            mainwindow = qobject_cast<QMainWindow *>(widget);
            break;
        } else if (widget->isWindow()) {
            // found a window that isn't our QMainWindow, stop looking
            widget = 0;
        } else {
            widget = widget->parentWidget();
        }
    }
    return mainwindow;
}

void QDockWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;

    if (!titleArea.contains(event->pos()))
        return;
    // check if the tool window is movable... do nothing if it is not
    if (!::hasFeature(q, QDockWidget::DockWidgetMovable))
        return;

    if (!::hasFeature(q, QDockWidget::DockWidgetFloatable))
        return;

    if (qobject_cast<QMainWindow*>(q->parentWidget()) == 0)
        return;

    if (state != 0)
        return;

    state = new QDockWidgetPrivate::DragState;

    state->pressPos = event->pos();
    state->dragging = false;
    state->widgetItem = 0;
    state->ownWidgetItem = false;
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_Q(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;
    if (!titleArea.contains(event->pos()))
        return;
    if (!::hasFeature(q, QDockWidget::DockWidgetFloatable))
        return;
    _q_toggleTopLevel();
}

void QDockWidgetPrivate::mouseMoveEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);

    if (!state)
        return;

    QMainWindowLayout *layout
        = qobject_cast<QMainWindowLayout *>(q->parentWidget()->layout());
    Q_ASSERT(layout != 0);

    if (!state->dragging
        && layout->pluggingWidget == 0
        && (event->pos() - state->pressPos).manhattanLength() > QApplication::startDragDistance()) {
        state->widgetItem = layout->unplug(q);
        if (state->widgetItem == 0) {
            /* I have a QMainWindow parent, but I was never inserted with QMainWindow::addDockWidget,
               so the QMainWindowLayout has no widget item for me. :(
               I have to create it myself, and then delete it if I don't get dropped into a
               dock area. */
            state->widgetItem = new QWidgetItem(q);
            state->ownWidgetItem = true;
        }
        q->grabMouse();
        state->dragging = true;
    }

    if (state->dragging) {
        q->move(event->globalPos() - state->pressPos);

        if (!(event->modifiers() & Qt::ControlModifier))
            state->pathToGap = layout->hover(state->widgetItem, event->globalPos());
    }
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::mouseReleaseEvent(QMouseEvent *event)
{
#if !defined(QT_NO_MAINWINDOW)
    Q_Q(QDockWidget);
    if (event->button() != Qt::LeftButton)
        return;
    if (!state)
        return;

    if (state->dragging) {
        QMainWindowLayout *layout =
            qobject_cast<QMainWindowLayout *>(q->parentWidget()->layout());
        Q_ASSERT(layout != 0);

        q->releaseMouse();

        if (!(event->modifiers() & Qt::ControlModifier) && !state->pathToGap.isEmpty()) {
            layout->plug(state->widgetItem, state->pathToGap);
        } else {
            layout->restore();

            if (state->ownWidgetItem)
                delete state->widgetItem;

            q->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
            updateButtons();
            resizer->setActive(QWidgetResizeHandler::Resize, true);
            q->show();
        }
    }

    delete state;
    state = 0;
#endif // !defined(QT_NO_MAINWINDOW)
}

void QDockWidgetPrivate::unplug(const QRect &rect)
{
    Q_Q(QDockWidget);
    QRect r = rect;
    r.moveTopLeft(q->mapToGlobal(r.topLeft()));
    q->hide();
    q->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::X11BypassWindowManagerHint);
    resizer->setActive(QWidgetResizeHandler::Resize, false);
    q->setGeometry(r);
#ifdef Q_OS_MAC
    relayout();
#endif
    q->show();
    updateButtons();
    emit q->topLevelChanged(q->isWindow());
}

void QDockWidgetPrivate::plug(const QRect &rect)
{
    Q_Q(QDockWidget);
    q->hide();

    q->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);

    updateButtons();
    resizer->setActive(QWidgetResizeHandler::Resize, false);
    q->setGeometry(rect);
    q->show();

    emit q->topLevelChanged(false);
}




/*!
    \class QDockWidget

    \brief The QDockWidget class provides a widget that can be docked
    inside a QMainWindow or floated as a top-level window on the
    desktop.

    \ingroup application

    QDockWidget provides the concept of dock widgets, also know as
    tool palettes or utility windows.  Dock windows are secondary
    windows placed in the \e {dock widget area} around the
    \l{QMainWindow::centralWidget()}{central widget} in a
    QMainWindow.

    \image mainwindow-docks.png

    Dock windows can be moved inside their current area, moved into
    new areas and floated (e.g. undocked) by the end-user.  The
    QDockWidget API allows the programmer to restrict the dock widgets
    ability to move, float and close, as well as the areas in which
    they can be placed.

    \section1 Appearance

    A QDockWidget consists of a title bar and the content area.  The
    titlebar displays the dock widgets \link QWidget::windowTitle()
    window title\endlink, a \e float button and a \e close button.
    Depending on the state of the QDockWidget, the \e float and \e
    close buttons may be either disabled or not shown at all.

    The visual appearance of the title bar and buttons is dependent
    on the \l{QStyle}{style} in use.

    \sa QMainWindow, {Dock Widgets Example}
*/

/*!
    \enum QDockWidget::DockWidgetFeature

    \value DockWidgetClosable   The dock widget can be closed.
    \value DockWidgetMovable    The dock widget can be moved between docks
                                by the user.
    \value DockWidgetFloatable  The dock widget can be detached from the
                                main window, and floated as an independent
                                window.

    \value AllDockWidgetFeatures  The dock widget can be closed, moved,
                                  and floated.
    \value NoDockWidgetFeatures   The dock widget cannot be closed, moved,
                                  or floated.

    \omitvalue DockWidgetFeatureMask
    \omitvalue Reserved
*/

/*!
    \property QDockWidget::windowTitle
    \internal
*/

/*!
    Constructs a QDockWidget with parent \a parent and window flags \a
    flags. The dock widget will be placed in the left dock widget
    area.
*/
QDockWidget::QDockWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(*new QDockWidgetPrivate, parent, flags)
{
    Q_D(QDockWidget);
    d->init();
}

/*!
    Constructs a QDockWidget with parent \a parent and window flags \a
    flags. The dock widget will be placed in the left dock widget
    area.

    The window title is set to \a title. This title is used when the
    QDockWidget is docked and undocked. It is also used in the context
    menu provided by QMainWindow.

    \sa setWindowTitle()
*/
QDockWidget::QDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(*new QDockWidgetPrivate, parent, flags)
{
    Q_D(QDockWidget);
    d->init();
    setWindowTitle(title);
}

/*!
    Destroys the dock widget.
*/
QDockWidget::~QDockWidget()
{ }

/*!
    Returns the widget for the dock widget. This function returns zero
    if the widget has not been set.

    \sa setWidget()
*/
QWidget *QDockWidget::widget() const
{
    Q_D(const QDockWidget);
    return d->widget;
}

/*!
    Sets the widget for the dock widget to \a widget.

    \sa widget()
*/
void QDockWidget::setWidget(QWidget *widget)
{
    Q_D(QDockWidget);

    if (d->widget)
        d->box->removeWidget(d->widget);

    d->widget = widget;

    if (d->widget) {
        d->box->addChildWidget(widget);
        d->box->insertItem(1, new QWidgetItem(d->widget));
    }
}

/*!
    \property QDockWidget::features
    \brief whether the dock widget is movable, closable, and floatable

    \sa DockWidgetFeature
*/

void QDockWidget::setFeatures(QDockWidget::DockWidgetFeatures features)
{
    Q_D(QDockWidget);
    features &= DockWidgetFeatureMask;
    if (d->features == features)
        return;
    d->features = features;
    d->updateButtons();
    d->toggleViewAction->setEnabled((d->features & DockWidgetClosable) == DockWidgetClosable);
    emit featuresChanged(d->features);
    update();
}

QDockWidget::DockWidgetFeatures QDockWidget::features() const
{
    Q_D(const QDockWidget);
    return d->features;
}

/*!
    \property QDockWidget::floating
    \brief whether the dock widget is floating

    A floating dock widget is presented to the user as an independent
    window "on top" of its parent QMainWindow, instead of being
    docked in the QMainWindow.

    \sa isWindow()
*/
void QDockWidget::setFloating(bool floating)
{
    Q_D(QDockWidget);
    if (floating == isFloating())
        return;

    const bool visible = isVisible();

    if (!floating && parentWidget() != 0) {
        QMainWindowLayout *layout
            = qobject_cast<QMainWindowLayout*>(parentWidget()->layout());
        if (layout != 0)
            layout->keepSize(this);
    }

    setWindowFlags(Qt::FramelessWindowHint | (floating ? Qt::Tool : Qt::Widget));

    d->updateButtons();
    d->resizer->setActive(QWidgetResizeHandler::Resize, floating);

    if (visible)
        show();

    emit topLevelChanged(isWindow());
}

/*!
    \property QDockWidget::allowedAreas
    \brief areas where the dock widget may be placed

    The default is Qt::AllDockWidgetAreas.

    \sa Qt::DockWidgetArea
*/

void QDockWidget::setAllowedAreas(Qt::DockWidgetAreas areas)
{
    Q_D(QDockWidget);
    areas &= Qt::DockWidgetArea_Mask;
    if (areas == d->allowedAreas)
        return;
    d->allowedAreas = areas;
    emit allowedAreasChanged(d->allowedAreas);
}

Qt::DockWidgetAreas QDockWidget::allowedAreas() const
{
    Q_D(const QDockWidget);
    return d->allowedAreas;
}

/*!
    \fn bool QDockWidget::isAreaAllowed(Qt::DockWidgetArea area) const

    Returns true if this dock widget can be placed in the given \a area;
    otherwise returns false.
*/

/*! \reimp */
void QDockWidget::changeEvent(QEvent *event)
{
    Q_D(QDockWidget);
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        update(d->titleArea);
#ifndef QT_NO_ACTION
        d->toggleViewAction->setText(windowTitle());
#endif
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

/*! \reimp */
void QDockWidget::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

/*! \reimp */
void QDockWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(this);
    // ### Add PixelMetric to change spacers, so style may show border
    // when not floating.
    if (isFloating()) {
        QStyleOptionFrame framOpt;
        framOpt.init(this);
        style()->drawPrimitive(QStyle::PE_FrameDockWidget, &framOpt, &p, this);
    }

    // Title must be painted after the frame, since the areas overlap, and
    // the title may wish to extend out to all sides (eg. XP style)
    Q_D(QDockWidget);
    QStyleOptionDockWidget titleOpt = d->getStyleOption();
    style()->drawControl(QStyle::CE_DockWidgetTitle, &titleOpt, &p, this);
}

/*! \reimp */
bool QDockWidget::event(QEvent *event)
{
    Q_D(QDockWidget);

    QMainWindow *win = qobject_cast<QMainWindow*>(parentWidget());
    QMainWindowLayout *layout = 0;
    if (win != 0)
        layout = qobject_cast<QMainWindowLayout*>(win->layout());

    switch (event->type()) {
#ifndef QT_NO_ACTION
    case QEvent::Hide:
        if (layout != 0)
            layout->keepSize(this);
        if (!isHidden())
            break;
        // fallthrough intended
    case QEvent::Show:
        d->toggleViewAction->setChecked(event->type() == QEvent::Show);
        break;
#endif
    case QEvent::ApplicationLayoutDirectionChange:
    case QEvent::LayoutDirectionChange:
    case QEvent::StyleChange:
        d->updateButtons();
        break;
    case QEvent::ZOrderChange: {
        if (isFloating())
            break;
        if (layout == 0)
            break;
        layout->raise(this);
        break;
    }
    case QEvent::ContextMenu:
        if (d->state) {
            event->accept();
            return true;
        }
        break;
    case QEvent::Resize:
        d->relayout();
        break;
        // return true after calling the handler since we don't want
        // them to be passed onto the default handlers
    case QEvent::MouseButtonPress:
        d->mousePressEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonDblClick:
        d->mouseDoubleClickEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseMove:
        d->mouseMoveEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonRelease:
        d->mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::ChildRemoved:
        if (d->widget == static_cast<QChildEvent *>(event)->child())
            d->widget = 0;
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

#ifndef QT_NO_ACTION
/*!
  Returns a checkable action that can be used to show or close this
  dock widget.

  The action's text is set to the dock widget's window title.

  \sa QAction::text QWidget::windowTitle
 */
QAction * QDockWidget::toggleViewAction() const
{
    Q_D(const QDockWidget);
    return d->toggleViewAction;
}
#endif // QT_NO_ACTION

/*!
    \fn void QDockWidget::featuresChanged(QDockWidget::DockWidgetFeatures features)

    This signal is emitted when the \l features property changes. The
    \a features parameter gives the new value of the property.
*/

/*!
    \fn void QDockWidget::topLevelChanged(bool topLevel)

    This signal is emitted when the \l floating property changes.
    The \a topLevel parameter is true if the dock widget is now floating;
    otherwise it is false.

    \sa isWindow()
*/

/*!
    \fn void QDockWidget::allowedAreasChanged(Qt::DockWidgetAreas allowedAreas)

    This signal is emitted when the \l allowedAreas property changes. The
    \a allowedAreas parameter gives the new value of the property.
*/

#include "qdockwidget.moc"
#include "moc_qdockwidget.cpp"
#endif // QT_NO_DOCKWIDGET
