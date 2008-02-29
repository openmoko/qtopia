/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "dayview.h"
#include "dayviewheaderimpl.h"
#include "layoutmanager.h"
#include "datebookdb.h"

#ifdef Q_WS_QWS
#include <qtopia/ir.h>
#endif

#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/timestring.h>
#include <qtopia/qpedebug.h>

#include <qheader.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <qsimplerichtext.h>
#include <qpopupmenu.h>
#include <qtextcodec.h>
#include <qpalette.h>
#include <qabstractlayout.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qstyle.h>
#include <qtimer.h>
#include <qlist.h>
#include <qtl.h>

static const int short_time_height = 20;
static int day_height = 18;
static const int day_time_margin = 2;

inline QRect expandRect(QRect r)
{
    QRect nr(r.x() - 1, r.y() - 1, r.width() + 1, r.height() + 1);
    return nr;
}

// ====================================================================

class DayItem : public LayoutItem
{
    friend class DayViewContents;
public:
    DayItem(const QDate &referenceDate, const Occurrence &ev);
    ~DayItem();

    void setFocus(bool b);

    void drawItem( QPainter *p, int x, int y, const QColorGroup &cg );

    void setCompressed();

private:
    void loadPMs();

    QPixmap pm_repeat, pm_alarmbell, pm_repeatE, pm_globe;
    QString strDesc;
    QString headingText;

    QString noteTZ;
    QDateTime noteStart;
    QDateTime noteEnd;
    bool noteShowStartDate;
    bool noteShowEndDate;
    QString noteStr;

    bool hasFocus;
    bool compressed;
};

// ====================================================================

class DayViewLayout : public LayoutManager
{
public:
    DayViewLayout(int w, int h);

    void clear();

    DayItem *focusedItem() const;
    void setFocusedItem(DayItem *i);

    virtual DayItem *firstItem();
    virtual DayItem *nextItem();
    virtual DayItem *previousItem();
    virtual DayItem *lastItem();

    void addOccurrence(Occurrence &e, const QDate &cDate );

    virtual void setTimeHeight( int th );

protected:
    DayItem *fItem;
    int time_height;
};

// ====================================================================

class AllDayLayout : public DayViewLayout
{
public:
    AllDayLayout(int w, int h);

    void layoutItems(bool = FALSE);
};

// ====================================================================

class CompressedDayLayout : public DayViewLayout
{
public:
    CompressedDayLayout( int w, int h, const int &sod );

    void clear();
    void eventsChanged();
    int timeToHeight( const QTime &time ) const;
    void setSize( int w, int h );
    void setTimeHeight( int th );

    TimeList times; 

private:
    const int &start_of_day;
};

// ====================================================================

DayItem::DayItem(const QDate &referenceDate, const Occurrence &ev)
    : LayoutItem(ev) , hasFocus(FALSE), compressed( FALSE )
{
    strDesc = ev.event().description();
    QString location = ev.event().location();
    if ( !location.isEmpty() )
        strDesc += " <" + location + ">";

    // why would someone use "<"?  Oh well, fix it up...
    // I wonder what other things may be messed up...
    int where = strDesc.find( "<" );
    while ( where != -1 ) {
	strDesc.remove( where, 1 );
	strDesc.insert( where, "&#60;" );
	where = strDesc.find( "<", where );
    }

    QString strCat;
    // ### Fix later...
    //     QString strCat = ev.category();
    //     where = strCat.find( "<" );
    //     while ( where != -1 ) {
    // 	strCat.remove( where, 1 );
    // 	strCat.insert( where, "&#60;" );
    // 	where = strCat.find( "<", where );
    //     }

    QString strNote = ev.event().notes();
    where = strNote.find( "<" );
    while ( where != -1 ) {
	strNote.remove( where, 1 );
	strNote.insert( where, "&#60;" );
	where = strNote.find( "<", where );
    }

    headingText = strDesc;

    if (ev.event().timeZone().isValid()) {
	noteTZ=ev.event().timeZone().id();
	// ########### translate with new TZ class, not this mess
	int i = noteTZ.find('/');
	noteTZ = noteTZ.mid( i + 1 );
	noteTZ = noteTZ.replace(QRegExp("_"), " ");
    }
    noteStart = ev.event().start();
    noteEnd = ev.event().end();
    noteShowStartDate = referenceDate != ev.date();
    noteShowEndDate = referenceDate != ev.endDate();
    noteStr = strNote;

    //setBackgroundMode( PaletteBase );
}

DayItem::~DayItem()
{
}

void DayItem::setFocus(bool b)
{
    hasFocus = b;
}

