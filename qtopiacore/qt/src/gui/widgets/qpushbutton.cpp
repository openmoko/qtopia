/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qapplication.h"
#include "qbitmap.h"
#include "qdesktopwidget.h"
#include "qdialog.h"
#include <private/qdialog_p.h>
#include "qdrawutil.h"
#include "qevent.h"
#include "qfontmetrics.h"
#include "qmenu.h"
#include "qstylepainter.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qpushbutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtoolbar.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#include "private/qabstractbutton_p.h"


class QPushButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QPushButton)
public:
    enum AutoDefaultValue {Off = 0, On = 1, Auto = 2};
    QPushButtonPrivate():autoDefault(Auto), defaultButton(false), flat(false), menuOpen(false){}
    void init();
    void _q_popupPressed();
    QStyleOptionButton getStyleOption() const;
    QDialog *dialogParent() const;
    QPointer<QMenu> menu;
    uint autoDefault : 2;
    uint defaultButton : 1;
    uint flat : 1;
    uint menuOpen : 1;
};

/*!
    \class QPushButton qpushbutton.h
    \brief The QPushButton widget provides a command button.

    \ingroup basic
    \mainclass

    The push button, or command button, is perhaps the most commonly
    used widget in any graphical user interface. Push (click) a button
    to command the computer to perform some action, or to answer a
    question. Typical buttons are OK, Apply, Cancel, Close, Yes, No
    and Help.

    A command button is rectangular and typically displays a text
    label describing its action. A shortcut key can be specified by
    preceding the preferred character with an ampersand in the
    text. For example:

    \code
        QPushButton *button = new QPushButton("&Download", this);
    \endcode

    In this example the shortcut is \e{Alt+D}. See the \l
    {QShortcut#mnemonic}{QShortcut} documentation for details (to
    display an actual ampersand, use '&&').

    Push buttons display a textual label, and optionally a small
    icon. These can be set using the constructors and changed later
    using setText() and setIcon().  If the button is disabled the
    appearance of the text and icon will be manipulated with respect
    to the GUI style to make the button look "disabled".

    A push button emits the signal clicked() when it is activated by
    the mouse, the Spacebar or by a keyboard shortcut. Connect to
    this signal to perform the button's action. Push buttons also
    provide less commonly used signals, for example, pressed() and
    released().

    Command buttons in dialogs are by default auto-default buttons,
    i.e. they become the default push button automatically when they
    receive the keyboard input focus. A default button is a push
    button that is activated when the user presses the Enter or Return
    key in a dialog. You can change this with setAutoDefault(). Note
    that auto-default buttons reserve a little extra space which is
    necessary to draw a default-button indicator. If you do not want
    this space around your buttons, call setAutoDefault(false).

    Being so central, the button widget has grown to accommodate a
    great many variations in the past decade. The Microsoft style
    guide now shows about ten different states of Windows push buttons
    and the text implies that there are dozens more when all the
    combinations of features are taken into consideration.

    The most important modes or states are:
    \list
    \i Available or not (grayed out, disabled).
    \i Standard push button, toggling push button or menu button.
    \i On or off (only for toggling push buttons).
    \i Default or normal. The default button in a dialog can generally
       be "clicked" using the Enter or Return key.
    \i Auto-repeat or not.
    \i Pressed down or not.
    \endlist

    As a general rule, use a push button when the application or
    dialog window performs an action when the user clicks on it (such
    as Apply, Cancel, Close and Help) \e and when the widget is
    supposed to have a wide, rectangular shape with a text label.
    Small, typically square buttons that change the state of the
    window rather than performing an action (such as the buttons in
    the top-right corner of the QFileDialog) are not command buttons,
    but tool buttons. Qt provides a special class (QToolButton) for
    these buttons.

    If you need toggle behavior (see setCheckable()) or a button
    that auto-repeats the activation signal when being pushed down
    like the arrows in a scroll bar (see setAutoRepeat()), a command
    button is probably not what you want. When in doubt, use a tool
    button.

    A variation of a command button is a menu button. These provide
    not just one command, but several, since when they are clicked
    they pop up a menu of options. Use the method setMenu() to
    associate a popup menu with a push button.

    Other classes of buttons are option buttons (see QRadioButton) and
    check boxes (see QCheckBox).

    \table 100%
    \row \o \inlineimage macintosh-pushbutton.png Screenshot of a Macintosh style push button
         \o A push button shown in the \l{Macintosh Style Widget Gallery}{Macintosh widget style}.

         Note that when a button's width becomes smaller than 50 or
         its height becomes smaller than 30, the button's corners are
         changed from round to square. Use the setMinimumSize()
         function to prevent this behavior.

    \row \o \inlineimage windowsxp-pushbutton.png Screenshot of a Windows XP style push button
         \o A push button shown in the \l{Windows XP Style Widget Gallery}{Windows XP widget style}.
    \row \o \inlineimage plastique-pushbutton.png Screenshot of a Plastique style push button
         \o A push button shown in the \l{Plastique Style Widget Gallery}{Plastique widget style}.
    \endtable

    In Qt, the QAbstractButton base class provides most of the modes
    and other API, and QPushButton provides GUI logic.
    See QAbstractButton for more information about the API.

    \sa QToolButton, QRadioButton, QCheckBox, {fowler}{GUI Design Handbook: Push Button}
*/

