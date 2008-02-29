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

#include "datebookday.h"
#include "datebookdayheaderimpl.h"
#include "layoutmanager.h"
#include "datebookdb.h"

#include <qtopia/ir.h>
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

static const int time_height = 20;
static const int day_height = 18;

inline QRect expandRect(QRect r)
{
    QRect nr(r.x() - 1, r.y() - 1, r.width() + 1, r.height() + 1);
    return nr;
}

class DayItem : public LayoutItem
{
public:
    DayItem(const QDate &referenceDate, const Occurrence &ev, bool ampm = FALSE) 
	: LayoutItem(ev) , hasFocus(FALSE)
    {

	// why would someone use "<"?  Oh well, fix it up...
	// I wonder what other things may be messed up...
	QString strDesc = ev.event().description();
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

	headingText = "<b>" + strDesc + "</b><br>";
	if (!ev.event().timeZone().isEmpty()) {
	    QString tzText = ev.event().timeZone();
	    int i = tzText.find('/');
	    tzText = tzText.mid( i + 1 );
	    tzText = tzText.replace(QRegExp("_"), " ");
	    
	    noteText = "<b>Time zone:</b> " + tzText + "<br>";
	}
	noteText += "<b>" + QObject::tr("Start") + "</b>: ";


	if ( referenceDate != ev.date() ) {
	    // multi-day event.  Show start date
	    noteText += TimeString::longDateString( ev.event().start().date() );
	} else {
	    // Show start time.
	    noteText += TimeString::timeString( ev.event().start().time(), ampm, FALSE );
	}

	noteText += "<br><b>" + QObject::tr("End") + "</b>: ";
	if ( referenceDate != ev.endDate() ) {
	    // multi-day event.  Show end date
	    noteText += TimeString::longDateString( ev.event().end().date() );
	} else {
	    // Show end time.
	    noteText += TimeString::timeString( ev.event().end().time(), ampm, FALSE );
	}
	noteText += "<br><br>" + strNote;
	//setBackgroundMode( PaletteBase );

    }
    ~DayItem() { }

    void setFocus(bool b) { hasFocus = b; }

    void drawItem( QPainter *p, int x, int y, const QColorGroup &cg )
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

	loadPMs();

	if ( event().hasAlarm() ) {
	    p->drawPixmap( x + w - 13, y, pm_alarmbell );
	    d += 17;
	}

	if ( event().hasRepeat() ) {
	    p->drawPixmap( x + w - 13 - d, y, pm_repeat );
	    d += 17;
	}

	QSimpleRichText rt( headingText, p->font() );
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

private:
    void loadPMs()
    {
	if ( pm_repeat.isNull() ) {
	    pm_repeat = Resource::loadPixmap("repeat");
	    pm_alarmbell = Resource::loadPixmap("smallalarm");
	}
    }
    QPixmap pm_repeat, pm_alarmbell;
    QString headingText;
    QString noteText;
    bool hasFocus;
};

class DayViewLayout : public LayoutManager
{
public:
    DayViewLayout(int w, int h) : LayoutManager(w,h), fItem(0) { }

    void clear() {
	fItem = 0;
	LayoutManager::clear();
    }

    DayItem *focusedItem() const { return fItem; };
    void setFocusedItem(DayItem *i) { 
	if (fItem)
	    fItem->setFocus(FALSE);
	fItem = 0;
	if (i && items().containsRef(i)) {
	    i->setFocus(TRUE);
	    fItem = i;
	}
    }

    virtual DayItem *firstItem() 
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

    virtual DayItem *nextItem() 
    {
	if (fItem)
	    fItem->setFocus(FALSE);
	else
	    return 0;

	int pos = items().findRef(fItem);
	pos++;
	if (pos >= items().count()) {
	    fItem = 0;
	} else {
	    fItem = (DayItem *)items().at(pos);
	    fItem->setFocus(TRUE);
	}
	return fItem; 
    }

    virtual DayItem *previousItem() 
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

    virtual DayItem *lastItem()
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

    void addOccurrence(Occurrence &e, bool ampm, const QDate &cDate ) 
    {
	DayItem *di = new DayItem(cDate, e, ampm);
	initializeGeometry(di);
	addItem(di);
    }
protected:
    DayItem *fItem;
};

class AllDayLayout : public DayViewLayout
{
public:
    AllDayLayout(int w, int h) : DayViewLayout(w,h) { }

