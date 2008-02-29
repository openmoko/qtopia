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
    WeekView( DateBookTable *, bool ampm, bool weekOnMonday,
		  QWidget *parent = 0, const char *name = 0 );

    bool hasSelection() const { return FALSE; }
    PimEvent currentEvent() const { return PimEvent(); }
    bool calcWeek( const QDate &d, int &week, int &year ) const;

    QDate weekDate() const;

public slots:
    void selectDate( const QDate & );

    void setTwelveHour( bool );
    void setStartsOnMonday( bool );
    void setDayStarts( int h );

    void databaseUpdated();


public slots:
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

#endif
