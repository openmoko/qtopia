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
#include "datebookweek.h"
#include "datebookweekheaderimpl.h"

#include <qtopia/calendar.h>
#include "datebookdb.h"

#include <qtopia/pim/event.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/timestring.h>

#include <qdatetime.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qstyle.h>

static const int allDayHeight = 8;
static const int hourMargin = 4;

WeekViewContents::WeekViewContents( WeekView *parent, const char *name )
    : QScrollView( parent, name ),
      showingEvent( false ), wv(parent)
{
    items.setAutoDelete( true );
    dayItems.setAutoDelete( true );

    viewport()->setBackgroundMode( PaletteBase );

    header = new QHeader( this );
    header->addLabel( "" );

    header->setMovingEnabled( false );
    header->setResizeEnabled( false );
    header->setClickEnabled( false, 0 );
    updateWeekNames(parent->startsOnMonday());

    connect( header, SIGNAL(clicked(int)), this, SIGNAL(activateWeekDay(int)) );
    TimeString::connectChange( this, SLOT(timeStringChanged()) );

    QFontMetrics fm( font() );
    rowHeight = fm.height()+2;

    resizeContents( width(), 24*rowHeight );
}

void WeekViewContents::updateWeekNames(bool bOnMonday)
{
    static bool bFirst = true;
    if ( bFirst ) {
	bFirst = false;
	// NO TR. We are inserting dummy labels that are overwritten below.
	header->addLabel( "Monday" );
	header->addLabel( "Tuesday");
	header->addLabel( "Wednesday" );
	header->addLabel( "Thursday" );
	header->addLabel( "Friday" );
	header->addLabel( "Saturday" );
	header->addLabel( "Sunday" );
    }

    TimeString::Length len = TimeString::Short;
    int approxSize = QFontMetrics(QFont()).width(" Wed ") * 7;
    if ( QApplication::desktop()->width() > approxSize )
	len = TimeString::Medium;

    for ( int i = 0; i < 7; i++ ) {
	if ( bOnMonday )
	    header->setLabel( i + 1, TimeString::localDayOfWeek( i + 1, len ) );
	else
	    header->setLabel( i + 1, TimeString::localDayOfWeek( ((i + 6) % 7) + 1, len ) );
    }
}



void WeekViewContents::showEvents( QValueList<Occurrence> &ev, const QDate &startWeek )
{
    items.clear();
    items.resize(7);
    int i;
    for (i = 0; i < 7; i++) {
	LayoutManager *lm = new LayoutManager(header->sectionSize(i+1), rowHeight*24);
	lm->setDate(startWeek.addDays(i));
	items.insert(i, lm);

    }
    dayItems.clear();

    QValueListIterator<Occurrence> it;
    for ( it = ev.begin(); it != ev.end(); ++it ) {
	if ((*it).event().isAllDay()) {
	    LayoutItem *i = new LayoutItem(*it);
	    positionItem(i);
	    dayItems.append(i);
	} else {
	    int firstdow = startWeek.daysTo((*it).startInCurrentTZ().date());
	    int lastdow = startWeek.daysTo((*it).endInCurrentTZ().date());
	    if (firstdow < 0) firstdow = 0;
	    if (lastdow > 6) lastdow = 6;

	    for (int i = firstdow; i <= lastdow; i++)
		items.at(i)->addOccurrence( *it );
	}
    }

    int mWidth = contentsWidth();
    for (i = 0; i < 7; i++) {
	items.at(i)->layoutItems(FALSE);
	mWidth = QMIN(mWidth, items.at(i)->maximumColumnWidth());
    }
    for (i = 0; i < 7; i++) {
	items.at(i)->setMaximumColumnWidth(mWidth);
	items.at(i)->layoutItems(FALSE);
    }

    viewport()->update();
    update();
}

void WeekViewContents::moveToHour( int h )
{
    int offset = h*rowHeight;
    setContentsPos( 0, offset );
}

void WeekViewContents::keyPressEvent( QKeyEvent *e )
{
    e->ignore();
}

void WeekViewContents::timeStringChanged()
{
    viewport()->update();
}

static inline int db_round30min( int m )
{
    if ( m < 15 )
	m = 0;
    else if ( m < 45 )
	m = 1;
    else
	m = 2;

    return m;
}

void WeekViewContents::alterDay( int day )
{
    if ( !wv->startsOnMonday() ) {
	    day--;
    }
    emit activateWeekDay( day );
}

