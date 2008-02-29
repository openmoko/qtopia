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

#ifndef QPOWERSOURCE_H
#define QPOWERSOURCE_H

#include <QHardwareInterface>
#include <qtopiaipcmarshal.h>

class QTOPIA_EXPORT QPowerSource : public QHardwareInterface
{
Q_OBJECT
public:
    enum Type { Wall, Battery, Virtual };
    enum Availability { Available, NotAvailable, Failed };

    explicit QPowerSource(const QString &id = QString(), QObject *parent = 0);
    virtual ~QPowerSource();


    Type type() const;
    Availability availability() const;
    bool charging() const;
    int charge() const;
    int capacity() const;
    int timeRemaining() const;

signals:
    void availabilityChanged(QPowerSource::Availability);
    void chargingChanged(bool);
    void chargeChanged(int);
    void capacityChanged(int);
    void timeRemainingChanged(int);

private:
    friend class QPowerSourceProvider;
    QPowerSource(const QString &id, QObject *parent, 
                 QAbstractIpcInterface::Mode mode);
};
Q_DECLARE_USER_METATYPE_ENUM(QPowerSource::Availability);

class QPowerSourceProviderPrivate;
class QTOPIA_EXPORT QPowerSourceProvider : public QPowerSource
{
Q_OBJECT
public:
    explicit QPowerSourceProvider(Type type, const QString &id, QObject *parent = 0);
    virtual ~QPowerSourceProvider();

public slots:
    void setAvailability(QPowerSource::Availability availability);
    void setCharging(bool charging);
    void setCharge(int);
    void setCapacity(int);
    void setTimeRemaining(int);

private:
    QPowerSourceProviderPrivate *d;
};

class QPowerStatusPrivate;
class QTOPIA_EXPORT QPowerStatus : public QObject
{
Q_OBJECT
public:
    QPowerStatus(QObject * = 0);
    virtual ~QPowerStatus();

    enum WallStatus { Available, NotAvailable, NoWallSource };
    enum BatteryStatus { Normal, Low, VeryLow, Critical, NoBattery };

    WallStatus wallStatus() const;
    bool batteryCharging() const;
    BatteryStatus batteryStatus() const;

    QPowerSource *wallSource() const;
    QPowerSource *batterySource() const;

signals:
    void wallStatusChanged(QPowerStatus::WallStatus);
    void batteryStatusChanged(QPowerStatus::BatteryStatus);
    void batteryChargingChanged(bool);

private slots:
    void wallAvailabilityChanged();
    void batteryChargeChanged();

private:
    QPowerStatusPrivate *d;
};

#endif // QPOWERSOURCE_H
