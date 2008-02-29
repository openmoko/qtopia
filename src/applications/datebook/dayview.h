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
#ifndef DATEBOOKDAY_H
#define DATEBOOKDAY_H

#include "periodview.h"
#include <qtopia/pim/event.h>

#include <qdatetime.h>
#include <qtable.h>
#include <qvbox.h>
#include <qlist.h>

class DayViewHeader;
class DateBookTable;
class QDateTime;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QTimer;
class DayView;
class DayViewContents;

class DayViewLayout;
class DayItem;

class Occurrence;

// Is both the 'all day' and '9-10' view.  depending on how its constructed.
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

public slots:
    void startAtTime(int);
    void moveUp();  // scrolling with keys from DayView
    void moveDown();
    void selectDate(const QDate &);
    void showStartTime();

signals:
    void sigColWidthChanged();
    void keyPressed( const QString &txt );

    void removeOccurrence( const Occurrence& );
    void editOccurrence( const Occurrence& );
    void removeEvent( const PimEvent& );
    void editEvent( const PimEvent& );
    void beamEvent( const PimEvent& );

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

    Type typ;

    DayViewLayout *itemList;
    int hourAtPos(int) const;
    int posOfHour(int) const;
    //QList<DayItem> itemList;
    //DayViewWidget *intersects( const DayViewWidget *item, const QRect &geom );

    int startSel, endSel;
    bool dragging;

    int start_of_day;
    int time_width;

    int time_height;
    QRect visRect;

    QTimer *visibleTimer;
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
    void setCurrentEvent(const PimEvent &);
    void setCurrentItem(const Occurrence &);
    Occurrence currentItem() const;

public slots:
    //void setDate( int y, int m, int d );
    void selectDate( const QDate & );
    void redraw();
    void setStartOnMonday( bool bStartOnMonday );
    void setDayStarts( int startHere );

signals:
    void removeOccurrence( const Occurrence& );
    void editOccurrence( const Occurrence& );
    void removeEvent( const PimEvent& );
    void editEvent( const PimEvent& );
    void beamEvent( const PimEvent& );
    void newEvent();
    void newEvent( const QString & );

protected slots:
    void keyPressEvent(QKeyEvent *);

private slots:
    void slotColWidthChanged() { relayoutPage(); };

private:
    void fontChange( const QFont &);
    void getEvents();
    void relayoutPage( bool fromResize = false );
    DayViewContents *view;
    DayViewContents *allView;
    DayViewHeader *header;
    int viewWithFocus;
};

#endif
