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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qdrawutil.h>
#include "qdecorationdefault_qws.h"

#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)

QPixmap *QDecorationDefault::staticHelpPixmap = 0;
QPixmap *QDecorationDefault::staticMenuPixmap = 0;
QPixmap *QDecorationDefault::staticClosePixmap = 0;
QPixmap *QDecorationDefault::staticMinimizePixmap = 0;
QPixmap *QDecorationDefault::staticMaximizePixmap = 0;
QPixmap *QDecorationDefault::staticNormalizePixmap = 0;

#ifndef QT_NO_IMAGEFORMAT_XPM

/* XPM */
static const char * const default_menu_xpm[] = {
/* width height ncolors chars_per_pixel */
"16 16 11 1",
/* colors */
"  c #000000",
". c #336600",
"X c #666600",
"o c #99CC00",
"O c #999933",
"+ c #333300",
"@ c #669900",
"# c #999900",
"$ c #336633",
"% c #666633",
"& c #99CC33",
/* pixels */
"oooooooooooooooo",
"oooooooooooooooo",
"ooooo#.++X#ooooo",
"ooooX      Xoooo",
"oooX  XO#%  X&oo",
"oo#  Ooo&@O  Ooo",
"oo. Xoo#+ @X Xoo",
"oo+ OoO+ +O# +oo",
"oo+ #O+  +## +oo",
"oo. %@ ++ +. Xoo",
"oo#  O@OO+   #oo",
"oooX  X##$   Ooo",
"ooooX        Xoo",
"oooo&OX++X#OXooo",
"oooooooooooooooo",
"oooooooooooooooo"
};

static const char * const default_help_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"    ......      ",
"   ..XXXXXX     ",
"   .XX   .XX    ",
"   .XX   .XX    ",
"        ..XX    ",
"       ..XX     ",
"      ..XX      ",
"      .XX       ",
"      .XX       ",
"      ..        ",
"      .XX       ",
"      .XX       ",
"                ",
"                "};

static const char * const default_close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};

static const char * const default_maximize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  ...........   ",
"  .XXXXXXXXXX   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X       .X   ",
"  .X........X   ",
"  .XXXXXXXXXX   ",
"                ",
"                ",
"                "};

static const char * const default_minimize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"       ...      ",
"       . X      ",
"       .XX      ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                ",
"                "};

static const char * const default_normalize_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"     ........   ",
"     .XXXXXXXX  ",
"     .X     .X  ",
"     .X     .X  ",
"  ....X...  .X  ",
"  .XXXXXXXX .X  ",
"  .X     .XXXX  ",
"  .X     .X     ",
"  .X     .X     ",
"  .X......X     ",
"  .XXXXXXXX     ",
"                ",
"                ",
"                "};

#endif // QT_NO_IMAGEFORMAT_XPM

QDecorationDefault::QDecorationDefault()
    : QDecoration()
{
    menu_width = 20;
    help_width = 20;
    close_width = 20;
    minimize_width = 20;
    maximize_width = 20;
    normalize_width = 20;
}

QDecorationDefault::~QDecorationDefault()
{
    delete staticMenuPixmap;
    delete staticClosePixmap;
    delete staticMinimizePixmap;
    delete staticMaximizePixmap;
    delete staticNormalizePixmap;

    // This makes it safe to delete and then create a QDecorationDefault
    staticMenuPixmap = 0;
    staticClosePixmap = 0;
    staticMinimizePixmap = 0;
    staticMaximizePixmap = 0;
    staticNormalizePixmap = 0;
}

const char **QDecorationDefault::xpmForRegion(int reg)
{
#ifdef QT_NO_IMAGEFORMAT_XPM
    Q_UNUSED(reg);
#else
    switch(reg)
    {
    case Help:
        return (const char **)default_help_xpm;
    case Menu:
        return (const char **)default_menu_xpm;
    case Close:
        return (const char **)default_close_xpm;
    case Minimize:
        return (const char **)default_minimize_xpm;
    case Maximize:
        return (const char **)default_maximize_xpm;
    case Normalize:
        return (const char **)default_normalize_xpm;
    }
#endif
    return 0;
}

