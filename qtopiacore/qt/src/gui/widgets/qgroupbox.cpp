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

#include "qgroupbox.h"
#ifndef QT_NO_GROUPBOX
#include "qapplication.h"
#include "qbitmap.h"
#include "qdrawutil.h"
#include "qevent.h"
#include "qlayout.h"
#include "qradiobutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qstylepainter.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif
#include <private/qwidget_p.h>

class QGroupBoxPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QGroupBox)
public:

    void skip();
    void init();
    void calculateFrame();
    QString title;
    int align;
#ifndef QT_NO_SHORTCUT
    int shortcutId;
#endif

    void _q_fixFocus(Qt::FocusReason reason);
    void _q_setChildrenEnabled(bool b);
    void click();
    bool flat;
    bool checkable;
    bool checked;
    bool hover;
    QStyle::SubControl pressedControl;

    QStyleOptionGroupBox getStyleOption() const;
};

QStyleOptionGroupBox QGroupBoxPrivate::getStyleOption() const
{
    Q_Q(const QGroupBox);
    QStyleOptionGroupBox option;
    option.init(q);
    option.text = title;
    option.lineWidth = 1;
    option.midLineWidth = 0;
    option.textAlignment = Qt::Alignment(align);
    option.activeSubControls |= pressedControl;
    option.subControls = QStyle::SC_GroupBoxFrame;

    if (hover)
        option.state |= QStyle::State_MouseOver;
    else
        option.state &= ~QStyle::State_MouseOver;

    if (flat)
        option.features |= QStyleOptionFrameV2::Flat;

    if (checkable) {
        option.subControls |= QStyle::SC_GroupBoxCheckBox;
        option.state |= (checked ? QStyle::State_On : QStyle::State_Off);
        if (pressedControl == QStyle::SC_GroupBoxCheckBox || pressedControl == QStyle::SC_GroupBoxLabel)
            option.state |= QStyle::State_Sunken;
    }

    if (!q->testAttribute(Qt::WA_SetPalette))
        option.textColor = QColor(q->style()->styleHint(QStyle::SH_GroupBox_TextLabelColor, &option, q));

    if (!title.isEmpty())
        option.subControls |= QStyle::SC_GroupBoxLabel;

    return option;
}

void QGroupBoxPrivate::click()
{
    Q_Q(QGroupBox);

    QPointer<QGroupBox> guard(q);
    q->setChecked(!checked);
    if (!guard)
        return;
    emit q->clicked(checked);
}

/*!
    \class QGroupBox
    \brief The QGroupBox widget provides a group box frame with a title.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance
    \mainclass

    A group box provides a frame, a title and a keyboard shortcut, and
    displays various other widgets inside itself. The title is on top,
    the keyboard shortcut moves keyboard focus to one of the group
    box's child widgets.

    QGroupBox also lets you set the \l title (normally set in the
    constructor) and the title's \l alignment. Group boxes can be
    \l checkable; child widgets in checkable group boxes are enabled or
    disabled depending on whether or not the group box is \l checked.

    You can minimize the space consumption of a group box by enabling
    the \l flat property. In most \l{QStyle}{styles}, enabling this
    property results in the removal of the left, right and bottom
    edges of the frame.

    QGroupBox doesn't automatically lay out the child widgets (which
    are often \l{QCheckBox}es or \l{QRadioButton}s but can be any
    widgets). The following example shows how we can set up a
    QGroupBox with a layout:

    \quotefromfile widgets/groupbox/window.cpp
    \skipto = new QGroupBox
    \printuntil setLayout(

    \table 100%
    \row \o \inlineimage windowsxp-groupbox.png Screenshot of a Windows XP style group box
         \o \inlineimage macintosh-groupbox.png Screenshot of a Macintosh style group box
         \o \inlineimage plastique-groupbox.png Screenshot of a Plastique style group box
    \row \o A \l{Windows XP Style Widget Gallery}{Windows XP style} group box.
         \o A \l{Macintosh Style Widget Gallery}{Macintosh style} group box.
         \o A \l{Plastique Style Widget Gallery}{Plastique style} group box.
    \endtable

    \sa QButtonGroup, {Group Box Example}
*/



/*!
    Constructs a group box widget with the given \a parent but with no title.
*/

QGroupBox::QGroupBox(QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
}

/*!
    Constructs a group box with the given \a title and \a parent.
*/

QGroupBox::QGroupBox(const QString &title, QWidget *parent)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    d->init();
    setTitle(title);
}


/*!
    Destroys the group box.
*/
QGroupBox::~QGroupBox()
{
}

