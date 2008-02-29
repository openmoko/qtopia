/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qphonestyle.h"
#include "qexportedbackground.h"

#include <custom.h>
#include <qtopianamespace.h>
#include <QStyleOption>
#include <QPainter>
#include <qdrawutil.h>
#include <QApplication>
#include <QTextEdit>
#include <QDebug>
#include <QScrollArea>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPaintEvent>
#include <QDesktopWidget>
#include <QWSManager>
#include <QMap>
#include <QDesktopWidget>
#include <QPaintEngine>
#include <private/qwidget_p.h>

#include <values.h>

static const int phoneItemFrame        =  1; // menu item frame width
static const int phoneSepHeight        =  1; // separator item height
static const int phoneItemHMargin      =  2; // menu item hor text margin
static const int phoneItemVMargin      =  1; // menu item ver text margin
static const int phoneArrowHMargin     =  6; // arrow horizontal margin
static const int phoneCheckMarkHMargin =  2; // horiz. margins of check mark
static const int phoneRightBorder      =  8; // right border on windows
static const int phoneCheckMarkWidth   = 12; // checkmarks width on windows

static bool isSingleFocusWidget(QWidget *focus)
{
    bool singleFocusWidget = false;
    if (focus) {
        QWidget *w = focus;
        singleFocusWidget = true;
        while ((w = w->nextInFocusChain()) != focus) {
            if (w->isVisible() && focus != w->focusProxy() && w->focusPolicy() & Qt::TabFocus) {
                singleFocusWidget = false;
                break;
            }
        }
    }

    return singleFocusWidget;
}


class ManagerAccessorPrivate;

class ManagerAccessor : public QWidget
{
public:
    QWSManager *manager();

    Q_DECLARE_PRIVATE(ManagerAccessor);
};

class ManagerAccessorPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(ManagerAccessor);
};

QWSManager *ManagerAccessor::manager() {
    Q_D(ManagerAccessor);
    return d->topData()->qwsManager;
}

class QPhoneStylePrivate : public QObject
{
    Q_OBJECT
public:
    QPhoneStylePrivate(QPhoneStyle *style) : QObject(0), useExported(false) {
        q = style;
        bgExport = bgForScreen(QApplication::desktop()->primaryScreen());
        connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
                SLOT(focusChangedSlot(QWidget*, QWidget*)));
    }

    void updateDecoration() {
        QWidget *active = QApplication::activeWindow();
        if (active) {
            QWSManager *manager = ((ManagerAccessor*)active)->manager();
            if (manager) {
                QDesktopWidget *desktop = QApplication::desktop();
                QApplication::postEvent(manager,
                    new QPaintEvent(desktop->screenGeometry(desktop->primaryScreen())));
            }
        }
    }

    QExportedBackground *bgForScreen(int screen) {
        if (!bgExportMap.contains(screen)) {
            QExportedBackground *bge = new QExportedBackground(screen, this);
            bgExportMap[screen] = bge;
            connect(bge, SIGNAL(changed()), this, SLOT(bgUpdated()));
        }

        return bgExportMap.value(screen);
    }
    
    void toggleTextColor(QWidget *w) {
        if (w) {
            QPalette pal = w->palette();
            QColor temptext = pal.color(QPalette::Text);
            pal.setColor(QPalette::Text, pal.color(QPalette::HighlightedText));
            pal.setColor(QPalette::HighlightedText, temptext);
            w->setPalette(pal);
        }
    }

    bool eventFilter(QObject *, QEvent *);
    QExportedBackground *bgExport;
    QMap<int,QExportedBackground*> bgExportMap;
    QPalette bgPal;
    QPalette origPal;
    bool useExported;
    QMap<QWidget*,int> bgManaged;
    
private:
    QPhoneStyle *q;

private slots:
    void bgUpdated();
    void focusChangedSlot(QWidget *, QWidget *);
};

bool QPhoneStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    QWidget *wgt = qobject_cast<QWidget*>(o);
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (wgt && bgManaged.contains(wgt)) {
        if (e->type() == QEvent::Paint) {
            QWidget *w = qobject_cast<QWidget*>(o);
            if (w && w->isWindow()) {
                QPaintEvent *pe = (QPaintEvent*)e;
                QBrush b = w->palette().brush(QPalette::Window);
                QPainter p(w);
                p.setBrushOrigin(-w->geometry().topLeft());
                // We know we have a solid brush, so we can gain a little speed
                // using QPainter::CompositionMode_Source
                if (p.paintEngine()->hasFeature(QPaintEngine::PorterDuff))
                    p.setCompositionMode(QPainter::CompositionMode_Source);
                p.fillRect(pe->rect(), b);
            }
        } else if (e->type() == QEvent::ParentChange) {
            QWidget *widget = qobject_cast<QWidget*>(o);
            if (!widget->isWindow()) {
                widget->setPalette(bgPal);
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                widget->removeEventFilter(this);
                bgManaged.remove(widget);
            }
        } else if (e->type() == QEvent::ApplicationPaletteChange) {
            foreach (QWidget *w, QApplication::topLevelWidgets()) {
                QApplication::style()->polish(w);
                foreach (QObject *o, w->children()) {
                    QWidget *sw = qobject_cast<QWidget*>(o);
                    if (sw) {
                        QApplication::style()->polish(sw);
                    }
                }
            }
        } else if (e->type() == QEvent::Move) {
            QWidget *w = qobject_cast<QWidget*>(o);
            int screen = QApplication::desktop()->screenNumber(w);
            if (screen >= 0 && bgManaged.contains(w) && bgManaged[w] != screen) {
                bgManaged[w] = screen;
                QExportedBackground *bge = bgForScreen(screen);
                QPalette pal = bgPal;
                QColor windowCol = pal.color(QPalette::Window);
                windowCol.setAlpha(255);
                pal.setBrush(QPalette::Window, QBrush(windowCol, bge->background()));
                w->setPalette(pal);
            }
    //        w->repaint();  //XXX maybe nice if we allow windows to be moved by user
        } else if (e->type() == QEvent::Destroy) {
            QWidget *widget = qobject_cast<QWidget*>(o);
            if (widget)
                bgManaged.remove(widget);
        } else if (e->type() == QEvent::PaletteChange) {
            // XXX hackery to fix a problem with category selectors created
            // from .ui files ending up with transparent popup combo boxes...
            QWidget *widget = qobject_cast<QWidget*>(o);
            if (bgExport->isAvailable() ) {
                QPalette pal = bgPal;
                QColor windowCol = pal.color(QPalette::Window);
                windowCol.setAlpha(255);
                pal.setBrush(QPalette::Window, QBrush(windowCol, bgExport->background()));
                widget->setPalette(pal);
                return true;
            }
        }
    }
