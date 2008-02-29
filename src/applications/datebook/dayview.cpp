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

#include "dayview.h"
#include "dayviewheaderimpl.h"
#include "layoutmanager.h"
#include "datebookdb.h"

#ifdef Q_WS_QWS
#include <qtopia/ir.h>
#endif

#ifdef QTOPIA_DESKTOP
#include <common/iconloader.h>
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

static const int short_time_height = 20;
static int day_height = 18;

inline QRect expandRect(QRect r)
{
    QRect nr(r.x() - 1, r.y() - 1, r.width() + 1, r.height() + 1);
    return nr;
}

class DayItem : public LayoutItem
{
public:
    DayItem(const QDate &referenceDate, const Occurrence &ev)
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
	} else if ( event().isException() ) {
	    p->drawPixmap( x + w - 13 - d, y, pm_repeatE );
	    d += 17;
	}

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

private:
    void loadPMs()
    {
	if ( pm_repeat.isNull() ) {
	    pm_repeat = Resource::loadIconSet("repeat").pixmap();
#ifdef QTOPIA_DESKTOP
	    pm_repeatE = IconLoader::loadPixmap("repeatException");
	    pm_alarmbell = IconLoader::loadPixmap("smallalarm");
#else
	    pm_repeatE = Resource::loadIconSet("repeatException").pixmap();
	    pm_alarmbell = Resource::loadIconSet("smallalarm").pixmap();
#endif
	}
    }
    QPixmap pm_repeat, pm_alarmbell, pm_repeatE;
    QString headingText;

    QString noteTZ;
    QDateTime noteStart;
    QDateTime noteEnd;
    bool noteShowStartDate;
    bool noteShowEndDate;
    QString noteStr;

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
	if (pos >= (int)items().count()) {
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

    void addOccurrence(Occurrence &e, const QDate &cDate )
    {
	DayItem *di = new DayItem(cDate, e);
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

DayViewContents::DayViewContents( Type t,
	QWidget *parent, const char *name )
    : QScrollView( parent, name, WRepaintNoErase ),
      typ(t), start_of_day(8),
	time_height(short_time_height)
{
    setResizePolicy(Manual);
    startSel = endSel = -1;
    dragging = FALSE;
    QFontMetrics fm( font() );
    time_width = fm.width( TimeString::hourString(12,TRUE)+" " )+2;
    time_height = fm.height();
    day_height = fm.height() + 5;
#ifdef QTOPIA_DESKTOP
    day_height += 2;	// difference in font metrics?
#endif

    setHScrollBarMode(AlwaysOff);

    if (typ == AllDay) {
	setMargins(0, 0, 0, 0);
	setBackgroundMode(PaletteDark);
	itemList = new AllDayLayout(contentsWidth(), contentsHeight());
    } else {
	itemList = new DayViewLayout(contentsWidth() - time_width, contentsHeight());
    }

    viewport()->setBackgroundColor( white );
    visibleTimer = new QTimer( this );
    connect( visibleTimer, SIGNAL(timeout()), this, SLOT(makeVisible()) );

    TimeString::connectChange(this,SLOT(timeStringChanged()));
}

DayViewContents::~DayViewContents()
{
    clearItems();
    delete itemList;
}

void DayViewContents::addOccurrence(Occurrence &ev, const QDate &cDate)
{
    itemList->addOccurrence(ev, cDate);

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
    if (dim && dim != orig)
	itemList->setFocusedItem(dim);
    if (show)
	moveSelection( orig, dim );
}

void DayViewContents::moveSelection( DayItem *from, DayItem *to )
{
    if (to) {
	QRect r = expandRect(to->geometry());
	if (typ != AllDay)
	    r.moveBy(time_width, 0);
	visRect = r;
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
    setMetrics( QFontMetrics(qApp->font() ));
}

void DayView::fontChange( const QFont & )
{
    view->setMetrics( QFontMetrics(qApp->font() ));
    allView->setMetrics( QFontMetrics(qApp->font() ));
    getEvents();
}

void DayViewContents::setMetrics( const QFontMetrics &fm)
{
    time_height = QMIN(fm.height() * 3, QMAX(fm.height(), height() / 10));
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
	    contentsWidth() - (typ == AllDay ? 0 : time_width) - 1,
	    contentsHeight());
    itemList->layoutItems();

    updateContents(0,0, contentsWidth(), contentsHeight());
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
	    Occurrence oc = itm->occurrence();
	    PimEvent ev = itm->event();
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
	emit keyPressed(QString::null);
    }
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
	    di->drawItem(p, typ == AllDay ? 0 : time_width, 0, palette().active());
	    bgr -= r;
	}
    }

    bgr.translate( -contentsX(), -contentsY() );
    p->setClipRegion( bgr );

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
		int t = 0;
		while (t < 24) {
		    QString s = TimeString::localH(t);
		    s += " ";
		    ry = posOfHour(t);
		    t++;
		    rh = posOfHour(t) - ry;

		    if (ry > y + h)
			break;
		    if (ry + rh < y)
			continue;

		    // draw this rect.
		    QPen pn = p->pen();
		    //p->setPen(black);
		    p->drawLine( time_width, ry - 1, width(), ry - 1);

		    bool isDown;
		    if ( t > firstSel && t <= lastSel + 1 ){
			p->setFont(selectFont);
			isDown = TRUE;
		    } else {
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
		    style().drawItem(p, QRect(rx, ry, time_width, time_height),
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#else
		    p->fillRect( rx, ry, time_width, time_height, white );
		    style().drawToolButton(p, rx, ry, time_width, time_height,
			    cgUp, t > firstSel && t <= lastSel + 1 );
		    style().drawItem(p, rx, ry, time_width, time_height,
			    AlignRight | AlignTop, cgUp, TRUE, 0, s);
#endif
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
    if ( isClipped )
	p->setClipRegion( oldClip );
    p->setClipping( isClipped );
}

int DayViewContents::posOfHour(int h) const {
    switch (typ) {
	case ScrollingDay:
	    // all hours, regardless of height.
	    return h < 0 ? 0 : h*time_height;
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
#ifdef QTOPIA_NO_POINTER_INPUT
    header = new QLabel(this, "day header");
    header->setBackgroundMode(PaletteButton);
    header->setAlignment(AlignHCenter);
    header->setText(headerText(cDate));
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
#ifndef QTOPIA_NO_POINTER_INPUT
    connect( header, SIGNAL( dateChanged( const QDate & ) ),
             this, SLOT( selectDate( const QDate &) ) );
#endif
    connect( view, SIGNAL( sigColWidthChanged() ),
             this, SLOT( slotColWidthChanged() ) );

    connect( view, SIGNAL(keyPressed(const QString &)),
	     this, SIGNAL(newEvent(const QString&)) );

    connect( view, SIGNAL(editEvent(const PimEvent &)),
	     this, SIGNAL(editEvent(const PimEvent&)) );
    connect( view, SIGNAL(removeEvent(const PimEvent &)),
	     this, SIGNAL(removeEvent(const PimEvent&)) );
    connect( view, SIGNAL(beamEvent(const PimEvent &)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );
    connect( view, SIGNAL(editOccurrence(const Occurrence &)),
	     this, SIGNAL(editOccurrence(const Occurrence&)) );
    connect( view, SIGNAL(removeOccurrence(const Occurrence &)),
	     this, SIGNAL(removeOccurrence(const Occurrence&)) );

    connect( allView, SIGNAL(editOccurrence(const Occurrence &)),
	     this, SIGNAL(editOccurrence(const Occurrence&)) );
    connect( allView, SIGNAL(removeOccurrence(const Occurrence &)),
	     this, SIGNAL(removeOccurrence(const Occurrence&)) );
    connect( allView, SIGNAL(beamEvent(const PimEvent &)),
	     this, SIGNAL(beamEvent(const PimEvent&)) );
    setFocusPolicy(StrongFocus);
    setFocus();
}

#ifdef QTOPIA_NO_POINTER_INPUT
QString DayView::headerText(const QDate &d) const
{
    return tr("%1 - %2", "Date as day of week, followed by date in local format")
	.arg(TimeString::localDayOfWeek(d, TimeString::Long)).arg(TimeString::localYMD(d));
}
#endif

void DayView::selectedDates( QDateTime &start, QDateTime &end )
{
    start.setDate( cDate );
    end.setDate( cDate );

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

void DayView::selectDate( const QDate &d)
{
    if ( cDate == d )
	return;
    PeriodView::selectDate(d);

#ifdef QTOPIA_NO_POINTER_INPUT
    header->setText(headerText(d));
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
    viewWithFocus = 0;
    DayItem *orig = allView->currentItem();
    DayItem *dim = allView->firstItem(FALSE);
    if (dim) {
	// check all view;
	do {
	    if (dim->event() == e) {
		allView->setCurrentItem( dim, FALSE );
		allView->moveSelection(orig, dim);
		if (!orig)
		    view->setCurrentItem(0);
		return;
	    }
	} while ((dim = allView->nextItem(FALSE)) != 0);

    }
    orig = view->currentItem();
    // if found in all view, would have returned.
    viewWithFocus = 1;
    dim = view->firstItem(FALSE);
    if (dim) {
	// check all view;
	do {
	    if (dim->event() == e) {
		view->setCurrentItem( dim, FALSE );
		view->moveSelection(orig, dim);
		if (!orig)
		    allView->setCurrentItem(0);
		return;
	    }
	} while ((dim = view->nextItem(FALSE)) != 0);

    }
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
    view->layoutItems();
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
#ifndef QTOPIA_NO_POINTER_INPUT
    header->setStartOfWeek( bStartOnMonday );
#endif
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
