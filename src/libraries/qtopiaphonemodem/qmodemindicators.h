/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef QMODEMINDICATORS_H
#define QMODEMINDICATORS_H

#include <qmodemservice.h>

class QModemIndicatorsPrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemIndicators : public QObject
{
    Q_OBJECT
public:
    explicit QModemIndicators( QModemService *service );
    ~QModemIndicators();

    enum ChargeState
    {
        PoweredByBattery,
        NotPoweredByBattery,
        NoBattery,
        PowerFault
    };

    enum SmsMemoryFullStatus
    {
        SmsMemoryOK,
        SmsMemoryFull,
        SmsMemoryMessageRejected
    };

public slots:
    void setSignalQuality( int value, int maxValue );
    void setBatteryCharge( int value, int maxValue,
                           QModemIndicators::ChargeState value2 );
    void setSmsMemoryFull( QModemIndicators::SmsMemoryFullStatus value );
    void setNetworkTimeZone( int zone, int dst );
    void setNetworkTime( uint time, int zone, int dst );

private slots:
    void resetModem();
    void csq( bool ok, const QAtResult& result );
    void csqn( const QString& msg );
    void cbc( bool ok, const QAtResult& result );
    void cbcn( const QString& msg );
    void ciev( const QString& msg );
    void cind( bool ok, const QAtResult& result );
    void ctzv( const QString& msg );
    void pollSignalQuality();
    void pollBatteryCharge();

private:
    QModemIndicatorsPrivate *d;

    void setSignalQualityInternal( int value, int maxValue );
    void setBatteryChargeInternal( int value, int maxValue,
                                   QModemIndicators::ChargeState state );
};

#endif // QMODEMINDICATORS_H