void QGroupBoxPrivate::init()
{
    align = Qt::AlignLeft;
#ifndef QT_NO_SHORTCUT
    shortcutId = 0;
#endif
    flat = false;
    checkable = false;
    checked = true;
    hover = false;
    pressedControl = QStyle::SC_None;
    calculateFrame();
}


void QGroupBox::setTitle(const QString &title)
{
    Q_D(QGroupBox);
    if (d->title == title)                                // no change
        return;
    d->title = title;
#ifndef QT_NO_SHORTCUT
    releaseShortcut(d->shortcutId);
    d->shortcutId = grabShortcut(QKeySequence::mnemonic(title));
#endif
    d->calculateFrame();

    update();
    updateGeometry();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::NameChanged);
#endif
}

/*!
    \property QGroupBox::title
    \brief the group box title text

    The group box title text will have a keyboard shortcut if the title
    contains an ampersand ('&') followed by a letter.

    \code
        g->setTitle("&User information");
    \endcode

    In the example above, \key Alt+U moves the keyboard focus to the
    group box. See the \l {QShortcut#mnemonic}{QShortcut}
    documentation for details (to display an actual ampersand, use
    '&&').

    There is no default title text.

    \sa alignment
*/

QString QGroupBox::title() const
{
    Q_D(const QGroupBox);
    return d->title;
}

/*!
    \property QGroupBox::alignment
    \brief the alignment of the group box title.

    Most styles place the title at the top of the frame. The horizontal
    alignment of the title can be specified using single values from
    the following list:

    \list
    \i Qt::AlignLeft aligns the title text with the left-hand side of the group box.
    \i Qt::AlignRight aligns the title text with the right-hand side of the group box.
    \i Qt::AlignHCenter aligns the title text with the horizontal center of the group box.
    \endlist

    The default alignment is Qt::AlignLeft.

    \sa Qt::Alignment
*/
Qt::Alignment QGroupBox::alignment() const
{
    Q_D(const QGroupBox);
    return QFlag(d->align);
}

void QGroupBox::setAlignment(int alignment)
{
    Q_D(QGroupBox);
    d->align = alignment;
    updateGeometry();
    update();
}

/*! \reimp
*/
void QGroupBox::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    updateGeometry();
}

/*! \reimp
*/

void QGroupBox::paintEvent(QPaintEvent *)
{
    Q_D(QGroupBox);
    QStylePainter paint(this);
    paint.drawComplexControl(QStyle::CC_GroupBox, d->getStyleOption());
}

/*! \reimp  */
bool QGroupBox::event(QEvent *e)
{
    Q_D(QGroupBox);
#ifndef QT_NO_SHORTCUT
    if (e->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
        if (se->shortcutId() == d->shortcutId) {
            if (!isCheckable()) {
                d->_q_fixFocus(Qt::ShortcutFocusReason);
            } else {
                d->click();
                setFocus(Qt::ShortcutFocusReason);
            }
            return true;
        }
    }
#endif
    QStyleOptionGroupBox box = d->getStyleOption();
    switch (e->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
        QStyle::SubControl control = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                                    static_cast<QHoverEvent *>(e)->pos(),
                                                                    this);
        bool oldHover = d->hover;
        d->hover = d->checkable && (control == QStyle::SC_GroupBoxLabel || control == QStyle::SC_GroupBoxCheckBox);
        if (oldHover != d->hover) {
            QRect rect = style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this)
                         | style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxLabel, this);
            update(rect);
        }
        return true;
    }
    case QEvent::HoverLeave:
        d->hover = false;
        if (d->checkable) {
            QRect rect = style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this)
                         | style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxLabel, this);
            update(rect);
        }
        return true;
    case QEvent::KeyPress: {
        QKeyEvent *k = static_cast<QKeyEvent*>(e);
        if (!k->isAutoRepeat() && (k->key() == Qt::Key_Select || k->key() == Qt::Key_Space)) {
            d->pressedControl = QStyle::SC_GroupBoxCheckBox;
            update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
            return true;
        }
        break;
    }
    case QEvent::KeyRelease: {
        QKeyEvent *k = static_cast<QKeyEvent*>(e);
        if (!k->isAutoRepeat() && (k->key() == Qt::Key_Select || k->key() == Qt::Key_Space)) {
            bool toggle = (d->pressedControl == QStyle::SC_GroupBoxLabel
                           || d->pressedControl == QStyle::SC_GroupBoxCheckBox);
            d->pressedControl = QStyle::SC_None;
            if (toggle)
                d->click();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QWidget::event(e);
}

/*!\reimp */
void QGroupBox::childEvent(QChildEvent *c)
{
    Q_D(QGroupBox);
    if (c->type() != QEvent::ChildAdded || !c->child()->isWidgetType())
        return;
    QWidget *w = (QWidget*)c->child();
    if (d->checkable) {
        if (d->checked) {
            if (!w->testAttribute(Qt::WA_ForceDisabled))
                w->setEnabled(true);
        } else {
            if (w->isEnabled()) {
                w->setEnabled(false);
                w->setAttribute(Qt::WA_ForceDisabled, false);
            }
        }
    }
}


/*!
    \internal

    This private slot finds a widget in this group box that can accept
    focus, and gives the focus to that widget.
*/

void QGroupBoxPrivate::_q_fixFocus(Qt::FocusReason reason)
{
    Q_Q(QGroupBox);
    QWidget *fw = q->focusWidget();
    if (!fw) {
        QWidget * best = 0;
        QWidget * candidate = 0;
        QWidget * w = q;
        while ((w = w->nextInFocusChain()) != q) {
            if (q->isAncestorOf(w) && (w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus && w->isVisibleTo(q)) {
                if (!best && qobject_cast<QRadioButton*>(w) && ((QRadioButton*)w)->isChecked())
                    // we prefer a checked radio button or a widget that
                    // already has focus, if there is one
                    best = w;
                else
                    if (!candidate)
                        // but we'll accept anything that takes focus
                        candidate = w;
            }
        }
        if (best)
            fw = best;
        else
            if (candidate)
                fw = candidate;
    }
    if (fw)
        fw->setFocus(reason);
}


/*
    Sets the right frame rect depending on the title.
*/
void QGroupBoxPrivate::calculateFrame()
{
    Q_Q(QGroupBox);
    QStyleOptionGroupBox box = getStyleOption();
    QRect contentsRect = q->style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxContents, q);
    q->setContentsMargins(contentsRect.left() - box.rect.left(), contentsRect.top() - box.rect.top(),
                          box.rect.right() - contentsRect.right(), box.rect.bottom() - contentsRect.bottom());
}



/*! \reimp
 */
void QGroupBox::focusInEvent(QFocusEvent *fe)
{ // note no call to super
    Q_D(QGroupBox);
    if (focusPolicy() == Qt::NoFocus) {
        d->_q_fixFocus(fe->reason());
    } else {
        QStyleOptionGroupBox box = d->getStyleOption();
        QRect rect = style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this)
            | style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxLabel, this);
        update(rect);
    }
}


