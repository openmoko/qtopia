/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "qthumbstyle_p.h"
#include <QApplication>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QAbstractItemView>
#include <QDebug>

class QThumbStylePrivate : public QObject
{
    Q_OBJECT
public:
    QThumbStylePrivate();
    ~QThumbStylePrivate();

    bool handleMousePress(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);
    bool handleMouseMove(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);
    bool handleMouseRelease(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e);

    bool eventFilter(QObject *o, QEvent *e);

    QSize editableStrut;

private:
    QPoint mousePos;
    QAbstractScrollArea *scrollArea;
    QWidget *target;
    bool filterPress;
    bool waitRelease;
    int moveThreshold;
};

QThumbStylePrivate::QThumbStylePrivate()
{
    scrollArea = 0;
    target = 0;
    filterPress = true;
    waitRelease = false;
    moveThreshold = QApplication::desktop()->screenGeometry().width()/40;
    qApp->installEventFilter(this);
}

QThumbStylePrivate::~QThumbStylePrivate()
{
    qApp->removeEventFilter(this);
}

bool QThumbStylePrivate::handleMousePress(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e)
{
    if (waitRelease)
        return true;
    if (e->button() == Qt::LeftButton && !qobject_cast<QSlider*>(t)) {
        target = t;
        QWidget *cw = t;
        while (cw) {
            if (cw->focusPolicy() != Qt::NoFocus)
                break;
            cw = cw->parentWidget();
        }
        if (!cw) {
            // If we click outside a focusable widget, clear focus
            // to hide the input method.
            if (QApplication::focusWidget() && w)
                QApplication::focusWidget()->clearFocus();
        }

        if (w) {
            waitRelease = true;
            if (!filterPress) {
                filterPress = true;
                return false;
            }
            scrollArea = w;
            mousePos = e->globalPos();
            filterPress = false;
            e->accept();
            return true;
        }
    }

    return false;
}

bool QThumbStylePrivate::handleMouseMove(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e)
{
    if (scrollArea) {
        QPoint diff = mousePos - e->globalPos();
        if (!filterPress
            && (qAbs(diff.y()) > moveThreshold 
            || qAbs(diff.x()) > moveThreshold)) {
            filterPress = true;
        }
        if (filterPress) {
            QScrollBar *sb = scrollArea->verticalScrollBar();
            if (diff.y() && sb->isVisible() && sb->isEnabled() && sb->height()) {
                int moveY = diff.y() * sb->pageStep() / sb->height();
                sb->setValue(sb->value() + moveY);
            }

            sb = scrollArea->horizontalScrollBar();
            if (diff.x() && sb->isVisible() && sb->isEnabled() && sb->width()) {
                int moveX = diff.x() * sb->pageStep() / sb->width();
                sb->setValue(sb->value() + moveX);
            }

            mousePos = e->globalPos();
        }
        e->accept();
        return true;
    }

    return false;
}

bool QThumbStylePrivate::handleMouseRelease(QAbstractScrollArea *w, QWidget *t, QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        waitRelease = false;
        if (target) {
            if (scrollArea) {
                scrollArea = 0;
                if (!filterPress) {
                    // We haven't moved enough, so repeat click but don't filter.
                    QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonPress, t->mapFromGlobal(mousePos), e->button(), e->buttons(), QApplication::keyboardModifiers()));
                    QApplication::postEvent(target, new QMouseEvent(QEvent::MouseButtonRelease, t->mapFromGlobal(mousePos), e->button(), e->buttons(), QApplication::keyboardModifiers()));
                }
                e->accept();
                target = 0;
                return true;
            } else {
                QWidget *fw = target;
                while (fw) {
                    if (fw->isEnabled() && fw->focusPolicy() != Qt::NoFocus) {
                        fw->setFocus(Qt::MouseFocusReason);
                        break;
                    }
                    if (fw->isWindow())
                        break;
                    fw = fw->parentWidget();
                }
            }
            target = 0;
        }
    }

    return false;
}

