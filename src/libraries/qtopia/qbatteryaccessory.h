/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QBATTERYACCESSORY_H
#define QBATTERYACCESSORY_H

// Local includes
#include "qhardwareinterface.h"

// ============================================================================
//
// QBatteryAccessory
//
// ============================================================================

class QTOPIA_EXPORT QBatteryAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QBatteryAccessory( const QString& id = QString(), QObject *parent = 0,
                                QAbstractIpcInterface::Mode mode = Client );
    ~QBatteryAccessory();

    bool charging() const;
    bool good() const;
    int charge() const;
    int timeRemaining() const;

signals:
    void chargingModified();
    void goodModified();
    void chargeModified();
    void timeRemainingModified();
};

// ============================================================================
//
// QBatteryAccessoryProvider
//
// ============================================================================

class QTOPIA_EXPORT QBatteryAccessoryProvider : public QBatteryAccessory
{
    Q_OBJECT
public:
    explicit QBatteryAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QBatteryAccessoryProvider();

public slots:
    void setCharging( bool charging );
    void setGood( bool good );
    void setCharge( int charge );
    void setTimeRemaining( int timeRemaining );
};

#endif //QBATTERYACCESSORY_H
