/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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
#include "qinfraredaccessory.h"

// Constants
static const char* const QINFRAREDACCESSORY_NAME = "QInfraredAccessory";
static const char* const QINFRAREDACCESSORY_MODE = "mode";

// ============================================================================
//
// QInfraredAccessory
//
// ============================================================================

/*!
    \class QInfraredAccessory
    \brief The QInfraredAccessory class provides access to an infrared accessory on the device.

    The QInfraredAccessory class provides access to an infrared accessory
    on the device. Infrared device implementations should inherit from
    QInfraredAccessoryProvider.

    \sa QInfraredAccessoryProvider, QHardwareInterface
*/

/*!
    \enum QInfraredAccessory::Mode
    Defines the location of the infra red accessory

    \value Internal The infra red accessory is internal to the device
    \value External The infra red accessory is external to the device
*/

/*!
    Construct a new infrared acessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory that supports the infrared interface.  If there is more
    than one service that supports the infrared interface, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QInfraredAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QInfraredAccessory::QInfraredAccessory(
    const QString& id,
    QObject *parent,
    QAbstractIpcInterface::Mode mode )
:   QHardwareInterface( QINFRAREDACCESSORY_NAME, id, parent, mode )
{
}

/*!
    Destroys this infrared accessory
*/
QInfraredAccessory::~QInfraredAccessory()
{
}

/*!
    Returns the mode of the infrared accessory
*/
QInfraredAccessory::Mode QInfraredAccessory::mode() const
{
    return static_cast<Mode>(
        value( QINFRAREDACCESSORY_MODE,
               static_cast<int>( Internal ) ).toInt() );
}

// ============================================================================
//
// QInfraredAccessoryProvider
//
// ============================================================================

/*!
    \class QInfraredAccessoryProvider
    \brief The QInfraredAccessoryProvider class provides an interface for infrared devices to integrate into Qtopia.

    The QInfraredAccessoryProvider class provides an interface for
    infrared devices to integrate into Qtopia.  Infrared devices inherit from
    this class and call setMode() to indicate the level of functionality that
    is supported.

    \sa QInfraredAccessory
*/

/*!
    Create a infrared device called \a id and attach it to \a parent.
*/
QInfraredAccessoryProvider::QInfraredAccessoryProvider(
    const QString& id,
    QObject *parent )
:   QInfraredAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys this infrared accessory provider.
*/
QInfraredAccessoryProvider::~QInfraredAccessoryProvider()
{
}

/*!
    Sets the \a mode of the accessory. This is typically called from the
    constructor of subclass implementations.
*/
void QInfraredAccessoryProvider::setMode( const Mode mode )
{
    setValue( QINFRAREDACCESSORY_MODE, static_cast<int>( mode ) );
}

