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
#ifndef DATEPICKER_H
#define DATEPICKER_H

#include <qvbox.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qtable.h>
#include <qpushbutton.h>

#include <qpe/timestring.h>

class QToolButton;
class QComboBox;
class QSpinBox;

class DatePickerHeader;
class DatePickerTable;

class DatePickerPrivate;
class DatePicker : public QVBox
{
    Q_OBJECT

public:
    DatePicker( QWidget *parent = 0, const char *name = 0, bool ac = FALSE );
    virtual ~DatePicker();
    QDate  selectedDate() const;

signals:
    void dateClicked( int year, int month, int day );

public slots:
    void setDate( int y, int m );
    //virtual void setDate( int y, int m );
    virtual void setDate( int y, int m, int d );
    void setDate( QDate );
    void slotWeekChange( bool );

protected slots:
    virtual void keyPressEvent(QKeyEvent *e);

protected:
    virtual void paintDay(const QDate &, QPainter *p, const QRect &cr, 
	    bool selected, const QColorGroup &cg);
    void repaintContents();

    bool weekStartsOnMonday() const;

private slots:
    void forwardDateClicked( int y, int m, int d ) { emit dateClicked(  y, m, d  ); }

    void calendarClicked(int, int);
    void calendarChanged(int, int);

private:
    friend class DatePickerTable;

    void paintCell(int, int, QPainter *p, const QRect &cr, 
	    bool selected, const QColorGroup &cg);

    DatePickerHeader *header;
    DatePickerTable *table;
    int year, month, day;
    bool autoClose;
    class DatePickerPrivate *d;
};


class QPEDateButton : public QPushButton
{
    Q_OBJECT

public:
    QPEDateButton( QWidget *parent, const char * name = 0 );
    QPEDateButton( bool longDate, QWidget *parent, const char * name = 0 );

    QDate date() const { return currDate; }
    void setLongFormat( bool l );
    bool longFormat() const { return longFmt; }

public slots:
    void setDate( int y, int m, int d );
    void setDate( QDate );
    void setWeekStartsMonday( int );
    void setDateFormat( DateFormat );

signals:
    void dateSelected( int year, int month, int day );

private:
    void init();

    bool longFmt;
    bool weekStartsMonday;
    QDate currDate;
    ::DateFormat df;
    DatePicker *monthView;
};


#endif