#endif
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (wgt && QApplication::keypadNavigationEnabled()) {
        if (e->type() == QEvent::Paint) {   //handle background of textedit
            QTextEdit *te = qobject_cast<QTextEdit*>(wgt);
            if (te) {
                if (te->hasFocus() && !te->hasEditFocus()) {
                    QPainter p(te);
                    QRect r = te->rect();
                    p.fillRect(r.x()+4, r.y()+4, r.width()-8, r.height()-8,
                               te->palette().brush(QPalette::Highlight));
        
                    QPen oldPen = p.pen();
                    QColor color = te->palette().highlight().color();
                    p.setPen(color);
                    p.drawRect(r.x()+3, r.y()+3, r.width()-7, r.height()-7);
                    color.setAlpha(color.alpha()/2);
                    p.setPen(color);
                    p.drawRect(r.x()+4, r.y()+4, r.width()-9, r.height()-9);
                    p.setPen(oldPen);
                }   
            }
        } else if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
            if (wgt->inherits("QLineEdit") || wgt->inherits("QAbstractSpinBox")) {
                toggleTextColor(wgt);
            } else if (QTextEdit *te = qobject_cast<QTextEdit*>(wgt)) {
                te->viewport()->setBackgroundRole(e->type() == QEvent::EnterEditFocus
                                                   ? QPalette::Base : QPalette::Background);
                toggleTextColor(te);
            }
        }
    }
#endif
    return false;
}

void QPhoneStylePrivate::bgUpdated()
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (bgExport->isAvailable()) {
        if (!useExported)
            QApplication::setPalette(QApplication::palette());
        foreach (QWidget *w, QApplication::topLevelWidgets())
            w->update();
    }
#endif
}

void QPhoneStylePrivate::focusChangedSlot(QWidget *old, QWidget *now)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (q != qApp->style()) {
        disconnect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
                SLOT(focusChangedSlot(QWidget*, QWidget*)));
        return;
    }

    QTextEdit *te = qobject_cast<QTextEdit*>(old);
    if (te) {
        te->viewport()->setBackgroundRole(QPalette::Background);
        toggleTextColor(te);
        te->removeEventFilter(this);
    }
    
    te = qobject_cast<QTextEdit*>(now);
    if (te) {
        te->viewport()->setBackgroundRole(QPalette::Background);
        if (!te->hasEditFocus())    //HACK: there are a few strange cases where we can get edit focus before we get focus
            toggleTextColor(te);
        te->installEventFilter(this);
    }
    
    if (old && (old->inherits("QLineEdit") || old->inherits("QAbstractSpinBox"))) {
        toggleTextColor(old);
        old->removeEventFilter(this);
    }
    
    if (now && (now->inherits("QLineEdit") || now->inherits("QAbstractSpinBox"))) {
        toggleTextColor(now);
        now->installEventFilter(this);
    }
#endif   
}

void drawShadePanel(QPainter *p, const QRect &r,
        const QPalette &pal, bool sunken,
        int lineWidth, const QBrush *fill)
{
    qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(), pal, sunken,
            lineWidth, fill);
}

void drawShadePanel(QPainter *p, int x, int y, int w, int h,
                      const QPalette &pal, bool sunken,
                      int lineWidth, const QBrush *fill)
{
    if (w == 0 || h == 0)
        return;
    if (!(w > 0 && h > 0 && lineWidth >= 0)) {
        qWarning("drawShadePanel() Invalid parameters.");
    }
    QColor shade = pal.dark().color();
    QColor light = pal.light().color();
    if (fill) {
        if (fill->color() == shade)
            shade = pal.shadow().color();
        if (fill->color() == light)
            light = pal.midlight().color();
    }
    QPen oldPen = p->pen();                        // save pen
    QVector<QLineF> lines;
    lines.reserve(2*lineWidth);

    if (sunken)
        p->setPen(shade);
    else
        p->setPen(light);
    int x1, y1, x2, y2;
    int i;
    x1 = x;
    y1 = y2 = y;
    x2 = x+w-2;
    for (i=0; i<lineWidth; i++) {                // top shadow
        lines << QLineF(x1, y1++, x2--, y2++);
    }
    x2 = x1;
    y1 = y+h-2;
    for (i=0; i<lineWidth; i++) {                // left shado
        lines << QLineF(x1++, y1, x2++, y2--);
    }
    p->drawLines(lines);
    lines.clear();
    if (sunken)
        p->setPen(light);
    else
        p->setPen(shade);
    x1 = x;
    y1 = y2 = y+h-1;
    x2 = x+w-1;
    for (i=0; i<lineWidth; i++) {                // bottom shadow
        lines << QLineF(x1++, y1--, x2, y2--);
    }
    x1 = x2;
    y1 = y;
    y2 = y+h-lineWidth-1;
    for (i=0; i<lineWidth; i++) {                // right shadow
        lines << QLineF(x1--, y1++, x2--, y2);
    }
    p->drawLines(lines);
    if (fill)                                // fill with fill color
        p->fillRect(x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2, *fill);
    p->setPen(oldPen);                        // restore pen
}

QPhoneStyle::QPhoneStyle() : QtopiaStyle()
{
    d = new QPhoneStylePrivate(this);
}

QPhoneStyle::~QPhoneStyle()
{
    delete d;
}

#include <sys/types.h>
#include <unistd.h>

void QPhoneStyle::polish(QPalette &pal)
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        d->origPal = pal;
        d->bgPal = pal;
        QColor col = pal.color(QPalette::Window);
        col.setAlpha(0);
        d->bgPal.setColor(QPalette::Window, col);
        col = pal.color(QPalette::Base);
        col.setAlpha(64);
        d->bgPal.setColor(QPalette::Base, col);
        col = pal.color(QPalette::AlternateBase);
        col.setAlpha(176);
        d->bgPal.setColor(QPalette::AlternateBase, col);
        col = pal.color(QPalette::Highlight);

        // Make a nice horizontal gradient for highlight.
        QDesktopWidget *desktop = QApplication::desktop();
        int desktopWidth = desktop->screenGeometry(desktop->primaryScreen()).width();
        QLinearGradient g(0, 0, desktopWidth-1, 0);
        QColor gradCol(col);
        gradCol = gradCol.dark(110);
        gradCol.setAlpha(176);
        g.setColorAt(0, gradCol);
        gradCol = col.light(130);
        gradCol.setAlpha(96);
        g.setColorAt(1, gradCol);
        col.setAlpha(176);
        QPixmap pm(desktopWidth, 1);
        pm.fill(QColor(0,0,0,0));
        QPainter p(&pm);
        p.fillRect(0, 0, pm.width(), pm.height(), g);
        d->bgPal.setBrush(QPalette::Highlight, QBrush(col, pm));

        col = pal.color(QPalette::Button);
        col.setAlpha(176);
        d->bgPal.setColor(QPalette::Button, col);
        pal = d->bgPal;
        d->useExported = true;
    } else if (d->useExported) {
        d->useExported = false;
        pal = d->origPal;
    }
#endif
}

