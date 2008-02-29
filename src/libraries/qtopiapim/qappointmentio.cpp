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

#include "qappointmentio_p.h"
#include <qtopianamespace.h>

#include <qtopiaipcenvelope.h>

#ifndef Q_WS_WIN
#include <unistd.h>
#endif

QList<QAppointmentIO *> QAppointmentIO::activeAppointments;

QAppointmentIO::QAppointmentIO(QObject *parent)
    : QRecordIO(parent)
{
    QtopiaChannel *channel = new QtopiaChannel( "QPE/PIM",  this );

    connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(pimMessage(const QString&,const QByteArray&)) );

    activeAppointments.append(this);
}

QAppointmentIO::~QAppointmentIO()
{
    activeAppointments.removeAll(this);
}

QVariant QAppointmentIO::appointmentField(int row, QAppointmentModel::Field k) const
{
    return QAppointmentModel::appointmentField(appointment(row), k);
}

bool QAppointmentIO::setAppointmentField(int row, QAppointmentModel::Field k,  const QVariant &v)
{
    QAppointment t = appointment(row);
    if (QAppointmentModel::setAppointmentField(t, k, v))
        return updateAppointment(t);
    return false;
}

void QAppointmentIO::checkAdded(const QAppointment &appointment)
{
    Q_UNUSED(appointment);
    emit recordsUpdated();
}

void QAppointmentIO::checkRemoved(const QUniqueId &id)
{
    Q_UNUSED(id);
    emit recordsUpdated();
}

void QAppointmentIO::checkRemoved(const QList<QUniqueId> &ids)
{
    Q_UNUSED(ids);
    emit recordsUpdated();
}

void QAppointmentIO::checkUpdated(const QAppointment &appointment)
{
    Q_UNUSED(appointment);
    emit recordsUpdated();
}


void QAppointmentIO::notifyAdded(const QAppointment &appointment)
{
    {
        QtopiaIpcEnvelope e("QPE/PIM", "addedAppointment(int,QUuid,QAppointment)");
        e << getpid();
        e << contextId();
        e << appointment;
    }

    foreach(QAppointmentIO *c, activeAppointments) {
        if (c != this && c->contextId() == contextId())
            c->checkAdded(appointment);
    }
}

void QAppointmentIO::notifyUpdated(const QAppointment &appointment)
{
    {
        QtopiaIpcEnvelope e("QPE/PIM", "updatedAppointment(int,QUuid,QAppointment)");
        e << getpid();
        e << contextId();
        e << appointment;
    }

    foreach(QAppointmentIO *c, activeAppointments) {
        if (c != this && c->contextId() == contextId())
            c->checkUpdated(appointment);
    }
}

void QAppointmentIO::notifyRemoved(const QUniqueId &id)
{
    {
        QtopiaIpcEnvelope e("QPE/PIM", "removedAppointment(int,QUuid,QAppointment)");
        e << getpid();
        e << contextId();
        e << id;
    }

    foreach(QAppointmentIO *c, activeAppointments) {
        if (c != this && c->contextId() == contextId())
            c->checkRemoved(id);
    }
}

void QAppointmentIO::notifyRemoved(const QList<QUniqueId> &ids)
{
    {
        QtopiaIpcEnvelope e("QPE/PIM", "removedAppointments(int,QUuid,QAppointment)");
        e << getpid();
        e << contextId();
        e << ids;
    }

    foreach(QAppointmentIO *c, activeAppointments) {
        if (c != this && c->contextId() == contextId())
            c->checkRemoved(ids);
    }
}

void QAppointmentIO::ensureDataCurrent(bool force)
{
    Q_UNUSED(force);
    emit recordsUpdated();
}

QDateTime QAppointmentIO::nextAlarm( const QAppointment &appointment)
{
    QDateTime now = QDateTime::currentDateTime();
    // -1 days to make room for timezone shift.
    QOccurrence o = appointment.nextOccurrence(now.date().addDays(-1));
    while (o.isValid()) {
        if (now <= o.alarmInCurrentTZ())
            return o.alarmInCurrentTZ();
        o = o.nextOccurrence();
    }
    return QDateTime();
}

// alarm functions
void QAppointmentIO::removeAlarm(const QAppointment &appointment)
{
#ifdef Q_WS_QWS
    // TODO Needs to be able to set up the return to be a service.
    QDateTime when = nextAlarm(appointment);
    if (!when.isNull())
        Qtopia::deleteAlarm(when, "Calendar", "alarm(QDateTime,int)", appointment.alarmDelay());
#else
    Q_UNUSED( appointment );
#endif
}

void QAppointmentIO::addAlarm(const QAppointment &appointment)
{
#ifdef Q_WS_QWS
    // TODO Needs to be able to set up the return to be a service.
    QDateTime when = nextAlarm(appointment);
    if (!when.isNull())
        Qtopia::addAlarm(when, "Calendar", "alarm(QDateTime,int)", appointment.alarmDelay());
#else
    Q_UNUSED( appointment );
#endif
}

void QAppointmentIO::pimMessage(const QString &message, const QByteArray &data)
{
    QDataStream ds(data);
    if (message == "addedAppointment(int,QUuid,QAppointment)") {
        int pid;
        QUuid u;
        ds >> pid;
        ds >> u;
        if (pid != getpid() && u == contextId()) {
            QAppointment appointment;
            ds >> appointment;
            checkAdded(appointment);
        }
    } else if (message == "removedAppointment(int,QUuid,QUniqueId)") {
        int pid;
        QUuid u;
        ds >> pid;
        ds >> u;
        if (pid != getpid() && u == contextId()) {
            QUniqueId id;
            ds >> id;
            checkRemoved(id);
        }
    } else if (message == "removedAppointments(int,QUuid,QList<QUniqueId>)") {
        int pid;
        QUuid u;
        ds >> pid;
        ds >> u;
        if (pid != getpid() && u == contextId()) {
            QList<QUniqueId> ids;
            ds >> ids;
            checkRemoved(ids);
        }
    } else if (message == "updatedAppointment(int,QUuid,QAppointment)") {
        int pid;
        QUuid u;
        ds >> pid;
        ds >> u;
        if (pid != getpid() && u == contextId()) {
            QAppointment appointment;
            ds >> appointment;
            checkUpdated(appointment);
        }
    } else if (message == "reloadAppointments()") {
        ensureDataCurrent();
    } else if (message == "reload(int)") {
        int force;
        ds >> force;
        ensureDataCurrent(force != 0);
    }
}
