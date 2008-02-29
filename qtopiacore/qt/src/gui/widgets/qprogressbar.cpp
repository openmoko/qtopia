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

#include "qprogressbar.h"
#ifndef QT_NO_PROGRESSBAR
#include <qevent.h>
#include <qpainter.h>
#include <qstylepainter.h>
#include <qstyleoption.h>
#include <private/qwidget_p.h>
#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif
#include <limits.h>

class QProgressBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QProgressBar)
public:
    QProgressBarPrivate();
    void init();

    int minimum;
    int maximum;
    int value;
    Qt::Alignment alignment;
    uint textVisible : 1;
    int lastPaintedValue;
    Qt::Orientation orientation;
    bool invertedAppearance;
    QProgressBar::Direction textDirection;
    QString format;
    QStyleOptionProgressBarV2 getStyleOption() const;
    inline int bound(int val) const { return qMax(minimum-1, qMin(maximum, val)); }
    bool repaintRequired() const;
};

QProgressBarPrivate::QProgressBarPrivate()
    : minimum(0), maximum(100), value(-1), alignment(Qt::AlignLeft), textVisible(true),
      lastPaintedValue(-1), orientation(Qt::Horizontal), invertedAppearance(false),
      textDirection(QProgressBar::TopToBottom), format(QLatin1String("%p%"))
{
}

void QProgressBarPrivate::init()
{
    Q_Q(QProgressBar);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed);
    if (orientation == Qt::Vertical)
        sp.transpose();
    q->setSizePolicy(sp);
    q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

QStyleOptionProgressBarV2 QProgressBarPrivate::getStyleOption() const
{
    QStyleOptionProgressBarV2 opt;
    opt.init(q_func());

    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.progress = value;
    opt.textAlignment = alignment;
    opt.textVisible = textVisible;
    opt.text = q_func()->text();
    opt.orientation = orientation;
    opt.invertedAppearance = invertedAppearance;
    opt.bottomToTop = (textDirection == QProgressBar::BottomToTop);

    return opt;
}

bool QProgressBarPrivate::repaintRequired() const
{
    Q_Q(const QProgressBar);
    if (value == lastPaintedValue)
        return false;

    int valueDifference = qAbs(value - lastPaintedValue);

    // Check if the text needs to be repainted
    if ((value == minimum || value == maximum)
            || (textVisible && valueDifference >= qAbs((maximum - minimum) / 100)))
        return true;

    // Check if the bar needs to be repainted
    QStyleOptionProgressBarV2 opt = getStyleOption();
    int cw = q->style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, q);
    QRect groove  = q->style()->subElementRect(QStyle::SE_ProgressBarGroove, &opt, q);
    // This expression is basically
    // (valueDifference / (maximum - minimum) > cw / groove.width())
    // transformed to avoid integer division.
    return (valueDifference * groove.width() > cw * (maximum - minimum));
}

/*!
    \class QProgressBar qprogressbar.h
    \brief The QProgressBar widget provides a horizontal or vertical progress bar.

    \ingroup advanced
    \mainclass

    A progress bar is used to give the user an indication of the
    progress of an operation and to reassure them that the application
    is still running.

    The progress bar uses the concept of \e steps. You set it up by
    specifying the minimum and maximum possible step values, and it
    will display the percentage of steps that have been completed
    when you later give it the current step value. The percentage is
    calculated by dividing the progress (value() - minimum()) divided
    by maximum() - minimum().

    You can specify the minimum and maximum number of steps with
    setMinimum() and setMaximum. The current number of steps is set
    with setValue(). The progress bar can be rewound to the
    beginning with reset().

    If minimum and maximum both are set to 0, the bar shows a busy indicator
    instead of a percentage of steps. This is useful, for example, when using
    QFtp or QHttp to download items when they are unable to determine the
    size of the item being downloaded.

    \table
    \row \o \inlineimage macintosh-progressbar.png Screenshot of a Macintosh style progress bar
         \o A progress bar shown in the Macintosh widget style.
    \row \o \inlineimage windowsxp-progressbar.png Screenshot of a Windows XP style progress bar
         \o A progress bar shown in the Windows XP widget style.
    \row \o \inlineimage plastique-progressbar.png Screenshot of a Plastique style progress bar
         \o A progress bar shown in the Plastique widget style.
    \endtable

    \sa QTimeLine, QProgressDialog, {fowler}{GUI Design Handbook: Progress Indicator}
*/

