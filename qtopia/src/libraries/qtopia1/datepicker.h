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
#ifndef DATEPICKER_H
#define DATEPICKER_H

#include <qvbox.h>
#include <qhbox.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qtable.h>
#include <qpushbutton.h>

#include <qtopia/timestring.h>

class QToolButton;
class QComboBox;
class QSpinBox;

class DatePickerHeader;
class DatePickerTable;

class QPEDatePickerPrivate;
class QTOPIA_EXPORT QPEDatePicker : public QVBox
{
    Q_OBJECT

public:
    QPEDatePicker( QWidget *parent = 0, const char *name = 0);
    virtual ~QPEDatePicker();
    QDate  selectedDate() const;

    bool weekStartsMonday() const;

signals:
    void dateClicked( const QDate &);

public slots:
    void setDate( int y, int m );
    //virtual void setDate( int y, int m );
    void setDate( int y, int m, int d );
    virtual void setDate( const QDate & );

    void setWeekStartsMonday( bool );

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void paintDay(const QDate &, QPainter *p, const QRect &cr,
	    const QColorGroup &cg);

    virtual void paintDayBackground(const QDate &, QPainter *p, const QRect &cr,
	    const QColorGroup &cg);

    // not so sure about this....
    void updateContents();

private slots:
    void calendarClicked(int, int);
    void calendarChanged(int, int);

private:
    friend class DatePickerTable;

    void paintCell(int, int, QPainter *p, const QRect &cr,
	    bool selected, const QColorGroup &cg);

    DatePickerHeader *header;
    DatePickerTable *table;
    int year, month, day;
    class DatePickerPrivate *d;
};

#endif
