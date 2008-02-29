/****************************************************************************
** $Id: qt/src/kernel/qwskdedecoration_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/Embedded KDE decorations
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
#include "qwskdedecoration_qws.h"

#ifndef QT_NO_QWS_KDE_WM_STYLE

QWSKDEDecoration::QWSKDEDecoration()
    : QWSDefaultDecoration()
{
}

QWSKDEDecoration::~QWSKDEDecoration()
{
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QWSKDEDecoration::region(const QWidget *widget, const QRect &rect, QWSDecoration::Region type)
{
    int titleHeight = getTitleHeight(widget);
//  int titleWidth = getTitleWidth(widget);
//  int bw = rect.isEmpty() ? 0 : BORDER_WIDTH;
    QRegion region;

    switch (type) {
	case Maximize: {
	    QRect r(rect.right() - 2*titleHeight, rect.top() - titleHeight,
			    titleHeight, titleHeight);
	    if (r.left() > rect.left() + titleHeight)
		    region = r;
	    break;
	}
	case Minimize: {
	    QRect r(rect.right() - 3*titleHeight, rect.top() - titleHeight,
		    	    titleHeight, titleHeight);
	    if (r.left() > rect.left() + titleHeight)
		    region = r;
	    break;
	}
	case Menu:
	case Close:
	case All:
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

void QWSKDEDecoration::paint(QPainter *painter, const QWidget *widget)
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

	{

#if !defined(QT_NO_STYLE)
	    style.drawPanel(painter, tr.x(), tr.y(), tr.width(), tr.height(),
			    cg, TRUE, 1, &titleBrush);
#elif !defined(QT_NO_DRAWUTIL)
	    qDrawWinPanel(painter, tr.x(), tr.y(), tr.width(), tr.height(),
			    cg, TRUE, &titleBrush);
#endif

	    painter->setPen(titlePen);
	    painter->setFont(widget->font());
	    painter->drawText( titleLeft, -titleHeight,
			    titleWidth-5, titleHeight - 1,
			    QPainter::AlignVCenter, widget->caption());
	    return;
	}

	painter->setPen(titlePen);
	painter->setFont(widget->font());
	painter->drawText( titleLeft, -titleHeight,
	 		rect.width() - titleHeight - 10, titleHeight-1,
			QPainter::AlignVCenter, widget->caption());
    }

#endif //QT_NO_PALETTE

}

void QWSKDEDecoration::paintButton(QPainter *painter, const QWidget *w,
			QWSDecoration::Region type, int state)
{
#ifndef QT_NO_PALETTE
#ifndef QT_NO_STYLE
    QStyle &style = QApplication::style();
#endif
    const QColorGroup &cg = w->palette().active();

    QRect brect(region(w, w->rect(), type).boundingRect());

    int xoff=2;
    int yoff=2;

    const QPixmap *pm=pixmapFor(w,type,state & QWSButton::On, xoff, yoff);

    {

	if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked)) {
#if !defined(QT_NO_STYLE)
	    style.drawToolButton(painter, brect.x(), brect.y(), brect.width()-1,
		        brect.height()-1, cg, TRUE,
			&cg.brush(QColorGroup::Background));
#elif !defined(QT_NO_DRAWUTIL)
	    qDrawWinPanel(painter, brect.x(), brect.y(), brect.width()-1,
			brect.height()-1, cg, TRUE,
			&cg.brush(QColorGroup::Background));
#endif	
	    if (pm) painter->drawPixmap(brect.x()+xoff+1, brect.y()+yoff+1, *pm);
	} else {
	    painter->fillRect(brect.x(), brect.y(), brect.width()-1,
			brect.height()-1, cg.brush(QColorGroup::Background));
	    if (pm) painter->drawPixmap(brect.x()+xoff, brect.y()+yoff, *pm);
	}
    }

#endif

}

#endif // QT_NO_QWS_KDE_WM_STYLE