// Only for all day items which we have a very custom layout for.
void WeekViewContents::positionItem( LayoutItem *i )
{
    const Occurrence ev = i->occurrence();

    int dow, duration;
    if (ev.startInCurrentTZ().date() < ((WeekView *)parent())->weekDate())  {
	dow = 0;
	duration = ((WeekView *)parent())->weekDate().daysTo(
		ev.endInCurrentTZ().date());
    } else  {
	dow = ev.startInCurrentTZ().date().dayOfWeek();
	duration = ev.startInCurrentTZ().date().daysTo(
	    ev.endInCurrentTZ().date());
    }

    if ( !wv->startsOnMonday() ) {
	if ( dow == 7 )
	    dow = 1;
	else
	    dow++;
    }

    // now make sure duration doesn't extend of the end fo the week.
    if (dow + duration > 7) {
	duration = 7 - dow;
    }

    int x = header->sectionPos( dow );
    int hwidth = header->sectionSize( dow );
    hwidth += hwidth*duration;
    LayoutItem *inter = 0;
    i->setGeometry(x, header->height(), hwidth, allDayHeight * 2);
    inter = intersects( i );
    if (inter) {
	// need to change geom of both;
	i->setGeometry( x, header->height() + 
		(ev.event().hasRepeat() ? allDayHeight : 0), 
		hwidth , allDayHeight);
	inter->setGeometry( inter->geometry().x(), header->height() +
		(inter->event().hasRepeat() ? allDayHeight : 0), 
		inter->geometry().width(), allDayHeight);
    }
}

LayoutItem *WeekViewContents::intersects( const LayoutItem *item )
{
    QRect geom = item->geometry();

    // We allow the edges to overlap
    geom.moveBy( 1, 1 );
    geom.setSize( geom.size()-QSize(2,2) );

    const PimEvent itemEvent = item->event();

    QListIterator<LayoutItem> it(dayItems);
    for ( ; it.current(); ++it ) {
	LayoutItem *i = it.current();
	const PimEvent iEvent = i->event();
	if (i != item && iEvent.hasRepeat() != itemEvent.hasRepeat()) {
	    if (i->geometry().intersects( geom ) )
		return i;
	}
    }

    return 0;
}

void WeekViewContents::contentsMousePressEvent( QMouseEvent *e )
{
    QValueList<Occurrence> list;
    for (int j = 0; j < 7; j++) {
	QVector<LayoutItem> v = items.at(j)->items();
	int x = header->sectionPos(j+1)-1;
	for ( uint k = 0; k < v.size(); k++) {
	    LayoutItem *i = v.at(k);
	    // Change to
	    QRect geo = i->geometry();
	    geo.moveBy(x,0);
	    if ( geo.contains( e->pos() ) ) {
		showingEvent = true;
		list.append(i->occurrence());
	    }
	}
    }
    if (list.count())
	emit eventsSelected( list );
}

void WeekViewContents::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( showingEvent ) {
	showingEvent = false;
	emit selectionCleared();
    } else {
	int d = header->sectionAt( e->pos().x() );
	if ( d > 0 ) {
// 	    if ( !bOnMonday )
// 		d--;
	    emit activateWeekDay( d );
	}
    }
}

void WeekViewContents::mousePressEvent( QMouseEvent *e )
{
//#if (QT_VERSION-0 >= 0x030000)
#if 0
    // hack to get around dumb 3.1 scrollview mouse events.
    if  (e->pos().y() > topMargin())
	return;
#endif

    QValueList<Occurrence> list;
    QListIterator<LayoutItem> it(dayItems);
    for ( ; it.current(); ++it ) {
	LayoutItem *i = it.current();
	if ( i->geometry().contains( e->pos() ) ) {
	    showingEvent = true;
	    list.append(i->occurrence());
	}
    }
    if (list.count())
	emit eventsSelected( list );
}

void WeekViewContents::mouseReleaseEvent( QMouseEvent *e )
{
//#if (QT_VERSION-0 >= 0x030000)
#if 0
    // hack to get around dumb 3.1 scrollview mouse events.
    if  (e->pos().y() > topMargin())
	return;
#endif

    if ( showingEvent ) {
	showingEvent = false;
	emit selectionCleared();
    } else {
	int d = header->sectionAt( e->pos().x() );
	if ( d > 0 ) {
	    emit activateWeekDay( d );
	}
    }
}

