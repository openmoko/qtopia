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
#ifndef DATEBOOKWEEK
#define DATEBOOKWEEK

#include "periodview.h"

#include <qlist.h>
#include <qvaluelist.h>
#include <qvector.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qvaluelist.h>
#include "layoutmanager.h"

class QLabel;
class QResizeEvent;
class QSpinBox;
class QTimer;
class QHeader;

class WeekViewHeader;
class WeekViewContents;

class WeekView : public PeriodView
{
    Q_OBJECT

public:
    WeekView( DateBookTable *, bool weekOnMonday,
		  QWidget *parent = 0, const char *name = 0 );

    bool hasSelection() const { return FALSE; }
    PimEvent currentEvent() const { return PimEvent(); }
    bool calcWeek( const QDate &d, int &week, int &year ) const;

    QDate weekDate() const;

public slots:
    void selectDate( const QDate & );

    void setStartsOnMonday( bool );
    void setDayStarts( int h );

    void databaseUpdated();

    void redraw();

protected slots:
    void keyPressEvent(QKeyEvent *);

private slots:
    void dayActivated( int day );

    void setWeek( int y, int w );
    void setYear( int );
    void showEventsLabel( QValueList<Occurrence> & );
    void hideEventsLabel();

private:
    void setTotalWeeks( int totalWeeks );
    int totalWeeks() const;

    void getEvents();
    int year;
    int _week;
    WeekViewHeader *header;
    WeekViewContents *contents;
    QLabel *lblDesc;
    QTimer *tHide;
};

class WeekViewContents : public QScrollView
{
    Q_OBJECT
public:
    WeekViewContents( WeekView *parent = 0, const char *name = 0 );

    void showEvents( QValueList<Occurrence> &ev , const QDate &startDate );
    void moveToHour( int h );
    void setStartOfWeek( bool bOnMonday );

    void alterDay( int );

signals:
    void activateWeekDay( int d );
    void eventsSelected( QValueList<Occurrence> & );
    void selectionCleared();

protected slots:
    void keyPressEvent(QKeyEvent *);
    void timeStringChanged();

private:
    void positionItem( LayoutItem *i );
    LayoutItem *intersects( const LayoutItem * );
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    void drawContents( QPainter *p);
    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void resizeEvent( QResizeEvent * );
    void updateWeekNames(bool wsom);

private:
    int posOfHour(int h) const;
    int hourAtPos(int p) const;
    QHeader *header;
    QVector<LayoutManager> items;
    QList<LayoutItem> dayItems;
    int rowHeight;
    bool showingEvent;
    WeekView *wv;
};


#endif
