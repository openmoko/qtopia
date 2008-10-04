/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qhash.h>
#include <qlabel.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <private/qeffects_p.h>
#include <qtextdocument.h>
#include <qdebug.h>
#ifndef QT_NO_TOOLTIP

#ifdef Q_WS_MAC
# include <private/qcore_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QToolTip

    \brief The QToolTip class provides tool tips (balloon help) for any
    widget.

    \ingroup helpsystem
    \mainclass

    The tip is a short piece of text reminding the user of the
    widget's function. It is drawn immediately below the given
    position in a distinctive black-on-yellow color combination. The
    tip can be any \l{QTextEdit}{rich text} formatted string.

    Rich text displayed in a tool tip is implicitly word-wrapped unless
    specified differently with \c{<p style='white-space:pre'>}.

    The simplest and most common way to set a widget's tool tip is by
    calling its QWidget::setToolTip() function.

    It is also possible to show different tool tips for different
    regions of a widget, by using a QHelpEvent of type
    QEvent::ToolTip. Intercept the help event in your widget's \l
    {QWidget::}{event()} function and call QToolTip::showText() with
    the text you want to display. The \l{widgets/tooltips}{Tooltips}
    example illustrates this technique.

    Note that, if you want to show tooltips in an item view, the
    model/view architecture provides functionality to set an item's
    tool tip; e.g., the QTableWidgetItem::setToolTip() function.
    However, if you want to provide custom tool tips in an item view,
    you must intercept the help event in the
    QAbstractItemView::viewportEvent() function and handle it yourself.

    The default tool tip color and font can be customized with
    setPalette() and setFont(). When a tooltip is currently on
    display, isVisible() returns true and text() the currently visible
    text.

    \note Tool tips use the inactive color group of QPalette, because tool
    tips are not active windows.

    \sa QWidget::toolTip, QAction::toolTip, {Tool Tips Example}
*/

class QTipLabel : public QLabel
{
    Q_OBJECT
public:
    QTipLabel(const QString &text, QWidget *w);
    ~QTipLabel();
    static QTipLabel *instance;

    bool eventFilter(QObject *, QEvent *);

    QBasicTimer hideTimer;
    bool fadingOut;

    void reuseTip(const QString &text);
    void hideTip();
    void hideTipImmediately();
    void setTipRect(QWidget *w, const QRect &r);
    void restartHideTimer();
    bool tipChanged(const QPoint &pos, const QString &text, QObject *o);
    void placeTip(const QPoint &pos, QWidget *w);

    static int getTipScreen(const QPoint &pos, QWidget *w);
protected:
    void timerEvent(QTimerEvent *e);
    void paintEvent(QPaintEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

private:
    QWidget *widget;
    QRect rect;
};

QTipLabel *QTipLabel::instance = 0;

QTipLabel::QTipLabel(const QString &text, QWidget *w)
    : QLabel(w, Qt::ToolTip), widget(0)
{
    delete instance;
    instance = this;
    setForegroundRole(QPalette::ToolTipText);
    setBackgroundRole(QPalette::ToolTipBase);
    setPalette(QToolTip::palette());
    ensurePolished();
    setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this));
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignLeft);
    setIndent(1);
    setWordWrap(Qt::mightBeRichText(text));
    qApp->installEventFilter(this);
    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / 255.0);
    setMouseTracking(true);
    fadingOut = false;
    reuseTip(text);
}

void QTipLabel::restartHideTimer()
{
    int time = 10000 + 40 * qMax(0, text().length()-100);
    hideTimer.start(time, this);
}

void QTipLabel::reuseTip(const QString &text)
{
    setText(text);
    QFontMetrics fm(font());
    QSize extra(1, 0);
    // Make it look good with the default ToolTip font on Mac, which has a small descent.
    if (fm.descent() == 2 && fm.ascent() >= 11)
        ++extra.rheight();
    resize(sizeHint() + extra);
    restartHideTimer();
}

void QTipLabel::paintEvent(QPaintEvent *ev)
{
    QStylePainter p(this);
    QStyleOptionFrame opt;
    opt.init(this);
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    p.end();

    QLabel::paintEvent(ev);
}

void QTipLabel::resizeEvent(QResizeEvent *e)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.init(this);
    if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
        setMask(frameMask.region);

    QLabel::resizeEvent(e);
}

