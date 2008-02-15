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

#ifndef QMODEMADVICEOFCHARGE_H
#define QMODEMADVICEOFCHARGE_H

#include <qadviceofcharge.h>

class QModemService;
class QModemAdviceOfChargePrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemAdviceOfCharge : public QAdviceOfCharge
{
    Q_OBJECT
public:
    explicit QModemAdviceOfCharge( QModemService *service );
    ~QModemAdviceOfCharge();

public slots:
    void requestCurrentCallMeter();
    void requestAccumulatedCallMeter();
    void requestAccumulatedCallMeterMaximum();
    void requestPricePerUnit();
    void resetAccumulatedCallMeter( const QString& password );
    void setAccumulatedCallMeterMaximum( int value, const QString& password );
    void setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password );

private slots:
    void resetModem();
    void caoc( bool ok, const QAtResult& result );
    void cccm( const QString& msg );
    void cacm( bool ok, const QAtResult& result );
    void cacmSet( bool ok, const QAtResult& result );
    void camm( bool ok, const QAtResult& result );
    void cammSet( bool ok, const QAtResult& result );
    void cpuc( bool ok, const QAtResult& result );
    void cpucSet( bool ok, const QAtResult& result );

private:
    QModemAdviceOfChargePrivate *d;
};

#endif /* QMODEMADVICEOFCHARGE_H */
