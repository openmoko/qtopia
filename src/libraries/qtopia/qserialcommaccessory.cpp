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
#include "qserialcommaccessory.h"

// Constants
static const char* const QSERIALCOMMACCESSORY_NAME    = "QSerialCommAccessory";
static const char* const QSERIALCOMMACCESSORY_CONTYPE = "connectionType";

// ============================================================================
//
// QSerialCommAccessory
//
// ============================================================================

/*!
    \class QSerialCommAccessory
    \brief The QSerialCommAccessory class provides access to a serial communication accessory on the device.

    The QSerialCommAccessory class provides access to an serial communication accessory
    on the device. Serial communication device implementations should inherit from
    QSerialCommAccessoryProvider.

    \sa QSerialCommAccessoryProvider, QHardwareInterface

  \ingroup hardware
*/

/*!
    \enum QSerialCommAccessory::ConnectionType
    Defines the mode of serial communication provided by the device

    \value Invalid The accessory does not support a serial connection
    \value RS232 The serial communication uses RS-232
    \value USB The serial communication uses USB
    \value FireWire The serial communication uses FireWire
*/

/*!
    Construct a new serial communication acessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory that supports the serial communication interface.  If there is more
    than one service that supports the serial communication interface, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QSerialCommAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QSerialCommAccessory::QSerialCommAccessory(
    const QString& id,
    QObject *parent,
    QAbstractIpcInterface::Mode mode )
:   QHardwareInterface( QSERIALCOMMACCESSORY_NAME, id, parent, mode )
{
}

/*!
    Destroys this serial communication accessory.
*/
QSerialCommAccessory::~QSerialCommAccessory()
{
}

/*!
    Returns the supported serial connection type.
*/
QSerialCommAccessory::ConnectionType
QSerialCommAccessory::connectionType() const
{
    return static_cast<ConnectionType>(
        value( QSERIALCOMMACCESSORY_CONTYPE,
               static_cast<int>( Invalid ) ).toInt() );
}

// ============================================================================
//
// QSerialCommAccessoryProvider
//
// ============================================================================

/*!
    \class QSerialCommAccessoryProvider
    \brief The QSerialCommAccessoryProvider class provides an interface for serial communication devices to integrate into Qtopia.

    The QSerialCommAccessoryProvider class provides an interface for
    serial communication devices to integrate into Qtopia.  Serial communication
    devices inherit from this class and call setConnectionType() to indicate the
    level of functionality that is supported.

    \sa QSerialCommAccessory
  \ingroup hardware
*/

/*!
    Create a serial communication accessory called \a id and attach it to
    \a parent.
*/
QSerialCommAccessoryProvider::QSerialCommAccessoryProvider(
    const QString& id,
    QObject *parent )
:   QSerialCommAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this serial communicaion accessory provider.
*/
QSerialCommAccessoryProvider::~QSerialCommAccessoryProvider()
{
}

/*!
    Sets the supported connection type to \a connectionType. This is typically
    called from the constructor of subclass implementations.
*/
void QSerialCommAccessoryProvider::setConnectionType(
    const ConnectionType connectionType )
{
    setValue( QSERIALCOMMACCESSORY_CONTYPE,
              static_cast<int>( connectionType ) );
}