void QPhoneStyle::polish(QWidget *widget)
{
    QTextEdit *te = qobject_cast<QTextEdit*>(widget);
    if (te)
        te->viewport()->setBackgroundRole(QPalette::Window);
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(widget);
    if (aiv)
        aiv->viewport()->setBackgroundRole(QPalette::Window);
    QScrollBar *sb = qobject_cast<QScrollBar*>(widget);
    if (sb)
        sb->setAttribute(Qt::WA_OpaquePaintEvent, false); // For transparency
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        bool isManaged = false;
        if (d->bgManaged.contains(widget)) {
            if (!widget->isWindow()) {
                // We were created without a parent, but now we have one.
                widget->setPalette(d->bgPal);
                widget->removeEventFilter(d);
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                d->bgManaged.remove(widget);
            } else {
                isManaged = true;
            }
        }
        QScrollArea *sa = qobject_cast<QScrollArea*>(widget);
        if (sa && sa->widget()) {
            sa->widget()->setAutoFillBackground(false);
            if (!sa->viewport()->testAttribute(Qt::WA_SetPalette))
                sa->viewport()->setAutoFillBackground(false);
        }

        if (widget->isWindow()) {
            bool isTransparent = widget->palette().color(QPalette::Window) == QColor(0,0,0,0);
            if ((!widget->testAttribute(Qt::WA_SetPalette)
                || widget->palette() == d->bgPal
                || d->bgManaged.contains(widget))
                && !widget->testAttribute(Qt::WA_NoSystemBackground)
                && !isTransparent) {
                QPalette pal = d->bgPal;
                QColor windowCol = pal.color(QPalette::Window);
                windowCol.setAlpha(255);
                pal.setBrush(QPalette::Window, QBrush(windowCol, d->bgExport->background()));
                widget->setPalette(pal);
                widget->setAttribute(Qt::WA_OpaquePaintEvent, true);
                if (!isManaged) {
                    d->bgManaged.insert(widget, QApplication::desktop()->primaryScreen());
                    widget->installEventFilter(d);
                }
            } else if(isTransparent && d->bgManaged.contains(widget)) {
                // unmanage transparent window
                widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
                widget->removeEventFilter(this);
                d->bgManaged.remove(widget);
            }
        } else if (widget->parentWidget() && widget->parentWidget()->isWindow()) {
            widget->setPalette(d->bgPal);
        }
    }
#endif
}

void QPhoneStyle::unpolish(QWidget *widget)
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
        if (d->bgManaged.contains(widget)) {
            widget->setPalette(QApplication::palette());
            widget->removeEventFilter(d);
            widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
        } else if (widget->parentWidget() && widget->parentWidget()->isWindow()) {
            widget->setPalette(QApplication::palette());
        }
    }
#endif
}

void QPhoneStyle::unpolish(QApplication *app)
{
    app->setPalette(d->origPal);
}

int QPhoneStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ScrollBarExtent:
        //XXX use DPI
        if (Qtopia::mousePreferred()) {
            ret = 15;
        } else {
            QDesktopWidget *desktop = QApplication::desktop();
            ret = desktop->screenGeometry(widget
                    ? desktop->screenNumber(widget)
                    : desktop->primaryScreen()).width() >= 240 ? 12 : 8;
        }
        break;

    case PM_TabBarTabHSpace:
        ret = 14;
        break;

    default:
        ret = QtopiaStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}

QSize QPhoneStyle::sizeFromContents(ContentsType type, const QStyleOption* opt,
                                const QSize &csz, const QWidget *widget ) const
{
    QSize sz(csz);
    switch (type) {
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            bool checkable = mi->menuHasCheckableItems;
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(), h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                h = 2;
            } else {
                h = qMax(h, mi->fontMetrics.height() + 4);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height() + 2);
            }
            if (mi->text.contains('\t'))
                w += 12;
            w += qMax(maxpmw,18) + 4;
            if (checkable && maxpmw < 18)
                w += 18 - maxpmw;
            if (checkable || maxpmw > 0)
                w += 2;
            w += 2;
            sz = QSize(w, h);
        }
        break;
    /* Doesn't work yet (Qt issue).
    case CT_Menu:
        {
            sz = QWindowsStyle::sizeFromContents(type, opt, csz, widget);
            QDesktopWidget *desktop = QApplication::desktop();
            int deskh = desktop->screenGeometry(desktop->primaryScreen()).height();
            if ( sz.height() > deskh*2/3 )
                sz.setHeight(deskh*2/3);
        }
        break;
    */
    default:
        sz = QWindowsStyle::sizeFromContents(type, opt, csz, widget);
        break;
    }

    return sz;
}

QRect QPhoneStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const
{
    QRect ret;

    switch (cc) {
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spinbox = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QSize bs;
            int fw = spinbox->frame ? pixelMetric(PM_SpinBoxFrameWidth, spinbox, w) : 0;
            bs.setHeight(spinbox->rect.height());
            bs.setWidth(8);
            bs = bs.expandedTo(QApplication::globalStrut());
            //int y = fw;
            int x, lx, rx;
            x = spinbox->rect.width() - bs.width()*2;
            lx = fw;
            rx = x - fw;
            switch (sc) {
            case SC_SpinBoxUp:
                ret = QRect(x + bs.width(), 0, bs.width(), bs.height());
                break;
            case SC_SpinBoxDown:
                ret = QRect(x, 0, bs.width(), bs.height());
                break;
            case SC_SpinBoxEditField:
                ret = QRect(lx, fw, rx, spinbox->rect.height() - 2*fw);
                break;
            case SC_SpinBoxFrame:
                ret = spinbox->rect;
                ret.setRight(ret.right()-bs.width()*2);
            default:
                break;
            }
            ret = visualRect(spinbox->direction, spinbox->rect, ret);
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            int x = 0,
                y = 0,
                wi = cb->rect.width(),
                he = cb->rect.height();
            int xpos = x;
            int margin = cb->frame ? 3 : 0;
            int bmarg = cb->frame ? 2 : 0;
            xpos += wi - bmarg - 16;


            switch (sc) {
            case SC_ComboBoxFrame:
                ret = cb->rect;
                ret.setRight(ret.right()-16);
                break;
            case SC_ComboBoxArrow:
                ret.setRect(xpos, y + bmarg, 16, he);
                break;
            case SC_ComboBoxEditField:
                ret.setRect(x + margin, y + margin, wi - 2 * margin - 16, he - 2 * margin);
                break;
            case SC_ComboBoxListBoxPopup:
                ret = cb->rect;
                break;
            default:
                break;
            }
            ret = visualRect(cb->direction, cb->rect, ret);
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            int sbextent = pixelMetric(PM_ScrollBarExtent, scrollbar, w);
            int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                          scrollbar->rect.width() : scrollbar->rect.height());
            if (Qtopia::mousePreferred())
                maxlen -= sbextent * 2;
            int btnextent = Qtopia::mousePreferred() ? sbextent : 0;
            int sliderlen;

            // calculate slider length
            if (scrollbar->maximum != scrollbar->minimum) {
                uint range = scrollbar->maximum - scrollbar->minimum;
                sliderlen = (qint64(scrollbar->pageStep) * maxlen) / (range + scrollbar->pageStep);

                int slidermin = pixelMetric(PM_ScrollBarSliderMin, scrollbar, w);
                if (sliderlen < slidermin || range > INT_MAX / 2)
                    sliderlen = slidermin;
                if (sliderlen > maxlen)
                    sliderlen = maxlen;
            } else {
                sliderlen = maxlen;
            }

            int sliderstart = sliderPositionFromValue(scrollbar->minimum,
                                                         scrollbar->maximum,
                                                         scrollbar->sliderPosition,
                                                         maxlen - sliderlen,
                                                         scrollbar->upsideDown);
            if (Qtopia::mousePreferred())
                sliderstart += sbextent;
            switch (sc) {
            case SC_ScrollBarSubLine:            // top/left button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width() / 2, btnextent);
                    ret.setRect(0, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height() / 2, btnextent);
                    ret.setRect(0, 0, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarAddLine:            // bottom/right button
                if (scrollbar->orientation == Qt::Horizontal) {
                    int buttonWidth = qMin(scrollbar->rect.width()/2, btnextent);
                    ret.setRect(scrollbar->rect.width() - buttonWidth, 0, buttonWidth, sbextent);
                } else {
                    int buttonHeight = qMin(scrollbar->rect.height()/2, btnextent);
                    ret.setRect(0, scrollbar->rect.height() - buttonHeight, sbextent, buttonHeight);
                }
                break;
            case SC_ScrollBarSubPage:            // between top/left button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(btnextent, 0, sliderstart - btnextent, sbextent);
                else
                    ret.setRect(0, btnextent, sbextent, sliderstart - btnextent);
                break;
            case SC_ScrollBarAddPage:            // between bottom/right button and slider
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart + sliderlen, 0,
                                maxlen - sliderstart - sliderlen + btnextent, sbextent);
                else
                    ret.setRect(0, sliderstart + sliderlen, sbextent,
                                maxlen - sliderstart - sliderlen + btnextent);
                break;
            case SC_ScrollBarGroove:
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(btnextent, 0, scrollbar->rect.width() - btnextent * 2,
                                scrollbar->rect.height());
                else
                    ret.setRect(0, btnextent, scrollbar->rect.width(),
                                scrollbar->rect.height() - btnextent * 2);
                break;
            case SC_ScrollBarSlider:
                if (scrollbar->orientation == Qt::Horizontal)
                    ret.setRect(sliderstart, 0, sliderlen, sbextent);
                else
                    ret.setRect(0, sliderstart, sbextent, sliderlen);
                break;
            default:
                break;
            }
            ret = visualRect(scrollbar->direction, scrollbar->rect, ret);
        }
        break;
    default:
        ret = QWindowsStyle::subControlRect(cc, opt, sc, w);
    }

    return ret;
}


void QPhoneStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt,
                                  QPainter *p, const QWidget *widget) const
{
    bool doRestore = false;

    switch (pe) {
    case PE_FrameButtonBevel:
    case PE_FrameButtonTool:
        qDrawShadeRect(p, opt->rect, opt->palette,
                       opt->state & (State_Sunken | State_On), 1, 0);
        break;
    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel:
    case PE_PanelButtonTool:
    case PE_IndicatorButtonDropDown:
        drawShadePanel(p, opt->rect, opt->palette,
                        opt->state & (State_Sunken | State_On), 1,
                        &opt->palette.brush(QPalette::Button));
        break;
    case PE_FrameFocusRect:
        if (const QStyleOptionFocusRect *fropt = qstyleoption_cast<const QStyleOptionFocusRect *>(opt)) {
            if (Qtopia::mousePreferred() && !(fropt->state & State_KeyboardFocusChange))
                return;
            QRect r = opt->rect;
            r.adjust(0, 0, -1, -1);
            QPen oldPen = p->pen();
            QColor color = fropt->palette.highlight().color();
            p->setPen(color);
            p->drawRect(r);
            r.adjust(1, 1, -1, -1);
            color.setAlpha(color.alpha()/2);
            p->setPen(color);
            p->drawRect(r);
            p->setPen(oldPen);
        }
        break;
    case PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *frame = qstyleoption_cast<const QStyleOptionTabWidgetFrame*>(opt)) {
            qDrawShadePanel(p, frame->rect, frame->palette, false, 1);
        }
        break;
    case PE_Frame:
    case PE_FrameMenu:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if ((frame->state & State_Sunken) || (frame->state & State_Raised)) {
                if (widget && widget->inherits("QTextEdit") && !widget->hasFocus()) {
                    QPen oldPen = p->pen();
                    QPen pen(Qt::DotLine);
                    pen.setColor(opt->palette.color(QPalette::Shadow));
                    p->setPen(pen);
                    p->drawRect(frame->rect.adjusted(0, 0, -1, -1));
                    p->setPen(oldPen);
                } else {
                    qDrawShadePanel(p, frame->rect, frame->palette, frame->state & State_Sunken,
                            frame->lineWidth);
                }
            } else {
                qDrawPlainRect(p, frame->rect, frame->palette.foreground().color(), frame->lineWidth);
            }
        }
        break;
    case PE_FrameLineEdit:
        if (opt->state & State_HasFocus) {
            QStyleOptionFocusRect fropt;
            fropt.state = State_KeyboardFocusChange;
            fropt.rect = opt->rect;
            fropt.palette = opt->palette;
            drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
        } else {
            QPen oldPen = p->pen();
            const QRect &r = opt->rect;
            p->setPen(QPen(opt->palette.background(), 1));
            p->drawLine(r.x(), r.bottom(), r.x(), r.top());
            p->drawLine(r.x(), r.top(), r.right(), r.top());
            p->drawLine(r.right(), r.top(), r.right(), r.bottom());
            QPen pen(Qt::DotLine);
            pen.setColor(opt->palette.color(QPalette::Shadow));
            p->setPen(pen);
            p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
            p->setPen(oldPen);
        }
        break;
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *panel = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            QBrush bg = panel->palette.brush(QPalette::Window);
            if (panel->state & State_Enabled && panel->state & State_HasEditFocus)
                bg = panel->palette.brush(QPalette::Base);
            p->fillRect(panel->rect.adjusted(panel->lineWidth, panel->lineWidth, -panel->lineWidth, -panel->lineWidth), bg);

            if (panel->lineWidth > 0)
                drawPrimitive(PE_FrameLineEdit, panel, p, widget);
                        
            if (opt->state & State_HasFocus && !(panel->state & State_HasEditFocus)) {
                
                const QRect &r = opt->rect;
                p->fillRect(r.x()+4, r.y()+4, r.width()-8, r.height()-8,
                            panel->palette.brush(QPalette::Highlight));

                QStyleOptionFocusRect focus;
                focus.QStyleOption::operator=(*panel);
                focus.rect = QRect(r.x()+3, r.y()+3, r.width()-6, r.height()-6);
                focus.state |= State_FocusAtBorder;
                focus.backgroundColor = panel->palette.highlight().color();
                drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
            }
        }
        break;
    case PE_IndicatorCheckBox: {
        QBrush fill;
        if (opt->state & State_NoChange)
            fill = QBrush(opt->palette.base().color(), Qt::Dense4Pattern);
        else if (opt->state & State_Sunken)
            fill = opt->palette.button();
        else if (opt->state & State_Enabled)
            fill = opt->palette.base();
        else
            fill = opt->palette.background();
        p->save();
        doRestore = true;
        qDrawShadePanel(p, opt->rect.x(), opt->rect.y()+1, opt->rect.width(), opt->rect.height(),
                        opt->palette, true, 1, &fill);
        if (opt->state & State_NoChange)
            p->setPen(opt->palette.dark().color());
        else
            p->setPen(opt->palette.text().color());
        } // fall through
    case PE_IndicatorViewItemCheck:
        if (!doRestore) {
            p->save();
            doRestore = true;
        }
        if (pe == PE_IndicatorViewItemCheck) {
            if (opt->state & State_Enabled)
                p->setPen(QPen(opt->palette.text().color(), 1));
            else
                p->setPen(QPen(opt->palette.dark().color(), 1));
            if (opt->state & State_NoChange)
                p->setBrush(opt->palette.brush(QPalette::Button));
            p->drawRect(opt->rect.x() + 1, opt->rect.y() + 1,
                pixelMetric(PM_IndicatorWidth, opt, widget),
                pixelMetric(PM_IndicatorHeight, opt, widget));
        }
        if (!(opt->state & State_Off) && (opt->rect.height() > 6)) {
            int hh = opt->rect.height()-6;
            QLineF *lines = new QLineF [hh];
            int i, xx, yy;
            xx = opt->rect.x() + 3;
            yy = opt->rect.y() + 3 + hh/2;
            for (i = 0; i < hh/3+1; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                ++yy;
            }
            yy -= 2;
            for (i = hh/3+1; i < hh; ++i) {
                lines[i] = QLineF(xx, yy, xx, yy + 2);
                ++xx;
                --yy;
            }
            p->drawLines(lines, hh);
            delete [] lines;
        }
        if (doRestore)
            p->restore();
        break;
    case PE_IndicatorRadioButton: {
            p->save();
            p->setRenderHint(QPainter::Antialiasing);
            bool down = opt->state & State_Sunken;
            bool enabled = opt->state & State_Enabled;
            bool on = opt->state & State_On;

            QRect ir(opt->rect);
            ir.adjust(0, 1, 0, 1);

            p->setPen(opt->palette.shadow().color());
            p->drawArc(ir, 45*16, 180*16);

            p->setPen(opt->palette.light().color());
            p->drawArc(ir, 225*16, 180*16);

            ir.adjust(1, 1, -1, -1);
            QColor fillColor = (down || !enabled)
                               ? opt->palette.button().color()
                               : opt->palette.base().color();
            p->setPen(fillColor);
            p->setBrush(fillColor) ;
            p->drawEllipse(ir);

            if (on) {
                p->setPen(Qt::NoPen);
                p->setBrush(opt->palette.text());
                ir.adjust(2, 2, -2, -2);
                if (ir.width() < 5)
                    ir.adjust(-1, -1, 1, 1);
                p->drawEllipse(ir);
            }
            p->restore();
        }
        break;
    case PE_IndicatorSpinUp:
        drawPrimitive(PE_IndicatorArrowRight, opt, p, widget);
        break;
    case PE_IndicatorSpinDown:
        drawPrimitive(PE_IndicatorArrowLeft, opt, p, widget);
        break;
    default:
        QWindowsStyle::drawPrimitive(pe, opt, p, widget);
    }
}