    void layoutItems(bool = FALSE)
    {
	for ( int i = 0; i < count(); i++) {
	    LayoutItem *w = items().at(i);
	    // one under the other, full width.
	    QRect geom = w->geometry();
	    geom.setX( i % 2 ? size().width() / 2 : 0 );
	    geom.setY( day_height * (i / 2) );
	    geom.setWidth( i % 2 == 0 && i == count() - 1 ? 
		    size().width() :
		    size().width() / 2 );
	    geom.setHeight( day_height );
	    w->setGeometry(geom);
	}
    }
};

/*
   // Preserved since we may need to take this algorithm later
static int place( const DayViewWidget *item, bool *used, int maxn )
{
    int place = 0;
    int start = item->event().start().hour();
    QTime e = item->event().end();
    int end = e.hour();
    if ( e.minute() < 5 )
	end--;
    if ( end < start )
	end = start;
    while ( place < maxn ) {
	bool free = TRUE;
	int s = start;
	while( s <= end ) {
	    if ( used[10*s+place] ) {
		free = FALSE;
		break;
	    }
	    s++;
	}
	if ( free ) break;
	place++;
    }
    if ( place == maxn ) {
	return -1;
    }
    while( start <= end ) {
	used[10*start+place] = TRUE;
	start++;
    }
    return place;
}
*/

DayViewContents::DayViewContents( Type t, bool whichClock,
	QWidget *parent, const char *name )
    : QScrollView( parent, name ),
      ampm( whichClock ), typ(t), start_of_day(8)
{
    setResizePolicy(Manual);
    startSel = endSel = -1;
    dragging = FALSE;
    QFontMetrics fm( font() );
    time_width = fm.width( "p00:00p" );

    setHScrollBarMode(AlwaysOff);
    enableClipper(TRUE); // makes scrolling with child widgets faster.

    if (typ == AllDay) {
	setMargins(0, 3, 0, 0);
	setBackgroundMode(PaletteDark);
	itemList = new AllDayLayout(contentsWidth() - 1, contentsHeight());
    } else {
	itemList = new DayViewLayout(contentsWidth() - time_width - 1, contentsHeight());
    }

    QObject::connect( qApp, SIGNAL(clockChanged(bool)),
                      this, SLOT(slotChangeClock(bool)) );
}

DayViewContents::~DayViewContents()
{
    clearItems();
    delete itemList;
}

void DayViewContents::slotChangeClock( bool newClock )
{
    ampm = newClock;
}

bool DayViewContents::whichClock() const
{
    return ampm;
}

void DayViewContents::addOccurrence(Occurrence &ev, const QDate &cDate)
{
    itemList->addOccurrence(ev, ampm, cDate);

    if (typ == AllDay) {
	resizeContents(
		itemList->count() > 4 
		? width() - style().scrollBarExtent().width() : width(), 
		day_height * ((itemList->count() + 1) / 2 ));
	itemList->setSize(
		contentsWidth() - (typ == AllDay ? 0 : time_width) - 1, 
		contentsHeight());
    }
}

void DayViewContents::clearItems()
{
    itemList->clear();
    itemList->setFocusedItem(0);
}

void DayViewContents::layoutItems()
{
    itemList->layoutItems(TRUE);
    updateContents(0, 0, contentsWidth(), contentsHeight());
}

void DayViewContents::startAtTime(int t)
{
    start_of_day = t;
    if (typ != AllDay) 
	setContentsPos(0, posOfHour(t));
}

void DayViewContents::selectDate(const QDate &d) 
{
    itemList->setDate(d);
    showStartTime();
}

void DayViewContents::showStartTime()
{
    setContentsPos(0, posOfHour(start_of_day));
}


