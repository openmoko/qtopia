/****************************************************************************
** $Id: qt/src/kernel/qwsbeosdecoration_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/Embedded beos decoration
**
** Created : 000101
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include "qwsbeosdecoration_qws.h"

#ifndef QT_NO_QWS_BEOS_WM_STYLE

QWSBeOSDecoration::QWSBeOSDecoration()
    : QWSDefaultDecoration()
{
}

QWSBeOSDecoration::~QWSBeOSDecoration()
{
}

const char **QWSBeOSDecoration::menuPixmap()
{
    return (const char **)0;
}

int QWSBeOSDecoration::getTitleWidth(const QWidget *w)
{
    int titleWidth = w->width();
    return (titleWidth > 100) ? titleWidth : 100;
}

int QWSBeOSDecoration::getTitleHeight(const QWidget *)
{
    return 15;
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QWSBeOSDecoration::region(const QWidget *widget, const QRect &rect, QWSDecoration::Region type)
{
//    int titleWidth = getTitleWidth(widget);
    int titleHeight = getTitleHeight(widget);
    int bw = rect.isEmpty() ? 0 : BORDER_WIDTH;
    QRegion region;

    switch (type) {
	case All: {
	    int w = rect.width() + 2 * bw;
	    QRect tr(rect.left() - bw,
		    rect.top() - titleHeight - bw,
		    (w < 110) ? w : 110,
		    titleHeight);
	    QRect r(rect.left() - bw,
		    rect.top() - bw,
		    rect.width() + 2 * bw,
		    rect.height() + 2 * bw);
	    region = r;
	    region += tr;
	    region -= rect;
	    break;
	}
/*
	case Title: {
	    int w = rect.width();
	    QRect r(rect.left() + titleHeight, rect.top() - titleHeight,
		    (w < 110) ? w : 110, titleHeight);
	    if (r.width() > 0)
		region = r;
	    break;
	}
	case Top: {
	    int w = rect.width();
	    QRect r(rect.left() + CORNER_GRAB,
		    rect.top() - titleHeight - bw,
		    (w < 110) ? w : 110,
		    bw);
	    if (r.width() > 0)
		region = r;
	    break;
	}
*/
	case Menu:
	    break;
	case Close: {
		QRect r;
		r = QRect(rect.left(), rect.top() - titleHeight - 4,
		    titleHeight, titleHeight);
		region = r;
		break;
	}
	case Maximize: {
		QRect r;
		int w = rect.width();
		r = QRect(((w < 100) ? w : 100) - 16, rect.top() - titleHeight - 4,
		    titleHeight, titleHeight);
		if (r.left() > rect.left() + titleHeight)
		    region = r;
	    break;
	}
	case Minimize: {
		QRect r;
		int w = rect.width();
		r = QRect(((w < 100) ? w : 100) - 32, rect.top() - titleHeight - 4,
		    titleHeight, titleHeight);
		if (r.left() > rect.left() + titleHeight)
		    region = r;
	    break;
	}
	case Title:
	case Top:
	case Left:
	case Right:
	case Bottom:
	case TopLeft:
	case TopRight:
	case BottomLeft:
	case BottomRight:
	default:
	    region = QWSDefaultDecoration::region(widget, rect, type);
	    break;
    }

    return region;
}

void QWSBeOSDecoration::paint(QPainter *painter, const QWidget *widget)
{
#ifndef QT_NO_STYLE
    QStyle &style = QApplication::style();
#endif

    int titleWidth = getTitleWidth(widget);
    int titleHeight = getTitleHeight(widget);

    QRect rect(widget->rect());

    // Border rect
    QRect br( rect.left() - BORDER_WIDTH,
                rect.top() - BORDER_WIDTH - titleHeight,
                rect.width() + 2 * BORDER_WIDTH,
                rect.height() + BORDER_WIDTH + BOTTOM_BORDER_WIDTH + titleHeight );

    // title bar rect
    QRect tr;

    br = QRect( );
    tr = QRect( titleHeight, -titleHeight,  titleWidth, titleHeight - 1);

    QRegion oldClip = painter->clipRegion();
    painter->setClipRegion( oldClip - QRegion( tr ) );	// reduce flicker

#ifndef QT_NO_PALETTE
    // const QColorGroup &cg = QApplication::palette().active();
    const QColorGroup &cg = widget->palette().active();

#if !defined(QT_NO_STYLE)
    style.drawPanel(painter, br.x(), br.y(), br.width(),
		    br.height() - 4, cg, FALSE, 2,
		    &cg.brush(QColorGroup::Background));
#elif !defined(QT_NO_DRAWUTIL)
    qDrawWinPanel(painter, br.x(), br.y(), br.width(),
		  br.height() - 4, cg, FALSE,
		  &cg.brush(QColorGroup::Background));
#endif

    painter->setClipRegion( oldClip );

    if (titleWidth > 0) {
	QBrush titleBrush;
	QPen   titlePen;
	int    titleLeft = titleHeight + 4;

	if (widget == qApp->activeWindow()) {
	    titleBrush = cg.brush(QColorGroup::Highlight);
	    titlePen   = cg.color(QColorGroup::HighlightedText);
	} else {
	    titleBrush = cg.brush(QColorGroup::Background);
	    titlePen   = cg.color(QColorGroup::Text);
	}

#define CLAMP(x, y)	    ( ((x) > (y)) ? (y) : (x) )

	    QColor c1;
	    if (widget == qApp->activeWindow()) {
		c1 = QColor( 0xC0, 0xB0, 0x40 );
	    } else {
		c1 = QColor( 0x20, 0x30, 0x50 );
		titlePen = QPen( QColor( 0x90, 0x90, 0x90 ) );
	    }
	    painter->fillRect( rect.left() - 4, rect.top() - titleHeight - 4,
			    rect.width() + 8, rect.height() + titleHeight + 8, QBrush( c1 ) );

	painter->setPen(titlePen);
	painter->setFont(widget->font());
	painter->drawText( titleLeft, -titleHeight,
	 		rect.width() - titleHeight - 10, titleHeight-1,
			QPainter::AlignVCenter, widget->caption());
    }

#endif //QT_NO_PALETTE

}

void QWSBeOSDecoration::paintButton(QPainter *painter, const QWidget *w,
			QWSDecoration::Region type, int state)
{
#ifndef QT_NO_PALETTE
#ifndef QT_NO_STYLE
//    QStyle &style = QApplication::style();
#endif
//    const QColorGroup &cg = w->palette().active();

    QRect brect(region(w, w->rect(), type).boundingRect());

    int xoff=2;
    int yoff=2;

    const QPixmap *pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    xoff++;

    if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
	if (pm) painter->drawPixmap(brect.x()+1+xoff, brect.y()+1+yoff, *pm);
    } else {
	if (pm) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, *pm);
    }
#endif
}

#endif // QT_NO_QWS_BEOS_WM_STYLE
