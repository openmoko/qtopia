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

#ifdef EZX_A780

#include "ezxphonestyle_p.h"

#include <QPainter>
#include <QWidget>
#include <QStyle>
#include <QStyleOption>
#include <QWindowsStyle>
#include <QDebug>
#include <QStringList>
#include <QStylePlugin>

#include <limits.h>

#if 0
class EzXPhoneStylePlugin : public QStylePlugin
{
public:
    EzXPhoneStylePlugin(QObject* par = 0) : QStylePlugin(par) {}

    QStyle* create(const QString& key) {
        if(key == "ezxphonestyle") {
            return new EzXPhoneStyle;
        }
        return 0;
    }

    QStringList keys() const {
        QStringList k;
        k << "ezxphonestyle";
        return k;
    }
};

Q_EXPORT_PLUGIN2(ezxphonestyle,EzXPhoneStylePlugin)
#endif

QPixmap* g_scrollDownPressed;
QPixmap* g_scrollDown;
QPixmap* g_scrollUpPressed;
QPixmap* g_scrollUp;
QPixmap* g_scrollBg;
QPixmap* g_scrollSliderMiddle;
QPixmap* g_scrollSliderTop;
QPixmap* g_scrollSliderBottom;

EzXPhoneStyle::EzXPhoneStyle()
{
}


void EzXPhoneStyle::loadPixmaps() const
{
    static bool loaded = false;
    if(!loaded) {
        g_scrollDownPressed = new QPixmap(":image/ezx/scrollbar-down-pressed");
        g_scrollDown = new QPixmap(":image/ezx/scrollbar-down");
        g_scrollUpPressed = new QPixmap(":image/ezx/scrollbar-up-pressed");
        g_scrollUp = new QPixmap(":image/ezx/scrollbar-up");
        g_scrollBg = new QPixmap(":image/ezx/scrollbar-bg");
        g_scrollSliderMiddle = new QPixmap(":image/ezx/scrollbar-slider-middle");
        g_scrollSliderBottom = new QPixmap(":image/ezx/scrollbar-slider-bottom");
        g_scrollSliderTop = new QPixmap(":image/ezx/scrollbar-slider-top");
        loaded = true;
    }
}

int EzXPhoneStyle::pixelMetric(PixelMetric metric, const QStyleOption *option,
                            const QWidget *widget) const {
    loadPixmaps();
    switch(metric) {
        case PM_ScrollBarExtent:
            return g_scrollUp->width();
        default:
            return QPhoneStyle::pixelMetric(metric, option, widget);
    }
}

int EzXPhoneStyle::scrollLineHeight() const
{
    loadPixmaps();
    return g_scrollUp->height();
}