void DayItem::drawItem( QPainter *p, int x, int y, const QColorGroup &cg )
{
    p->save();
    QRect r = expandRect(geometry());
    x += r.x();
    y += r.y();
    int w = r.width();
    int h = r.height();
    p->setBrush(event().color().light(175));

    if (hasFocus) {
	p->setPen( QPen(QColor(0,0,0)) );
	p->drawRect( x, y, w, h );
	p->drawRect( x + 1, y + 1, w - 2, h - 2 );
    } else {
	p->setPen( QColor(100, 100, 100) );
	p->drawRect( x, y, w, h );
    }
    // now for the inner rect;
    x += 4;
    w -= 8;
    y += 2;
    h -= 4;

    int d = 0;
    int pw;

    loadPMs();

    if ( event().timeZone().isValid() ) {
	pw = pm_globe.width();
	p->drawPixmap( x + w - pw - d, y, pm_globe );
	d += pw+1;
    }

    if ( event().hasAlarm() ) {
	pw = pm_alarmbell.width();
	p->drawPixmap( x + w - pw - d, y, pm_alarmbell );
	d += pw+1;
    }

    if ( event().hasRepeat() ) {
	pw = pm_repeat.width();
	p->drawPixmap( x + w - pw - d, y, pm_repeat );
	d += pw+1;
    } else if ( event().isException() ) {
	pw = pm_repeatE.width();
	p->drawPixmap( x + w - pw - d, y, pm_repeatE );
	d += pw+1;
    }

    if ( compressed ) {
	QSimpleRichText rt( headingText, p->font() );
	// modified clip region for rt.
	// better to clip than to draw over our icons.
	QRect rect(x, y, w - d, h);
	rt.setWidth( w - d );

	p->setClipRect(p->xForm(QRect(x,y,w,h)));
	p->setClipRect(p->xForm(rect));
	rt.draw( p, x, y,  QRegion(rect), cg );
	p->restore();
    } else {
	QSimpleRichText rt( headingText, p->font() );

	QString noteText;
	if (!noteTZ.isEmpty())
	    noteText = "<b>"+DayViewContents::tr("Time zone:")+"</b> " + noteTZ + "<br>";
	noteText += "<b>" + DayViewContents::tr("Start") + "</b>: ";

	if ( noteShowStartDate ) {
	    // multi-day event.  Show start date
	    noteText += TimeString::localYMD( noteStart.date(), TimeString::Long );
	} else {
	    // Show start time.
	    noteText += TimeString::localHM( noteStart.time() );
	}

	noteText += "<br><b>" + DayViewContents::tr("End") + "</b>: ";
	if ( noteShowEndDate ) {
	    // multi-day event.  Show end date
	    noteText += TimeString::localYMD( noteEnd.date(), TimeString::Long );
	} else {
	    // Show end time.
	    noteText += TimeString::localHM( noteEnd.time() );
	}
	noteText += "<br><br>" + noteStr;

	QSimpleRichText rtNotes( noteText, p->font() );

	// modified clip region for rt.
	// better to clip than to draw over our icons.
	QRect rect(x, y, w - d, h);
	rt.setWidth( w - d );
	rtNotes.setWidth( w );

	p->setClipRect(p->xForm(QRect(x,y,w,h)));
	rtNotes.draw( p, x, y + rt.height(),
		QRegion(x, y, w, h), cg );
	p->setClipRect(p->xForm(rect));
	rt.draw( p, x, y,  QRegion(rect), cg );
	p->restore();
    }
}

void DayItem::setCompressed()
{
    compressed = TRUE;
}

void DayItem::loadPMs()
{
    if ( pm_repeat.isNull() ) {
	pm_repeat = Resource::loadIconSet("repeat").pixmap(QIconSet::Small, TRUE);
	pm_repeatE = Resource::loadIconSet("repeatException").pixmap(QIconSet::Small, TRUE);
	pm_alarmbell = Resource::loadIconSet("smallalarm").pixmap(QIconSet::Small, TRUE);
	pm_globe = Resource::loadIconSet("globe").pixmap(QIconSet::Small, TRUE);
    }
}

// ====================================================================

DayViewLayout::DayViewLayout(int w, int h)
    : LayoutManager(w,h), fItem(0)
{
}

void DayViewLayout::clear()
{
    fItem = 0;
    LayoutManager::clear();
}

DayItem *DayViewLayout::focusedItem() const
{
    return fItem;
}

void DayViewLayout::setFocusedItem(DayItem *i)
{
    if (fItem)
	fItem->setFocus(FALSE);
    fItem = 0;
    if (i && items().containsRef(i)) {
	i->setFocus(TRUE);
	fItem = i;
    }
}

DayItem *DayViewLayout::firstItem()
{
    if (fItem)
	fItem->setFocus(FALSE);
    if (items().size() > 0) {

	// really should be 'top' item, most to left.
	fItem = (DayItem *)items().at(0);
	fItem->setFocus(TRUE);
    } else
	fItem = 0;
    return fItem;
}

DayItem *DayViewLayout::nextItem()
{
    if (fItem)
	fItem->setFocus(FALSE);
    else
	return 0;

    int pos = items().findRef(fItem);
    pos++;
    if (pos >= (int)items().count()) {
	fItem = 0;
    } else {
	fItem = (DayItem *)items().at(pos);
	fItem->setFocus(TRUE);
    }
    return fItem;
}

DayItem *DayViewLayout::previousItem()
{
    if (fItem)
	fItem->setFocus(FALSE);
    else
	return 0;

    int pos = items().findRef(fItem);
    pos--;
    if (pos < 0 || items().count() < 1) {
	fItem = 0;
    } else {
	fItem = (DayItem *)items().at(pos);
	fItem->setFocus(TRUE);
    }
    return fItem;
}

DayItem *DayViewLayout::lastItem()
{
    if (fItem)
	fItem->setFocus(FALSE);
    if (items().size() > 0) {

	// really should be 'top' item, most to left.
	fItem = (DayItem *)items().at(items().size()-1);
	fItem->setFocus(TRUE);
    } else
	fItem = 0;
    return fItem;
}

void DayViewLayout::addOccurrence(Occurrence &e, const QDate &cDate )
{
    DayItem *di = new DayItem(cDate, e);
    addItem(di);
    eventsChanged();
    initializeGeometry(di);
}

void DayViewLayout::setTimeHeight( int th )
{
    time_height = th;
}

// ====================================================================

AllDayLayout::AllDayLayout(int w, int h)
    : DayViewLayout(w,h)
{
}

void AllDayLayout::layoutItems(bool)
{
    for ( int i = 0; i < count(); i++) {
	LayoutItem *w = items().at(i);
	// one under the other, full width.
	QRect geom = w->geometry();
	geom.setX( i % 2 ? size().width() / 2 : 0 );
	geom.setY( day_height * (i / 2) );

	int width = size().width() - (size().width() % 2); // round down to even.
	int day_width = i % 2 == 0 && i == count() - 1 ?
		size().width() :
		width / 2;
	if ( i % 2 )
	    day_width += size().width() % 2; // round up the second item.

	geom.setWidth( day_width );
	geom.setHeight( day_height );
	w->setGeometry(geom);
    }
}

