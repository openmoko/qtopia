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
#ifndef DATEBOOKDAY_H
#define DATEBOOKDAY_H

#include "periodview.h"
#include <qtopia/pim/event.h>

#include <qdatetime.h>
#include <qlabel.h>
#include <qtable.h>
#include <qvbox.h>
#include <qlist.h>

// Bounds check access to the hourInfo array
//#define HOURINFO_INDEX_CHECK

#if defined (QTOPIA_NO_POINTER_INPUT) || defined(QTOPIA_PHONE)
#define USE_LABEL_HEADER
#endif

class DayViewHeader;
class DateBookTable;
class QDateTime;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QTimer;
class DayView;
class DayViewLayout;
class DayItem;
class Occurrence;
class LayoutItem;

// This can be 3 different views depending on how it's constructed
class DayViewContents : public QScrollView
{
    Q_OBJECT
public:
    enum Type {
	AllDay,
	ScrollingDay,
	CompressedDay
    };

    DayViewContents( Type viewType,
	    QWidget *parent = 0, const char *name = 0 );

    ~DayViewContents();

    bool whichClock() const;

    void addOccurrence(Occurrence &, const QDate &);
    void takeOccurrence(Occurrence &);
    void clearItems();

    void layoutItems();

    int startHour() const { return QMIN(startSel, endSel); }
    int endHour() const { return QMAX(startSel, endSel) + 1; }
    void clearSelectedTimes();

    DayItem *firstItem( bool show=TRUE );
    DayItem *lastItem( bool show=TRUE );
    DayItem *nextItem( bool show=TRUE );
    DayItem *previousItem( bool show=TRUE );
    DayItem *currentItem() const;
    void setCurrentItem(DayItem *, bool show=TRUE);
    void moveSelection( DayItem *, DayItem * );
    void setMetrics( const QFontMetrics &);
    void setType( Type viewType );

public slots:
    void startAtTime(int);
    void moveUp();  // scrolling with keys from DayView
    void moveDown();
    void selectDate(const QDate &);

signals:
    void sigColWidthChanged();
    void keyPressed( const QString &txt );

    void removeOccurrence( const Occurrence& );
    void editOccurrence( const Occurrence& );
    void removeEvent( const PimEvent& );
    void editEvent( const PimEvent& );
    void beamEvent( const PimEvent& );
    void showDetails();
    void setCurrentEvent( const PimEvent &ev );

protected:
    void resizeEvent( QResizeEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void drawContents( QPainter * p, int x, int y, int w, int h );

    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );
    void contentsMouseMoveEvent( QMouseEvent * );

private slots:
    void makeVisible();
    void timeStringChanged();

private:
    QFont dayViewFont() const;
    void newItemList();

    Type typ;

    DayViewLayout *itemList;
    int hourAtPos(int) const;
    int posOfHour(int) const;

    int startSel, endSel;
    bool dragging;

    int start_of_day;
    int time_width;

    int time_height;
    QRect visRect;

    QTimer *visibleTimer;

    bool dataChanged;

    bool clicking;
    LayoutItem *mClickedItem;
};


class DayView : public PeriodView
{
    Q_OBJECT

public:
    DayView( DateBookTable *newDb, bool startOnMonday,
		 QWidget *parent, const char *name );

    void selectedDates( QDateTime &start, QDateTime &end );
    void clearSelectedDates();
    
    //int startViewTime() const;

    bool hasSelection() const;
    PimEvent currentEvent() const;
    void setCurrentItem(const Occurrence &);
    Occurrence currentItem() const;
    void setCompressDay( bool );

public slots:
    //void setDate( int y, int m, int d );
    void selectDate( const QDate & );
    void redraw();
    void setStartOnMonday( bool bStartOnMonday );
    void setDayStarts( int startHere );
    void previousEvent();
    void nextEvent();
    void setCurrentEvent(const PimEvent &);

signals:
    void removeOccurrence( const Occurrence& );
    void editOccurrence( const Occurrence& );
    void removeEvent( const PimEvent& );
    void editEvent( const PimEvent& );
    void beamEvent( const PimEvent& );
    void newEvent();
    void newEvent( const QString & );
    void showDetails();

protected slots:
    void keyPressEvent(QKeyEvent *);

private slots:
    void slotColWidthChanged() { relayoutPage(); };

private:
    void selectDate( int, int );
    void fontChange( const QFont &);
    void getEvents();
    void relayoutPage( bool fromResize = FALSE );
#ifdef USE_LABEL_HEADER
    QString header1Text(const QDate &d) const;
    QString header2Text(const QDate &d) const;
#endif

    DayViewContents *view;
    DayViewContents *allView;
#ifdef USE_LABEL_HEADER
    QHBox *header;
    QLabel *header1;
    QLabel *header2;
#else 
    DayViewHeader *header;
#endif
    int viewWithFocus;
};

struct HourInfoNode {
    bool stretch;
    bool empty;
    int hits;
};

#ifdef HOURINFO_INDEX_CHECK
class HourInfo {
public:
    const HourInfoNode &operator[]( int index ) const {
	ASSERT( index >= 0 && index < 25 );
	return hourInfo[index];
    }
    HourInfoNode &operator[]( int index ) {
	ASSERT( index >= 0 && index < 25 );
	return hourInfo[index];
    }
    HourInfoNode hourInfo[25];
};
#endif

typedef QValueList<QTime> TimeVals;
class LayoutItem;
class TimeList
{
public:
    TimeList();

    void clear();
    void add( const QDate &, LayoutItem * );
    void addDone();

    int height( const QTime & ) const;
    int height( const int ) const;
    int scale( const int, const int ) const;
    int hits( const int index ) const { return hourInfo[index].hits; }
    void resize( int, int );
    QTime time( int, int = 1 ) const;
    void setTimeHeight( int );

private:
    void add( const QTime & );
    void hit( int );

    TimeVals list;
#ifdef HOURINFO_INDEX_CHECK
    HourInfo hourInfo;
#else
    HourInfoNode hourInfo[25];
#endif
    int height24;
    bool infoModified;
    int time_height;
};


#endif
