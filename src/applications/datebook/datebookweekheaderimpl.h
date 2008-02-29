/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef DATEBOOKDAYHEADER_H
#define DATEBOOKDAYHEADER_H
#include <qdatetime.h>
#include "datebookweekheader.h"


class WeekViewHeader : public QWidget, public Ui::DateBookWeekHeaderBase
{
    Q_OBJECT

public:
    WeekViewHeader( bool startOnMonday, QWidget* parent = 0,
                        const char* name = 0, Qt::WFlags fl = 0 );
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
    void keyPressAppointment(QKeyAppointment *e);

private:
    int year,
        week;
    bool bStartOnMonday;

};

QDate dateFromWeek( int week, int year, bool startOnMonday );

#endif // DATEBOOKDAYHEADER_H