void WeekViewContents::drawContents( QPainter *p ) 
{
    p->fillRect(0, header->height(), header->width(), allDayHeight * 2, 
	    white);
	    //palette().color(QPalette::Normal , QColorGroup::Dark ));

    // draw day events.  
    QListIterator<LayoutItem> it(dayItems);
    for ( ; it.current(); ++it ) {
	LayoutItem *i = it.current();
	if ( i->event().isAllDay() ) {
	    p->fillRect( i->geometry(), i->event().color().light(175));
	}
    }

    QPen pen = p->pen();
    p->setPen( lightGray );

    // header->sectionPos(i)-1 because we want the end of the last header,
    // not the start of this one.
    for ( int i = 1; i <= 7; i++ )
	p->drawLine( header->sectionPos(i)-1, header->height(), 
		header->sectionPos(i)-1, header->height() + 2 * allDayHeight );

    p->drawLine( 0, header->height() + 2 * allDayHeight, 
	    header->width(), header->height() + 2 * allDayHeight );
    p->setPen(pen);
}

void WeekViewContents::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
    QRect ur( cx, cy, cw, ch );
    p->setPen( lightGray );

    // -1 because end of last header, not this.
    for ( int i = 1; i <= 7; i++ )
	p->drawLine( header->sectionPos(i)-1, cy, header->sectionPos(i)-1, cy+ch );

    int w = header->sectionPos(1);
    p->setPen( black );
    for ( int t = 0; t < 24; t++ ) {
	int y = t*rowHeight;
	QRect r( hourMargin, y, w-hourMargin*2, rowHeight );
	if ( r.intersects( ur ) ) {
	    p->drawText( r, AlignRight | AlignTop, TimeString::localH(t) );
	}
    }

    for (int j = 0; j < 7; j++) {
	QVector<LayoutItem> v = items.at(j)->items();
	int x = header->sectionPos(j+1)-1; // 1 for end of last, 1 for contenst shift.
	for ( uint k = 0; k < v.size(); k++) {
	    LayoutItem *i = v.at(k);
	    // Change to
	    QRect geo = i->geometry();
	    geo.moveBy(x,-1); // up and to the left
	    geo.setSize(geo.size() + QSize(1,1));
	    if ( geo.intersects( ur ) ) {
		p->setBrush( i->event().color());
		p->drawRect( geo );
	    }
	}
    }
}

void WeekViewContents::resizeEvent( QResizeEvent *e )
{
    QFontMetrics fm(qApp->font());
    rowHeight = QMIN(fm.height() * 3, QMAX(fm.height(), e->size().height() / 10));
    int hourWidth = fm.width(tr("%1am").arg(12))+hourMargin*2;
    QScrollView::resizeEvent( e );
    int avail = width() - ( qApp->style().scrollBarExtent().width() );
    resizeContents( avail, posOfHour(24) );
    header->setGeometry( 0, 0, avail, header->sizeHint().height() );
    setMargins( 0, header->height() + allDayHeight * 2 + 1, 0, 0 );
    header->resizeSection( 0, hourWidth );
    int sw = (avail - hourWidth) / 7;
    int i;
    for ( i = 1; i < 7; i++ )
	header->resizeSection( i, sw );
    header->resizeSection( 7, avail - hourWidth - sw*6 );

    ((WeekView*)parent())->redraw();
}

void WeekViewContents::setStartOfWeek( bool bStartOnMonday )
{
    updateWeekNames(bStartOnMonday);
}

int WeekViewContents::posOfHour(int h) const
{
    return h*rowHeight;
}

int WeekViewContents::hourAtPos(int h) const
{
    return h == 0 ? 0 : h/rowHeight;
}

//-------------------------------------------------------------------

WeekView::WeekView( DateBookTable *newDB, bool startOnMonday, 
			QWidget *parent, const char *name )
    : PeriodView( newDB, startOnMonday,  parent, name ), year(0), _week(0)
{
    setFocusPolicy(StrongFocus);
    QVBoxLayout *vb = new QVBoxLayout( this );
    header = new WeekViewHeader( startOnMonday, this );
    contents = new WeekViewContents( this );
    contents->setFrameStyle( QFrame::NoFrame );
    contents->setFrameRect( QRect(0,0,0,0) );
    vb->addWidget( header );
    vb->addWidget( contents );

    lblDesc = new QLabel( this, "event label" ); // No tr
    lblDesc->setFrameStyle( QFrame::Plain | QFrame::Box );
    lblDesc->setBackgroundColor( yellow );
    lblDesc->hide();

    tHide = new QTimer( this );

    connect( contents, SIGNAL( activateWeekDay(int) ),
             this, SLOT( dayActivated(int) ) );
    connect( contents, SIGNAL(eventsSelected(QValueList<Occurrence>&)),
	     this, SLOT(showEventsLabel(QValueList<Occurrence>&)) );
    connect( contents, SIGNAL(selectionCleared()),
	     this, SLOT(hideEventsLabel()) );
    connect( header, SIGNAL( dateChanged(int,int) ),
             this, SLOT( setWeek(int,int) ) );
    connect( tHide, SIGNAL( timeout() ),
             lblDesc, SLOT( hide() ) );
    connect( header->spinYear, SIGNAL(valueChanged(int)),
	     this, SLOT(setYear(int)) );
    connect( qApp, SIGNAL( weekChanged(bool) ),
             this, SLOT( setStartsOnMonday(bool) ) );
    selectDate(QDate::currentDate());

}

