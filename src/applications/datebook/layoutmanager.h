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

#include <qvector.h>
#include <qvaluelist.h>
#include <qrect.h>
#include <qdatetime.h>
#include <qpe/pim/event.h>
#include "datebookdb.h"

class LayoutItem
{
public:
    LayoutItem( const Occurrence e );

    void setGeometry(const QRect &rect) { r = rect; }
    void setGeometry(int x, int y, int w, int h) 
    { setGeometry(QRect(x,y,w,h)); }
    QRect geometry() const { return r; }

    Occurrence occurance() const { return eevent; }
    PimEvent event() const { return eevent.event(); }

private:
    Occurrence eevent;
    QRect r;
};

class SortedLayoutItems : public QVector<LayoutItem>
{
public:
    SortedLayoutItems() : QVector<LayoutItem>() { }
    int compareItems( Item d1, Item d2 );
};

class LayoutManager
{
public:
    LayoutManager(int w, int h);
    virtual ~LayoutManager();


    virtual void setDate(const QDate &d);
    QDate date() const { return referenceDate; }

    void setSize(int w, int h);
    void setMaximumColumnWidth(int x) { maxWidth = x; };
    int maximumColumnWidth() const { return maxWidth; };
    void setOccurrences(QValueList<Occurrence> &events);
    virtual void addOccurrence(Occurrence &event);

    virtual void clear() { mItems.clear(); }

    SortedLayoutItems items() const { return mItems; }
    QSize size() const { return QSize(width, height); }
    int count() const { return mItems.count(); }

    virtual void layoutItems(bool resetMaxWidth = FALSE);

    virtual int timeToHeight(const QTime &) const;
    virtual QTime heightToTime(int) const;

protected:
    void initializeGeometry(LayoutItem *);
    LayoutItem *intersects(LayoutItem *, QRect) const;
    void addItem(LayoutItem *);

private:
    SortedLayoutItems mItems;
    int width;
    int height;
    int maxWidth;
    QDate referenceDate;
};
