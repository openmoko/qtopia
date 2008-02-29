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
#include <qtopia/pim/calendar.h>
#include <qtopia/config.h>
#include <qpainter.h>
#include <qvaluestack.h>

static QColor repeatColor(0,0,0);
static QColor normalColor(0,0,0);
static bool foundRColor = FALSE;
static bool foundNColor = FALSE;

void MonthView::paintDay( const QDate &cDay, QPainter *p, 
	const QRect &cr, bool selected, const QColorGroup &cg )
{
    // one color for repeating, one for not, one for travel.
    //
    // red/pink normal.  blue/lightblue repeating.
    // Green/light green travel.
    //
    // show for each day, that day traveling into next :)
    // e.g. work out the pixel values.  the lines.


    int yea = selectedDate().year();
    int mont = selectedDate().month();

    bool sMon = weekStartsOnMonday();

    int index = PimCalendar::positionOfDate(yea, mont, sMon, cDay);
    if (index < 0)
	return; // wrong action, but.... 
    DayPaintCache *dpc = paintCache.at(index);

    if (!dpc) {
	DatePicker::paintDay(cDay, p, cr, selected, cg);
	return;
    }

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



	// in any case, we now need to draw the day, the outline, etc.
	QPen tPen = p->pen();
	p->setPen( cg.mid() );
	p->drawLine( 0, cr.height() - 1, cr.width() - 1, cr.height() - 1 );
	p->drawLine( cr.width() - 1, 0, cr.width() - 1, cr.height() - 1 );

	p->setPen( cg.text() );

	QFont f = p->font();
	f.setPointSize( ( f.pointSize() / 3 ) * 2 );
	p->setFont( f );
	QFontMetrics fm( f );
	if (QDate::currentDate() == cDay) {
	    p->setPen(QColor(255,255,255));
	    p->fillRect(1, 1, fm.width(QString::number( cDay.day() )) + 1, fm.height(), QColor(0,0,0));
	    p->drawText( 1, 1 + fm.ascent(), QString::number( cDay.day() ) );
	} else {
	    p->setPen(QColor(0,0,0));
	    p->drawText( 1, 1 + fm.ascent(), QString::number( cDay.day() ) );
	}
	p->setPen(tPen);
	p->restore();
    } else
	DatePicker::paintDay(cDay, p, cr, selected, cg);

    // now for the lines.
    int h = 5;
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

MonthView::MonthView( DateBookTable *db, QWidget *parent, const char *name, bool ac)
    : DatePicker(parent, name, ac), mDb(db), paintCache(6*7)
{
    paintCache.setAutoDelete(TRUE);
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
    repaintContents();
}

void MonthView::getEventsForMonth(int y, int m)
{
    paintCache.clear();
    paintCache.resize(6*7);

    bool sMon = weekStartsOnMonday();

    //QDate from(y, m, 1);
    //QDate to(y, m, from.daysInMonth());
    QDate from = PimCalendar::dateAtPosition(y, m, sMon, 0, 0);
    QDate to = PimCalendar::dateAtPosition(y, m, sMon, 5, 6);


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

	QDate f = (*it).startInTZ().date();
	QDate t = (*it).endInTZ().date();


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
	    startPos =  ev.startInTZ().time().hour();
	    endPos = ev.endInTZ().time().hour();
	}

	int startIndex = PimCalendar::positionOfDate(y, m, sMon, f);
	int endIndex = PimCalendar::positionOfDate(y, m, sMon, t);
	if (f < from) {
	    startIndex = 0;
	    startPos = 0;
	}
	if (t > to) {
	    endIndex = 41;
	    endPos = 23;
	}

	if (endIndex < startIndex)
	    continue;


	for (int i = (startIndex < 0 ? 0 : startIndex); 
		i <= (endIndex > 41 ? 41 : endIndex); i++) {

	    // get item.
	    DayPaintCache *dpc = paintCache.at(i);
	    if (!dpc) {
		dpc = new DayPaintCache();
		paintCache.insert(i, dpc);
	    }

	    if (normalAllDay) {
		dpc->nAllDay = TRUE;
	    } else if (repeatAllDay) {
		dpc->rAllDay = TRUE;
	    } else {
		if (ev.repeatType() == PimEvent::NoRepeat) {
		    if (i == startIndex) 
			dpc->nLine.append(startPos);
		    else 
			dpc->nLine.append(0);

		    if (i == endIndex) 
			dpc->nLine.append(endPos);
		    else 
			dpc->nLine.append(24);
		} else {
		    if (i == startIndex) 
			dpc->rLine.append(startPos);
		    else 
			dpc->rLine.append(0);

		    if (i == endIndex) 
			dpc->rLine.append(endPos);
		    else 
			dpc->rLine.append(24);
		}
	    }
	}
    }
}

/*
void MonthView::setDate( int y, int m )
{
    if (y != selectedDate().year() || m != selectedDate().month()) {
	getEventsForMonth(y, m);
	DatePicker::setDate(y, m);
	//repaintContents();
    }
}
*/

void MonthView::setDate( int y, int m, int d )
{
    if (y != selectedDate().year() || m != selectedDate().month()) {
	qDebug("getting for %d, %d", y, m);
	getEventsForMonth(y, m);
    }
    DatePicker::setDate(y, m, d);
    //repaintContents(); setting the date does a repeaint already
}