void WeekView::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
	case Key_Up:
	    contents->scrollBy(0, -20);
	    break;
	case Key_Down:
	    contents->scrollBy(0, 20);
	    break;
	case Key_Left:
	    selectDate(currentDate().addDays(-7));
	    break;
	case Key_Right:
	    selectDate(currentDate().addDays(7));
	    break;
	default:
	    e->ignore();
    }
}

void WeekView::dayActivated( int day )
{
    QDate d;
    d = dateFromWeek( _week, year, bOnMonday );
    day--;
    d = d.addDays( day );
    emit dateActivated(d);
}

bool WeekView::calcWeek( const QDate &d, int &week, int &year ) const
{
// For Weeks that start on Monday... (EASY!)
// At the moment we will use ISO 8601 method for computing
// the week.  Granted, other countries use other methods,
// bet we aren't doing any Locale stuff at the moment.  So,
// this should pass.  This Algorithim is public domain and
// available at:
// http://personal.ecu.edu/mccartyr/ISOwdALG.txt
// the week number is return, and the year number is returned in year
// for Instance 2001/12/31 is actually the first week in 2002.
// There is a more mathematical definition, but I will implement it when
// we are pass our deadline.

// For Weeks that start on Sunday... (ahh... home rolled)
// okay, if Jan 1 is on Friday or Saturday,
// it will go to the pervious
// week...

    int weekNumber;
    int yearNumber;

    // remove a pesky warning, (Optimizations on g++)
    weekNumber = -1;
    int jan1WeekDay = QDate(d.year(), 1, 1).dayOfWeek();
    int dayOfWeek = d.dayOfWeek();

    if ( !d.isValid() )
	return false;

    if ( startsOnMonday() ) {
	// find the Jan1Weekday;
	if ( d.dayOfYear() <= ( 8 - jan1WeekDay) && jan1WeekDay > 4 ) {
	    yearNumber = d.year() - 1;
	    if ( jan1WeekDay == 5 || ( jan1WeekDay == 6 && QDate::leapYear(yearNumber) ) )
		weekNumber = 53;
	    else
		weekNumber = 52;
	} else
	    yearNumber = d.year();
	if ( yearNumber == d.year() ) {
	    int totalDays = 365;
	    if ( QDate::leapYear(yearNumber) )
		totalDays++;
	    if ( ((totalDays - d.dayOfYear()) < (4 - dayOfWeek) )
		 || (jan1WeekDay == 7) && (totalDays - d.dayOfYear()) < 3) {
		yearNumber++;
		weekNumber = 1;
	    }
	}
	if ( yearNumber == d.year() ) {
	    int j = d.dayOfYear() + (7 - dayOfWeek) + ( jan1WeekDay - 1 );
	    weekNumber = j / 7;
	    if ( jan1WeekDay > 4 )
		weekNumber--;
	}
    } else {
	// it's better to keep these cases separate...
	if ( d.dayOfYear() <= (7 - jan1WeekDay) && jan1WeekDay > 4
	     && jan1WeekDay != 7 ) {
	    yearNumber = d.year() - 1;
	    if ( jan1WeekDay == 6
		 || (jan1WeekDay == 7 && QDate::leapYear(yearNumber) ) ) {
		weekNumber = 53;
	    }else
		weekNumber = 52;
	} else
	    yearNumber = d.year();
	if ( yearNumber == d.year() ) {
	    int totalDays = 365;
	    if ( QDate::leapYear( yearNumber ) )
		totalDays++;
	    if ( ((totalDays - d.dayOfYear()) < (4 - dayOfWeek % 7)) ) {
		yearNumber++;
		weekNumber = 1;
	    }
	}
	if ( yearNumber == d.year() ) {
	    int j = d.dayOfYear() + (7 - dayOfWeek % 7) + ( jan1WeekDay - 1 );
	    weekNumber = j / 7;
	    if ( jan1WeekDay > 4 ) {
		weekNumber--;
	    }
	}
    }
    year = yearNumber;
    week = weekNumber;
    return true;
}