// ====================================================================

TimeList::TimeList() :
    list(), height24( 0 ), infoModified( FALSE ), time_height(short_time_height)
{
    clear();
}

void TimeList::clear()
{
    for ( int i = 0; i <= 24; i++ ) {
	hourInfo[i].empty = TRUE;
	hourInfo[i].stretch = FALSE;
	hourInfo[i].hits = 0;
    }
    list.clear();
}

void TimeList::add( const QDate &referenceDate, LayoutItem *item )
{
    const QTime &t1 = item->occurrence().startInCurrentTZ().time();
    const QTime &t2 = item->occurrence().endInCurrentTZ().time();
    bool need1 = referenceDate <= item->occurrence().startInCurrentTZ().date();
    bool need2 = referenceDate >= item->occurrence().startInCurrentTZ().date().addDays( 
	item->occurrence().startInCurrentTZ().date().daysTo(
	    item->occurrence().endInCurrentTZ().date() )
	);

    if ( need1 )
	add( t1 );
    else
	add( QTime(0,0,0) );

    if ( need2 )
	add( t2 );

    int start = need1 ? t1.hour() - 1 : 0;
    int end = need2 ? t2.hour() - 1 : 23;
    for ( int i = (start >= 0) ? start : 0; i < end; i++ )
	hourInfo[i].empty = FALSE;
}

void TimeList::add( const QTime &val )
{
    if ( list.contains( val ) == 0 )
	list.append( val );
}

void TimeList::addDone()
{
    qHeapSort( list );

    for ( TimeVals::ConstIterator it = list.begin(); it != list.end(); it++ ) {
	const QTime &t = *it;
	hit( t.hour() );
    }

    height24 = height(25);
    infoModified = FALSE;
}

void TimeList::hit( int index )
{
    for ( int i = index; i <= 24; i++ )
	hourInfo[i].hits++;
}

int TimeList::height( const QTime &time ) const
{
    int h1 = (time.hour() > 0) ? hourInfo[time.hour() - 1].hits : 0;
    if ( time.hour() < 24 ) {
	int h2 = hourInfo[time.hour()].hits;
	if ( h2 - h1 > 1 ) {
	    // There are multiple appointments that start at this hour
	    // so we count the ones before us and add an "hour" for each
	    // of them. This is because of the longer hours.
	    TimeVals::ConstIterator it = list.find( time );
	    if ( it != list.end() ) {
		while ( it != list.begin() ) {
		    --it;
		    const QTime &t = *it;
		    if ( t.hour() != time.hour() )
			break;
		    h1++;
		}
	    }
	}
    }
    return h1;
}

int TimeList::height( const int hour ) const
{
    int h1 = (hour > 0) ? hourInfo[hour - 1].hits : 0;
    return h1;
}

int TimeList::scale( const int h, const int widgetHeight ) const
{
    int rv;
    if ( height24 == 0 || height24 * time_height >= widgetHeight )
	rv = h * time_height;
    else
	rv = h * widgetHeight / height24;
    return rv;
}

void TimeList::resize( int h, int start_of_day )
{
    if ( infoModified ) {
	TimeVals tmp = list;
	clear();
	list = tmp;
	addDone();
    }

    if ( height24 != 0 && height24 * time_height >= h )
	return;

    infoModified = TRUE;
    int spotsNeeded = h / time_height - height24;

    int i = 0;

    // Add extra hours until the day is full
    int hour = start_of_day;
    for ( ; i < spotsNeeded; i++ ) {
	bool found = FALSE;
	while ( !found ) {
	    if ( hour == 24 )
		goto AFTER_ADD;
	    if ( hour > 0 && hourInfo[hour].hits == hourInfo[hour - 1].hits ) {
		hit( hour );
		hourInfo[hour - 1].empty = FALSE;
		found = TRUE;
	    }
	    hour++;
	}
    }
AFTER_ADD:

    // Just in case there weren't enough hours to add...
    for ( ; i < spotsNeeded; i++ ) {
	hit( 23 );
    }

    height24 = height(25);
}

QTime TimeList::time( int hour, int number ) const
{
    int counter = 0;
    for ( TimeVals::ConstIterator it = list.begin(); it != list.end(); it++ ) {
	const QTime &t = *it;
	if ( t.hour() == hour )
	    counter++;
	if ( counter == number )
	    return t;
    }
    return QTime( hour, 0 );
}

void TimeList::setTimeHeight( int th )
{
    time_height = th;
}

// ====================================================================

CompressedDayLayout::CompressedDayLayout( int w, int h, const int &sod )
    : DayViewLayout(w,h),
    start_of_day( sod )
{
}

void CompressedDayLayout::clear()
{
    DayViewLayout::clear();
    eventsChanged();
}

void CompressedDayLayout::eventsChanged()
{
    times.clear();
    for ( int i = 0; i < (int)mItems.count(); i++ ) {
	LayoutItem *item = mItems.at( i );
	times.add( referenceDate, item );
    }
    times.addDone();
}

int CompressedDayLayout::timeToHeight( const QTime &time ) const
{
    int r1 = times.height( time );
    int r2 = times.scale( r1, height );
    return r2;
}

void CompressedDayLayout::setSize( int w, int h )
{
    times.resize( h, start_of_day );
    LayoutManager::setSize( w, h );
}

void CompressedDayLayout::setTimeHeight( int th )
{
    DayViewLayout::setTimeHeight( th );
    times.setTimeHeight( time_height );
}

// ====================================================================

