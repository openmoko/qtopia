/****************************************************************************
** $Id: qt/examples/winmanager/minimal.cpp   2.3.12   edited 2005-10-27 $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "qapplication.h"
#include "qstyle.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qregion.h"
#include "minimal.h"

#define CORNER_GRAB	16
#undef  BORDER_WIDTH
#define BORDER_WIDTH	2
#define TITLE_HEIGHT	20

static const char * const close_xpm[] = {
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
 
static const char * const maximize_xpm[] = {
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

static const char * const minimize_xpm[] = {
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
 
static const char * const normalize_xpm[] = {
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

QWSMinimalDecoration::QWSMinimalDecoration()
    : QWSDecoration()
{
    closePixmap = new QPixmap((const char **)close_xpm);
    minimizePixmap = new QPixmap((const char **)minimize_xpm);
    maximizePixmap = new QPixmap((const char **)maximize_xpm);
    normalizePixmap = new QPixmap((const char **)normalize_xpm);
}


QWSMinimalDecoration::~QWSMinimalDecoration()
{
    delete closePixmap;
    delete minimizePixmap;
    delete maximizePixmap;
    delete normalizePixmap;
}


QRegion QWSMinimalDecoration::region(const QWidget *w, const QRect &rect, Region type)
{
    QRegion rgn;

    switch (type) {
	case All: {
		rgn = QRect( rect.left() - BORDER_WIDTH,
			    rect.top() - BORDER_WIDTH,
			    rect.width() + 2 * BORDER_WIDTH,
			    rect.height() + 2 * BORDER_WIDTH );
		rgn += region(w, rect, Title);
		rgn += region(w, rect, Close);
		rgn += region(w, rect, BottomRight);
		rgn -= rect;
	    }
	    break;

	case Title: {
		QFontMetrics fm( QApplication::font() );
		int width = fm.width( w->caption() ) + 10;
		if (width > rect.height() - TITLE_HEIGHT)
		    width = rect.height() - TITLE_HEIGHT;
		QRect r(rect.left() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.top() + TITLE_HEIGHT - BORDER_WIDTH,
			TITLE_HEIGHT, width + 2*BORDER_WIDTH);
		rgn = r;
	    }
	    break;

	case BottomRight: {
		QRect r1(rect.right() - CORNER_GRAB,
			rect.bottom() + 1,
			CORNER_GRAB + 2 * BORDER_WIDTH + 2,
			2 * BORDER_WIDTH + 1);

		QRect r2(rect.right() + 1,
			rect.bottom() - CORNER_GRAB,
			2 * BORDER_WIDTH + 1,
			CORNER_GRAB + 2 * BORDER_WIDTH + 2);
		rgn = QRegion(r1) + r2;
	    }
	    break;

	case Close: {
		QRect r(rect.left() - TITLE_HEIGHT - BORDER_WIDTH,
			rect.top() - BORDER_WIDTH, TITLE_HEIGHT, TITLE_HEIGHT);
		rgn = r;
	    }
	    break;

	case Menu:
	case Maximize:
	case Minimize:
	    break;

	default:
	    break;
    }

    return rgn;
}

void QWSMinimalDecoration::paint(QPainter *painter, const QWidget *widget)
{
    QStyle &style = QApplication::style();
    const QColorGroup &cg = QApplication::palette().active();

    QRect rect(widget->rect());

    style.drawPanel(painter, rect.right() - CORNER_GRAB + 1,
		    rect.bottom() - CORNER_GRAB + 1,
		    CORNER_GRAB + 2 * BORDER_WIDTH + 1,
		    CORNER_GRAB + 2 * BORDER_WIDTH + 1,
		    cg, FALSE, 2, &cg.brush(QColorGroup::Background));

    style.drawPanel(painter, rect.left() - BORDER_WIDTH,
		    rect.top() - BORDER_WIDTH,
		    rect.width() + 2 * BORDER_WIDTH,
		    rect.height() + 2 * BORDER_WIDTH,
		    cg, FALSE, 2, &cg.brush(QColorGroup::Background));

    QBrush titleBrush;
    QPen   titlePen;

    if (widget == qApp->activeWindow()) {
	titleBrush = cg.brush(QColorGroup::Highlight);
	titlePen   = cg.color(QColorGroup::HighlightedText);
    } else {
	titleBrush = cg.brush(QColorGroup::Background);
	titlePen   = cg.color(QColorGroup::Text);
    }

    painter->setFont(QApplication::font());
    int width = painter->fontMetrics().width( widget->caption() ) + 10;
    if (width > rect.height() - TITLE_HEIGHT)
	width = rect.height() - TITLE_HEIGHT;

    style.drawPanel(painter, -TITLE_HEIGHT - BORDER_WIDTH,
		    TITLE_HEIGHT - BORDER_WIDTH,
		    TITLE_HEIGHT, width + 2*BORDER_WIDTH,
		    cg, FALSE, 2, &titleBrush);

    painter->setPen(titlePen);
    painter->rotate(-90);
    painter->drawText(-width - TITLE_HEIGHT + BORDER_WIDTH + 5, -TITLE_HEIGHT,
		    width - 10, TITLE_HEIGHT-1,
		    QPainter::AlignVCenter, widget->caption());
    painter->rotate(90);

}

void QWSMinimalDecoration::paintButton(QPainter *painter, const QWidget *w,
			Region type, int state)
{
    QStyle &style = QApplication::style();
    const QColorGroup &cg = QApplication::palette().active();

    QRect brect(region(w, w->rect(), type).boundingRect());
    const QPixmap *pm = 0;

    switch (type) {
	case Menu:
	    break;
	case Close:
	    pm = closePixmap;
	    break;
	case Maximize:
	    if (state & QWSButton::On)
		pm = normalizePixmap;
	    else
		pm = maximizePixmap;
	    return;
	case Minimize:
	    pm = minimizePixmap;
	    return;
	default:
	    return;
    }

    if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
	style.drawToolButton(painter, brect.x(), brect.y(), brect.width(),
		    brect.height(), cg, TRUE,
		    &cg.brush(QColorGroup::Background));
	if (pm) painter->drawPixmap(brect.x()+3, brect.y()+3, *pm);
    } else {
	style.drawToolButton(painter, brect.x(), brect.y(), brect.width(),
		    brect.height(), cg, FALSE,
		    &cg.brush(QColorGroup::Background));
	if (pm) painter->drawPixmap(brect.x()+2, brect.y()+2, *pm);
    }
}