/*!
    \property QPushButton::autoDefault
    \brief whether the push button is the auto default button

    If this property is set to true then the push button is the auto
    default button in a dialog.

    In some GUI styles a default button is drawn with an extra frame
    around it, up to 3 pixels or more. Qt automatically keeps this
    space free around auto-default buttons, i.e. auto-default buttons
    may have a slightly larger size hint.

    This property's default is true for buttons that have a QDialog
    parent; otherwise it defaults to false.

    See the \l default property for details of how \l default and
    auto-default interact.
*/

/*!
    \property QPushButton::default
    \brief whether the push button is the default button

    If this property is set to true then the push button will be
    pressed if the user presses the Enter (or Return) key in a dialog.

    Regardless of focus, if the user presses Enter: If there is a
    default button the default button is pressed; otherwise, if
    there are one or more \l autoDefault buttons the first \l autoDefault
    button that is next in the tab order is pressed. If there are no
    default or \l autoDefault buttons only pressing Space on a button
    with focus, mouse clicking, or using a shortcut will press a
    button.

    In a dialog, only one push button at a time can be the default
    button. This button is then displayed with an additional frame
    (depending on the GUI style).

    The default button behavior is provided only in dialogs. Buttons
    can always be clicked from the keyboard by pressing Spacebar when
    the button has focus.

    This property's default is false.
*/

/*!
    \property QPushButton::flat
    \brief whether the border is disabled

    This property's default is false.
*/



/*!
    Constructs a push button with no text and a \a parent.
*/

QPushButton::QPushButton(QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    d->init();
}

/*!
    Constructs a push button with the parent \a parent and the text \a
    text.
*/

QPushButton::QPushButton(const QString &text, QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    d->init();
    setText(text);
}


/*!
    Constructs a push button with an \a icon and a \a text, and a \a parent.

    Note that you can also pass a QPixmap object as an icon (thanks to
    the implicit type conversion provided by C++).

*/
QPushButton::QPushButton(const QIcon& icon, const QString &text, QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    d->init();
    setText(text);
    setIcon(icon);
}


/*!
    Destroys the push button.
*/
QPushButton::~QPushButton()
{
}

QDialog *QPushButtonPrivate::dialogParent() const
{
    Q_Q(const QPushButton);
    const QWidget *p = q;
    while (p && !p->isWindow()) {
        p = p->parentWidget();
        if (const QDialog *dialog = qobject_cast<const QDialog *>(p))
            return const_cast<QDialog *>(dialog);
    }
    return 0;
}