QRect EzXPhoneStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                                SubControl sc, const QWidget *w) const
{
    loadPixmaps();
    switch(cc) {
        case CC_ScrollBar:
        {
            QRect ret;
            const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(opt);
            if(scrollbar) {
                int maxlen = ((scrollbar->orientation == Qt::Horizontal) ?
                              scrollbar->rect.width() : scrollbar->rect.height());
                maxlen -= g_scrollUp->height()*2;
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
                sliderstart += g_scrollUp->height();
                switch(sc) {
                    case SC_ScrollBarSubLine:
                        if (scrollbar->orientation == Qt::Horizontal) {
                            ret = QRect(0, 0, g_scrollUp->width(), g_scrollUp->height());
                        } else {
                            ret = QRect(0, 0, g_scrollUp->width(), g_scrollUp->height());
                        }
                        break;
                    case SC_ScrollBarAddLine:
                        if (scrollbar->orientation == Qt::Horizontal) {
                            ret = QRect(scrollbar->rect.width() - g_scrollDown->height(), 0, g_scrollDown->height(), g_scrollDown->width());
                        } else {
                            ret = QRect(0, scrollbar->rect.height()-g_scrollDown->height(), g_scrollDown->width(), g_scrollDown->height());
                        }
                        break;
                    case SC_ScrollBarAddPage:
                    case SC_ScrollBarSubPage:
                        {
                        if(sc == SC_ScrollBarAddPage) {
                            if (scrollbar->orientation == Qt::Horizontal)
                                ret.setRect(sliderstart + sliderlen, 0,
                                            maxlen - sliderstart - sliderlen + g_scrollDown->height(), g_scrollDown->width());
                            else
                                ret.setRect(0, sliderstart + sliderlen+1, g_scrollBg->width(),
                                            maxlen - sliderstart - sliderlen + g_scrollUp->height());
                        } else {
                            if (scrollbar->orientation == Qt::Horizontal)
                                ret.setRect(g_scrollUp->width(), 0, sliderstart - g_scrollUp->height(), g_scrollUp->width());
                            else
                                ret.setRect(0, g_scrollUp->height(), g_scrollUp->width(), sliderstart - g_scrollUp->height());

                            break;
                        }
                        break;
                    }
                    case SC_ScrollBarGroove:
                        if (scrollbar->orientation == Qt::Horizontal)
                            ret.setRect(g_scrollUp->height(), 0, scrollbar->rect.width() - g_scrollUp->height()* 2,
                                        scrollbar->rect.height());
                        else
                            ret.setRect(0, g_scrollUp->height(), scrollbar->rect.width(),
                                            scrollbar->rect.height() - (g_scrollUp->height()*2));
                        break;
                    case SC_ScrollBarSlider:
                        if (scrollbar->orientation == Qt::Horizontal)
                            ret.setRect(sliderstart, 0, sliderlen, pixelMetric(PM_ScrollBarExtent, scrollbar, w));
                        else
                            ret.setRect(0, sliderstart, pixelMetric(PM_ScrollBarExtent, scrollbar, w), sliderlen);
                        break;
                    default:
                        return QPhoneStyle::subControlRect(cc, opt, sc, w);
                }
                return visualRect(scrollbar->direction, scrollbar->rect, ret);
            }
            // fall through
        }
        default:
            return QPhoneStyle::subControlRect(cc, opt, sc, w);
    }
    return QPhoneStyle::subControlRect(cc,opt,sc,w);
}

void EzXPhoneStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                                const QWidget *widget) const
{
    loadPixmaps();
    switch(ce) {
        case CE_ScrollBarAddLine: {
            if ((opt->state & State_Sunken)) {
                p->drawPixmap(opt->rect, *g_scrollDownPressed);
            } else {
                p->drawPixmap(opt->rect, *g_scrollDown);
            }
            break;
        }
        case CE_ScrollBarSubLine:
        {
            if ((opt->state & State_Sunken)) {
                p->drawPixmap(opt->rect, *g_scrollUpPressed);
            } else {
                p->drawPixmap(opt->rect, *g_scrollUp);
            }
            break;
        }
        case CE_ScrollBarAddPage:
        case CE_ScrollBarSubPage: {
#if 0
                QBrush br;
                QBrush bg = p->background();
                Qt::BGMode bg_mode = p->backgroundMode();
                p->setPen(Qt::NoPen);
                p->setBackgroundMode(Qt::OpaqueMode);

                if (opt->state & State_Sunken) {
                    br = QBrush(opt->palette.shadow().color(), Qt::Dense4Pattern);
                    p->setBackground(opt->palette.dark().color());
                    p->setBrush(br);
                } else {
                    QPixmap pm = opt->palette.brush(QPalette::Light).texture();
                    br = !pm.isNull() ? QBrush(pm) : QBrush(opt->palette.light().color(), Qt::Dense4Pattern);
                    p->setBackground(opt->palette.background().color());
                    p->setBrush(br);
                }
                p->drawRect(opt->rect);
                p->setBackground(bg);
                p->setBackgroundMode(bg_mode);
#endif
                p->drawTiledPixmap(opt->rect,*g_scrollBg);
                break; }
        case CE_ScrollBarSlider:
            {
                /*
                QStyleOptionButton buttonOpt;
                buttonOpt.QStyleOption::operator=(*opt);
                buttonOpt.state = State_Enabled | State_Raised;
                drawPrimitive(PE_PanelButtonBevel, &buttonOpt, p, widget);
                */
                p->drawTiledPixmap(opt->rect, *g_scrollSliderMiddle);
                p->drawPixmap(QPoint(opt->rect.x(), opt->rect.y()), *g_scrollSliderTop);
                p->drawPixmap(QPoint(opt->rect.x(), (opt->rect.y()+opt->rect.height())), *g_scrollSliderBottom);
            }
            break;
        default:
            QPhoneStyle::drawControl(ce, opt, p, widget);
    }
}
#endif