bool QThumbStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return QObject::eventFilter(o, e);

    QWidget *widget = (QWidget*)o;

    if (e->type() == QEvent::MouseButtonPress
            || e->type() == QEvent::MouseMove
            || e->type() == QEvent::MouseButtonRelease) {
        QAbstractScrollArea *sa = 0;
        while (widget->parentWidget()) {
            if (widget->objectName() == QLatin1String("qt_scrollarea_viewport")) {
                sa = qobject_cast<QAbstractScrollArea*>(widget->parentWidget());
            }
            widget = widget->parentWidget();
        }
        switch (e->type()) {
            case QEvent::MouseButtonPress:
                if (handleMousePress(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            case QEvent::MouseMove:
                if (handleMouseMove(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            case QEvent::MouseButtonRelease:
                if (handleMouseRelease(sa, (QWidget*)o, (QMouseEvent*)e))
                    return true;
                break;
            default:
                break;
        }
    }

    return QObject::eventFilter(o, e);
}

//===========================================================================

QThumbStyle::QThumbStyle() : QPhoneStyle()
{
    d = new QThumbStylePrivate;

    int dpi = QApplication::desktop()->screen()->logicalDpiY();

    // 18 pixels on a 100dpi screen
    int strutSize = qRound(18.0 * dpi / 100.0);
    d->editableStrut = QSize(strutSize, strutSize);
}

QThumbStyle::~QThumbStyle()
{
    delete d;
}

void QThumbStyle::polish(QWidget *widget)
{
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView*>(widget);
    if (aiv) {
        aiv->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        aiv->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    }
    if (qobject_cast<QTabWidget*>(widget))
        widget->setFocusPolicy(Qt::NoFocus);

    if (widget->focusPolicy() & Qt::ClickFocus) {
        // We'll handle click to focus ourselves.
        widget->setFocusPolicy(Qt::TabFocus);
    }

    QPhoneStyle::polish(widget);
}

QSize QThumbStyle::sizeFromContents(ContentsType type, const QStyleOption* opt,
                                const QSize &csz, const QWidget *widget ) const
{
    QSize sz(csz);
    switch (type) {
    case CT_LineEdit:
    case CT_ComboBox:
    case CT_TabBarTab:
    case CT_CheckBox:
    case CT_ToolButton:
    case CT_PushButton:
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        sz = sz.expandedTo(d->editableStrut);
        break;
    case CT_MenuItem: {
        QStyleOptionMenuItem *mopt = (QStyleOptionMenuItem*)opt;
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        if (mopt->menuItemType == QStyleOptionMenuItem::Normal
            || mopt->menuItemType == QStyleOptionMenuItem::DefaultItem
            || mopt->menuItemType == QStyleOptionMenuItem::SubMenu)
            sz = sz.expandedTo(d->editableStrut);
        break; }
    default:
        sz = QPhoneStyle::sizeFromContents(type, opt, csz, widget);
        break;
    }

    return sz;
}

int QThumbStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_TabBarIconSize:
        ret = QtopiaStyle::pixelMetric(PM_TabBarIconSize, option, widget);  //bigger than normal QPhoneStyle tabicons
        break;

    case PM_ButtonIconSize:
    case PM_ToolBarIconSize:
    case PM_SmallIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 15x15 icon at 100dpi
                size = (15 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;

    case PM_LargeIconSize:
    case PM_MessageBoxIconSize:
    case PM_IconViewIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 32x32 icon at 100dpi
                size = (32 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;

    case PM_ListViewIconSize: {
            static int size = 0;
            if (!size) {
                // We would like a 22x20 icon at 100dpi
                size = (20 * QApplication::desktop()->screen()->logicalDpiY()+50) / 100;
            }
            ret = size;
        }
        break;
    case PM_MenuScrollerHeight:
        ret = 16;   //TODO: make dpi-aware
        break;
    default:
        ret = QPhoneStyle::pixelMetric(metric, option, widget);
    }

    return ret;
}


#include "qthumbstyle_p.moc"
