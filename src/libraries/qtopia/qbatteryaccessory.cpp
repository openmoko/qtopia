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

// Local includes
#include "qbatteryaccessory.h"

// Constants
static const char* const QBATTERYACCESSORY_NAME     = "QBatteryAccessory";
static const char* const QBATTERYACCESSORY_CHARGING = "charging";
static const char* const QBATTERYACCESSORY_GOOD     = "good";
static const char* const QBATTERYACCESSORY_CHARGE   = "charge";
static const char* const QBATTERYACCESSORY_TIME     = "timeRemaining";

// ============================================================================
//
// QBatteryAccessory
//
// ============================================================================

/*!
    \class QBatteryAccessory
    \mainclass
    \brief The QBatteryAccessory class provides access to a battery on the device.

    The QBatteryAccessory class provides access to a battery on the device.
    Battery implementations should inherit from QBatteryAccessoryProvider.

    \sa QBatteryAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/


/*!
    \fn void QBatteryAccessory::chargingModified()

    Signal that is emitted when charging() is changed.
*/

/*!
    \fn void QBatteryAccessory::goodModified()

    Signal that is emitted when good() is changed.
*/

/*!
    \fn void QBatteryAccessory::chargeModified()

    Signal that is emitted when charge() is changed.
*/

/*!
    \fn void QBatteryAccessory::timeRemainingModified()

    Signal that is emitted when timeRemaining() is changed.
*/

/*!
    Construct a new battery acessory object for provider \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory provider that supports the battery interface.  If there is more
    than one provider that supports the battery interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QBatteryAccessory objects for each.

*/
QBatteryAccessory::QBatteryAccessory
        ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( QBATTERYACCESSORY_NAME, id, parent, mode )
{
    proxy( SIGNAL( chargingModified() ) );
    proxy( SIGNAL( goodModified() ) );
    proxy( SIGNAL( chargeModified() ) );
    proxy( SIGNAL( timeRemainingModified() ) );
}

/*!
    Destroy this battery accessory.
*/
QBatteryAccessory::~QBatteryAccessory()
{
}

/*!
    Returns true if the battery is currently charging; otherwise returns false.
*/
bool QBatteryAccessory::charging() const
{
    return value( QBATTERYACCESSORY_CHARGING, false ).toBool();
}

/*!
    Returns true if the battery is in good working condition; otherwise returns false.
*/
bool QBatteryAccessory::good() const
{
    return value( QBATTERYACCESSORY_GOOD, true ).toBool();
}

/*!
    Returns the percentage charge that remains in the battery, or -1 if
    the battery cannot detect the charge remaining.
*/
int QBatteryAccessory::charge() const
{
    return value( QBATTERYACCESSORY_CHARGE, -1 ).toInt();
}

/*!
    Returns the number of seconds of battery time remaining until
    depletion, or -1 if the battery cannot detect the time remaining.
*/
int QBatteryAccessory::timeRemaining() const
{
    return value( QBATTERYACCESSORY_TIME, -1 ).toInt();
}

// ============================================================================
//
// QBatteryAccessoryProvider
//
// ============================================================================

/*!
    \class QBatteryAccessoryProvider
    \mainclass
    \brief The QBatteryAccessoryProvider class provides an interface for battery devices to integrate into Qtopia.

    The QBatteryAccessoryProvider class provides an interface for
    battery devices to integrate into Qtopia.  Battery devices inherit from
    this and call setCharging(), setGood(), setCharge and setTimeRemaining() to
    indicate the level of functionality that is supported.

    \sa QBatteryAccessory

    \ingroup hardware
*/

/*!
    Create a battey accessory provider called \a id and attaches it to \a parent.
*/
QBatteryAccessoryProvider::QBatteryAccessoryProvider
        ( const QString& id, QObject *parent )
    : QBatteryAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this battery accessory provider.
*/
QBatteryAccessoryProvider::~QBatteryAccessoryProvider()
{
}

/*!
    Sets the charging attribute to \a charging.
*/
void QBatteryAccessoryProvider::setCharging( bool charging )
{
    setValue( QBATTERYACCESSORY_CHARGING, charging );
    emit chargingModified();
}

/*!
    Sets the physical condition of the battery to \a good.
*/
void QBatteryAccessoryProvider::setGood( bool good )
{
    setValue( QBATTERYACCESSORY_GOOD, good );
    emit goodModified();
}

/*!
    Sets the available charge in the battery to \a charge percent.
*/
void QBatteryAccessoryProvider::setCharge( int charge )
{
    setValue( QBATTERYACCESSORY_CHARGE, charge );
    emit chargeModified();
}

/*!
    Sets the number of seconds of time remaining in the battery to
    \a timeRemaining.
*/
void QBatteryAccessoryProvider::setTimeRemaining( int timeRemaining )
{
    setValue( QBATTERYACCESSORY_TIME, timeRemaining );
    emit timeRemainingModified();
}