DayItem *DayViewContents::firstItem() 
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->firstItem();
    if (orig == dim)
	return dim;
    QRect r;
    if (dim) {
	r = expandRect(dim->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (orig) {
	QRect ogr = expandRect(orig->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
    if (dim)
	ensureVisible(r.x(), r.y(), r.width(), r.height());
    return dim;
}


DayItem *DayViewContents::lastItem() 
{
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->lastItem();
    if (orig == dim)
	return dim;
    QRect r;
    if (dim) {
	r = expandRect(dim->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (orig) {
	QRect ogr = expandRect(orig->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
    if (dim)
	ensureVisible(r.x(), r.y(), r.width(), r.height());
    return dim;
}

DayItem *DayViewContents::nextItem() 
{ 
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->nextItem();
    if (orig == dim)
	return dim;
    QRect r;
    if (dim) {
	r = expandRect(dim->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (orig) {
	QRect ogr = expandRect(orig->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
    if (dim)
	ensureVisible(r.x(), r.y(), r.width(), r.height());
    return dim;
}

DayItem *DayViewContents::previousItem() 
{ 
    DayItem *orig = itemList->focusedItem();
    DayItem *dim = itemList->previousItem();
    if (orig == dim)
	return dim;
    QRect r;
    if (dim) {
	r = expandRect(dim->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (orig) {
	QRect ogr = expandRect(orig->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
    if (dim)
	ensureVisible(r.x(), r.y(), r.width(), r.height());
    return dim;
}

DayItem *DayViewContents::currentItem() const 
{
    return itemList->focusedItem();
}

void DayViewContents::setCurrentItem(DayItem *dim) 
{
    DayItem *orig = itemList->focusedItem();
    if (dim == orig)
	return;
    QRect r;
    if (dim) {
	itemList->setFocusedItem(dim);
	r = expandRect(dim->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	updateContents(r.x(),r.y(), r.width(), r.height());
    }
    if (orig) {
	QRect ogr = expandRect(orig->geometry());
	if (typ != AllDay)
	    ogr.moveBy(time_width, 0);
	updateContents(ogr.x(),ogr.y(), ogr.width(), ogr.height());
    }
    if (dim)
	ensureVisible(r.x(), r.y(), r.width(), r.height());
}

void DayViewContents::moveUp()
{
    scrollBy(0, -20);
}

void DayViewContents::moveDown()
{
    scrollBy(0, 20);
}

void DayViewContents::resizeEvent( QResizeEvent *e )
{
    // resize children to fit if that is our type.
    switch(typ) {
	case ScrollingDay:
	    resizeContents(
		    e->size().width() - style().scrollBarExtent().width(), 
		    posOfHour(24));
	    setContentsPos(0, posOfHour(start_of_day));
	    break;
	case CompressedDay:
	    break;
	case AllDay:
	    resizeContents(
		    itemList->count() > 4 
		    ? e->size().width() - style().scrollBarExtent().width() 
		    : e->size().width(), 
		    day_height * ((itemList->count() + 1) / 2 ));
	    break;
    }
    itemList->setSize(
	    contentsWidth() - (typ == AllDay ? 0 : time_width) - 1, 
	    contentsHeight());
    itemList->layoutItems();
}

void DayViewContents::contentsMousePressEvent( QMouseEvent *e )
{
    SortedLayoutItems list = itemList->items();
    dragging = TRUE;

    for (uint i = 0; i < list.size(); i++) {
	LayoutItem *itm = list.at(i);
	QRect geo = itm->geometry();
	if (typ != AllDay)
	    geo.moveBy(time_width, 0);
	if (geo.contains( e->pos() ) ) {
	    PimEvent ev = itm->event();
	    QPopupMenu m;
	    m.insertItem( tr( "Edit" ), 1 );
	    m.insertItem( tr( "Delete" ), 2 );
	    if (Ir::supported()) {
		m.insertItem( tr( "Beam" ), 3 );
	    }
	    int r = m.exec( e->globalPos() );
	    if ( r == 1 ) {
		emit editEvent( ev );
	    } else if ( r == 2 ) {
		emit removeEvent( ev );
	    } else if ( r == 3 ) {
		emit beamEvent( ev );
	    }
	    dragging = FALSE;
	    break;
	}
    }

    if (dragging) {
	int h = hourAtPos(e->pos().y());
	int oldSelMin = QMIN(startSel, endSel);
	int oldSelMax = QMAX(startSel, endSel);
	startSel = endSel = h;

	oldSelMin = oldSelMin == -1 ? startSel : QMIN(oldSelMin, startSel);
	oldSelMax = oldSelMax == -1 ? startSel : QMAX(oldSelMax, startSel);

	repaintContents(0, posOfHour(oldSelMin - 1), 
		time_width, posOfHour(oldSelMax + 1), FALSE);
	dragging = TRUE;
    }
}

void DayViewContents::contentsMouseMoveEvent( QMouseEvent *e )
{
    if (dragging) {
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
    if (dragging) {
	int h = hourAtPos(e->pos().y());
	int oldSelMin = QMIN(startSel, endSel);
	int oldSelMax = QMAX(startSel, endSel);
	endSel = h;

	oldSelMin = QMIN(oldSelMin, endSel);
	oldSelMax = QMAX(oldSelMax, endSel);
	repaintContents(0, posOfHour(oldSelMin - 1), 
		time_width, posOfHour(oldSelMax + 1), FALSE);
	// XXX should signal new event.
	emit sigCapturedKey(QString::null);
    }
}

void DayViewContents::keyPressEvent( QKeyEvent *e )
{
    QString txt = e->text();
    if ( !txt.isNull() && txt[0] > ' ' && e->key() < 0x1000 ) {
	// we this is some sort of thing we know about...
	e->accept();
	emit sigCapturedKey( txt );
    } else {
	// I don't know what this key is, do you?
	e->ignore();
    }
}

void DayViewContents::drawContents( QPainter *p, int, int y, int, int h)
{
    //p->fillRect(time_width, x+w-time_width, y, h, palette().active().color(QColorGroup::Base));
    p->fillRect(time_width, 0, 
	    contentsWidth() - time_width, contentsHeight(), white);

    int firstSel = QMIN(startSel, endSel);
    int lastSel = QMAX(startSel, endSel);

    // draw each hour.
    QFont f = p->font();
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
		int i = 0;
		while (i < 24) {
		    QString strTmp;
		    if ( ampm ) {
			if ( i == 0 )
			    strTmp = QString::number(12) + ":00";
			else if ( i == 12 )
			    strTmp = QString::number(12) + tr(":00p");
			else if ( i > 12 )
			    strTmp = QString::number( i - 12 ) + tr(":00p");
			else
			    strTmp = QString::number(i) + ":00";
		    } else {
			if ( i < 10 )
			    strTmp = "0" + QString::number(i) + ":00";
			else
			    strTmp = QString::number(i) + ":00";
		    }
		    strTmp += " ";
		    ry = posOfHour(i);
		    i++;
		    rh = posOfHour(i) - ry;

		    if (ry > y + h)
			break;
		    if (ry + rh < y)
			continue;

		    // draw this rect.
		    QPen pn = p->pen();
		    //p->setPen(black);
		    p->drawLine( time_width, ry - 1, width(), ry - 1);
		    int offset = fm.width(strTmp);
		    if (ampm &&  i <= 12)
			offset += fm.width("p");

		    if ( i > firstSel && i <= lastSel + 1 )
			p->setFont(selectFont);

		    style().drawToolButton(p, rx, ry, time_width, time_height, 
			    cgUp, i > firstSel && i <= lastSel + 1 );
		    style().drawItem(p, rx, ry, time_width, time_height, 
			    AlignRight | AlignVCenter, cgUp, TRUE, 0, strTmp);
		    p->setFont(f);
		    p->setPen(pn);
		}
	    }
	    break;
	case CompressedDay:
	    break;
	case AllDay:
	    break;
    }
    // and now the items.

    for (int k = 0; k < itemList->count(); k++) {
	DayItem *di = (DayItem*)itemList->items().at(k);

	di->drawItem(p, typ == AllDay ? 0 : time_width, 0, palette().active());
    }
}

int DayViewContents::posOfHour(int h) const {
    switch (typ) {
	case ScrollingDay:
	    // all hours, regardless of height.
	    return h*time_height;
	case CompressedDay:
	    // all hours, in height.
	    break;
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
	    // all hours, in height.
	    break;
	default:
	    return 0;
    }
    return 0;
}

//===========================================================================

DayView::DayView( DateBookTable *newDb, bool ampm, bool startOnMonday,
			  QWidget *parent, const char *name )
    : PeriodView( newDb, ampm, startOnMonday, parent, name )
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    header = new DayViewHeader( startOnMonday, this, "day header" );
    header->setDate( cDate.year(), cDate.month(), cDate.day() );
    layout->addWidget(header);
    allView = new DayViewContents( DayViewContents::AllDay, ampm, 
	    this, "all day view" );
    allView->setFocusPolicy(NoFocus);
    layout->addWidget(allView);
    view = new DayViewContents( DayViewContents::ScrollingDay, ampm, 
	    this, "day view" );
    view->setSizePolicy(
	    QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    view->setFrameStyle( QFrame::NoFrame );
    layout->addWidget(view);
    connect( header, SIGNAL( dateChanged( int, int, int ) ),
             this, SLOT( dateChanged( int, int, int ) ) );
    connect( view, SIGNAL( sigColWidthChanged() ),
             this, SLOT( slotColWidthChanged() ) );
    connect( qApp, SIGNAL(weekChanged(bool)),
	     this, SLOT(setStartOnMonday(bool)) );
    connect( view, SIGNAL(sigCapturedKey(const QString &)),
	     this, SIGNAL(sigNewEvent(const QString&)) );

    connect( view, SIGNAL(editEvent(const PimEvent &)),
	     this, SIGNAL(editEvent(const PimEvent&)) );
    connect( view, SIGNAL(removeEvent(const PimEvent &)),
	     this, SIGNAL(removeEvent(const PimEvent&)) );
    connect( view, SIGNAL(beamEvent(const PimEvent &)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );

    connect( allView, SIGNAL(editEvent(const PimEvent &)),
	     this, SIGNAL(editEvent(const PimEvent&)) );
    connect( allView, SIGNAL(removeEvent(const PimEvent &)),
	     this, SIGNAL(removeEvent(const PimEvent&)) );
    connect( allView, SIGNAL(beamEvent(const PimEvent &)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );
    setFocusPolicy(StrongFocus);
    setFocus();
}

void DayView::selectedDates( QDateTime &start, QDateTime &end )
{
    start.setDate( cDate );
    end.setDate( cDate );

    int sh = view->startHour();
    int eh = view->endHour();

    if (sh >= 0) { 
	start.setTime( QTime( sh, 0, 0 ) );
	end.setTime( QTime( eh, 0, 0 ) );
    }
}

void DayView::selectDate( QDate d)
{
    if ( cDate == d )
	return;
    PeriodView::selectDate(d);

    header->setDate( d.year(), d.month(), d.day() );
    view->selectDate(d);

    relayoutPage();
}

void DayView::dateChanged( int y, int m, int d )
{
    selectDate(QDate(y,m,d));
#if 0
    QDate date( y, m, d );
    if ( cDate == date )
	return;
    cDate.setYMD( y, m, d );
    relayoutPage();
#endif 
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
	return dim->occurance();
    dim = view->currentItem();
    if (dim)
	return dim->occurance();

    return Occurrence();
}

void DayView::setCurrentItem( const Occurrence &o )
{
    setCurrentEvent(o.event());
}

void DayView::setCurrentEvent( const PimEvent &e ) 
{
    viewWithFocus = 0;
    if (allView->firstItem()) {
	// check all view;
	do {
	    if (allView->currentItem()->event() == e)
		return;
	} while (allView->nextItem());

    }
    // if found in all view, would have returned.
    viewWithFocus = 1;
    if (view->firstItem()) {
	// check all view;
	do {
	    if (view->currentItem()->event() == e)
		return;
	} while (view->nextItem());

    }
}

void DayView::redraw()
{
    if ( isUpdatesEnabled() )
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
    view->layoutItems();
    if (allDayCount) {
	allDayCount++;
	allView->setMaximumHeight(QMIN(50, day_height * (allDayCount / 2 ) + 5));
	allView->setMinimumHeight(QMIN(50, day_height * (allDayCount / 2 ) + 5));
	allView->layoutItems();
	allView->show();
	allView->firstItem();
	viewWithFocus = 0;
    } else {
	allView->hide();
	view->firstItem();
	viewWithFocus = 1;
    }

    emit selectionChanged();
}


void DayView::relayoutPage( bool fromResize )
{
    if ( !fromResize )
        getEvents();    // no need we already have them!
    view->layoutItems();
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
    header->setStartOfWeek( bStartOnMonday );
    //    redraw();
}

void DayView::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
	case Key_Up:
	    if (viewWithFocus == 0) {
		if (!allView->previousItem())
		    allView->firstItem();
	    } else  {
		if (!view->previousItem()) {
		    if (allView->lastItem())
			viewWithFocus = 0;
		    else
			view->firstItem();
		}
	    }
	    break;
	case Key_Down:
	    if (viewWithFocus == 0) {
		if (!allView->nextItem()) {
		    if (view->firstItem())
			viewWithFocus = 1;
		    else
			allView->lastItem();
		}
	    } else  {
		if (!view->nextItem())
		    view->lastItem();
	    }
	    break;
	case Key_Left:
	    selectDate(QDate(cDate).addDays(-1));
	    break;
	case Key_Right:
	    selectDate(QDate(cDate).addDays(1));
	    break;
	default:
	    e->ignore();
    }
}