/*!
  \reimp
*/
QSize QGroupBox::minimumSizeHint() const
{
    Q_D(const QGroupBox);
    QStyleOptionGroupBox option = d->getStyleOption();

    int baseWidth = fontMetrics().width(d->title + QLatin1Char(' '));
    int baseHeight = fontMetrics().height();
    if (d->checkable) {
        baseWidth += style()->pixelMetric(QStyle::PM_IndicatorWidth);
        baseWidth += style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
        baseHeight = qMax(baseHeight, style()->pixelMetric(QStyle::PM_IndicatorHeight));
    }

    QSize size = QWidget::minimumSizeHint().expandedTo(QSize(baseWidth, baseHeight));
    return style()->sizeFromContents(QStyle::CT_GroupBox, &option, size, this);
}

/*!
    \property QGroupBox::flat
    \brief whether the group box is painted flat or has a frame

    A group box usually consists of a surrounding frame with a title
    at the top. If this property is enabled, only the top part of the frame is
    drawn in most styles; otherwise the whole frame is drawn.

    By default, this property is disabled; i.e. group boxes are not flat unless
    explicitly specified.

    \bold{Note:} In some styles, flat and non-flat group boxes have similar
    representations and may not be as distinguishable as they are in other
    styles.

    \sa title
*/
bool QGroupBox::isFlat() const
{
    Q_D(const QGroupBox);
    return d->flat;
}

void QGroupBox::setFlat(bool b)
{
    Q_D(QGroupBox);
    if (d->flat == b)
        return;
    d->flat = b;
    updateGeometry();
    update();
}


/*!
    \property QGroupBox::checkable
    \brief whether the group box has a checkbox in its title

    If this property is true, the group box displays its title using
    a checkbox in place of an ordinary label. If the checkbox is checked,
    the group box's children are enabled; otherwise they are disabled and
    inaccessible.

    By default, group boxes are not checkable.

    If this property is enabled for a group box, it will also be initially
    checked to ensure that its contents are enabled.

    \sa checked
*/
void QGroupBox::setCheckable(bool checkable)
{
    Q_D(QGroupBox);

    bool wasCheckable = d->checkable;
    d->checkable = checkable;

    if (checkable) {
        setChecked(true);
        if (!wasCheckable) {
            setFocusPolicy(Qt::StrongFocus);
            d->_q_setChildrenEnabled(true);
            updateGeometry();
        }
    } else {
        if (wasCheckable) {
            setFocusPolicy(Qt::NoFocus);
            d->_q_setChildrenEnabled(true);
            updateGeometry();
        }
        d->_q_setChildrenEnabled(true);
    }

    if (wasCheckable != checkable)
        update();
}