static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget = 0)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
        case Qt::LeftArrow:
            pe = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            pe = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            pe = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            pe = QStyle::PE_IndicatorArrowDown;
            break;
        default:
            return;
    }
    QStyleOption arrowOpt;
    arrowOpt.rect = rect;
    arrowOpt.palette = toolbutton->palette;
    arrowOpt.state = toolbutton->state;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}

void QPhoneStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    switch (ce) {
    case CE_MenuItem:
        if (const QStyleOptionMenuItem *menuitem = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int x, y, w, h;
            menuitem->rect.getRect(&x, &y, &w, &h);
            int tab = menuitem->tabWidth;
            bool dis = !(menuitem->state & State_Enabled);
            bool checked = menuitem->checkType != QStyleOptionMenuItem::NotCheckable
                            ? menuitem->checked : false;
            bool act = menuitem->state & State_Selected;

            // windows always has a check column, regardless whether we have an icon or not
            int checkcol = qMax(menuitem->maxIconWidth, 18);

            if (menuitem->menuItemType == QStyleOptionMenuItem::Separator) {
                p->setPen(menuitem->palette.dark().color());
                p->drawLine(x, y, x + w, y);
                p->setPen(menuitem->palette.light().color());
                p->drawLine(x, y + 1, x + w, y + 1);
                return;
            }

            QBrush fill = menuitem->palette.brush(act ? QPalette::Highlight : QPalette::Button);
            p->fillRect(menuitem->rect, fill);

            QRect vCheckRect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x(), menuitem->rect.y(), checkcol, menuitem->rect.height()));
            if (checked) {
                if (act && !dis) {
                    qDrawShadePanel(p, vCheckRect,
                                    menuitem->palette, true, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                } else {
                    QBrush fill(menuitem->palette.light().color(), Qt::Dense4Pattern);
                    qDrawShadePanel(p, vCheckRect, menuitem->palette, true, 1, &fill);
                }
            } else if (!act) {
                p->fillRect(vCheckRect, menuitem->palette.brush(QPalette::Button));
            }

            // On Windows Style, if we have a checkable item and an icon we
            // draw the icon recessed to indicate an item is checked. If we
            // have no icon, we draw a checkmark instead.
            if (!menuitem->icon.isNull()) {
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                if (act && !dis)
                    mode = QIcon::Active;
                QPixmap pixmap = menuitem->icon.pixmap(
                        pixelMetric(PM_SmallIconSize), mode, checked ? QIcon::On : QIcon::Off);
                if (act && !dis && !checked)
                    qDrawShadePanel(p, vCheckRect,  menuitem->palette, false, 1,
                                    &menuitem->palette.brush(QPalette::Button));
                QRect pmr(0, 0, pixmap.width(), pixmap.height());
                pmr.moveCenter(vCheckRect.center());
                p->setPen(menuitem->palette.text().color());
                p->drawPixmap(pmr.topLeft(), pixmap);
            } else if (checked) {
                QStyleOptionMenuItem newMi = *menuitem;
                newMi.state = State_None;
                if (!dis)
                    newMi.state |= State_Enabled;
                if (act)
                    newMi.state |= State_On;
                newMi.rect = visualRect(opt->direction, menuitem->rect, QRect(menuitem->rect.x() + phoneItemFrame, menuitem->rect.y() + phoneItemFrame,
                                                                              checkcol - 2 * phoneItemFrame, menuitem->rect.height() - 2*phoneItemFrame));
                drawPrimitive(PE_IndicatorMenuCheckMark, &newMi, p, widget);
            }
            p->setPen(act ? menuitem->palette.highlightedText().color() : menuitem->palette.buttonText().color());

            QColor discol;
            if (dis) {
                discol = menuitem->palette.text().color();
                p->setPen(discol);
            }

            int xm = phoneItemFrame + checkcol + phoneItemHMargin;
            int xpos = menuitem->rect.x() + xm;
            QRect textRect(xpos, y + phoneItemVMargin, w - xm - phoneRightBorder - tab + 1, h - 2 * phoneItemVMargin);
            QRect vTextRect = visualRect(opt->direction, menuitem->rect, textRect);
            QString s = menuitem->text;
            if (!s.isEmpty()) {                     // draw text
                p->save();
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine;
                if (!styleHint(SH_UnderlineShortcut, menuitem, widget))
                    text_flags |= Qt::TextHideMnemonic;
                text_flags |= Qt::AlignLeft;
                if (t >= 0) {
                    QRect vShortcutRect = visualRect(opt->direction, menuitem->rect, QRect(textRect.topRight(), menuitem->rect.bottomRight()));
                    if (dis && !act) {
                        p->setPen(menuitem->palette.light().color());
                        p->drawText(vShortcutRect.adjusted(1,1,1,1), text_flags, s.mid(t + 1));
                        p->setPen(discol);
                    }
                    p->drawText(vShortcutRect, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                QFont font = menuitem->font;
                if (menuitem->menuItemType == QStyleOptionMenuItem::DefaultItem)
                    font.setBold(true);
                p->setFont(font);
                if (dis && !act) {
                    p->setPen(menuitem->palette.light().color());
                    p->drawText(vTextRect.adjusted(1,1,1,1), text_flags, s.left(t));
                    p->setPen(discol);
                }
                p->drawText(vTextRect, text_flags, s.left(t));
                p->restore();
            }
            if (menuitem->menuItemType == QStyleOptionMenuItem::SubMenu) {// draw sub menu arrow
                int dim = (h - 2 * phoneItemFrame) / 2;
                PrimitiveElement arrow;
                arrow = QApplication::isRightToLeft() ? PE_IndicatorArrowLeft : PE_IndicatorArrowRight;
                xpos = x + w - phoneArrowHMargin - phoneItemFrame - dim;
                QRect  vSubMenuRect = visualRect(opt->direction, menuitem->rect, QRect(xpos, y + h / 2 - dim / 2, dim, dim));
                QStyleOptionMenuItem newMI = *menuitem;
                newMI.rect = vSubMenuRect;
                newMI.state = dis ? State_None : State_Enabled;
                if (act)
                    newMI.palette.setColor(QPalette::ButtonText,
                                           newMI.palette.highlightedText().color());
                drawPrimitive(arrow, &newMI, p, widget);
            }

        }
        break;
    case CE_ScrollBarAddPage:
    case CE_ScrollBarSubPage: {
            QBrush br;
            p->setPen(Qt::NoPen);
            if (opt->state & State_Sunken) {
                br = QBrush(opt->palette.shadow());
                p->setBrush(br);
            } else {
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
                if (d->bgExport->isAvailable()) {
                    QColor col = opt->palette.light().color();
                    col.setAlpha(192);
                    br = QBrush(col);
                } else
#endif
                {
                    QPixmap pm = opt->palette.brush(QPalette::Light).texture();
                    br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color());
                }
                p->setBrush(br);
            }
            p->drawRect(opt->rect);
            break; }
    case CE_RadioButton:
    case CE_CheckBox:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            bool isRadio = (ce == CE_RadioButton);
            if (btn->state & State_HasFocus) {
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = subElementRect(isRadio ? SE_RadioButtonFocusRect
                    : SE_CheckBoxFocusRect, btn, widget);
                p->fillRect(fropt.rect.x(), fropt.rect.y(), fropt.rect.width(), fropt.rect.height(), opt->palette.brush(QPalette::Highlight));
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(isRadio ? SE_RadioButtonIndicator
                : SE_CheckBoxIndicator, btn, widget);
            drawPrimitive(isRadio ? PE_IndicatorRadioButton : PE_IndicatorCheckBox,
                          &subopt, p, widget);
            subopt.rect = subElementRect(isRadio ? SE_RadioButtonContents
                : SE_CheckBoxContents, btn, widget);
            drawControl(isRadio ? CE_RadioButtonLabel : CE_CheckBoxLabel, &subopt, p, widget);
        }
        break;
    case CE_RadioButtonLabel:
    case CE_CheckBoxLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            uint alignment = visualAlignment(btn->direction, Qt::AlignLeft | Qt::AlignVCenter);

            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                alignment |= Qt::TextHideMnemonic;
            QPixmap pix;
            QRect textRect = btn->rect;
            if (!btn->icon.isNull()) {
                pix = btn->icon.pixmap(btn->iconSize, btn->state & State_Enabled ? QIcon::Normal : QIcon::Disabled);
                drawItemPixmap(p, btn->rect, alignment, pix);
                if (btn->direction == Qt::RightToLeft)
                    textRect.setRight(textRect.right() - btn->iconSize.width() - 4);
                else
                    textRect.setLeft(textRect.left() + btn->iconSize.width() + 4);
            }
            if (!btn->text.isEmpty()){
                drawItemText(p, textRect, alignment | Qt::TextShowMnemonic,
                             btn->palette, btn->state & State_Enabled, btn->text, 
                             btn->state & State_HasFocus ? QPalette::HighlightedText : QPalette::WindowText);
            }
        }
        break;
    case CE_PushButton:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            drawControl(CE_PushButtonBevel, btn, p, widget);
            if (btn->state & State_HasFocus) {
                QRect r = subElementRect(SE_PushButtonFocusRect, btn, widget);
                p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2,
                            btn->palette.brush(QPalette::Highlight));
                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*btn);
                fropt.rect = subElementRect(SE_PushButtonFocusRect, btn, widget);
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
            QStyleOptionButton subopt = *btn;
            subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
            drawControl(CE_PushButtonLabel, &subopt, p, widget);
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            QRect ir = btn->rect;
            uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
            if (!styleHint(SH_UnderlineShortcut, btn, widget))
                tf |= Qt::TextHideMnemonic;

            if (btn->state & (State_On | State_Sunken))
                ir.translate(pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                                pixelMetric(PM_ButtonShiftVertical, opt, widget));
            if (!btn->icon.isNull()) {
                QIcon::Mode mode = btn->state & State_Enabled ? QIcon::Normal
                    : QIcon::Disabled;
                if (mode == QIcon::Normal && btn->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (btn->state & State_On)
                    state = QIcon::On;
                QPixmap pixmap = btn->icon.pixmap(btn->iconSize, mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                //Center the icon if there is no text

                QPoint point;
                if (btn->text.isEmpty()) {
                    point = QPoint(ir.x() + ir.width() / 2 - pixw / 2,
                                    ir.y() + ir.height() / 2 - pixh / 2);
                } else {
                    point = QPoint(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2);
                }
                if (btn->direction == Qt::RightToLeft)
                    point.rx() += pixw;

                if ((btn->state & (State_On | State_Sunken)) && btn->direction == Qt::RightToLeft)
                    point.rx() -= pixelMetric(PM_ButtonShiftHorizontal, opt, widget) * 2;

                p->drawPixmap(visualPos(btn->direction, btn->rect, point), pixmap);

                if (btn->direction == Qt::RightToLeft)
                    ir.translate(-4, 0);
                else
                    ir.translate(pixw + 4, 0);
                ir.setWidth(ir.width() - (pixw + 4));
                // left-align text if there is
                if (!btn->text.isEmpty())
                    tf |= Qt::AlignLeft;
            } else {
                tf |= Qt::AlignHCenter;
            }
            drawItemText(p, ir, tf, btn->palette, (btn->state & State_Enabled),
                         btn->text, btn->state & State_HasFocus ? QPalette::HighlightedText : QPalette::ButtonText);
        }
        break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if ((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty()
                    || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
                if (!styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                drawItemText(p, rect, alignment, toolbutton->palette,
                                opt->state & State_Enabled, toolbutton->text,
                                QPalette::ButtonText);
                } else {
                    QPixmap pm;
                    QSize pmSize = toolbutton->iconSize;
                    if (!toolbutton->icon.isNull()) {
                        QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                        QIcon::Mode mode;
                        if (!(toolbutton->state & State_Enabled))
                            mode = QIcon::Disabled;
                        else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                            mode = QIcon::Active;
                        else
                            mode = QIcon::Normal;
                        pm = toolbutton->icon.pixmap(toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                mode, state);
                        pmSize = pm.size();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    p->setFont(toolbutton->font);
                    QRect pr = rect,
                    tr = rect;
                    int alignment = Qt::TextShowMnemonic;
                    if (!styleHint(SH_UnderlineShortcut, opt, widget))
                        alignment |= Qt::TextHideMnemonic;

                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        pr.setHeight(pmSize.height() + 6);

                        tr.adjust(0, pr.bottom(), 0, -3);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(this, toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignCenter;
                    } else {
                        pr.setWidth(pmSize.width() + 8);
                        tr.adjust(pr.right(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                        } else {
                            drawArrow(this, toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                    }
                    tr.translate(shiftX, shiftY);
                    drawItemText(p, tr, alignment, toolbutton->palette,
                                toolbutton->state & State_Enabled, toolbutton->text,
                                toolbutton->state & State_HasFocus ? QPalette::HighlightedText : QPalette::ButtonText);
                } else {
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        drawArrow(this, toolbutton, rect, p, widget);
                    } else {
                        drawItemPixmap(p, rect, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        break;
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (!(tab->state & State_Selected)) {
                QRect rect = opt->rect.adjusted(0, 4, 0, -1);
                p->fillRect(rect, opt->palette.brush(QPalette::Button));
            }
            if ((tab->state & State_HasFocus)) {
                QRect r;
                QStyleOptionTabV2 tabV2(*tab);
                const int OFFSET = 1 + pixelMetric(PM_DefaultFrameWidth);

                int x1, x2;
                x1 = tabV2.rect.left();
                x2 = tabV2.rect.right() - 1;
                r.setRect(x1 + 1 + OFFSET, tabV2.rect.y() + OFFSET,
                    x2 - x1 - 2*OFFSET, tabV2.rect.height() - 2*OFFSET);                
                
                p->fillRect(r.x(), r.y(), r.width(), r.height(), opt->palette.brush(QPalette::Highlight));
            }
        }
        QWindowsStyle::drawControl(ce, opt, p, widget);
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTabV2 tabV2(*tab);
            QRect tr = tabV2.rect;
            bool verticalTabs = tabV2.shape == QTabBar::RoundedEast
                                || tabV2.shape == QTabBar::RoundedWest
                                || tabV2.shape == QTabBar::TriangularEast
                                || tabV2.shape == QTabBar::TriangularWest;
            bool selected = tabV2.state & State_Selected;
            if (verticalTabs) {
                p->save();
                int newX, newY, newRot;
                if (tabV2.shape == QTabBar::RoundedEast || tabV2.shape == QTabBar::TriangularEast) {
                    newX = tr.width();
                    newY = tr.y();
                    newRot = 90;
                } else {
                    newX = 0;
                    newY = tr.y() + tr.height();
                    newRot = -90;
                }
                tr.setRect(0, 0, tr.height(), tr.width());
                QMatrix m;
                m.translate(newX, newY);
                m.rotate(newRot);
                p->setMatrix(m, true);
            }
            tr.adjust(0, 0, pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget),
                            pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget));

            if (selected)
            {
                tr.setBottom(tr.bottom() - pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab,
                                                    widget));
                tr.setRight(tr.right() - pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab,
                                                    widget));
            }

            int alignment = Qt::AlignCenter | Qt::TextShowMnemonic;
            if (!styleHint(SH_UnderlineShortcut, opt, widget))
                alignment |= Qt::TextHideMnemonic;
            if (!tabV2.icon.isNull()) {
                QSize iconSize = tabV2.iconSize;
                if (!iconSize.isValid()) {
                    int iconExtent = pixelMetric(PM_SmallIconSize);
                    iconSize = QSize(iconExtent, iconExtent);
                }
                QPixmap tabIcon = tabV2.icon.pixmap(iconSize,
                                                    (tabV2.state & State_Enabled) ? QIcon::Normal
                                                                                  : QIcon::Disabled);
                p->drawPixmap(tr.left() + 6, tr.center().y() - tabIcon.height() / 2, tabIcon);
                tr.setLeft(tr.left() + iconSize.width() + 4);
            }

            drawItemText(p, tr, alignment, tab->palette, tab->state & State_Enabled, tab->text,
                         tab->state & State_HasFocus ? QPalette::HighlightedText : QPalette::WindowText);
            if (verticalTabs)
                p->restore();

            if (tabV2.state & State_HasFocus) {
                const int OFFSET = 1 + pixelMetric(PM_DefaultFrameWidth);

                int x1, x2;
                x1 = tabV2.rect.left();
                x2 = tabV2.rect.right() - 1;

                QStyleOptionFocusRect fropt;
                fropt.QStyleOption::operator=(*tab);
                fropt.rect.setRect(x1 + 1 + OFFSET, tabV2.rect.y() + OFFSET,
                                   x2 - x1 - 2*OFFSET, tabV2.rect.height() - 2*OFFSET);
                drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
            }
        }
        break;
    default:
        QWindowsStyle::drawControl(ce, opt, p, widget);
        break;
    }
}

void QPhoneStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                      QPainter *p, const QWidget *widget) const
{
    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (cmb->subControls & SC_ComboBoxArrow) {
                State flags = State_None;

                QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxFrame, widget);
                QBrush editBrush = (cmb->state & State_Enabled && cmb->state & State_HasEditFocus)
                                    ? cmb->palette.brush(QPalette::Base)
                                    : cmb->palette.brush(QPalette::Window);
                p->fillRect(re, editBrush);
                if (cmb->frame && cmb->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.state = State_KeyboardFocusChange;
                    fropt.rect = opt->rect;
                    fropt.palette = opt->palette;
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                } else if (!(cmb->state & State_HasFocus)) {
                    QPen oldPen = p->pen();
                    QPen pen(Qt::DotLine);
                    pen.setColor(opt->palette.color(QPalette::Shadow));
                    p->setPen(pen);
                    p->drawLine(re.x(), re.bottom(), re.right(), re.bottom());
                    p->setPen(oldPen);
                }

                QRect ar = subControlRect(CC_ComboBox, cmb, SC_ComboBoxArrow, widget);
                if (cmb->activeSubControls == SC_ComboBoxArrow) {
                    p->setPen(cmb->palette.dark().color());
                    p->setBrush(cmb->palette.brush(QPalette::Button));
                    p->drawRect(ar.adjusted(0,0,-1,-1));
                }

                ar.adjust(2, 2, -2, -2);
                if (opt->state & State_Enabled)
                    flags |= State_Enabled;

                if (cmb->activeSubControls == SC_ComboBoxArrow)
                    flags |= State_Sunken;
                QStyleOption arrowOpt(0);
                arrowOpt.rect = ar;
                arrowOpt.palette = cmb->palette;
                arrowOpt.state = flags;
                drawPrimitive(PE_IndicatorArrowDown, &arrowOpt, p, widget);
            }
            if (cmb->subControls & SC_ComboBoxEditField) {
                QRect re = subControlRect(CC_ComboBox, cmb, SC_ComboBoxEditField, widget);
                if (cmb->state & State_HasFocus && !cmb->editable)
                    p->fillRect(re.x(), re.y(), re.width(), re.height(),
                                cmb->palette.brush(QPalette::Highlight));

                if (cmb->state & State_HasFocus) {
                    p->setPen(cmb->palette.highlightedText().color());
                    p->setBackground(cmb->palette.highlight());

                } else {
                    p->setPen(cmb->palette.text().color());
                    p->setBackground(cmb->palette.background());
                }

                if (cmb->state & State_HasFocus && !cmb->editable) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*cmb);
                    focus.rect = subElementRect(SE_ComboBoxFocusRect, cmb, widget);
                    focus.state |= State_FocusAtBorder;
                    focus.backgroundColor = cmb->palette.highlight().color();
                    drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox copy = *sb;
            PrimitiveElement pe;

            if (sb->frame && (sb->subControls & SC_SpinBoxFrame)) {
                QRect r = subControlRect(CC_SpinBox, sb, SC_SpinBoxFrame, widget);
                if (sb->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.state = State_KeyboardFocusChange;
                    fropt.rect = opt->rect;
                    fropt.palette = opt->palette;
                    drawPrimitive(PE_FrameFocusRect, &fropt, p, widget);
                } else {
                    QPen oldPen = p->pen();
                    QPen pen(Qt::DotLine);
                    pen.setColor(sb->palette.color(QPalette::Shadow));
                    p->setPen(pen);
                    p->drawLine(r.x(), r.bottom(), r.right(), r.bottom());
                    p->setPen(oldPen);
                }
            }

            if (sb->subControls & SC_SpinBoxUp && !(sb->state & State_HasEditFocus)) {
                copy.subControls = SC_SpinBoxUp;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepUpEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }

                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxUp && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinPlus
                                                                       : PE_IndicatorSpinUp);

                copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxUp, widget);