void QTipLabel::mouseMoveEvent(QMouseEvent *e)
{
    if (rect.isNull())
        return;
    QPoint pos = e->globalPos();
    if (widget)
        pos = widget->mapFromGlobal(pos);
    if (!rect.contains(pos))
        hideTip();
    QLabel::mouseMoveEvent(e);
}

QTipLabel::~QTipLabel()
{
    instance = 0;
}

void QTipLabel::hideTip()
{
    hideTimer.start(300, this);
}

void QTipLabel::hideTipImmediately()
{
    close(); // to trigger QEvent::Close which stops the animation
    deleteLater();
}

void QTipLabel::setTipRect(QWidget *w, const QRect &r)
{
    if (!rect.isNull() && !w)
        qWarning("QToolTip::setTipRect: Cannot pass null widget if rect is set");
    else{
        widget = w;
        rect = r;
    }
}

void QTipLabel::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == hideTimer.timerId()){
        hideTimer.stop();
#if defined(Q_WS_MAC) && !defined(QT_NO_EFFECTS)
        if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)){
            // Fade out tip on mac (makes it invisible).
            // The tip will not be deleted until a new tip is shown.
            TransitionWindowOptions options = {0, 0, 0, 0};
            TransitionWindowWithOptions(qt_mac_window_for(this), kWindowFadeTransitionEffect, kWindowHideTransitionAction, 0, 1, &options);
            QTipLabel::instance->fadingOut = true; // will never be false again.
        }
        else
            hideTipImmediately();
#else
        hideTipImmediately();
#endif
    }
}

bool QTipLabel::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
#ifdef Q_WS_MAC
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        int key = static_cast<QKeyEvent *>(e)->key();
        Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();
        if (!(mody & Qt::KeyboardModifierMask)
            && key != Qt::Key_Shift && key != Qt::Key_Control
            && key != Qt::Key_Alt && key != Qt::Key_Meta)
            hideTip();
        break;
    }
#endif
    case QEvent::Leave:
        hideTip();
        break;
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hideTipImmediately();
        break;

    case QEvent::MouseMove:
        if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent*>(e)->pos()))
            hideTip();
    default:
        break;
    }
    return false;
}

int QTipLabel::getTipScreen(const QPoint &pos, QWidget *w)
{
    if (QApplication::desktop()->isVirtualDesktop())
        return QApplication::desktop()->screenNumber(pos);
    else
        return QApplication::desktop()->screenNumber(w);
}

void QTipLabel::placeTip(const QPoint &pos, QWidget *w)
{
#ifdef Q_WS_MAC
    QRect screen = QApplication::desktop()->availableGeometry(getTipScreen(pos, w));
#else
    QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
#endif

    QPoint p = pos;
    p += QPoint(2,
#ifdef Q_WS_WIN
                21
#else
                16
#endif
        );
    if (p.x() + this->width() > screen.x() + screen.width())
        p.rx() -= 4 + this->width();
    if (p.y() + this->height() > screen.y() + screen.height())
        p.ry() -= 24 + this->height();
    if (p.y() < screen.y())
        p.setY(screen.y());
    if (p.x() + this->width() > screen.x() + screen.width())
        p.setX(screen.x() + screen.width() - this->width());
    if (p.x() < screen.x())
        p.setX(screen.x());
    if (p.y() + this->height() > screen.y() + screen.height())
        p.setY(screen.y() + screen.height() - this->height());
    this->move(p);
}

bool QTipLabel::tipChanged(const QPoint &pos, const QString &text, QObject *o)
{
    if (QTipLabel::instance->text() != text)
        return true;

    if (o != widget)
        return true;

    if (!rect.isNull())
        return !rect.contains(pos);
    else
       return false;
}

/*!
    Shows \a text as a tool tip, with the global position \a pos as
    the point of interest. The tool tip will be shown with a platform
    specific offset from this point of interest.

    If you specify a non-empty rect the tip will be hidden as soon
    as you move your cursor out of this area.

    The \a rect is in the coordinates of the widget you specify with
    \a w. If the \a rect is not empty you must specify a widget.
    Otherwise this argument can be 0 but it is used to determine the
    appropriate screen on multi-head systems.

    If \a text is empty the tool tip is hidden. If the text is the
    same as the currently shown tooltip, the tip will \e not move.
    You can force moving by first hiding the tip with an empty text,
    and then showing the new tip at the new position.
*/

