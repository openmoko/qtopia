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

#include "e1_battery.h"

E1Battery::E1Battery(QObject *parent)
: QObject(parent), m_timerId(0), m_battery("/Hardware/Accessories/QPowerSource/DefaultBattery")
{
    QObject::connect(&m_battery, SIGNAL(contentsChanged()), this, SLOT(batteryChanged()));
    batteryChanged();
}

E1Battery::~E1Battery()
{
}

int E1Battery::charge()
{
    return m_charge;
}

void E1Battery::batteryChanged()
{
    bool isCharging = m_battery.value("Charging").toBool();
    int charge = m_battery.value("Charge").toInt();

    if(isCharging) {
        startcharge();
    } else {
        stopcharge();
        m_charge = charge;
        emit chargeChanged(this->charge());
    }
}

void E1Battery::timerEvent(QTimerEvent *)
{
    m_charge = (m_charge + 20) % 100;
    emit chargeChanged(charge());
}

void E1Battery::startcharge()
{
    if(!m_timerId) {
        m_timerId = startTimer(500);
        m_charge = 0;
        emit chargeChanged(charge());
    }
}

void E1Battery::stopcharge()
{
    if(m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}
