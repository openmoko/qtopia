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

#include "qkeypadlightaccessory.h"

/*!
    \class QKeypadLightAccessory
    \brief The QKeypadLightAccessory class provides access to the keypad light on a phone.

    The QKeypadLightAccessory class provides access to the keypad light
    on a phone.  The usual way to turn on the keypad light within a
    client application is as follows:

    \code
    QKeypadLightAccessory light;
    light.setOn( true );
    \endcode

    Keypad light implementations should inherit from
    QKeypadLightAccessoryProvider.

    \sa QKeypadLightAccessoryProvider, QHardwareInterface
*/

/*!
    Construct a new keypad light acessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    accessory that supports the keypad light.  If there is more
    than one service that supports the keypad light, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QKeypadLightAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QKeypadLightAccessory::QKeypadLightAccessory
      ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QKeypadLightAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this keypad light accessory object.
*/
QKeypadLightAccessory::~QKeypadLightAccessory()
{
}

/*!
    Determine if the keypad light is currently on or off.
*/
bool QKeypadLightAccessory::on() const
{
    return value( "on", false ).toBool();
}

/*!
    Set the keypad light on or off depending upon \a value.
*/
void QKeypadLightAccessory::setOn( const bool value )
{
    invoke( SLOT(setOn(bool)), value );
}

/*!
    \fn void QKeypadLightAccessory::onModified()

    Signal that is emitted when on() is modified.
*/

/*!
    \class QKeypadLightAccessoryProvider
    \brief The QKeypadLightAccessoryProvider class provides an interface for keypad light devices to integrate into Qtopia.

    The QKeypadLightAccessoryProvider class provides an interface for
    keypad light devices to integrate into Qtopia.  Keypad light devices
    inherit from this class and override setOn() to implement the required
    functionality.

    \sa QKeypadLightAccessory
*/

/*!
    Create a keypad light device called \a id and attach it to \a parent.
*/
QKeypadLightAccessoryProvider::QKeypadLightAccessoryProvider
        ( const QString& id, QObject *parent )
    : QKeypadLightAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this keypad light provider.
*/
QKeypadLightAccessoryProvider::~QKeypadLightAccessoryProvider()
{
}

/*!
    \reimp
*/
void QKeypadLightAccessoryProvider::setOn( const bool value )
{
    setValue( "on", value );
    emit onModified();
}