void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect)
{
    if (QTipLabel::instance){ // a tip does already exist
        if (text.isEmpty()){ // empty text means hide current tip
            QTipLabel::instance->hideTip();
            return;
        }
        else if (!QTipLabel::instance->fadingOut){
            // If the tip has changed, reuse the one
            // that is showing (removes flickering)
            QPoint localPos = pos;
            if (w)
                localPos = w->mapFromGlobal(pos);
            if (QTipLabel::instance->tipChanged(localPos, text, w)){
                QTipLabel::instance->reuseTip(text);
                QTipLabel::instance->setTipRect(w, rect);
                QTipLabel::instance->placeTip(pos, w);
            }
            return;
        }
    }

    if (!text.isEmpty()){ // no tip can be reused, create new tip:
        new QTipLabel(text, 
#ifdef Q_WS_WIN
            QApplication::desktop()->screen(QTipLabel::getTipScreen(pos, w))
#else
            w
#endif
            ); // sets QTipLabel::instance to itself
        QTipLabel::instance->setTipRect(w, rect);
        QTipLabel::instance->placeTip(pos, w);
        QTipLabel::instance->setObjectName(QLatin1String("qtooltip_label"));

#if !defined(QT_NO_EFFECTS) && !defined(Q_WS_MAC)
        if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip))
            qFadeEffect(QTipLabel::instance);
        else if (QApplication::isEffectEnabled(Qt::UI_AnimateTooltip))
            qScrollEffect(QTipLabel::instance);
        else
            QTipLabel::instance->show();
#else
        QTipLabel::instance->show();
#endif
    }
}

/*!
    \overload

    This is analogous to calling QToolTip::showText(\a pos, \a text, \a w, QRect())
*/

void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w)
{
    QToolTip::showText(pos, text, w, QRect());
}


/*!
    \fn void QToolTip::hideText()
    \since 4.2

    Hides the tool tip. This is the same as calling showText() with an
    empty string.

    \sa showText()
*/


/*!
  \since 4.4

  Returns true if this tooltip is currently shown.

  \sa showText()
 */
bool QToolTip::isVisible()
{
    return (QTipLabel::instance != 0 && QTipLabel::instance->isVisible());
}

/*!
  \since 4.4

  Returns the tooltip text, if a tooltip is visible, or an
  empty string if a tooltip is not visible.
 */
QString QToolTip::text()
{
    if (QTipLabel::instance)
        return QTipLabel::instance->text();
    return QString();
}


Q_GLOBAL_STATIC(QPalette, tooltip_palette)

/*!
    Returns the palette used to render tooltips.

    \note Tool tips use the inactive color group of QPalette, because tool
    tips are not active windows.
*/
QPalette QToolTip::palette()
{
    return *tooltip_palette();
}

/*!
    \since 4.2

    Returns the font used to render tooltips.
*/
QFont QToolTip::font()
{
    return QApplication::font("QTipLabel");
}

/*!
    \since 4.2

    Sets the \a palette used to render tooltips.

    \note Tool tips use the inactive color group of QPalette, because tool
    tips are not active windows.
*/
void QToolTip::setPalette(const QPalette &palette)
{
    *tooltip_palette() = palette;
    if (QTipLabel::instance)
        QTipLabel::instance->setPalette(palette);
}

/*!
    \since 4.2

    Sets the \a font used to render tooltips.
*/
void QToolTip::setFont(const QFont &font)
{
    QApplication::setFont(font, "QTipLabel");
}


/*!
    \fn void QToolTip::add(QWidget *widget, const QString &text)

    Use QWidget::setToolTip() instead.

    \oldcode
    tip->add(widget, text);
    \newcode
    widget->setToolTip(text);
    \endcode
*/

/*!
    \fn void QToolTip::add(QWidget *widget, const QRect &rect, const QString &text)

    Intercept the QEvent::ToolTip events in your widget's
    QWidget::event() function and call QToolTip::showText() with the
    text you want to display. The \l{widgets/tooltips}{Tooltips}
    example illustrates this technique.
*/

/*!
    \fn void QToolTip::remove(QWidget *widget)

    Use QWidget::setToolTip() instead.

    \oldcode
    tip->remove(widget);
    \newcode
    widget->setToolTip("");
    \endcode
*/

QT_END_NAMESPACE

#include "qtooltip.moc"
#endif // QT_NO_TOOLTIP
