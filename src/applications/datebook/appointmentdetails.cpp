/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "appointmentdetails.h"

#include <qtopianamespace.h>
#include <qtimestring.h>
#include <qtopia/pim/qappointment.h>

#include <QDL>
#include <QDLBrowserClient>

#include <qglobal.h>
#include <QKeyEvent>

AppointmentDetails::AppointmentDetails( QWidget *parent )
:   QDLBrowserClient( parent, "editnote" ),
    previousDetails( 0 )
{
#ifdef QTOPIA_PHONE
    setFrameStyle(NoFrame);
#endif
}

void AppointmentDetails::init( const QOccurrence &occurrence )
{
    mOccurrence = occurrence;
    QAppointment ev = occurrence.appointment();
    QString text = createPreviewText( ev );
    if ( text != previousText ) {
        setHtml( text );
        verifyLinks();
    }
    previousText = text;
}

void AppointmentDetails::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
        case Qt::Key_Space:
        case Qt::Key_Return:
            // PHONE back is handled in DateBook::closeEvent()
            emit done();
            break;
        default:
            QTextBrowser::keyPressEvent( e );
            break;
    }
}

QString AppointmentDetails::createPreviewText( const QAppointment &ev )
{
    QString text;

    loadLinks( ev.customField( QDL::CLIENT_DATA_KEY ) );

    text = "<center><b>" + Qt::escape(ev.description()) + "</b></center><br>";
    if ( !ev.location().isEmpty() )
        text += "<b>" + tr("Location: ") + "</b>" + ev.location() + "<br>";
    if ( !ev.isAllDay() ) {
        text += "<b>" + tr("Start: ") + "</b>" +
                QTimeString::localYMD( ev.startInCurrentTZ().date() ) + " " +
                QTimeString::localHM( ev.startInCurrentTZ().time() ) + "<br>";
        text += "<b>" + tr("End: ") + "</b>" +
                QTimeString::localYMD( ev.endInCurrentTZ().date() ) + " " +
                QTimeString::localHM( ev.endInCurrentTZ().time() ) + "<br>";
    }
    if ( ev.hasAlarm() )
        text += "<b>" + tr("Alarm: ") + "</b>" +
            QString("%1").arg( ev.alarmDelay() ) + " " + tr("minutes") + " (" +
            tr(ev.alarm() == QAppointment::Visible ? "Silent" : "Audible") +
            ")" + "<br>";
    if ( ev.hasRepeat() ) {
        QString word;
        if ( ev.repeatRule() == QAppointment::Daily )
            if ( ev.frequency() > 1 )
                word = tr("every %1 days", "eg. every 2 days");
            else
                word = tr("every day");
        else if ( ev.repeatRule() == QAppointment::Weekly )
            if ( ev.frequency() > 1 )
                word = tr("every %1 weeks", "eg. every 2 weeks");
            else
                word = tr("every week");
        else if ( ev.repeatRule() == QAppointment::MonthlyDate ||
                  ev.repeatRule() == QAppointment::MonthlyDay ||
                  ev.repeatRule() == QAppointment::MonthlyEndDay )
            if ( ev.frequency() > 1 )
                word = tr("every %1 months", "eg. every 2 months");
            else
                word = tr("every month");
        else if ( ev.repeatRule() == QAppointment::Yearly )
            if ( ev.frequency() > 1 )
                word = tr("every %1 years", "eg. every 2 years");
            else
                word = tr("every year");
        text += "<b>" + tr("Repeat: ") + "</b>";
        if ( ev.frequency() > 1 )
            text += word.arg( ev.frequency() );
        else
            text += word;
        if ( !ev.repeatForever() )
            text += tr(" ending on %1", "eg. repeat every week ending on 21 june").arg( QTimeString::localYMD(ev.repeatUntil()) );
        text += "<br>";
    }
    if ( !ev.notes().isEmpty() )
    {
        QString txt = ev.notes();
        text += "<b>" + tr("Notes: ") + "</b>" +
             txt + "<br>";
    }
    return text;
}

