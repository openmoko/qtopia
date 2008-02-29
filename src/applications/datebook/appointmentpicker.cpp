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

#include "appointmentpicker.h"
#include "datebook.h"
#include "dayview.h"
#include "monthview.h"

#include <qtopia/pim/qappointment.h>
#include <qtopiaapplication.h>


#include <qaction.h>
#include <qlayout.h>
#include <QStackedWidget>


AppointmentPicker::AppointmentPicker( DateBook *db, QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    datebook( db )
{
    setWindowTitle( tr("Appointment Picker") );
    setWindowIcon( QPixmap( ":image/AppointmentPicker" ) );

    QAction *actionNextView = new QAction( this );
    connect( actionNextView, SIGNAL( triggered() ), this, SLOT( nextView() ) );
    actionNextView->setShortcut(Qt::Key_Asterisk);

    views = new QStackedWidget( this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( views );

    dayView = 0;
    monthView = 0;

    lastToday = QDate::currentDate();
    viewMonth( lastToday );
}

AppointmentPicker::~AppointmentPicker()
{
}

void AppointmentPicker::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay( lastToday );
}

void AppointmentPicker::viewDay()
{
    viewDay( currentDate() );
}

void AppointmentPicker::viewDay(const QDate& dt)
{
    initDay();
    dayView->selectDate( dt );
    dayView->firstTimed();
    views->setCurrentIndex( views->indexOf( dayView ) );
}

void AppointmentPicker::viewMonth()
{
    viewMonth( currentDate() );
}

void AppointmentPicker::viewMonth( const QDate& dt)
{
    initMonth();
    monthView->setSelectedDate( dt );
    views->setCurrentIndex( views->indexOf( monthView ) );
}

bool AppointmentPicker::appointmentSelected() const
{
    if (views->currentWidget() && views->currentWidget() == dayView) {
        return dayView->currentIndex().isValid();
    }
    return false;
}

QAppointment AppointmentPicker::currentAppointment() const
{
    return dayView->currentAppointment();
}

QDate AppointmentPicker::currentDate()
{
    if ( dayView && views->currentWidget() == dayView ) {
        return dayView->currentDate();
    } else if ( monthView && views->currentWidget() == monthView ) {
        return monthView->selectedDate();
    } else {
        return QDate(); // invalid;
    }
}

void AppointmentPicker::initDay()
{
    if ( !dayView ) {
        dayView = new DayView;
        views->addWidget( dayView );
        dayView->setDaySpan( datebook->startTime, 17 );

        connect( dayView, SIGNAL(showDetails()),
                this, SLOT(accept()) );
    }
}

void AppointmentPicker::initMonth()
{
    if ( !monthView ) {
        monthView = new MonthView;
        monthView->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
        // TODO monthView->setMargin(0);
        views->addWidget( monthView );
        connect( monthView, SIGNAL( activated(const QDate&) ),
             this, SLOT( viewDay(const QDate&) ) );
    }
}

void AppointmentPicker::nextView()
{
    QWidget* cur = views->currentWidget();
    if ( cur ) {
        if ( cur == dayView )
            viewMonth();
        else if ( cur == monthView )
            viewDay();
    }
}

