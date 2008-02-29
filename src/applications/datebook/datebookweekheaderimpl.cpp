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
#include "datebookweekheaderimpl.h"
#include <qtimestring.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qdatetime.h>

/*
 *  Constructs a DateBookWeekHeader which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
WeekViewHeader::WeekViewHeader( bool startOnMonday, QWidget* parent,
                                        Qt::WFlags fl )
    : QWidget( parent, fl ),
      bStartOnMonday( startOnMonday )
{
    setupWidget( this );
    setBackgroundMode( PaletteButton );
    labelDate->setBackgroundMode( PaletteButton );
    connect(qApp, SIGNAL(dateFormatChanged()), this, SLOT(timeStringChanged()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
WeekViewHeader::~WeekViewHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void  WeekViewHeader::keyPressAppointment(QKeyAppointment *e)
{
    e->ignore();
}

/*
 * public slot
 */
void WeekViewHeader::yearChanged( int y )
{
    setDate( y, week );
    emit dateChanged( year, week );
}
/*
 * public slot
 */
void WeekViewHeader::nextWeek()
{
    weekChanged(week+1);
}
/*
 * public slot
 */
void WeekViewHeader::prevWeek()
{
    weekChanged(week-1);
}
/*
 * public slot
 */
void WeekViewHeader::weekChanged( int w )
{
    setDate( year, w );
    emit dateChanged( year, week );
}

void WeekViewHeader::setDate( int y, int w )
{
    year = y;
    week = w;
    spinYear->setValue( y );
    spinWeek->setValue( w );

    QDate d = dateFromWeek( week, year, bStartOnMonday );
    QDate dend = d.addDays( 6 );

    QString s = tr("%1-%2","2 dates")
        .arg(QTimeString::localMD(d))
        .arg(QTimeString::localMD(dend));

    labelDate->setText( s );
}

void WeekViewHeader::setStartOfWeek( bool onMonday )
{
    bStartOnMonday = onMonday;
    setDate( year, week );
}

void WeekViewHeader::timeStringChanged()
{
    setDate( year, week );
}

// dateFromWeek
// compute the date from the week in the year

QDate dateFromWeek( int week, int year, bool startOnMonday )
{
    QDate d;
    d.setYMD( year, 1, 1 );
    int dayOfWeek = d.dayOfWeek();
    if ( startOnMonday ) {
        if ( dayOfWeek <= 4 ) {
            d = d.addDays( ( week - 1 ) * 7 - dayOfWeek + 1 );
        } else {
            d = d.addDays( (week) * 7 - dayOfWeek + 1 );
        }
    } else {
        if ( dayOfWeek <= 4 || dayOfWeek == 7) {
            d = d.addDays( ( week - 1 ) * 7 - dayOfWeek % 7 );
        } else {
            d = d.addDays( ( week ) * 7 - dayOfWeek % 7 );
        }
    }
    return d;
}

