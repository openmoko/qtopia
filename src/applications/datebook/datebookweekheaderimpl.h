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
#ifndef DATEBOOKDAYHEADER_H
#define DATEBOOKDAYHEADER_H
#include <qdatetime.h>
#include "datebookweekheader.h"


class WeekViewHeader : public DateBookWeekHeaderBase
{
    Q_OBJECT

public:
    WeekViewHeader( bool startOnMonday, QWidget* parent = 0,
			const char* name = 0, WFlags fl = 0 );
    ~WeekViewHeader();

    void setDate( int y, int w );
    void setStartOfWeek( bool onMonday );

signals:
    void dateChanged( int y, int w );

public slots:
    void yearChanged( int );
    void nextWeek();
    void prevWeek();
    void weekChanged( int );

protected slots:
    void timeStringChanged();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    int year,
	week;
    bool bStartOnMonday;

};

QDate dateFromWeek( int week, int year, bool startOnMonday );

#endif // DATEBOOKDAYHEADER_H
