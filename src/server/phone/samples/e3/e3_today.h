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

#ifndef _E3TODAY_H_
#define _E3TODAY_H_

#include <QObject>
#include <QAppointment>
#include <QTimer>
class QTask;
class QTaskModel;
class QAppointmentModel;

class E3Today : public QObject
{
Q_OBJECT
public:
    E3Today(QObject *parent = 0);

    enum DayStatus { NoAppointments, NoMoreAppointments, MoreAppointments };
    DayStatus dayStatus() const;

    QAppointment nextAppointment() const;

    int tasks() const;
    QTask task(int ii) const;

    void forceUpdate();

signals:
    void todayChanged();

private slots:
    void resetAppointments();

private:
    DayStatus m_status;
    QTaskModel *m_tasks;
    QAppointmentModel *m_appointments;
    QAppointment m_nextAppointment;
    bool blockReset;
    QTimer m_timer;
};

#endif // _E3TODAY_H_
