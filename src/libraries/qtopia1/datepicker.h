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