//                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.adjust(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }

            if (sb->subControls & SC_SpinBoxDown && !(sb->state & State_HasEditFocus)) {
                copy.subControls = SC_SpinBoxDown;
                copy.state = sb->state;
                QPalette pal2 = sb->palette;
                if (!(sb->stepEnabled & QAbstractSpinBox::StepDownEnabled)) {
                    pal2.setCurrentColorGroup(QPalette::Disabled);
                    copy.state &= ~State_Enabled;
                }
                copy.palette = pal2;

                if (sb->activeSubControls == SC_SpinBoxDown && (sb->state & State_Sunken)) {
                    copy.state |= State_On;
                    copy.state |= State_Sunken;
                } else {
                    copy.state |= State_Raised;
                    copy.state &= ~State_Sunken;
                }
                pe = (sb->buttonSymbols == QAbstractSpinBox::PlusMinus ? PE_IndicatorSpinMinus
                                                                       : PE_IndicatorSpinDown);

                copy.rect = subControlRect(CC_SpinBox, sb, SC_SpinBoxDown, widget);
//                drawPrimitive(PE_PanelButtonBevel, &copy, p, widget);
                copy.rect.adjust(3, 0, -4, 0);
                drawPrimitive(pe, &copy, p, widget);
            }
        }
        break;
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            if (slider->state & State_HasFocus) {
                QRect r = subElementRect(SE_SliderFocusRect, slider, widget);
                p->fillRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2,
                    slider->palette.highlight().color());
            }
        }
        QWindowsStyle::drawComplexControl(cc, opt, p, widget);
        break;
    case CC_ToolButton:
        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QRect button, menuarea;
            button = subControlRect(cc, toolbutton, SC_ToolButton, widget);
            menuarea = subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget);

            State bflags = toolbutton->state;

            if (bflags & State_AutoRaise) {
                if (!(bflags & State_MouseOver)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags;

            if (toolbutton->activeSubControls & SC_ToolButton)
                bflags |= State_Sunken;
            if (toolbutton->activeSubControls & SC_ToolButtonMenu)
                mflags |= State_Sunken;

            QStyleOption tool(0);
            tool.palette = toolbutton->palette;
            if (toolbutton->subControls & SC_ToolButton) {
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.rect = button;
                    tool.state = bflags;
                    drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                }
            }

            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (State_Sunken | State_On | State_Raised))
                    drawPrimitive(PE_IndicatorButtonDropDown, &tool, p, widget);
                drawPrimitive(PE_IndicatorArrowDown, &tool, p, widget);
            }

            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.QStyleOption::operator=(*toolbutton);
                fr.rect.adjust(3, 3, -3, -3);
                if (toolbutton->features & QStyleOptionToolButton::Menu)
                    fr.rect.adjust(0, 0, -pixelMetric(QStyle::PM_MenuButtonIndicator,
                                    toolbutton, widget), 0);
                p->fillRect(fr.rect.x()+1, fr.rect.y()+1, fr.rect.width()-2, fr.rect.height()-2,
                    toolbutton->palette.brush(QPalette::Highlight));
                drawPrimitive(PE_FrameFocusRect, &fr, p, widget);
            }
            QStyleOptionToolButton label = *toolbutton;
            int fw = pixelMetric(PM_DefaultFrameWidth, opt, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw);
            drawControl(CE_ToolButtonLabel, &label, p, widget);
        }
        break;
    default:
        QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
}

