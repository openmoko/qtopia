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
#ifndef DATEBOOKMONTH
#define DATEBOOKMONTH

#include <qtopia/qpeglobal.h>

#include <qtopia/private/event.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qtable.h>
#include <qpushbutton.h>
#include "calendar.h"
#include "timestring.h"

class QToolButton;
class QComboBox;
class QSpinBox;
class Event;
class EffectiveEvent;
class DateBookDB;


class DateBookMonthHeaderPrivate;
class QTOPIA_EXPORT DateBookMonthHeader : public QHBox
{
    Q_OBJECT

public:
    DateBookMonthHeader( QWidget *parent = 0, const char *name = 0 );
    ~DateBookMonthHeader();
    void setDate( int year, int month );

signals:
    void dateChanged( int year, int month );

protected slots:
    void keyPressEvent(QKeyEvent *e ) {
	e->ignore();
    }

private slots:
    void updateDate();
    void firstMonth();
    void lastMonth();
    void monthBack();
    void monthForward();

private:
    QToolButton *begin, *back, *next, *end;
    QComboBox *month;
    QSpinBox *year;
    DateBookMonthHeaderPrivate *d;
    int focus;
};

class DayItemMonthPrivate;
class QTOPIA_EXPORT DayItemMonth : public QTableItem
{
public:
    DayItemMonth( QTable *table, EditType et, const QString &t );
    ~DayItemMonth();
    void paint( QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected );
    void setDay( int d ) { dy = d; }
    void setEvents( const QValueList<Event> &events ) { daysEvents = events; };
    void setEvents( const QValueList<EffectiveEvent> &effEvents );
    void clearEvents() { daysEvents.clear(); };
    void clearEffEvents();
    int day() const { return dy; }
    void setType( Calendar::Day::Type t );
    Calendar::Day::Type type() const { return typ; }

private:
    QBrush back;
    QColor forg;
    int dy;
    Calendar::Day::Type typ;
    QValueList<Event> daysEvents; // not used anymore...
    DayItemMonthPrivate *d;
};

class DateBookMonthTablePrivate;
class QTOPIA_EXPORT DateBookMonthTable : public QTable
{
    Q_OBJECT

public:
    DateBookMonthTable( QWidget *parent = 0, const char *name = 0,
                        DateBookDB *newDb = 0 );
    ~DateBookMonthTable();
    void setDate( int y, int m, int d );
    void redraw();

    QSize minimumSizeHint() const { return sizeHint(); }
    QSize minimumSize() const { return sizeHint(); }
    void  getDate( int& y, int &m, int &d ) const {y=selYear;m=selMonth;d=selDay;}
    void setWeekStart( bool onMonday );
signals:
    void dateClicked( int year, int month, int day );

protected:
    void viewportMouseReleaseEvent( QMouseEvent * );

protected slots:

    void keyPressEvent(QKeyEvent *e ) {
	e->ignore();
    }

private slots:
    void dayClicked( int row, int col );
    void dragDay( int row, int col );

private:
    void setupTable();
    void setupLabels();

    void findDay( int day, int &row, int &col );
    void getEvents();
    void changeDaySelection( int row, int col );

    int year, month, day;
    int selYear, selMonth, selDay;
    QValueList<Event> monthsEvents; // not used anymore...
    DateBookDB *db;
    DateBookMonthTablePrivate *d;
};

class DateBookMonthPrivate;
class QTOPIA_EXPORT DateBookMonth : public QVBox
{
    Q_OBJECT

public:
    DateBookMonth( QWidget *parent = 0, const char *name = 0, bool ac = FALSE,
                   DateBookDB *data = 0 );
    ~DateBookMonth();
    QDate  selectedDate() const;

signals:
    void dateClicked( int year, int month, int day );

public slots:
    void setDate( int y, int m );
    void setDate( int y, int m, int d );
    void setDate( QDate );
    void redraw();
    void slotWeekChange( bool );

protected slots:
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void forwardDateClicked( int y, int m, int d ) { emit dateClicked(  y, m, d  ); }
    void finalDate(int, int, int);

private:
    DateBookMonthHeader *header;
    DateBookMonthTable *table;
    int year, month, day;
    bool autoClose;
    class DateBookMonthPrivate *d;
};

class QTOPIA_EXPORT DateButton : public QPushButton
{
    Q_OBJECT

public:
    DateButton( bool longDate, QWidget *parent, const char * name = 0 );
    QDate date() const { return currDate; }

    bool customWhatsThis() const;

signals:
    void dateSelected( int year, int month, int day );

public slots:
    void setDate( int y, int m, int d );
    void setDate( QDate );
    void setWeekStartsMonday( int );
    void setDateFormat( DateFormat );

private slots:
    void pickDate();
    void gotHide();

private:
    bool longFormat;
    bool weekStartsMonday;
    QDate currDate;
    DateFormat df;
};


#endif
