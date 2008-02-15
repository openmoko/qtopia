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

#ifndef QADVICEOFCHARGE_H
#define QADVICEOFCHARGE_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QAdviceOfCharge : public QCommInterface
{
    Q_OBJECT
public:
    explicit QAdviceOfCharge( const QString& service = QString(),
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QAdviceOfCharge();

public slots:
    virtual void requestCurrentCallMeter();
    virtual void requestAccumulatedCallMeter();
    virtual void requestAccumulatedCallMeterMaximum();
    virtual void requestPricePerUnit();
    virtual void resetAccumulatedCallMeter( const QString& password );
    virtual void setAccumulatedCallMeterMaximum
        ( int value, const QString& password );
    virtual void setPricePerUnit
        ( const QString& currency, const QString& unitPrice,
          const QString& password );

signals:
    void currentCallMeter( int value, bool explicitRequest );
    void accumulatedCallMeter( int value );
    void accumulatedCallMeterMaximum( int value );
    void callMeterMaximumWarning();
    void pricePerUnit( const QString& currency, const QString& unitPrice );
    void resetAccumulatedCallMeterResult( QTelephony::Result result );
    void setAccumulatedCallMeterMaximumResult( QTelephony::Result result );
    void setPricePerUnitResult( QTelephony::Result result );
};

#endif /* QADVICEOFCHARGE_H */
