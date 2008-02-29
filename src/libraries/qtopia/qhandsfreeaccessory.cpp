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
#include "qhandsfreeaccessory.h"

// Constants
static const char* const QHANDSFREEACCESSORY_NAME = "QHandsfreeAccessory";
static const char* const QHANDSFREEACCESSORY_MODE = "mode";

// ============================================================================
//
// QHandsfreeAccessory
//
// ============================================================================

/*!
    \class QHandsfreeAccessory
    \mainclass

    \brief The QHandsfreeAccessory class provides access to a handsfree accessory on the device.

    Handsfree device implementations should inherit from
    QHandsfreeAccessoryProvider.

    \sa QHandsfreeAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    \enum QHandsfreeAccessory::Mode
    Defines the mode for a handsfree accessory

    \value NotSupported This device does not support handsfree operation
    \value Portable The device provides portable handsfree operation
    \value Vehicle The handsfree device is part of a vehicle
*/

/*!
    Construct a new handsfree accessory object for \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory provider that supports the handsfree interface. If there is more
    than one service that supports the handsfree interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QHandsfreeAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QHandsfreeAccessory::QHandsfreeAccessory(
    const QString& id,
    QObject *parent,
    QAbstractIpcInterface::Mode mode )
:   QHardwareInterface( QHANDSFREEACCESSORY_NAME, id, parent, mode )
{
}

/*!
    Destroys the handsfree accessory.
*/
QHandsfreeAccessory::~QHandsfreeAccessory()
{
}

/*!
    Returns the mode of the handsfree accessory.
*/
QHandsfreeAccessory::Mode QHandsfreeAccessory::mode() const
{
    return static_cast<Mode>(
        value( QHANDSFREEACCESSORY_MODE,
               static_cast<int>( Invalid ) ).toInt() );
}

// ============================================================================
//
// QHandsfreeAccessoryProvider
//
// ============================================================================

/*!
    \class QHandsfreeAccessoryProvider
    \mainclass

    \brief The QHandsfreeAccessoryProvider class provides an interface for handsfree devices to integrate into Qtopia.

    Handsfree devices inherit from this class and call setMode() to indicate
    the level of functionality that is supported.

    \sa QHandsfreeAccessory

    \ingroup hardware
*/

/*!
    Create a handsfree device called \a id and attaches it to \a parent.
*/
QHandsfreeAccessoryProvider::QHandsfreeAccessoryProvider(
    const QString& id,
    QObject *parent )
:   QHandsfreeAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the handsfree device provider.
*/
QHandsfreeAccessoryProvider::~QHandsfreeAccessoryProvider()
{
}

/*!
    Set the mode of the handsfree device to \a mode.  This is typically called
    from the constructor of subclass implementations.
*/
void QHandsfreeAccessoryProvider::setMode( const Mode mode )
{
    setValue( QHANDSFREEACCESSORY_MODE, static_cast<int>( mode ) );
}