/*!
    \since 4.1
    \enum QProgressBar::Direction
    \brief Specifies the reading direction of the \l text for vertical progress bars.

    \value TopToBottom The text is rotated 90 degrees clockwise.
    \value BottomToTop The text is rotated 90 degrees counter-clockwise.

    Note that whether or not the text is drawn is dependent on the style.
    Currently CDE, CleanLooks, Motif, and Plastique draw the text. Mac, Windows
    and WindowsXP style do not.

    \sa textDirection
*/

/*!
    \fn void QProgressBar::valueChanged(int value)

    This signal is emitted when the value shown in the progress bar changes.
    \a value is the new value shown by the progress bar.
*/

/*!
    Constructs a progress bar with the given \a parent.

    By default, the minimum step value is set to 0, and the maximum to 100.

    \sa setRange()
*/

QProgressBar::QProgressBar(QWidget *parent)
    : QWidget(*(new QProgressBarPrivate), parent, 0)
{
    d_func()->init();
}

/*!
    Reset the progress bar. The progress bar "rewinds" and shows no
    progress.
*/

void QProgressBar::reset()
{
    Q_D(QProgressBar);
    d->value = d->minimum - 1;
    if (d->minimum == INT_MIN)
        d->value = INT_MIN;
    repaint();
}

/*!
    \property QProgressBar::minimum
    \brief the progress bar's minimum value

    When setting this property, the \l maximum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/
void QProgressBar::setMinimum(int minimum)
{
    setRange(minimum, qMax(d_func()->maximum, minimum));
}

int QProgressBar::minimum() const
{
    return d_func()->minimum;
}


/*!
    \property QProgressBar::maximum
    \brief the progress bar's maximum value

    When setting this property, the \l minimum is adjusted if
    necessary to ensure that the range remains valid. If the
    current value falls outside the new range, the progressbar is reset
    with reset().
*/

void QProgressBar::setMaximum(int maximum)
{
    setRange(qMin(d_func()->minimum, maximum), maximum);
}

int QProgressBar::maximum() const
{
    return d_func()->maximum;
}

/*!
    \property QProgressBar::value
    \brief the progress bar's current value

    Attempting to change the current value to one outside
    the minimum-maximum range has no effect on the current value.
*/
void QProgressBar::setValue(int value)
{
    Q_D(QProgressBar);
    if (d->value == value
            || ((value > d->maximum || value < d->minimum)
                && (d->maximum != 0 || d->minimum != 0)))
        return;
    d->value = value;
    emit valueChanged(value);
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::ValueChanged);
#endif
    if (d->repaintRequired())
        repaint();
}

int QProgressBar::value() const
{
    return d_func()->value;
}

/*!
    Sets the progressbar's minimum and maximum values to \a minimum and
    \a maximum respectively.

    If \a maximum is smaller than \a minimum, \a minimum becomes the only
    legal value.

    If the current value falls outside the new range, the progressbar is reset
    with reset().

    \sa minimum maximum
*/
void QProgressBar::setRange(int minimum, int maximum)
{
    Q_D(QProgressBar);
    d->minimum = minimum;
    d->maximum = qMax(minimum, maximum);
    if ( d->value <(d->minimum-1) || d->value > d->maximum)
        reset();
}
/*!
    \property QProgressBar::textVisible
    \brief whether the current completed percentage should be displayed

    \sa textDirection
*/
void QProgressBar::setTextVisible(bool visible)
{
    Q_D(QProgressBar);
    if (d->textVisible != visible) {
        d->textVisible = visible;
        repaint();
    }
}

bool QProgressBar::isTextVisible() const
{
    return d_func()->textVisible;
}

/*!
    \property QProgressBar::alignment
    \brief the alignment of the progress bar
*/
void QProgressBar::setAlignment(Qt::Alignment alignment)
{
    if (d_func()->alignment != alignment) {
        d_func()->alignment = alignment;
        repaint();
    }
}

Qt::Alignment QProgressBar::alignment() const
{
    return d_func()->alignment;
}

/*!
    \reimp
*/
void QProgressBar::paintEvent(QPaintEvent *)
{
    QStylePainter paint(this);
    QStyleOptionProgressBarV2 opt = d_func()->getStyleOption();
    paint.drawControl(QStyle::CE_ProgressBar, opt);
    d_func()->lastPaintedValue = d_func()->value;
}

/*!
    \reimp
*/
QSize QProgressBar::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QStyleOptionProgressBarV2 opt = d_func()->getStyleOption();
    int cw = style()->pixelMetric(QStyle::PM_ProgressBarChunkWidth, &opt, this);
    QSize size = QSize(cw * 7 + fm.width(QLatin1Char('0')) * 4, fm.height() + 8);
    if (opt.orientation == Qt::Vertical)
        size.transpose();
    return style()->sizeFromContents(QStyle::CT_ProgressBar, &opt, size, this);
}

