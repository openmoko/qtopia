/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "layoutmanager.h"

static const int min_height = 15;

LayoutItem::LayoutItem( const QOccurrence e ) : eappointment(e) { }

LayoutManager::LayoutManager(QWidget *parent)
    : QWidget(parent), maxWidth(-1), referenceDate(QDate::currentDate())
{
}

bool LayoutItemSorter::operator()(LayoutItem *d1, LayoutItem *d2)
{
    if (d1->occurrence().startInCurrentTZ() < d2->occurrence().startInCurrentTZ())
        return true;

    if (d2->occurrence().endInCurrentTZ() < d1->occurrence().endInCurrentTZ())
        return true;
    return false;
}

LayoutManager::~LayoutManager() { }

void LayoutManager::setDate(const QDate &d)
{
    if (d == referenceDate)
        return;

    referenceDate = d;
}

void LayoutManager::resizeEvent(QResizeEvent *)
{
    maxWidth = width();
    // with changed, re-init, re-layout items.
    for (int i = 0; i < mItems.count(); i++)
        initializeGeometry(mItems.at(i));
}

void LayoutManager::setOccurrences(QList<QOccurrence> &appointments)
{
    clear();

    QList<QOccurrence>::iterator it;
    for ( it = appointments.begin(); it != appointments.end(); ++it ) {
        addOccurrence(*it);
    }
    layoutItems(true);
}

void LayoutManager::addOccurrence(QOccurrence &appointment)
{
    LayoutItem *i = new LayoutItem(appointment);
    addItem(i);
    appointmentsChanged();
    initializeGeometry(i);
}

void LayoutManager::addItem(LayoutItem *i)
{
    mItems.append(i);
    LayoutItemSorter s;
    qSort(mItems.begin(), mItems.end(), s);
}

void LayoutManager::layoutItems(bool resetMaxWidth)
{
    int iCount = mItems.count();
    if ( width() < 1 || iCount == 0 )
        return;

    if (resetMaxWidth || maxWidth == -1)
        maxWidth = width();

    int itemWidth = qMin(width(), maxWidth);
    int n = 1;
    int groupStart = 0;
    int last_bottom = -1;

    if (iCount < (width()/4)) {
        int i = 0;
        while (i < iCount) {
            LayoutItem *item = mItems.at(i);
            int x = 0;
            int xp = 0;
            QRect geom = item->geometry();
            if ( geom.top() > last_bottom ) {
                groupStart = i;
                n = 1;
                itemWidth = qMin(width(), maxWidth);
            }
            int b = geom.bottom();
            if ( b > last_bottom )
                last_bottom = b;
            geom.setX( x );
            geom.setWidth(itemWidth);
            while ( xp < n && intersects(item, geom)) {
                x += itemWidth;
                xp++;
                geom.moveLeft(itemWidth);
            }
            if (xp >= n) {
                n++;
                itemWidth = qMin(width() / n, maxWidth);
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
        n = width() / itemWidth;
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
                geom.moveLeft(itemWidth);
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
        geom.moveLeft( 4 );
        if ( !intersects( item, geom, iCount ) ) {
            geom.moveLeft( -4 );
            int maxWidth = width();
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
        y = (y * (height() / 24)) / 60;
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
    geom.moveTopLeft(QPoint(1,1));
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
        yend = height();
    else
        yend = timeToHeight(item->occurrence().endInCurrentTZ().time());

    int h = yend - y;
    if (h < min_height)
        h = min_height;
    if (y + min_height > height()) {
        y = height() - min_height;
        h = min_height;
    }

    item->setGeometry(QRect(0, y, width(), h));
}

void LayoutManager::clear()
{
    qDeleteAll(mItems);
    mItems.clear();
}