QPixmap QDecorationDefault::pixmapFor(const QWidget *widget, int decorationRegion,
                                      int &xoff, int &/*yoff*/)
{
#ifdef QT_NO_IMAGEFORMAT_XPM
    Q_UNUSED(widget);
    Q_UNUSED(decorationRegion);
    Q_UNUSED(xoff);
    return QPixmap();
#else
    static const char **staticHelpPixmapXPM = 0;
    static const char **staticMenuPixmapXPM = 0;
    static const char **staticClosePixmapXPM = 0;
    static const char **staticMinimizePixmapXPM = 0;
    static const char **staticMaximizePixmapXPM = 0;
    static const char **staticNormalizePixmapXPM = 0;
    const char **xpm;

    // Why don't we just use/extend the enum type...

    if (staticHelpPixmapXPM != (xpm = xpmForRegion(Help)) || !staticHelpPixmap) {
        staticHelpPixmapXPM = xpm;
        staticHelpPixmap = new QPixmap(xpm);
    }
    if (staticMenuPixmapXPM != (xpm = xpmForRegion(Menu)) || !staticMenuPixmap) {
        staticMenuPixmapXPM = xpm;
        staticMenuPixmap = new QPixmap(xpm);
    }
    if (staticClosePixmapXPM != (xpm = xpmForRegion(Close)) || !staticClosePixmap) {
        staticClosePixmapXPM = xpm;
        staticClosePixmap = new QPixmap(xpm);
    }
    if (staticMinimizePixmapXPM != (xpm = xpmForRegion(Minimize)) || !staticMinimizePixmap) {
        staticMinimizePixmapXPM = xpm;
        staticMinimizePixmap = new QPixmap(xpm);
    }
    if (staticMaximizePixmapXPM != (xpm = xpmForRegion(Maximize)) || !staticMaximizePixmap) {
        staticMaximizePixmapXPM = xpm;
        staticMaximizePixmap = new QPixmap(xpm);
    }
    if (staticNormalizePixmapXPM != (xpm = xpmForRegion(Normalize)) || !staticNormalizePixmap) {
        staticNormalizePixmapXPM = xpm;
        staticNormalizePixmap = new QPixmap(xpm);
    }

    const QPixmap *pm = 0;

    switch (decorationRegion) {
        case Help:
            pm = staticHelpPixmap;
            break;
        case Menu:
            if (!widget->windowIcon().isNull())
                return widget->windowIcon().pixmap(16,16); //##### QIcon::pixmap() needs a size !!!!!!"
            if (!pm) {
                xoff = 1;
                pm = staticMenuPixmap;
            }
            break;
        case Close:
            pm = staticClosePixmap;
            break;
        case Maximize:
            pm = staticMaximizePixmap;
            break;
        case Normalize:
            pm = staticNormalizePixmap;
            break;
        case Minimize:
            pm = staticMinimizePixmap;
            break;
        default:
            break;
    }
    return *pm;
#endif
}

int QDecorationDefault::titleBarHeight(const QWidget *)
{
    return qMax(20, QApplication::fontMetrics().lineSpacing() + BORDER_WIDTH);
}

