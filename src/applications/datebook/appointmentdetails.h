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
#ifndef APPOINTMENTDETAILS_H
#define APPOINTMENTDETAILS_H

#include <qtextbrowser.h>
#include <QOccurrence>
#include <QDLBrowserClient>

class DateBook;

class AppointmentDetails : public QDLBrowserClient
{
    friend class DateBook;

    Q_OBJECT
public:
    AppointmentDetails( QWidget *parent = 0 );

    void init( const QOccurrence &ev );

    QOccurrence occurrence() const { return mOccurrence; }

signals:
    void done();

protected:
    void keyPressEvent( QKeyEvent * );
    QString createPreviewText( const QAppointment &ev );

private:
    QWidget   *previousDetails;
    QString   previousText;
    QOccurrence mOccurrence;
};

#endif
