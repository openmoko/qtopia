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

#include "layoutmanager.h"

static const int min_height = 15;

LayoutItem::LayoutItem( const Occurrence e ) : eevent(e) { }

LayoutManager::LayoutManager(int w, int h) 
    : width(w), height(h), maxWidth(w), referenceDate(QDate::currentDate())
{
    mItems.setAutoDelete(TRUE);
}

int SortedLayoutItems::compareItems(Item i1, Item i2) 
{
    LayoutItem *d1 = (LayoutItem *)i1;
    LayoutItem *d2 = (LayoutItem *)i2;

    if (d1->occurrence().startInCurrentTZ() < d2->occurrence().startInCurrentTZ())
	return -1;
    if (d2->occurrence().startInCurrentTZ() < d1->occurrence().startInCurrentTZ())
	return 1;
    //return 0;

    if (d1->occurrence().endInCurrentTZ() < d2->occurrence().endInCurrentTZ())
	return 1;
    if (d2->occurrence().endInCurrentTZ() < d1->occurrence().endInCurrentTZ())
	return -1;
    return 0;
}

LayoutManager::~LayoutManager() { }

void LayoutManager::setDate(const QDate &d) 
{
    if (d == referenceDate)
	return;

    referenceDate = d;
}

void LayoutManager::setSize(int w, int h) 
{
    height = h;
    maxWidth = width = w;
    // with changed, re-init, re-layout items.
    for (uint i = 0; i < mItems.count(); i++)
	initializeGeometry(mItems.at(i));
}

void LayoutManager::setOccurrences(QValueList<Occurrence> &events)
{
    mItems.clear();

    QValueListIterator<Occurrence> it;
    for ( it = events.begin(); it != events.end(); ++it ) {
	addOccurrence(*it);
    }
    layoutItems(TRUE);
}

void LayoutManager::addOccurrence(Occurrence &event)
{
    LayoutItem *i = new LayoutItem(event);
    addItem(i);
    eventsChanged();
    initializeGeometry(i);
}

void LayoutManager::addItem(LayoutItem *i)
{
    mItems.resize(mItems.size() + 1);
    mItems.insert(mItems.size() - 1, i);
    mItems.sort();
}

void LayoutManager::layoutItems(bool resetMaxWidth)
{
    if (resetMaxWidth)
	maxWidth = width;

    int iCount = mItems.count();
    if ( width < 1 || iCount == 0 ) {
	return;
    }
    int itemWidth = QMIN(width, maxWidth);
    int n = 1;
    int groupStart = 0;
    int last_bottom = -1;

    if (iCount < (width/4)) {
	int i = 0;
	while (i < iCount) {
	    LayoutItem *item = mItems.at(i);
	    int x = 0;
	    int xp = 0;
	    QRect geom = item->geometry();
	    if ( geom.top() > last_bottom ) {
		groupStart = i;
		n = 1;
		itemWidth = QMIN(width, maxWidth);
	    }
	    int b = geom.bottom();
	    if ( b > last_bottom )
		last_bottom = b;
	    geom.setX( x );
	    geom.setWidth(itemWidth);
	    while ( xp < n && intersects(item, geom)) {
		x += itemWidth;
		xp++;
		geom.moveBy(itemWidth, 0);
	    }
	    if (xp >= n) {
		n++;
		itemWidth = QMIN(width / n, maxWidth);
		i = groupStart; // Start again.
	    } else {
		item->setGeometry( geom );
		i++;
	    }
	}
    } else {
	// alturnate algorithm. // same as above, but just overlap
	// if fail.
	itemWidth = 4;
	n = width / itemWidth;
	int i = 0;
	int rovingXp = 0;
	while (i < iCount) {
	    LayoutItem *item = mItems.at(i);
	    int x = 0;
	    int xp = 0;
	    QRect geom = item->geometry();
	    geom.setX( x );
	    geom.setWidth(itemWidth);
	    while ( xp < n && intersects(item, geom)) {
		x += itemWidth;
		xp++;
		geom.moveBy(itemWidth, 0);
	    }
	    if (xp >= n) {
		geom.setX(rovingXp * itemWidth);
		geom.setWidth(itemWidth);
		if ( rovingXp++ == xp - 1 )
                    rovingXp = 0;
		item->setGeometry( geom );
	    } else {
		item->setGeometry( geom );
	    }
	    i++;
	}
    }
    if (itemWidth < maxWidth)
	maxWidth = itemWidth;

    // expand items that can be wider
    for ( int i = 0; i < iCount; i++ ) {
	LayoutItem *item = mItems.at( i );
	QRect geom = item->geometry();
	geom.moveBy( 4, 0 );
	if ( !intersects( item, geom, iCount ) ) {
	    geom.moveBy( -4, 0 );
	    int maxWidth = width;
	    geom.setWidth( maxWidth - geom.x() );
	    LayoutItem *other = intersects( item, geom, iCount );
	    if ( other ) {
		maxWidth = other->geometry().x();
	    }
	    geom.setWidth( maxWidth - geom.x() );
	    item->setGeometry( geom );
	}
    }
}

int LayoutManager::timeToHeight( const QTime &time ) const
{
    int y = time.hour() * 60 + time.minute();
    if (y)
	y = (y * (height / 24)) / 60;
    return y;
}

QTime LayoutManager::heightToTime( int ) const
{
    // broken
    return QTime(0,0,0);
}

LayoutItem *LayoutManager::intersects(LayoutItem *item, QRect geom, int maxIndex, int *startIndex) const
{
    // allow overlapping
    geom.moveBy(1,1);
    geom.setSize( geom.size() - QSize(2,2) );

    int count = mItems.count();
    if ( maxIndex && maxIndex < count )
	count = maxIndex;
    for ( int i = startIndex ? *startIndex : 0; i < count; i++ ) {
	if ( startIndex ) *startIndex = i;
	LayoutItem *it = mItems.at(i);
	if ( it == item )
	    if ( maxIndex ) continue;
	    else break;
	if (it->geometry().intersects( geom ) )
	    return it;
    }
    return 0;
}

void LayoutManager::initializeGeometry(LayoutItem *item)
{
    int y, yend;
    if (referenceDate > item->occurrence().startInCurrentTZ().date())
	y = 0;
    else 
	y = timeToHeight(item->occurrence().startInCurrentTZ().time());

    if (referenceDate < 
	    item->occurrence().startInCurrentTZ().date().addDays( 
		item->occurrence().startInCurrentTZ().date().daysTo(item->occurrence().endInCurrentTZ().date())
		)
	    )
	yend = height;
    else
	yend = timeToHeight(item->occurrence().endInCurrentTZ().time());

    int h = yend - y;
    if (h < min_height)
	h = min_height;
    if (y + min_height > height) {
	y = height - min_height;
	h = min_height;
    }

    item->setGeometry(QRect(0, y, width, h));
}