/*!
    \reimp
*/
QSize QProgressBar::minimumSizeHint() const
{
    QSize size;
    if (orientation() == Qt::Horizontal)
        size = QSize(sizeHint().width(), fontMetrics().height() + 2);
    else
        size = QSize(fontMetrics().height() + 2, sizeHint().height());
    return size;
}

/*!
    \property QProgressBar::text
    \brief the descriptive text shown with the progress bar

    The text returned is the same as the text displayed in the center
    (or in some styles, to the left) of the progress bar.

    The progress shown in the text may be smaller than the minimum value,
    indicating that the progress bar is in the "reset" state before any
    progress is set.

    In the default implementation, the text either contains a percentage
    value that indicates the progress so far, or it is blank because the
    progress bar is in the reset state.
*/
QString QProgressBar::text() const
{
    Q_D(const QProgressBar);
    if (d->maximum == 0 || d->value < d->minimum
            || (d->value == INT_MIN && d->minimum == INT_MIN))
        return QString();

    int totalSteps = d->maximum - d->minimum;

    QString result = d->format;
    result.replace(QLatin1String("%m"), QString::fromLatin1("%1").arg(totalSteps));
    result.replace(QLatin1String("%v"), QString::fromLatin1("%1").arg(d->value));

    // If max and min are equal and we get this far, it means that the
    // progress bar has one step and that we are on that step. Return
    // 100% here in order to avoid division by zero further down.
    if (totalSteps == 0) {
        result.replace(QLatin1String("%p"), QString::fromLatin1("%1").arg(100));
        return result;
    }

    int progress = d->value - d->minimum;
    // Get the values down to something usable.
    if (totalSteps > INT_MAX / 1000) {
        progress /= 1000;
        totalSteps /= 1000;
    }

    result.replace(QLatin1String("%p"), QString::fromLatin1("%1").arg((progress * 100) / totalSteps));
    return result;
}

/*!
    \since 4.1
    \property QProgressBar::orientation
    \brief the orientation of the progress bar

    The orientation must be \l Qt::Horizontal (the default) or \l
    Qt::Vertical.

    \sa invertedAppearance, textDirection
*/

void QProgressBar::setOrientation(Qt::Orientation orientation)
{
    Q_D(QProgressBar);
    if (d->orientation == orientation)
        return;
    d->orientation = orientation;
    if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy(sp);
        setAttribute(Qt::WA_WState_OwnSizePolicy, false);
    }
    update();
    updateGeometry();
}

Qt::Orientation QProgressBar::orientation() const
{
    Q_D(const QProgressBar);
    return d->orientation;
}

/*!
    \since 4.1
    \property QProgressBar::invertedAppearance
    \brief whether or not a progress bar shows its progress inverted

    If this property is false, the progress bar grows in the other
    direction (e.g. from right to left). By default, the progress bar
    is not inverted.

    \sa orientation, layoutDirection
*/

void QProgressBar::setInvertedAppearance(bool invert)
{
    Q_D(QProgressBar);
    d->invertedAppearance = invert;
    update();
}

bool QProgressBar::invertedAppearance()
{
    Q_D(QProgressBar);
    return d->invertedAppearance;
}

/*!
    \since 4.1
    \property QProgressBar::textDirection
    \brief the reading direction of the \l text for vertical progress bars

    This property has no impact on horizontal progress bars.
    By default, the reading direction is QProgressBar::TopToBottom.

    \sa orientation, textVisible
*/
void QProgressBar::setTextDirection(QProgressBar::Direction textDirection)
{
    Q_D(QProgressBar);
    d->textDirection = textDirection;
    update();
}

QProgressBar::Direction QProgressBar::textDirection()
{
    Q_D(QProgressBar);
    return d->textDirection;
}

/*! \reimp */
bool QProgressBar::event(QEvent *e)
{
    return QWidget::event(e);
}

/*!
    \since 4.2
    \property QProgressBar::format
    \brief the string used to generate the current text

    %p - is replaced by the percentage completed.
    %v - is replaced by the current value.
    %m - is replaced by the total number of steps.

    The default value is "%p%".

    \sa text()
*/
void QProgressBar::setFormat(const QString &format)
{
    Q_D(QProgressBar);
    d->format = format;
}

QString QProgressBar::format() const
{
    Q_D(const QProgressBar);
    return d->format;
}

#endif // QT_NO_PROGRESSBAR
