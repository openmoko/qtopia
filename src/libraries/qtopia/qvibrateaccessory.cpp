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

#include "qvibrateaccessory.h"

/*!
    \class QVibrateAccessory
    \brief The QVibrateAccessory class provides access to the vibrate device on a phone.

    The QVibrateAccessory class provides access to the vibrate device
    on a phone.  The usual way to turn on the vibrate device within a
    client application is as follows:

    \code
    QVibrateAccessory vib;
    vib.setVibrateNow( true );
    \endcode

    Vibrate device implementations should inherit from
    QVibrateAccessoryProvider.

    \sa QVibrateAccessoryProvider, QHardwareInterface
*/

/*!
    Construct a new vibrate acessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    accessory that supports the vibrate interface.  If there is more
    than one service that supports the vibrate interface, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QVibrateAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QVibrateAccessory::QVibrateAccessory
        ( const QString& id, QObject *parent,
          QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QVibrateAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this vibrate accessory.
*/
QVibrateAccessory::~QVibrateAccessory()
{
}

/*!
    Determine if the vibrate device will vibrate when an incoming call
    is detected.
*/
bool QVibrateAccessory::vibrateOnRing() const
{
    return value( "vibrateOnRing", false ).toBool();
}

/*!
    Determine if the vibrate device is currently vibrating.
*/
bool QVibrateAccessory::vibrateNow() const
{
    return value( "vibrateNow", false ).toBool();
}

/*!
    Determine if the vibrate device supports the vibrateOnRing() feature.
*/
bool QVibrateAccessory::supportsVibrateOnRing() const
{
    return value( "supportsVibrateOnRing", false ).toBool();
}

/*!
    Determine if the vibrate device supports the vibrateNow() feature.
*/
bool QVibrateAccessory::supportsVibrateNow() const
{
    return value( "supportsVibrateNow", false ).toBool();
}

/*!
    Change the vibrateOnRing() \a value.
*/
void QVibrateAccessory::setVibrateOnRing( const bool value )
{
    invoke( SLOT(setVibrateOnRing(bool)), value );
}

/*!
    Change the vibrateNow() \a value.
*/
void QVibrateAccessory::setVibrateNow( const bool value )
{
    invoke( SLOT(setVibrateNow(bool)), value );
}

/*!
    \fn void QVibrateAccessory::vibrateOnRingModified()

    Signal that is emitted when vibrateOnRing() is modified.
*/

/*!
    \fn void QVibrateAccessory::vibrateNowModified()

    Signal that is emitted when vibrateNow() is modified.
*/

/*!
    \class QVibrateAccessoryProvider
    \brief The QVibrateAccessoryProvider class provides an interface for vibrate devices to integrate into Qtopia.

    The QVibrateAccessoryProvider class provides an interface for
    vibrate devices to integrate into Qtopia.  Vibrate devices inherit from
    this class and override setVibrateOnRing() and setVibrateNow() to
    implement the required functionality.  Subclasses should also
    call setSupportsVibrateOnRing() and setSupportsVibrateNow() to
    indicate the level of functionality that is supported.

    \sa QVibrateAccessory
*/

/*!
    Create a vibrate device called \a id and attach it to \a parent.
*/
QVibrateAccessoryProvider::QVibrateAccessoryProvider
        ( const QString& id, QObject *parent )
    : QVibrateAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this vibrate device provider.
*/
QVibrateAccessoryProvider::~QVibrateAccessoryProvider()
{
}

/*!
    Indicate whether this vibrate accessory supports vibrateOnRing()
    with \a value.  This is typically called from the constructor
    of subclass implementations.
*/
void QVibrateAccessoryProvider::setSupportsVibrateOnRing( bool value )
{
    setValue( "supportsVibrateOnRing", value );
}

/*!
    Indicate whether this vibrate accessory supports vibrateNow()
    with \a value.  This is typically called from the constructor
    of subclass implementations.
*/
void QVibrateAccessoryProvider::setSupportsVibrateNow( bool value )
{
    setValue( "supportsVibrateNow", value );
}

/*!
    \reimp
*/
void QVibrateAccessoryProvider::setVibrateOnRing( const bool value )
{
    setValue( "vibrateOnRing", value );
    emit vibrateOnRingModified();
}

/*!
    \reimp
*/
void QVibrateAccessoryProvider::setVibrateNow( const bool value )
{
    setValue( "vibrateNow", value );
    emit vibrateNowModified();
}