DayViewContents::DayViewContents( Type t, QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase ),
      typ(t), start_of_day(8),
      time_height(short_time_height), dataChanged( TRUE ),
      clicking( FALSE )
{
    setResizePolicy(Manual);
#ifdef Q_WS_QWS
    QPEApplication::setStylusOperation( viewport(), QPEApplication::RightOnHold );
#endif
    startSel = endSel = -1;
    dragging = FALSE;
#ifdef QTOPIA_DESKTOP
    day_height += 2;	// difference in font metrics?
#endif

    setHScrollBarMode(AlwaysOff);

    newItemList();
    itemList->setTimeHeight( time_height );

    viewport()->setBackgroundMode(PaletteBase);
    visibleTimer = new QTimer( this );
    connect( visibleTimer, SIGNAL(timeout()), this, SLOT(makeVisible()) );

    TimeString::connectChange(this,SLOT(timeStringChanged()));
}

DayViewContents::~DayViewContents()
{
    clearItems();
    delete itemList;
}

void DayViewContents::newItemList()
{
    if (typ == AllDay) {
	setMargins(0, 0, 0, 0);
	setBackgroundMode(PaletteDark);
	itemList = new AllDayLayout(contentsWidth(), contentsHeight());
    } else if ( typ == CompressedDay ) {
	itemList = new CompressedDayLayout( contentsWidth() - time_width, contentsHeight(), start_of_day );
    } else {
	itemList = new DayViewLayout(contentsWidth() - time_width, contentsHeight());
    }
}

void DayViewContents::addOccurrence(Occurrence &ev, const QDate &cDate)
{
    itemList->addOccurrence(ev, cDate);

    if (typ == AllDay) {
	resizeContents(
		itemList->count() > 4
		? width() - style().scrollBarExtent().width() : width(),
		day_height * ((itemList->count() + 1) / 2 ));
	itemList->setSize( contentsWidth(), contentsHeight() );
    }
}

void DayViewContents::clearItems()
{
    itemList->clear();
    itemList->setFocusedItem(0);
    dataChanged = TRUE;
}

void DayViewContents::layoutItems()
{
    itemList->layoutItems(TRUE);
    updateContents(0, 0, contentsWidth(), contentsHeight());
}

void DayViewContents::timeStringChanged()
{
    updateContents(0, 0, contentsWidth(), contentsHeight());
}

void DayViewContents::clearSelectedTimes()
{
    int oldSelMin = QMIN(startSel, endSel);
    int oldSelMax = QMAX(startSel, endSel);
    startSel = endSel = -1;

    repaintContents(0, posOfHour(oldSelMin - 1),
	    time_width, posOfHour(oldSelMax + 1), FALSE);
}

void DayViewContents::startAtTime(int t)
{
    start_of_day = t;
}

void DayViewContents::selectDate(const QDate &d)
{
    itemList->setDate(d);
}

DayItem *DayViewContents::firstItem( bool show )
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->firstItem();
    if (orig != dim && show)
	moveSelection( orig, dim );
    return dim;
}

DayItem *DayViewContents::lastItem( bool show )
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->lastItem();
    if (orig != dim && show )
	moveSelection( orig, dim );
    return dim;
}

DayItem *DayViewContents::nextItem( bool show )
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->nextItem();
    if (orig != dim && show )
	moveSelection( orig, dim );
    return dim;
}

DayItem *DayViewContents::previousItem( bool show )
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->previousItem();
    if (orig != dim && show )
	moveSelection( orig, dim );
    return dim;
}

DayItem *DayViewContents::currentItem() const
{
    return itemList->focusedItem();
}

void DayViewContents::setCurrentItem(DayItem *dim, bool show)
{
    DayItem *orig = itemList->focusedItem();
    if (dim != orig)
	itemList->setFocusedItem(dim);
    if (show)
	moveSelection( orig, dim );
}

