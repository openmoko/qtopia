/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "rangecontrols.h"

#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qdebug.h>
#include <qglobal.h>

#ifndef QT_NO_ACCESSIBILITY
extern QString Q_GUI_EXPORT qt_accStripAmp(const QString &text);
#ifndef QT_NO_SCROLLBAR
extern QStyleOptionSlider Q_GUI_EXPORT qt_qscrollbarStyleOption(QScrollBar *scrollBar);
#endif
#ifndef QT_NO_SLIDER
extern QStyleOptionSlider Q_GUI_EXPORT qt_qsliderStyleOption(QSlider *slider);
#endif

#ifndef QT_NO_SPINBOX
/*!
  \class QAccessibleSpinBox qaccessiblewidget.h
  \brief The QAccessibleSpinBox class implements the QAccessibleInterface for spinbox widgets.
  \internal

  \ingroup accessibility
*/

/*!
    \enum QAccessibleSpinBox::SpinBoxElements

    This enum identifies the components of the spin box.

    \value SpinBoxSelf The spin box as a whole
    \value Editor The line edit sub-widget.
    \value ValueUp The up sub-widget (i.e. the up arrow or + button)
    \value ValueDown The down sub-widget (i.e. the down arrow or - button)
*/

/*!
  Constructs a QAccessibleSpinWidget object for \a w.
*/
QAccessibleSpinBox::QAccessibleSpinBox(QWidget *w)
: QAccessibleWidget(w, SpinBox)
{
    Q_ASSERT(spinBox());
    addControllingSignal("valueChanged(int)");
    addControllingSignal("valueChanged(QString)");
}

/*!
    Returns the underlying QSpinBox.
*/
QSpinBox *QAccessibleSpinBox::spinBox() const
{
    return qobject_cast<QSpinBox*>(object());
}

/*! \reimp */
int QAccessibleSpinBox::childCount() const
{
    return ValueDown;
}

