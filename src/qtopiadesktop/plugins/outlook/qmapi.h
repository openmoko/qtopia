/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#ifndef QMAPI_H
#define QMAPI_H

#include <QObject>
#include <QStringList>

struct IUnknown;

namespace QMAPI {

    class Session;
    class SessionPrivate;
    class Contact;
    class ContactPrivate;
    class Appointment;
    class AppointmentPrivate;
    class Task;
    class TaskPrivate;

    class Session : public QObject
    {
    public:
        Session( QObject *parent = 0 );
        ~Session();

        bool connected() const;

        Contact *openContact( IUnknown *obj );
        Appointment *openAppointment( IUnknown *obj, bool isException = false );
        Task *openTask( IUnknown *obj );

    private:
        SessionPrivate *d;
    };

    class Contact : public QObject
    {
        friend class Session;

    private:
        Contact( QObject *parent = 0 );

    public:
        ~Contact();

        QString Body();
        QString Email1Address();
        QString Email2Address();
        QString Email3Address();

    private:
        ContactPrivate *d;

    };

    class Appointment : public QObject
    {
        friend class Session;

    private:
        Appointment( QObject *parent = 0 );

    public:
        ~Appointment();

        QString Body();

    private:
        AppointmentPrivate *d;

    };

    class Task : public QObject
    {
        friend class Session;

    private:
        Task( QObject *parent = 0 );

    public:
        ~Task();

        QString Body();

    private:
        TaskPrivate *d;

    };

};

#endif