bool QPhoneStyle::event(QEvent *e)
{
    if (e->type() == QEvent::EnterEditFocus || e->type() == QEvent::LeaveEditFocus) {
        QWidget *focusWidget = QApplication::focusWidget();
        if (focusWidget && !QApplication::activePopupWidget()) {
            QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(focusWidget);
            if (aiv) {
                QPalette::ColorRole role = aiv->viewport()->backgroundRole();
                if (e->type() == QEvent::EnterEditFocus) {
                    if (role != QPalette::Base)
                        aiv->viewport()->setBackgroundRole(QPalette::Base);
                } else if (!isSingleFocusWidget(focusWidget)) {
                    // If a single focus widget is losing edit focus
                    // then the window is probably being closed, otherwise
                    // change the role.
                    if (role != QPalette::Window)
                        aiv->viewport()->setBackgroundRole(QPalette::Window);
                }
            }
            if (QLineEdit *le = qobject_cast<QLineEdit*>(focusWidget)) {
                le->update();
            }
            if (focusWidget->inherits("QAbstractSpinBox")) {
                focusWidget->update();
            }
            QPoint pos = focusWidget->mapTo(focusWidget->window(), QPoint(0,0));
            if (pos.y() == 0 && pos.x() < 20)
                d->updateDecoration();
        }
    }
    return QWindowsStyle::event(e);
}

#include "qphonestyle.moc"