QRegion QDecorationDefault::region(const QWidget *widget, const QRect &rect, int decorationRegion)
{
    Qt::WindowFlags flags = widget->windowFlags();
    bool hasBorder = !widget->isMaximized();
    bool hasTitle = flags & Qt::WindowTitleHint;
    bool hasSysMenu = flags & Qt::WindowSystemMenuHint;
    bool hasContextHelp = flags & Qt::WindowContextHelpButtonHint;
    bool hasMinimize = flags & Qt::WindowMinimizeButtonHint;
    bool hasMaximize = flags & Qt::WindowMaximizeButtonHint;
    int state = widget->windowState();
    bool isMinimized = state & Qt::WindowMinimized;
    bool isMaximized = state & Qt::WindowMaximized;

    int titleHeight = hasTitle ? titleBarHeight(widget) : 0;
    int bw = hasBorder ? BORDER_WIDTH : 0;
    int bbw = hasBorder ? BOTTOM_BORDER_WIDTH : 0;

    QRegion region;
    switch (decorationRegion) {
        case All: {
                QRect r(rect.left() - bw,
                        rect.top() - titleHeight - bw,
                        rect.width() + 2 * bw,
                        rect.height() + titleHeight + bw + bbw);
                region = r;
                region -= rect;
            }
            break;

        case Title: {
                QRect r(rect.left()
                        + (hasSysMenu ? menu_width : 0),
                        rect.top() - titleHeight,
                        rect.width()
                        - (hasSysMenu ? menu_width : 0)
                        - close_width
                        - (hasMaximize ? maximize_width : 0)
                        - (hasMinimize ? minimize_width : 0)
                        - (hasContextHelp ? help_width : 0),

                        titleHeight);
                if (r.width() > 0)
                    region = r;
            }
            break;

        case Top: {
                QRect r(rect.left() + CORNER_GRAB,
                        rect.top() - titleHeight - bw,
                        rect.width() - 2 * CORNER_GRAB,
                        bw);
                region = r;
            }
            break;

        case Left: {
                QRect r(rect.left() - bw,
                        rect.top() - titleHeight + CORNER_GRAB,
                        bw,
                        rect.height() + titleHeight - 2 * CORNER_GRAB);
                region = r;
            }
            break;

        case Right: {
                QRect r(rect.right() + 1,
                        rect.top() - titleHeight + CORNER_GRAB,
                        bw,
                        rect.height() + titleHeight - 2 * CORNER_GRAB);
                region = r;
            }
            break;

        case Bottom: {
                QRect r(rect.left() + CORNER_GRAB,
                        rect.bottom() + 1,
                        rect.width() - 2 * CORNER_GRAB,
                        bw);
                region = r;
            }
            break;

        case TopLeft: {
                QRect r1(rect.left() - bw,
                        rect.top() - bw - titleHeight,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.left() - bw,
                        rect.top() - bw - titleHeight,
                        bw,
                        CORNER_GRAB + bw);

                region = QRegion(r1) + r2;
            }
            break;

        case TopRight: {
                QRect r1(rect.right() - CORNER_GRAB,
                        rect.top() - bw - titleHeight,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.right() + 1,
                        rect.top() - bw - titleHeight,
                        bw,
                        CORNER_GRAB + bw);

                region = QRegion(r1) + r2;
            }
            break;

        case BottomLeft: {
                QRect r1(rect.left() - bw,
                        rect.bottom() + 1,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.left() - bw,
                        rect.bottom() - CORNER_GRAB,
                        bw,
                        CORNER_GRAB + bw);
                region = QRegion(r1) + r2;
            }
            break;

        case BottomRight: {
                QRect r1(rect.right() - CORNER_GRAB,
                        rect.bottom() + 1,
                        CORNER_GRAB + bw,
                        bw);

                QRect r2(rect.right() + 1,
                        rect.bottom() - CORNER_GRAB,
                        bw,
                        CORNER_GRAB + bw);
                region = QRegion(r1) + r2;
            }
            break;

        case Menu: {
                if (hasSysMenu) {
                    region = QRect(rect.left(), rect.top() - titleHeight,
                                   menu_width, titleHeight);
                }
            }
            break;

        case Help: {
                if (hasContextHelp) {
                    QRect r(rect.right()
                            - close_width
                            - (hasMaximize ? maximize_width : 0)
                            - (hasMinimize ? minimize_width : 0)
                            - help_width + 1, rect.top() - titleHeight,
                            help_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;


        case Minimize: {
                if (hasMinimize && !isMinimized) {
                    QRect r(rect.right() - close_width
                            - (hasMaximize ? maximize_width : 0)
                            - minimize_width + 1, rect.top() - titleHeight,
                            minimize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Maximize: {
                if (hasMaximize && !isMaximized) {
                    QRect r(rect.right() - close_width - maximize_width + 1,
                            rect.top() - titleHeight, maximize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Normalize: {
                if (hasMinimize && isMinimized) {
                    QRect r(rect.right() - close_width
                            - (hasMaximize ? maximize_width : 0)
                            - minimize_width + 1, rect.top() - titleHeight,
                            minimize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                } else if (hasMaximize && isMaximized) {
                    QRect r(rect.right() - close_width - maximize_width + 1,
                            rect.top() - titleHeight, maximize_width, titleHeight);
                    if (r.left() > rect.left() + titleHeight)
                        region = r;
                }
            }
            break;

        case Close: {
                QRect r(rect.right() - close_width + 1, rect.top() - titleHeight,
                        close_width, titleHeight);
                if (r.left() > rect.left() + titleHeight)
                    region = r;
            }
            break;

    default: {
        int i = 1;
        while (i) {
            if (i & decorationRegion)
                region += this->region(widget, rect, i);
            i <<= 1;
        }
    }
            break;
    }

    return region;
}

bool QDecorationDefault::paint(QPainter *painter, const QWidget *widget, int decorationRegion,
                               DecorationState state)
{
    if (decorationRegion == None)
        return false;

    const QRect titleRect = QDecoration::region(widget, Title).boundingRect();
    const QPalette pal = QApplication::palette();
    int titleHeight = titleRect.height();
    int titleWidth = titleRect.width();
    QRegion oldClipRegion = painter->clipRegion();


    Qt::WindowFlags flags = widget->windowFlags();
    bool hasBorder = !widget->isMaximized();
    bool hasTitle = flags & Qt::WindowTitleHint;
    bool hasSysMenu = flags & Qt::WindowSystemMenuHint;
    bool hasContextHelp = flags & Qt::WindowContextHelpButtonHint;
    bool hasMinimize = flags & Qt::WindowMinimizeButtonHint;
    bool hasMaximize = flags & Qt::WindowMaximizeButtonHint;

    bool paintAll = (decorationRegion == int(All));
    bool handled = false;

    bool porterDuff = painter->paintEngine()->hasFeature(QPaintEngine::PorterDuff);

    if ((paintAll || decorationRegion & Borders) && state == Normal && hasBorder) {
        if (hasTitle) { // reduce flicker
            QRect rect(widget->rect());
            QRect r(rect.left(), rect.top() - titleHeight,
                    rect.width(), titleHeight);
            painter->setClipRegion(oldClipRegion - r);
        }
        QRect br = QDecoration::region(widget).boundingRect();
        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_Source);
        qDrawWinPanel(painter, br.x(), br.y(), br.width(),
                    br.height(), pal, false,
                    &pal.brush(QPalette::Window));
        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        handled |= true;
    }

    if ((paintAll || decorationRegion & Title && titleWidth > 0) && state == Normal && hasTitle) {
        painter->setClipRegion(oldClipRegion);
        QBrush titleBrush;
        QPen   titlePen;

        if (widget == qApp->activeWindow()) {
            titleBrush = pal.brush(QPalette::Highlight);
            titlePen   = pal.color(QPalette::HighlightedText);
        } else {
            titleBrush = pal.brush(QPalette::Window);
            titlePen   = pal.color(QPalette::Text);
        }

        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_Source);
        qDrawShadePanel(painter,
                        titleRect.x(), titleRect.y(), titleRect.width(), titleRect.height(),
                        pal, true, 1, &titleBrush);
        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

        painter->setPen(titlePen);
        painter->drawText(titleRect.x() + 4, titleRect.y(),
                          titleRect.width() - 8, titleRect.height(),
                          Qt::AlignVCenter, widget->windowTitle());
        handled |= true;
    }

    if (state != Hover) {
        painter->setClipRegion(oldClipRegion);
        if ((paintAll || decorationRegion & Menu) && hasSysMenu) {
            paintButton(painter, widget, Menu, state, pal);
            handled |= true;
        }

        if ((paintAll || decorationRegion & Help) && hasContextHelp) {
            paintButton(painter, widget, Help, state, pal);
            handled |= true;
        }

        if ((paintAll || decorationRegion & Minimize) && hasMinimize) {
            paintButton(painter, widget, Minimize, state, pal);
            handled |= true;
        }

        if ((paintAll || decorationRegion & Maximize) && hasMaximize) {
            paintButton(painter, widget,
                        ((widget->windowState() & Qt::WindowMaximized)? Normalize : Maximize),
                        state, pal);
            handled |= true;
        }

        if (paintAll || decorationRegion & Close) {
            paintButton(painter, widget, Close, state, pal);
            handled |= true;
        }
    }
    return handled;
}

void QDecorationDefault::paintButton(QPainter *painter, const QWidget *widget,
                                     int buttonRegion, DecorationState state, const QPalette &pal)
{
    int xoff = 2;
    int yoff = 2;

    const QPixmap pm = pixmapFor(widget, buttonRegion, xoff, yoff);
    QRect brect(QDecoration::region(widget, buttonRegion).boundingRect());
    bool porterDuff = painter->paintEngine()->hasFeature(QPaintEngine::PorterDuff);

    if (state & QDecoration::Pressed) {
        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_Source);
        qDrawWinPanel(painter, brect, pal, true, &pal.brush(QPalette::Window));
        if (porterDuff)
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        ++xoff;
        ++yoff;
    } else {
        painter->fillRect(brect, pal.brush(QPalette::Window));
    }

    if (!pm.isNull())
        painter->drawPixmap(brect.x() + xoff, brect.y() + yoff, pm);
}


#endif // QT_NO_QWS_DECORATION_DEFAULT