void DayViewContents::moveSelection( DayItem *from, DayItem *to )
{
    if (to) {
	QRect r = expandRect(to->geometry());
	visRect = to->geometry();
	if (typ != AllDay) {
	    r.moveBy(time_width, 0);
	    visRect.moveBy(time_width, 0);
	}
	visibleTimer->start(5, TRUE);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (from && from != to) {
	QRect ogr = expandRect(from->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
}

void DayViewContents::makeVisible()
{
    if ( visRect.x() < 0 || visRect.y() < 0 || visRect.width() < 0 || visRect.height() < 0 )
	return;
    int cx = visRect.x() + visRect.width()/2;
    int cy = visRect.y() + visRect.height()/2;
    ensureVisible( cx, cy, visRect.width()/2+5, visRect.height()/2+5 );
}

void DayViewContents::moveUp()
{
    scrollBy(0, -20);
}

void DayViewContents::moveDown()
{
    scrollBy(0, 20);
}

void DayViewContents::resizeEvent( QResizeEvent * )
{
    setMetrics( QFontMetrics( qApp->font() ) );
}

void DayView::fontChange( const QFont & )
{
    view->setMetrics( QFontMetrics(qApp->font() ));
    allView->setMetrics( QFontMetrics(qApp->font() ));
}

void DayViewContents::setMetrics( const QFontMetrics &fm )
{
    time_width = QFontMetrics(dayViewFont()).width(
	typ == CompressedDay
		? TimeString::localHM(QTime(12,45),TimeString::Short)
		: TimeString::localH(12)
	   )+day_time_margin*2;
    time_height = QMIN(fm.height() * 3, QMAX(fm.height(), height() / 10));
    itemList->setTimeHeight( time_height );
    day_height = fm.height() + 5;
#ifdef QTOPIA_DESKTOP
    day_height += 2;	// difference in font metrics?
#endif

    // resize children to fit if that is our type.
    switch(typ) {
	case ScrollingDay:
	    resizeContents(
		    width() - style().scrollBarExtent().width(),
		    posOfHour(24));
	    setContentsPos(0, posOfHour(start_of_day));
	    break;
	case CompressedDay:
	    {
		if ( dataChanged )
		    dataChanged = FALSE;
		else {
		    // If the data isn't changed, the vertical space available is
		    // smaller than it was before. Remove any assumptions about the
		    // vertical size to prevent making the day too long.
		    itemList->setSize( width(), height() );
		}
		bool needScrollBar = posOfHour(24) > height();
		resizeContents( needScrollBar ? width() - style().scrollBarExtent().width() : width(),
				posOfHour(24) );
		int hour = QTime::currentTime().hour();
		setContentsPos(0, posOfHour(hour) );
	    }
	    break;
	case AllDay:
	    resizeContents(
		    itemList->count() > 4
		    ? width() - style().scrollBarExtent().width()
		    : width(),
		    day_height * ((itemList->count() + 1) / 2 ));
	    break;
    }
    itemList->setSize(
	    contentsWidth() - (typ == AllDay ? 0 : time_width),
	    contentsHeight());
    itemList->layoutItems();

    updateContents(0,0, contentsWidth(), contentsHeight());
}

void DayViewContents::setType( Type viewType )
{
    typ = viewType;
    QDate ref = itemList->date();
    delete itemList;
    newItemList();
    itemList->setDate( ref );
}

void DayViewContents::contentsMousePressEvent( QMouseEvent *e )
{
    dragging = TRUE;

    SortedLayoutItems list = itemList->items();
    for (uint i = 0; i < list.size(); i++) {
	LayoutItem *itm = list.at(i);
	QRect geo = itm->geometry();
	if (typ != AllDay)
	    geo.moveBy(time_width, 0);
	if (geo.contains( e->pos() ) ) {
	    Occurrence oc = itm->occurrence();
	    PimEvent ev = itm->event();
	    emit setCurrentEvent( ev );
	    if ( e->button() == RightButton ) {
		QPopupMenu m;
		m.insertItem( tr( "Edit" ), 1 );
		m.insertItem( tr( "Delete" ), 2 );
#ifdef Q_WS_QWS
		if (Ir::supported()) {
		    m.insertItem( tr( "Beam" ), 3 );
		}
#endif
		int r = m.exec( e->globalPos() );
		if ( r == 1 ) {
		    emit editOccurrence( oc );
		} else if ( r == 2 ) {
		    emit removeOccurrence( oc );
		} else if ( r == 3 ) {
		    emit beamEvent( ev );
		}
	    } else {
		clicking = TRUE;
		mClickedItem = itm;
	    }

	    dragging = FALSE;
	    break;
	}
    }

    if ( dragging ) {
	int h = hourAtPos(e->pos().y());
	int oldSelMin = QMIN(startSel, endSel);
	int oldSelMax = QMAX(startSel, endSel);
	startSel = endSel = h;

	oldSelMin = oldSelMin == -1 ? startSel : QMIN(oldSelMin, startSel);
	oldSelMax = oldSelMax == -1 ? startSel : QMAX(oldSelMax, startSel);

	repaintContents(0, posOfHour(oldSelMin - 1),
		time_width, posOfHour(oldSelMax + 1), FALSE);
    }
}

void DayViewContents::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( dragging ) {
	int h = hourAtPos(e->pos().y());
	if (h == endSel)
	    return;
	int oldSelMin = QMIN(startSel, endSel);
	int oldSelMax = QMAX(startSel, endSel);
	endSel = h;

	oldSelMin = QMIN(oldSelMin, endSel);
	oldSelMax = QMAX(oldSelMax, endSel);
	repaintContents(0, posOfHour(oldSelMin - 1),
		time_width, posOfHour(oldSelMax + 1), FALSE);
    }
}

void DayViewContents::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( dragging ) {
	dragging = FALSE;
	int h = hourAtPos(e->pos().y());
	int oldSelMin = QMIN(startSel, endSel);
	int oldSelMax = QMAX(startSel, endSel);
	endSel = h;

	oldSelMin = QMIN(oldSelMin, endSel);
	oldSelMax = QMAX(oldSelMax, endSel);
	repaintContents(0, posOfHour(oldSelMin - 1),
		time_width, posOfHour(oldSelMax + 1), FALSE);
	// XXX should signal new event.
	emit keyPressed(QString::null);
    }

    if ( clicking ) {
	QRect geo = mClickedItem->geometry();

	if (typ != AllDay)
	    geo.moveBy(time_width, 0);
	if (!geo.contains( e->pos() ) )
	    clicking = FALSE;
    }

    if ( clicking && e->button() == LeftButton ) {
	emit showDetails();
    }

    clicking = FALSE;
}

void DayViewContents::keyPressEvent( QKeyEvent *e )
{
    QString txt = e->text();
    if ( !txt.isNull() && txt[0] > ' ' && e->key() < 0x1000 ) {
	// we this is some sort of thing we know about...
	e->accept();
	emit keyPressed( txt );
    } else {
	// I don't know what this key is, do you?
	e->ignore();
    }
}

QFont DayViewContents::dayViewFont() const
{
#ifdef QTOPIA_PHONE
    return QFont(font().family(), QMIN(font().pointSize()-2,12));
#else
    return font();
#endif
}

void DayViewContents::drawContents( QPainter *p, int, int y, int, int h)
{
    bool isClipped = p->hasClipping();
    QRegion oldClip = p->clipRegion();
    QRegion bgr = isClipped ? oldClip : QRect(0, 0, contentsWidth(), contentsHeight() );

    QRect clipRect( 0, y, contentsWidth(), h );
    bgr &= clipRect;

    // draw items first and clip away their geometries to reduce flicker.
    for (int k = 0; k < itemList->count(); k++) {
	DayItem *di = (DayItem*)itemList->items().at(k);

	QRect r = di->geometry();
	r = expandRect( r );
	r.moveBy( typ == AllDay ? 0 : time_width, 0 );

	if ( r.intersects( clipRect ) ) {
	    // A temporary hack that will be removed once the pseudo-preview mode is gone
	    if ( typ == CompressedDay )
		di->setCompressed();
	    di->drawItem(p, typ == AllDay ? 0 : time_width, 0, palette().active());
	    bgr -= r;
	}
    }

    bgr.translate( -contentsX(), -contentsY() );
    p->setClipRegion( bgr );

    //p->fillRect(time_width, x+w-time_width, y, h, palette().active().color(QColorGroup::Base));
    p->fillRect(time_width, 0,
	    contentsWidth() - time_width, contentsHeight(), palette().active().brush(QColorGroup::Base));

    int firstSel = QMIN(startSel, endSel);
    int lastSel = QMAX(startSel, endSel);

    // draw each hour.
    QFont f = dayViewFont();

    QFontMetrics fm(f);
    int base = QMIN(time_height - fm.height(), 2);
    base = time_height - (base / 2) - (fm.height() - fm.ascent());
    base -= 2;

    QColorGroup cgUp = palette().active();
    cgUp.setColor(QColorGroup::Button,
	    //cgUp.color(QColorGroup::Button).light(100));
	    palette().active().color(QColorGroup::Midlight));
    QFont selectFont = f;
    selectFont.setBold(TRUE);

    // also of possible use, QColorGroup::BrightText.

    switch (typ) {
	case ScrollingDay:
	    {
		int rx = 0;
		int ry = 0;
		int rh = 0;
		int t = 0;
		while (t < 24) {
		    QString s = TimeString::localH(t);
		    ry = posOfHour(t);
		    t++;
		    rh = posOfHour(t) - ry;

		    if (ry > y + h)
			break;
		    if (ry + rh < y)
			continue;

		    // draw this rect.
		    QPen pn = p->pen();
		    p->setPen(lightGray);
		    p->drawLine( rx, ry - 1, width(), ry - 1);
		    p->setPen(pn);

		    bool isDown;
		    if ( t > firstSel && t <= lastSel + 1 ){
			p->setFont(selectFont);
			isDown = TRUE;
		    } else {
			p->setFont(f);
			isDown = FALSE;
		    }

#if (QT_VERSION-0 >= 0x030000)
		    style().drawPrimitive(
			    QStyle::PE_ButtonBevel,
			    p,
			    QRect(rx, ry, time_width, time_height),
			    cgUp, isDown ? QStyle::Style_Enabled | QStyle::Style_Down : QStyle::Style_Raised);//QStyle::Style_Enabled | QStyle::Style_Up);

		    //style().drawToolButton(p, rx, ry, time_width, time_height,
			    //cgUp, t > firstSel && t <= lastSel + 1 );
		    style().drawItem(p, QRect(rx+day_time_margin, ry, time_width-day_time_margin*2, time_height),
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#else
		    p->fillRect( rx, ry, time_width, time_height, palette().active().brush(QColorGroup::Base) );
# ifndef QTOPIA_PHONE
		    style().drawToolButton(p, rx, ry, time_width, time_height,
			    cgUp, t > firstSel && t <= lastSel + 1 );
#endif
		    style().drawItem(p, rx+day_time_margin, ry, time_width-day_time_margin*2, time_height,
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#endif
		    p->setPen(pn);
		}
	    }
	    break;
	case CompressedDay:
	    {
		int hours[24];
		QString hourStrings[24];
		int num_hours = 0;
		const TimeList &times = ((CompressedDayLayout *)itemList)->times;
		int cur = 0;
		int prev = 0;
		int i;
		for ( i = 0; i < 24; i++ ) {
		    cur = times.hits(i);
		    if ( cur != prev ) {
			prev = cur;
			hours[num_hours] = i;
			//hourStrings[num_hours] = TimeString::timeString( times.time( i ), FALSE );
			hourStrings[num_hours] = TimeString::localHM( times.time(i), TimeString::Short );
			num_hours++;
		    }
		}

		int rx = 0;
		int ry = 0;
		int rh = 0;
		int t = 0;
		for ( i = 0; i < num_hours; i++ ) {
		    t = hours[i];
		    QString s = hourStrings[i];
		    ry = posOfHour(t);
		    t++;
		    rh = posOfHour(t + 1) - ry;

		    if (ry > y + h)
			break;
		    if (ry + rh < y)
			continue;

		    // draw this rect.
		    QPen pn = p->pen();
		    p->setPen(lightGray);
		    p->drawLine( rx, ry - 1, width(), ry - 1);
		    p->setPen(pn);

		    bool isDown;
		    if ( t > firstSel && t <= lastSel + 1 ){
			p->setFont(selectFont);
			isDown = TRUE;
		    } else {
			p->setFont(f);
			isDown = FALSE;
		    }

#if (QT_VERSION-0 >= 0x030000)
		    style().drawPrimitive(
			    QStyle::PE_ButtonBevel,
			    p,
			    QRect(rx, ry, time_width, rh),
			    cgUp, isDown ? QStyle::Style_Enabled | QStyle::Style_Down : QStyle::Style_Raised);//QStyle::Style_Enabled | QStyle::Style_Up);

		    //style().drawToolButton(p, rx, ry, time_width, rh,
			    //cgUp, t > firstSel && t <= lastSel + 1 );
		    style().drawItem(p, QRect(rx+day_time_margin, ry, time_width-day_time_margin*2, rh),
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#else
		    p->fillRect( rx, ry, time_width, rh, palette().active().brush(QColorGroup::Base) );
# ifndef QTOPIA_PHONE
		    style().drawToolButton(p, rx, ry, time_width, rh,
			    cgUp, t > firstSel && t <= lastSel + 1 );
# endif
		    style().drawItem(p, rx+day_time_margin, ry, time_width-day_time_margin*2, rh,
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#endif
		    p->setPen(pn);
		}
	    }
	    break;
	case AllDay:
	    break;
    }
    if ( isClipped )
	p->setClipRegion( oldClip );
    p->setClipping( isClipped );
}

int DayViewContents::posOfHour(int h) const
{
    switch (typ) {
	case ScrollingDay:
	    // all hours, regardless of height.
	    return h < 0 ? 0 : h*time_height;
	case CompressedDay:
	    if ( h <= 0 )
		return 0;
	    else if ( h > 25 )
		return height();
	    else {
		const TimeList &times = ((CompressedDayLayout *)itemList)->times;
		int n = times.scale( times.height( h ), height() );
		if ( h == 24 && n == 0 )
		    return height();
		return n;
	    }
	default:
	    return 0;
    }
    return 0;
}

int DayViewContents::hourAtPos(int h) const {
    switch (typ) {
	case ScrollingDay:
	    // all hours, regardless of height.
	    return h == 0 ? 0 : h/time_height;
	case CompressedDay:
	    {
		const TimeList &times = ((CompressedDayLayout *)itemList)->times;
		for ( int i = 1; i <= 24; i++ ) {
		    int n = times.scale( times.height( i ), height() );
		    if ( n >= h )
			return i - 1;
		}
		return 0;
	    }
	default:
	    return 0;
    }
    return 0;
}

//===========================================================================

DayView::DayView( DateBookTable *newDb, bool startOnMonday,
			  QWidget *parent, const char *name )
    : PeriodView( newDb, startOnMonday, parent, name )
{
    QFontMetrics fm( font() );
    day_height = fm.height() + 5;
#ifdef QTOPIA_DESKTOP
    day_height += 2;	// difference in font metrics?
#endif

    QVBoxLayout *layout = new QVBoxLayout(this);

#ifdef USE_LABEL_HEADER
    QHBox *header = new QHBox( this );
    header->setMargin( 2 );

    header1 = new QLabel(header);
    header1->setAlignment(AlignLeft);
    header1->setText(header1Text(cDate));

    header2 = new QLabel(header);
    header2->setAlignment(AlignRight);
    header2->setText(header2Text(cDate));
#else
    header = new DayViewHeader( startOnMonday, this, "day header" ); // No tr
    header->setDate( cDate.year(), cDate.month(), cDate.day() );
#endif
    layout->addWidget(header);

    allView = new DayViewContents( DayViewContents::AllDay,
	    this, "all day view" ); // No tr
    allView->setFocusPolicy(NoFocus);
    layout->addWidget(allView);
    view = new DayViewContents( DayViewContents::ScrollingDay,
	    this, "day view" ); // No tr
    view->setSizePolicy(
	    QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    view->setFrameStyle( QFrame::NoFrame );
    allView->setFrameStyle( QFrame::NoFrame );
    layout->addWidget(view);
#ifndef USE_LABEL_HEADER
    connect( header, SIGNAL( dateChanged(const QDate&) ),
             this, SLOT( selectDate(const QDate&) ) );
#endif
    connect( view, SIGNAL( sigColWidthChanged() ),
             this, SLOT( slotColWidthChanged() ) );

    connect( view, SIGNAL(keyPressed(const QString&)),
	     this, SIGNAL(newEvent(const QString&)) );

    connect( view, SIGNAL(editEvent(const PimEvent&)),
	     this, SIGNAL(editEvent(const PimEvent&)) );
    connect( view, SIGNAL(removeEvent(const PimEvent&)),
	     this, SIGNAL(removeEvent(const PimEvent&)) );
    connect( view, SIGNAL(beamEvent(const PimEvent&)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );
    connect( view, SIGNAL(editOccurrence(const Occurrence&)),
	     this, SIGNAL(editOccurrence(const Occurrence&)) );
    connect( view, SIGNAL(removeOccurrence(const Occurrence&)),
	     this, SIGNAL(removeOccurrence(const Occurrence&)) );
    connect( view, SIGNAL(showDetails()),
	     this, SIGNAL(showDetails()) );
    connect( view, SIGNAL(setCurrentEvent(const PimEvent&)),
	     this, SLOT(setCurrentEvent(const PimEvent&)) );

    connect( allView, SIGNAL(editOccurrence(const Occurrence&)),
	     this, SIGNAL(editOccurrence(const Occurrence&)) );
    connect( allView, SIGNAL(removeOccurrence(const Occurrence&)),
	     this, SIGNAL(removeOccurrence(const Occurrence&)) );
    connect( allView, SIGNAL(beamEvent(const PimEvent&)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );
    connect( allView, SIGNAL(showDetails()),
	     this, SIGNAL(showDetails()) );
    connect( allView, SIGNAL(setCurrentEvent(const PimEvent&)),
	     this, SLOT(setCurrentEvent(const PimEvent&)) );
    setFocusPolicy(StrongFocus);
    setFocus();
}

#ifdef USE_LABEL_HEADER

QString DayView::header1Text(const QDate &d) const
{
    return tr("%1", "Date as day of week")
	.arg(TimeString::localDayOfWeek(d, TimeString::Long));
}

QString DayView::header2Text(const QDate &d) const
{
    return tr("%1", "Date in local format")
	.arg(TimeString::localYMD(d));
}

#endif

void DayView::selectedDates( QDateTime &start, QDateTime &end )
{
    int sh = view->startHour();
    int eh = view->endHour();

    if (sh >= 0) {
	start.setTime( QTime( sh, 0, 0 ) );
	if (eh == 24) {
	    end.setDate(cDate.addDays(1));
	    end.setTime( QTime(0,0,0) );
	} else {
	    end.setTime( QTime( eh, 0, 0 ) );
	}
    }
}

void DayView::clearSelectedDates()
{
    view->clearSelectedTimes();
}

void DayView::selectDate(int y, int m)
{
    if ( y != cDate.year() || m != cDate.month() ) {
	QDate nd( y, m, 1 );
	if ( nd.daysInMonth() < cDate.day() )
	    selectDate(QDate(y, m, nd.daysInMonth()));
	else
	    selectDate(QDate(y, m, cDate.day()));
    }
}

void DayView::selectDate( const QDate &d)
{
    if ( cDate == d )
	return;
    PeriodView::selectDate(d);

#ifdef USE_LABEL_HEADER
    header1->setText(header1Text(d));
    header2->setText(header2Text(d));
#else
    header->setDate( d.year(), d.month(), d.day() );
#endif
    view->selectDate(d);

    relayoutPage();
}

bool DayView::hasSelection() const
{
    return (allView->currentItem() || view->currentItem());
}

PimEvent DayView::currentEvent() const
{
    DayItem *dim = allView->currentItem();
    if (dim)
	return dim->event();
    dim = view->currentItem();
    if (dim)
	return dim->event();
    return PimEvent();
}

Occurrence DayView::currentItem() const
{
    DayItem *dim = allView->currentItem();
    if (dim)
	return dim->occurrence();
    dim = view->currentItem();
    if (dim)
	return dim->occurrence();

    return Occurrence();
}

void DayView::setCurrentItem( const Occurrence &o )
{
    setCurrentEvent(o.event());
}

void DayView::setCurrentEvent( const PimEvent &e )
{
    DayViewContents *views[2] = { allView, view };
    DayItem *curr[2] = { allView->currentItem(), view->currentItem() };
    for ( int i = 0; i < 2; i++ ) {
	int other = (i + 1) % 2;
	DayItem *orig = views[i]->currentItem();
	DayItem *dim = views[i]->firstItem(FALSE);
	if (dim) {
	    do {
		if (dim->event() == e) {
		    views[i]->setCurrentItem( dim, FALSE );
		    views[i]->moveSelection(orig, dim);
		    if (!orig)
			views[other]->moveSelection( curr[other], 0 );
		    break;
		    viewWithFocus = i;
		    break;
		}
	    } while ((dim = views[i]->nextItem(FALSE)) != 0);
	}
    }
}

void DayView::setCompressDay( bool compressDay )
{
    view->setType( compressDay ? DayViewContents::CompressedDay : DayViewContents::ScrollingDay );
}

void DayView::redraw()
{
    //if ( isUpdatesEnabled() )
	relayoutPage();
}

void DayView::getEvents()
{
    view->clearItems();
    allView->clearItems();

    QValueList<Occurrence> eventList = db->getOccurrences( cDate, cDate );

    int allDayCount = 0;
    QValueListIterator<Occurrence> it;
    for ( it = eventList.begin(); it != eventList.end(); ++it ) {
	if ((*it).event().isAllDay()) {
	    allView->addOccurrence(*it, cDate);
	    allDayCount++;
	} else {
	    view->addOccurrence(*it, cDate);
	}
    }
    if (allDayCount) {
	int fixed_height;
	if (allDayCount > 4)
	    fixed_height = 2 * day_height + (day_height / 2 );
	else
	    fixed_height = day_height * (++allDayCount / 2 );

	allView->setMaximumHeight(fixed_height);
	allView->setMinimumHeight(fixed_height);

	allView->layoutItems();
	allView->show();
	allView->firstItem();
	viewWithFocus = 0;
    } else {
	allView->hide();
	view->firstItem(FALSE);
	viewWithFocus = 1;
    }

    emit selectionChanged();
}


void DayView::relayoutPage( bool fromResize )
{
    if ( !fromResize )
        getEvents();    // no need we already have them!
    view->setMetrics( QFontMetrics( qApp->font() ) );
    allView->layoutItems();

    return;
}

void DayView::setDayStarts( int startHere )
{
    PeriodView::setDayStarts(startHere);
    view->startAtTime(startHere);
}

void DayView::setStartOnMonday( bool bStartOnMonday )
{
#ifndef USE_LABEL_HEADER
    header->setStartOfWeek( bStartOnMonday );
#else
    Q_UNUSED(bStartOnMonday)
#endif
}

void DayView::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
	case Key_Up:
	    previousEvent();
	    break;
	case Key_Down:
	    nextEvent();
	    break;
	case Key_Left:
	    selectDate(cDate.addDays(-1));
	    break;
	case Key_Right:
	    selectDate(cDate.addDays(1));
	    break;
#ifdef QTOPIA_PHONE
	case Key_1:
	    selectDate(cDate.addDays(-7));
	    break;
	case Key_3:
	    selectDate(cDate.addDays(7));
	    break;
	case Key_4:
	    if (cDate.month() == 1)
		selectDate(cDate.year()-1, 12);
	    else
		selectDate(cDate.year(), cDate.month()-1);
	    break;
	case Key_6:
	    if (cDate.month() == 12)
		selectDate(cDate.year()+1, 1);
	    else
		selectDate(cDate.year(), cDate.month()+1);
	    break;
	case Key_7:
	    selectDate(cDate.year()-1, cDate.month());
	    break;
	case Key_9:
	    selectDate(cDate.year()+1, cDate.month());
	    break;
	case Key_5:
	    selectDate(QDate::currentDate());
	    break;
	case Key_Select:
	    if ( hasSelection() )
		emit showDetails();
	    break;
#else
	case Key_Space:
	case Key_Return:
	    if ( hasSelection() )
		emit showDetails();
	    break;
#endif
	default:
	    e->ignore();
    }
}

void DayView::previousEvent()
{
    if (viewWithFocus == 0) {
	if (!allView->previousItem())
	    if ( view->lastItem() )
		viewWithFocus = 1;
	    else
		allView->lastItem();
    } else  {
	if (!view->previousItem()) {
	    if (allView->lastItem())
		viewWithFocus = 0;
	    else
		view->lastItem();
	}
    }
}

void DayView::nextEvent()
{
    if (viewWithFocus == 0) {
	if (!allView->nextItem()) {
	    if (view->firstItem())
		viewWithFocus = 1;
	    else
		allView->firstItem();
	}
    } else  {
	if (!view->nextItem())
	    if ( allView->firstItem() )
		viewWithFocus = 0;
	    else
		view->firstItem();
    }
}