void QPushButtonPrivate::init()
{
    Q_Q(QPushButton);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QStyleOptionButton QPushButtonPrivate::getStyleOption() const
{
    Q_Q(const QPushButton);
    QStyleOptionButton opt;
    opt.init(q);
    opt.features = QStyleOptionButton::None;
    if (flat)
        opt.features |= QStyleOptionButton::Flat;
#ifndef QT_NO_MENU
    if (menu)
        opt.features |= QStyleOptionButton::HasMenu;
#endif
    if (q->autoDefault() || defaultButton)
        opt.features |= QStyleOptionButton::AutoDefaultButton;
    if (defaultButton)
        opt.features |= QStyleOptionButton::DefaultButton;
    if (down || menuOpen)
        opt.state |= QStyle::State_Sunken;
    if (checked)
        opt.state |= QStyle::State_On;
    if (!flat && !down)
        opt.state |= QStyle::State_Raised;
    opt.text = text;
    opt.icon = icon;
    opt.iconSize = q->iconSize();
    return opt;
}

void QPushButton::setAutoDefault(bool enable)
{
    Q_D(QPushButton);
    uint state = enable ? QPushButtonPrivate::On : QPushButtonPrivate::Off;
    if (d->autoDefault != QPushButtonPrivate::Auto && d->autoDefault == state)
        return;
    d->autoDefault = state;
    update();
    updateGeometry();
}

bool QPushButton::autoDefault() const
{
    Q_D(const QPushButton);
    if(d->autoDefault == QPushButtonPrivate::Auto)
        return ( d->dialogParent() != 0 );
    return d->autoDefault;
}

void QPushButton::setDefault(bool enable)
{
    Q_D(QPushButton);
    if (d->defaultButton == enable)
        return;
    d->defaultButton = enable;
    if (d->defaultButton) {
        if (QDialog *dlg = d->dialogParent())
            dlg->d_func()->setMainDefault(this);
    }
    update();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
#endif
}

bool QPushButton::isDefault() const
{
    Q_D(const QPushButton);
    return d->defaultButton;
}

/*!
    \reimp
*/
QSize QPushButton::sizeHint() const
{
    Q_D(const QPushButton);
    ensurePolished();

    int w = 0, h = 0;

    QStyleOptionButton opt = d->getStyleOption();

    // calculate contents size...
#ifndef QT_NO_ICON
    if (!icon().isNull()) {
        int ih = opt.iconSize.height();
        int iw = opt.iconSize.width() + 4;
        w += iw;
        h = qMax(h, ih);
    }
#endif
#ifndef QT_NO_MENU
    if (menu())
        w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
#endif
    QString s(text());
    bool empty = s.isEmpty();
    if (empty)
        s = QString::fromLatin1("XXXX");
    QFontMetrics fm = fontMetrics();
    QSize sz = fm.size(Qt::TextShowMnemonic, s);
    if(!empty || !w)
        w += sz.width();
    if(!empty || !h)
        h = qMax(h, sz.height());
    return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this).
            expandedTo(QApplication::globalStrut()));
}



/*!\reimp
*/
void QPushButton::paintEvent(QPaintEvent *)
{
    Q_D(QPushButton);
    QStylePainter p(this);
    p.drawControl(QStyle::CE_PushButton, d->getStyleOption());
}


/*! \reimp */
void QPushButton::keyPressEvent(QKeyEvent *e)
{
    Q_D(QPushButton);
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (autoDefault() || d->defaultButton) {
            click();
            break;
        }
        // fall through
    default:
        QAbstractButton::keyPressEvent(e);
    }
}

/*!
    \reimp
*/
void QPushButton::focusInEvent(QFocusEvent *e)
{
    Q_D(QPushButton);
    if (e->reason() != Qt::PopupFocusReason && autoDefault() && !d->defaultButton) {
        d->defaultButton = true;
        QDialog *dlg = qobject_cast<QDialog*>(window());
        if (dlg)
            dlg->d_func()->setDefault(this);
    }
    QAbstractButton::focusInEvent(e);
}

/*!
    \reimp
*/
void QPushButton::focusOutEvent(QFocusEvent *e)
{
    Q_D(QPushButton);
    if (e->reason() != Qt::PopupFocusReason && autoDefault() && d->defaultButton) {
        QDialog *dlg = qobject_cast<QDialog*>(window());
        if (dlg)
            dlg->d_func()->setDefault(0);
        else
            d->defaultButton = false;
    }

    QAbstractButton::focusOutEvent(e);
#ifndef QT_NO_MENU
    if (d->menu && d->menu->isVisible())        // restore pressed status
        setDown(true);
#endif
}

