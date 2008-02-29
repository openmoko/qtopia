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

    if (d1->event().startInCurrentTZ() < d2->event().startInCurrentTZ())
	return -1;
    if (d2->event().startInCurrentTZ() < d1->event().startInCurrentTZ())
	return 1;
    return 0;

    if (d1->event().endInCurrentTZ() < d2->event().endInCurrentTZ())
	return -1;
    if (d2->event().endInCurrentTZ() < d1->event().endInCurrentTZ())
	return 1;
    return 0;
}

LayoutManager::~LayoutManager() { }

void LayoutManager::setDate(const QDate &d) 
{
    if (d == referenceDate)
	return;

    referenceDate = d;

    for (uint i = 0; i < mItems.count(); i++) 
	initializeGeometry(mItems.at(i));
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
    initializeGeometry(i);
    addItem(i);
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
    int itemWidth = QMIN(width, maxWidth);
    int n = 1;

    if (width < 1)
	return;
    if (iCount < (width/4)) {
	int i = 0;
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
		n++;
		itemWidth = QMIN(width / n, maxWidth);
		i = 0; // Start again.
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
		rovingXp++;
		item->setGeometry( geom );
	    } else {
		item->setGeometry( geom );
	    }
	    i++;
	}
    }
    if (itemWidth < maxWidth)
	maxWidth = itemWidth;
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

LayoutItem *LayoutManager::intersects(LayoutItem *item, QRect geom) const
{
    int i = 0;
    // allow overlapping
    geom.moveBy(1,1);
    geom.setSize( geom.size() - QSize(2,2) );

    LayoutItem *it = mItems.at(i);
    int count = mItems.count();
    while (i < count && it != item) {
	if (it->geometry().intersects( geom ) )
	    return it;
	it = mItems.at(++i);
    }
    return 0;
}

void LayoutManager::initializeGeometry(LayoutItem *item)
{
    int y, yend;
    if (referenceDate > item->occurrence().startInCurrentTZ().date())
	y = 0;
    else 
	y = timeToHeight(item->event().startInCurrentTZ().time());

    if (referenceDate < 
	    item->occurrence().startInCurrentTZ().date().addDays( 
		item->event().startInCurrentTZ().date().daysTo(item->event().endInCurrentTZ().date())
		)
	    )
	yend = timeToHeight(QTime(23,59,59));
    else
	yend = timeToHeight(item->event().endInCurrentTZ().time());

    int h = yend - y;
    if (h < min_height)
	h = min_height;
    if (y + min_height > height) {
	y = height - min_height;
	h = min_height;
    }

    item->setGeometry(QRect(0, y, width, h));
}