void WeekView::selectDate(const QDate &date)
{
    int w, y;
    calcWeek( date, w, y );
    header->setDate( y, w );
    cDate = date;
    getEvents();
}

void WeekView::setWeek( int y, int w )
{
    if (y == year && w == _week)
	return;
    year = y;
    _week = w;
    
    selectDate( weekDate() );
   // getEvents();
}

void WeekView::setYear( int y )
{
    if (y == year)
	return;
    setWeek(y, _week);
#if 0
    int totWeek;
    QDate d( y, 12, 31 );
    int throwAway;
    calcWeek( d, totWeek, throwAway, bOnMonday );
    while ( totWeek == 1 ) {
	d = d.addDays( -1 );
	calcWeek( d, totWeek, throwAway, bOnMonday );
    }
    if ( totWeek != totalWeeks() )
	setTotalWeeks( totWeek );
    cDate = weekDate();
#endif
}

void WeekView::databaseUpdated()
{
    getEvents();
}

void WeekView::getEvents()
{
    QDate startWeek = weekDate();

    QDate endWeek = startWeek.addDays( 6 );
    QValueList<Occurrence> eventList = db->getOccurrences(startWeek,
								  endWeek);
    contents->showEvents( eventList , startWeek );
    contents->moveToHour( sHour );
}

void WeekView::showEventsLabel( QValueList<Occurrence> &events )
{
    if ( tHide->isActive() )
        tHide->stop();

    QString str = "";

    // why would someone use "<"?  Oh well, fix it up...
    // I wonder what other things may be messed up...
    QValueListIterator<Occurrence> it;
    for(it = events.begin(); it != events.end(); ++it) {
	Occurrence ev = (*it);
	QString strDesc = ev.event().description();
	int where = strDesc.find( "<" );
	while ( where != -1 ) {
	    strDesc.remove( where, 1 );
	    strDesc.insert( where, "&#60;" );
	    where = strDesc.find( "<", where );
	}

	QString strCat;
	// ### FIX later...
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

	str += "<b>" + strDesc + "</b><br>" + "<i>"
	    + strCat + "</i><br>";

	if (ev.start().date() == ev.end().date()) {
	    str += TimeString::localYMD( ev.date(), TimeString::Long )
	    + "<br>";
	}

	/*
	if (ev.event().isAllDay()) {
	    if (ev.event().start().date() != ev.event().end().date()) {
		str += TimeString::longDateString( ev.event().end().date() );
		str += "<br>";
	    }
	    str += "<b>" + tr("All day") + "</b>";
	} else {
	*/
	if (ev.start().date() != ev.end().date() || !ev.event().isAllDay()) {
	    str += "<b>" + tr("Start") + "</b>: ";

	    if (ev.start().date() != ev.end().date()) {
		// multi-day event.  Show start date
		str += TimeString::localYMD( ev.start().date(), TimeString::Long );
		str += ", ";
	    }

	    if (! ev.event().isAllDay())
		str += TimeString::localHM(ev.start().time());

	    str += "<br><b>" + tr("End") + "</b>: ";
	    if (ev.start().date() != ev.end().date()) {
		str += TimeString::localYMD( ev.end().date(), TimeString::Long );
		str += ", ";
	    }
	    if (! ev.event().isAllDay())
		str += TimeString::localHM(ev.end().time());
	    str += "<br>";
	}

	if (ev.event().isAllDay())
	    str += "<b>" + tr("All day") + "</b><br>";
	str += "<br>" + strNote;
    }

    lblDesc->setText( str );
    lblDesc->resize( lblDesc->sizeHint() );
    // move the label so it is "centerd" horizontally...
    lblDesc->move( QMAX(0,(width() - lblDesc->width()) / 2), 0 );
    lblDesc->show();
}

void WeekView::hideEventsLabel()
{
    tHide->start( 2000, true );
}

void WeekView::setDayStarts( int startHere )
{
    sHour = startHere;
    contents->moveToHour( sHour );
}

void WeekView::redraw()
{
    getEvents();
}


void WeekView::setTotalWeeks( int numWeeks )
{
    header->spinWeek->setMaxValue( numWeeks );
}

int WeekView::totalWeeks() const
{
    return header->spinWeek->maxValue();
}

void WeekView::setStartsOnMonday( bool onMonday )
{
    bOnMonday = onMonday;
    contents->setStartOfWeek( bOnMonday );
    header->setStartOfWeek( bOnMonday );
    redraw();
}

// return the date at the beginning of the week...
QDate WeekView::weekDate() const
{
    return dateFromWeek( _week, year, bOnMonday );
}

