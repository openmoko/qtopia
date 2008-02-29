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

#ifndef _DEFAULTBATTERY_H_
#define _DEFAULTBATTERY_H_

#include <QPowerSourceProvider>
class QHardwareManager;

class DefaultBattery : public QPowerSourceProvider
{
Q_OBJECT
public:
    DefaultBattery(QObject *parent = 0);

private slots:
    void accessoryAdded(const QString &);
    
    void pAvailabilityChanged(QPowerSource::Availability);
    void pChargingChanged(bool);
    void pChargeChanged(int);
    void pCapacityChanged(int);
    void pTimeRemainingChanged(int);

private:
    void initPowerSource();
    void syncPowerSource();

    QString m_primary;
    QPowerSource *m_powerSource;
    QHardwareManager *m_accessories;
};

#endif // _DEFAULTBATTERY_H_