/*! \reimp */
QRect QAccessibleSpinBox::rect(int child) const
{
    QRect rect;
    QStyleOptionSpinBox so;
    so.rect = widget()->rect();
    switch(child) {
    case Editor:
        rect = widget()->style()->subControlRect(QStyle::CC_SpinBox, &so,
                                                 QStyle::SC_SpinBoxEditField, widget());
        break;
    case ValueUp:
        rect = widget()->style()->subControlRect(QStyle::CC_SpinBox, &so,
                                                 QStyle::SC_SpinBoxUp, widget());
        break;
    case ValueDown:
        rect = widget()->style()->subControlRect(QStyle::CC_SpinBox, &so,
                                                 QStyle::SC_SpinBoxDown, widget());
        break;
    default:
        rect = so.rect;
        break;
    }
    QPoint tl = widget()->mapToGlobal(QPoint(0, 0));
    return QRect(tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSpinBox::navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;

    if (entry) switch (rel) {
    case Child:
        return entry <= childCount() ? entry : -1;
    case QAccessible::Left:
        return (entry == ValueUp || entry == ValueDown) ? Editor : -1;
    case QAccessible::Right:
        return entry == Editor ? ValueUp : -1;
    case QAccessible::Up:
        return entry == ValueDown ? ValueUp : -1;
    case QAccessible::Down:
        return entry == ValueUp ? ValueDown : -1;
    default:
        break;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
}

/*! \reimp */
QString QAccessibleSpinBox::text(Text t, int child) const
{
    switch (t) {
    case Name:
        switch (child) {
        case ValueUp:
            return QSpinBox::tr("More");
        case ValueDown:
            return QSpinBox::tr("Less");
        }
        break;
    case Value:
        if (child == Editor || child == SpinBoxSelf)
            return spinBox()->text();
        break;
    default:
        break;
    }
    return QAccessibleWidget::text(t, 0);
}

/*! \reimp */
QAccessible::Role QAccessibleSpinBox::role(int child) const
{
    switch(child) {
    case Editor:
        return EditableText;
    case ValueUp:
    case ValueDown:
        return PushButton;
    default:
        break;
    }
    return QAccessibleWidget::role(child);
}

/*! \reimp */
QAccessible::State QAccessibleSpinBox::state(int child) const
{
    State state = QAccessibleWidget::state(child);
    switch(child) {
    case ValueUp:
        if (spinBox()->value() >= spinBox()->maximum())
            state |= Unavailable;
        return state;
    case ValueDown:
        if (spinBox()->value() <= spinBox()->minimum())
            state |= Unavailable;
        return state;
    default:
        break;
    }
    return state;
}

/*! \reimp */
bool QAccessibleSpinBox::doAction(int action, int /*child*/, const QVariantList &params)
{
    if (!widget()->isEnabled())
        return false;

/* // ### vohi - what's that code?
    if (action == Press) switch(child) {
    case ValueUp:
        if (spinBox()->value() >= spinBox()->maxValue())
            return false;
        spinBox()->stepUp();
        return true;
    case ValueDown:
        if (spinBox()->value() <= spinBox()->minValue())
            return false;
        spinBox()->stepDown();
        return true;
    default:
        break;
    }
    */
    return QAccessibleWidget::doAction(action, 0, params);
}
#endif // QT_NO_SPINBOX

#ifndef QT_NO_SCROLLBAR
/*!
  \class QAccessibleScrollBar qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for scroll bars.
  \internal

  \ingroup accessibility
*/

/*!
    \enum QAccessibleScrollBar::ScrollBarElements

    This enum identifies the components of the scroll bar.

    \value ScrollBarSelf The scroll bar as a whole.
    \value LineUp The up arrow button.
    \value PageUp The area between the position and the up arrow button.
    \value Position The position marking rectangle.
    \value PageDown The area between the position and the down arrow button.
    \value LineDown The down arrow button.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a w.
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollBar::QAccessibleScrollBar(QWidget *w, const QString &name)
: QAccessibleWidget(w, ScrollBar, name)
{
    Q_ASSERT(scrollBar());
    addControllingSignal("valueChanged(int)");
}

/*! Returns the scroll bar. */
QScrollBar *QAccessibleScrollBar::scrollBar() const
{
    return qobject_cast<QScrollBar*>(object());
}

/*! \reimp */
QRect QAccessibleScrollBar::rect(int child) const
{
    QStyle::SubControl subControl;
    switch (child) {
    case LineUp:
        subControl = QStyle ::SC_ScrollBarSubLine;
        break;
    case PageUp:
        subControl = QStyle::SC_ScrollBarSubPage;
        break;
    case Position:
        subControl = QStyle::SC_ScrollBarSlider;
        break;
    case PageDown:
        subControl = QStyle::SC_ScrollBarAddPage;
        break;
    case LineDown:
        subControl = QStyle::SC_ScrollBarAddLine;
        break;
    default:
        return QAccessibleWidget::rect(child);
    }

    const QStyleOptionSlider option = qt_qscrollbarStyleOption(scrollBar());
    const QRect rect = scrollBar()->style()->subControlRect(QStyle::CC_ScrollBar, &option,
                                                       subControl, scrollBar());

    const QPoint tp = scrollBar()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleScrollBar::childCount() const
{
    return LineDown;
}

/*! \reimp */
QString        QAccessibleScrollBar::text(Text t, int child) const
{
    switch (t) {
    case Value:
        if (!child || child == Position)
            return QString::number(scrollBar()->value());
        return QString();
    case Name:
        switch (child) {
        case LineUp:
            return QScrollBar::tr("Line up");
        case PageUp:
            return QScrollBar::tr("Page up");
        case Position:
            return QScrollBar::tr("Position");
        case PageDown:
            return QScrollBar::tr("Page down");
        case LineDown:
            return QScrollBar::tr("Line down");
        }
        break;
    default:
        break;
    }
    return QAccessibleWidget::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleScrollBar::role(int child) const
{
    switch (child) {
    case LineUp:
    case PageUp:
    case PageDown:
    case LineDown:
        return PushButton;
    case Position:
        return Indicator;
    default:
        return ScrollBar;
    }
}

/*! \reimp */
QAccessible::State QAccessibleScrollBar::state(int child) const
{
    const State parentState = QAccessibleWidget::state(0);

    if (child == 0)
        return parentState;

    // Inherit the Invisible state from parent.
    State state = parentState & QAccessible::Invisible;

    // Disable left/right if we are at the minimum/maximum.
    const QScrollBar * const scrollBar = QAccessibleScrollBar::scrollBar();
    switch (child) {
    case LineUp:
    case PageUp:
        if (scrollBar->value() <= scrollBar->minimum())
            state |= Unavailable;
        break;
    case LineDown:
    case PageDown:
        if (scrollBar->value() >= scrollBar->maximum())
            state |= Unavailable;
        break;
    case Position:
    default:
        break;
    }

    return state;
}

/*! \reimp */
bool QAccessibleScrollBar::doAction(int /*action*/, int /*child*/, const QVariantList &/*params*/)
{
/*
    if (action == Press) switch (child) {
    case LineUp:
        scrollBar()->subtractLine();
        return true;
    case PageUp:
        scrollBar()->subtractPage();
        return true;
    case PageDown:
        scrollBar()->addPage();
        return true;
    case LineDown:
        scrollBar()->addLine();
        return true;
    }
*/
    return false;
}
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_SLIDER
/*!
  \class QAccessibleSlider qaccessiblewidget.h
  \brief The QAccessibleSlider class implements the QAccessibleInterface for sliders.
  \internal

  \ingroup accessibility
*/

/*!
    \enum QAccessibleSlider::SliderElements

    This enum identifies the components of the slider.

    \value SliderSelf The slider as a whole.
    \value PageLeft The area to the left of the position.
    \value Position The position indicator.
    \value PageRight The area to the right of the position.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a w.
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleSlider::QAccessibleSlider(QWidget *w, const QString &name)
: QAccessibleWidget(w, Slider, name)
{
    Q_ASSERT(slider());
    addControllingSignal("valueChanged(int)");
}

/*! Returns the slider. */
QSlider *QAccessibleSlider::slider() const
{
    return qobject_cast<QSlider*>(object());
}

/*! \reimp */
QRect QAccessibleSlider::rect(int child) const
{
    QRect rect;
    const QStyleOptionSlider option = qt_qsliderStyleOption(slider());
    QRect srect = slider()->style()->subControlRect(QStyle::CC_Slider, &option,
                                                    QStyle::SC_SliderHandle, slider());

    switch (child) {
    case PageLeft:
        if (slider()->orientation() == Qt::Vertical)
            rect = QRect(0, 0, slider()->width(), srect.y());
        else
            rect = QRect(0, 0, srect.x(), slider()->height());
        break;
    case Position:
        rect = srect;
        break;
    case PageRight:
        if (slider()->orientation() == Qt::Vertical)
            rect = QRect(0, srect.y() + srect.height(), slider()->width(), slider()->height()- srect.y() - srect.height());
        else
            rect = QRect(srect.x() + srect.width(), 0, slider()->width() - srect.x() - srect.width(), slider()->height());
        break;
    default:
        return QAccessibleWidget::rect(child);
    }

    QPoint tp = slider()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSlider::childCount() const
{
    return PageRight;
}

/*! \reimp */
QString        QAccessibleSlider::text(Text t, int child) const
{
    switch (t) {
    case Value:
        if (!child || child == 2)
            return QString::number(slider()->value());
        return QString();
    case Name:
        switch (child) {
        case PageLeft:
            return slider()->orientation() == Qt::Horizontal ?
                QSlider::tr("Page left") : QSlider::tr("Page up");
        case Position:
            return QSlider::tr("Position");
        case PageRight:
            return slider()->orientation() == Qt::Horizontal ?
                QSlider::tr("Page right") : QSlider::tr("Page down");
        }
        break;
    default:
        break;
    }
    return QAccessibleWidget::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleSlider::role(int child) const
{
    switch (child) {
    case PageLeft:
    case PageRight:
        return PushButton;
    case Position:
        return Indicator;
    default:
        return Slider;
    }
}

/*! \reimp */
QAccessible::State QAccessibleSlider::state(int child) const
{
    const State parentState = QAccessibleWidget::state(0);

    if (child == 0)
        return parentState;

    // Inherit the Invisible state from parent.
    State state = parentState & QAccessible::Invisible;

    // Disable left/right if we are at the minimum/maximum.
    const QSlider * const slider = QAccessibleSlider::slider();
    switch (child) {
    case PageLeft:
        if (slider->value() <= slider->minimum())
            state |= Unavailable;
        break;
    case PageRight:
        if (slider->value() >= slider->maximum())
            state |= Unavailable;
        break;
    case Position:
    default:
        break;
    }

    return state;
}

/*!
    \fn int QAccessibleSlider::defaultAction(int child) const

    Returns the default action for the given \a child. The base class
    implementation returns 0.
*/
int QAccessibleSlider::defaultAction(int /*child*/) const
{
/*
    switch (child) {
    case SliderSelf:
        return SetFocus;
    case PageLeft:
        return Press;
    case PageRight:
        return Press;
    }
*/
    return 0;
}

/*! \internal */
QString QAccessibleSlider::actionText(int /*action*/, Text /*t*/, int /*child*/) const
{
    return QString("");
}

/*! \reimp */
bool QAccessibleSlider::doAction(int /*action*/, int /*child*/, const QVariantList &/*params*/)
{
/*
    switch(child) {
    case SliderSelf:
        if (action == SetFocus) {
            slider()->setFocus();
            return true;
        }
        break;
    case PageLeft:
        if (action == Press) {
            slider()->subtractPage();
            return true;
        }
        break;
    case Position:
        if (action == Increase) {
            slider()->addLine();
            return true;
        } else if (action == Decrease) {
            slider()->subtractLine();
            return true;
        }
        break;
    case PageRight:
        if (action == Press) {
            slider()->addPage();
            return true;
        }
        break;
    }
*/
    return false;
}

#endif // QT_NO_SLIDER
#endif // QT_NO_ACCESSIBILITY