#ifndef QT_NO_MENU
/*!
    Associates the popup menu \a menu with this push button. This
    turns the button into a menu button, which in some styles will
    produce a small triangle to the right of the button's text.

    Ownership of the menu is \e not transferred to the push button.

    \table 100%
    \row
    \o \inlineimage plastique-pushbutton-menu.png Screenshot of a Plastique style push button with popup menu.
    \o \inlineimage cleanlooks-pushbutton-menu.png Screenshot of a Cleanlooks style push button with popup menu.
    \o Push buttons with popup menus shown in the \l{Plastique Style Widget Gallery}{Plastique widget style}
    (left) and \l{Cleanlooks Style Widget Gallery}{Cleanlooks widget style} (right).
    \endtable

    \sa menu()
*/
void QPushButton::setMenu(QMenu* menu)
{
    Q_D(QPushButton);
    if (menu == d->menu)
        return;

    if (menu && !d->menu) {
        disconnect(this, SIGNAL(pressed()), this, SLOT(_q_popupPressed()));
        connect(this, SIGNAL(pressed()), this, SLOT(_q_popupPressed()));
    }
    if (d->menu)
        removeAction(d->menu->menuAction());
    d->menu = menu;
    if (d->menu)
        addAction(d->menu->menuAction());
    update();
    updateGeometry();
}

/*!
    Returns the button's associated popup menu or 0 if no popup menu
    has been set.

    \sa setMenu()
*/
QMenu* QPushButton::menu() const
{
    Q_D(const QPushButton);
    return d->menu;
}

/*!
    Shows (pops up) the associated popup menu. If there is no such
    menu, this function does nothing. This function does not return
    until the popup menu has been closed by the user.
*/
void QPushButton::showMenu()
{
    Q_D(QPushButton);
    if (!d || !d->menu)
        return;
    setDown(true);
    d->_q_popupPressed();
}

void QPushButtonPrivate::_q_popupPressed()
{
    Q_Q(QPushButton);
    if (!down || !menu)
        return;

    menu->setNoReplayFor(q);
    bool horizontal = true;
#if !defined(QT_NO_TOOLBAR)
    QToolBar *tb = qobject_cast<QToolBar*>(q->parentWidget());
    if (tb && tb->orientation() == Qt::Vertical)
        horizontal = false;
#endif
    QRect rect = q->rect();
    QSize menuSize = menu->sizeHint();
    QPoint globalPos = q->mapToGlobal(rect.topLeft());
    int x = globalPos.x();
    int y = globalPos.y();
    if (horizontal) {
        if (globalPos.y() + rect.height() + menuSize.height() <= qApp->desktop()->height()) {
            y += rect.height();
        } else {
            y -= menuSize.height();
        }
        if (q->layoutDirection() == Qt::RightToLeft)
            x += rect.width() - menuSize.width();
    } else {
        if (globalPos.x() + rect.width() + menu->sizeHint().width() <= qApp->desktop()->width())
            x += rect.width();
        else
            x -= menuSize.width();
    }
    QPointer<QPushButton> guard(q);

    //Because of a delay in menu effects, we must keep track of the
    //menu visibility to avoid flicker on button release
    menuOpen = true;
    menu->exec(QPoint(x, y));
    if (guard) {
        menuOpen = false;
        q->setDown(false);
    }
}
#endif // QT_NO_MENU

void QPushButton::setFlat(bool flat)
{
    Q_D(QPushButton);
    if (d->flat == flat)
        return;
    d->flat = flat;
    update();
    updateGeometry();
}

bool QPushButton::isFlat() const
{
    Q_D(const QPushButton);
    return d->flat;
}

/*! \reimp */
bool QPushButton::event(QEvent *e)
{
    return QAbstractButton::event(e);
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QPushButton::QPushButton(QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QPushButton::QPushButton(const QString &text, QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    setObjectName(QString::fromAscii(name));
    d->init();
    setText(text);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QPushButton::QPushButton(const QIcon& icon, const QString &text, QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    Q_D(QPushButton);
    setObjectName(QString::fromAscii(name));
    d->init();
    setText(text);
    setIcon(icon);
}
#endif

/*!
    \fn void QPushButton::openPopup()

    Use showMenu() instead.
*/

/*!
    \fn bool QPushButton::isMenuButton() const

    Use menu() != 0 instead.
*/

/*!
    \fn void QPushButton::setPopup(QMenu* popup)

    Use setMenu() instead.
*/

/*!
    \fn QMenu* QPushButton::popup() const

    Use menu() instead.
*/


#include "moc_qpushbutton.cpp"
