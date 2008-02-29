/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "monthview.h"
#include <qtopia/calendar.h>
#include <qtopia/config.h>
#include <qpainter.h>
#include <qvaluestack.h>

static QColor repeatColor(0,0,0);
static QColor normalColor(0,0,0);
static bool foundRColor = FALSE;
static bool foundNColor = FALSE;

// estimate cell height.
void MonthView::resizeEvent( QResizeEvent *e )
{
    int ch = e->size().height() / 7; // 6 cells, one more for header.
    if (ch > 0)
	line_height = QMAX(5, ch / 6);
    else
	line_height = 5;
    QPEDatePicker::resizeEvent(e);
}

void MonthView::paintDayBackground( const QDate &cDay, QPainter *p,
	const QRect &cr, const QColorGroup &cg )
{
    // one color for repeating, one for not, one for travel.
    //
    // red/pink normal.  blue/lightblue repeating.
    // Green/light green travel.
    //
    // show for each day, that day traveling into next :)
    // e.g. work out the pixel values.  the lines.

    if (!paintCache.contains(cDay)) {
	QPEDatePicker::paintDayBackground(cDay, p, cr, cg);
	return;
    }

    DayPaintCache *dpc = paintCache[cDay];

    // now, do the painting.
    // later will want to pattern on all day, for now, just paint
    // and draw.

    if (dpc->nAllDay || dpc->rAllDay || dpc->tAllDay) {
	p->save();

	if (dpc->nAllDay)
	    if (dpc->rAllDay) {
		p->fillRect( 0, 0, cr.width(), cr.height() / 2,
			normalColor.light(175) );
		p->fillRect( 0, cr.height() / 2, cr.width(), cr.height() / 2,
			repeatColor.light(175) );
	    } else
		p->fillRect( 0, 0, cr.width(), cr.height(),
			normalColor.light(175) );
	else if (dpc->rAllDay)
	    p->fillRect( 0, 0, cr.width(), cr.height(),
		    repeatColor.light(175) );

	p->restore();
    } else
	QPEDatePicker::paintDayBackground(cDay, p, cr, cg);

    // now for the lines.
    int h = line_height;
    int y = cr.height() / 2 - h;


    QValueList<int>::Iterator it = dpc->nLine.begin();
    while (it != dpc->nLine.end()) {
	int h1 = *it;
	++it;
	int h2 = *it;
	++it;

	//
	// Divide up the available day into 24 hour chunks, and use
	// 4 pixels to denote an event.  Be aware of the boundary case,
	// and step back a little if we run over.
	//
	int x1, x2;

	x1 = h1 * cr.width() / 24;
	x2 = (h2 * cr.width() / 24) + 4;

	if (x2 > cr.width()) {
	    if (x1 > cr.width() - 4) {
		x1 = cr.width() - 4;
	    }
	    x2 = cr.width();
	}

	p->fillRect(x1, y, x2 - x1, h, normalColor);
    }

    y += h;

    it = dpc->rLine.begin();
    while (it != dpc->rLine.end()) {
	int h1 = *it;
	++it;
	int h2 = *it;
	++it;

	//
	// Divide up the available day into 24 hour chunks, and use
	// 4 pixels to denote an event.  Be aware of the boundary case,
	// and step back a little if we run over.
	//
	int x1, x2;

	x1 = h1 * cr.width() / 24;
	x2 = (h2 * cr.width() / 24) + 4;
	if (x2 > cr.width()) {
	    if (x1 > cr.width() - 4) {
		x1 = cr.width() - 4;
	    }
	    x2 = cr.width();
	}

	p->fillRect(x1, y, x2 - x1, h, repeatColor);
    }
}

MonthView::MonthView( DateBookTable *db, QWidget *parent, const char *name)
    : QPEDatePicker(parent, name), mDb(db), line_height(5)
{
    // we need to get events from the mdb, we need to watch for changes as
    // well.
    connect(mDb, SIGNAL(datebookUpdated()),
	    this, SLOT(updateOccurrences()));
    updateOccurrences();
}

MonthView::~MonthView() {}

void MonthView::updateOccurrences()
{
    getEventsForMonth(selectedDate().year(), selectedDate().month());
    updateContents();
}

void MonthView::getEventsForMonth(int y, int m)
{
    // Clear the old cache
    QMap<QDate, DayPaintCache*>::Iterator ptit;
    for (ptit = paintCache.begin(); ptit != paintCache.end(); ++ptit)
	delete(*ptit);
    paintCache.clear();

    // Get enough days that it won't matter if the start of week changes.
    QDate from = Calendar::dateAtCoord(y, m, 0, 0, FALSE).addDays(-6);
    QDate to = Calendar::dateAtCoord(y, m, 5, 6, TRUE).addDays(6);

    daysEvents = mDb->getOccurrences(from, to);

    QValueListIterator<Occurrence> it = daysEvents.begin();
    for ( ; it != daysEvents.end(); ++it ) {
	PimEvent ev = (*it).event();

	if ( ev.hasRepeat() ) {
	    if ( !foundRColor ) {
		repeatColor = ev.color();
		foundRColor = TRUE;
	    }
	} else {
	    if ( !foundNColor ) {
		normalColor = ev.color();
		foundNColor = TRUE;
	    }
	}

	QDate f = (*it).startInCurrentTZ().date();
	QDate t = (*it).endInCurrentTZ().date();


	bool normalAllDay = FALSE;
	bool repeatAllDay = FALSE;
	int startPos = 0;
	int endPos = 24;
	if (ev.isAllDay()) {
	    if (ev.repeatType() == PimEvent::NoRepeat) {
		normalAllDay = TRUE;
	    } else {
		repeatAllDay = TRUE;
	    }
	} else {
	    startPos =  (*it).startInCurrentTZ().time().hour();
	    endPos = (*it).endInCurrentTZ().time().hour();
	}

	if (f < from) {
	    f = from;
	    startPos = 0;
	}
	if (t > to) {
	    t = to;
	    endPos = 23;
	}
	if (t < f)
	    continue;

	for (QDate i = f; i <= t; i = i.addDays(1)) {

	    // get item.
	    DayPaintCache *dpc;
	    if (!paintCache.contains(i)) {
		dpc = new DayPaintCache();
		paintCache.insert(i, dpc);
	    } else {
		dpc = paintCache[i];
	    }

	    if (normalAllDay) {
		dpc->nAllDay = TRUE;
	    } else if (repeatAllDay) {
		dpc->rAllDay = TRUE;
	    } else {
		if (ev.repeatType() == PimEvent::NoRepeat) {
		    if (i == f)
			dpc->nLine.append(startPos);
		    else
			dpc->nLine.append(0);

		    if (i == t)
			dpc->nLine.append(endPos);
		    else
			dpc->nLine.append(24);
		} else {
		    if (i == f)
			dpc->rLine.append(startPos);
		    else
			dpc->rLine.append(0);

		    if (i == t)
			dpc->rLine.append(endPos);
		    else
			dpc->rLine.append(24);
		}
	    }
	}
    }
}

void MonthView::setDate( const QDate &date )
{
    if (date.year() != selectedDate().year() || date.month() != selectedDate().month()) {
	getEventsForMonth(date.year(), date.month());
    }
    QPEDatePicker::setDate(date);
    updateContents();
}