bool QGroupBox::isCheckable() const
{
    Q_D(const QGroupBox);
    return d->checkable;
}


bool QGroupBox::isChecked() const
{
    Q_D(const QGroupBox);
    return d->checkable && d->checked;
}


/*!
    \fn void QGroupBox::toggled(bool on)

    If the group box is checkable, this signal is emitted when the check box
    is toggled. \a on is true if the check box is checked; otherwise it is false.

    \sa checkable
*/


/*!
    \fn void QGroupBox::clicked(bool checked)
    \since 4.2

    This signal is emitted when the check box is activated (i.e. pressed down
    then released while the mouse cursor is inside the button), or when the
    shortcut key is typed, Notably, this signal is \e not emitted if you call
    setChecked().

    If the check box is checked \a checked is true; it is false if the check
    box is unchecked.

    \sa checkable, toggled(), checked
*/

/*!
    \property QGroupBox::checked
    \brief whether the group box is checked

    If the group box is checkable, it is displayed with a check box.
    If the check box is checked, the group box's children are enabled;
    otherwise the children are disabled and are inaccessible to the user.

    By default, checkable group boxes are also checked.

    \sa checkable
*/
void QGroupBox::setChecked(bool b)
{
    Q_D(QGroupBox);
    if (d->checkable) {
        if (d->checked != b)
            update();
        bool wasToggled = (b != d->checked);
        d->checked = b;
        if (wasToggled) {
            d->_q_setChildrenEnabled(b);
            emit toggled(b);
        }
    }
}

/*
  sets all children of the group box except the qt_groupbox_checkbox
  to either disabled/enabled
*/
void QGroupBoxPrivate::_q_setChildrenEnabled(bool b)
{
    Q_Q(QGroupBox);
    QObjectList childList = q->children();
    for (int i = 0; i < childList.size(); ++i) {
        QObject *o = childList.at(i);
        if (o->isWidgetType()) {
            QWidget *w = static_cast<QWidget *>(o);
            if (b) {
                if (!w->testAttribute(Qt::WA_ForceDisabled))
                    w->setEnabled(true);
            } else {
                if (w->isEnabled()) {
                    w->setEnabled(false);
                    w->setAttribute(Qt::WA_ForceDisabled, false);
                }
            }
        }
    }
}

/*! \reimp */
void QGroupBox::changeEvent(QEvent *ev)
{
    Q_D(QGroupBox);
    if(ev->type() == QEvent::EnabledChange) {
        if (d->checkable && isEnabled()) {
            // we are being enabled - disable children
            if (!d->checked)
                d->_q_setChildrenEnabled(false);
        }
    } else if(ev->type() == QEvent::FontChange || ev->type() == QEvent::StyleChange) {
        updateGeometry();
        d->calculateFrame();
    }
    QWidget::changeEvent(ev);
}

/*! \reimp */
void QGroupBox::mousePressEvent(QMouseEvent *event)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    d->pressedControl = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                       event->pos(), this);
    if (d->checkable && (d->pressedControl & (QStyle::SC_GroupBoxCheckBox | QStyle::SC_GroupBoxLabel)))
        update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
}

/*! \reimp */
void QGroupBox::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    QStyle::SubControl pressed = style()->hitTestComplexControl(QStyle::CC_GroupBox, &box,
                                                                event->pos(), this);
    if (d->pressedControl == QStyle::SC_GroupBoxCheckBox && d->pressedControl != pressed)
        update(style()->subControlRect(QStyle::CC_GroupBox, &box, QStyle::SC_GroupBoxCheckBox, this));
}

/*! \reimp */
void QGroupBox::mouseReleaseEvent(QMouseEvent *)
{
    Q_D(QGroupBox);
    QStyleOptionGroupBox box = d->getStyleOption();
    bool toggle = d->checkable && (d->pressedControl == QStyle::SC_GroupBoxLabel
                   || d->pressedControl == QStyle::SC_GroupBoxCheckBox);
    d->pressedControl = QStyle::SC_None;
    if (toggle)
        d->click();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGroupBox::QGroupBox(const QString &title, QWidget *parent, const char *name)
    : QWidget(*new QGroupBoxPrivate, parent, 0)
{
    Q_D(QGroupBox);
    setObjectName(QString::fromAscii(name));
    d->init();
    setTitle(title);
}
#endif // QT3_SUPPORT

#include "moc_qgroupbox.cpp"

#endif //QT_NO_GROUPBOX
