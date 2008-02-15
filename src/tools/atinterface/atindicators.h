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

#ifndef ATINDICATORS_H
#define ATINDICATORS_H

#include <qobject.h>
#include "atcallmanager.h"

class AtIndicatorsPrivate;

class AtIndicators : public QObject
{
    Q_OBJECT
public:
    AtIndicators( QObject *parent );
    ~AtIndicators();

    int addIndicator( const QString& name, int maxValue, int initValue = 0 );
    QStringList indicators() const;
    int indicator( const QString& name ) const;
    int numIndicators() const;
    QString name( int ind ) const;

    int value( int ind ) const;
    int value( const QString& name ) const;
    int maxValue( int ind ) const;
    void setValue( int ind, int value );
    void setValue( const QString& name, int value );

signals:
    void indicatorChanged( int ind, int value );

private:
    AtIndicatorsPrivate *d;
};

class AtPhoneIndicatorsPrivate;

class AtPhoneIndicators : public AtIndicators
{
    Q_OBJECT
public:
    AtPhoneIndicators( QObject *parent );
    ~AtPhoneIndicators();

    int signalQuality() const;
    int batteryCharge() const;

public slots:
    void setOnCall( bool value );
    void setCallSetup( AtCallManager::CallSetup callSetup );
    void setCallHold( AtCallManager::CallHoldState callHold );

signals:
    void signalQualityChanged( int value );
    void batteryChargeChanged( int value );

private slots:
    void updateRegistrationState();
    void updateSignalQuality();
    void updateBatteryCharge();
#ifdef QTOPIA_CELL
    void unreadCountChanged();
    void updateSmsMemoryFull();
#endif

private:
    AtPhoneIndicatorsPrivate *d;
};

#endif // ATINDICATORS_H
