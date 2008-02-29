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

#include "qspeakerphoneaccessory.h"

/*!
    \class QSpeakerPhoneAccessory
    \brief The QSpeakerPhoneAccessory class provides access to the speaker phone.

    The QSpeakerPhoneAccessory class provides access to the speaker phone.
    The usual way to turn on the speaker phone within a client application
    is as follows:

    \code
    QSpeakerPhoneAccessory sp;
    sp.setOnSpeaker( true );
    \endcode

    Speaker phone device implementations should inherit from
    QSpeakerPhoneAccessoryProvider.

    \sa QSpeakerPhoneAccessoryProvider, QHardwareInterface
*/

/*!
    Construct a new speaker phone acessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    accessory that supports the speaker phone interface.  If there is more
    than one service that supports the speaker phone interface, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QSpeakerPhoneAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QSpeakerPhoneAccessory::QSpeakerPhoneAccessory
    ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QSpeakerPhoneAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this speaker phone accessory.
*/
QSpeakerPhoneAccessory::~QSpeakerPhoneAccessory()
{
}

/*!
    Determine if audio is currently being played via the speaker.
*/
bool QSpeakerPhoneAccessory::onSpeaker() const
{
    return value( "onSpeaker", false ).toBool();
}

/*!
    Set the speaker phone state to \a value.
*/
void QSpeakerPhoneAccessory::setOnSpeaker( bool value )
{
    invoke( SLOT(setOnSpeaker(bool)), value );
}

/*!
    \fn void QSpeakerPhoneAccessory::onSpeakerModified()

    Signal that is emitted when the onSpeaker() state changes.
*/

/*!
    \class QSpeakerPhoneAccessoryProvider
    \brief The QSpeakerPhoneAccessoryProvider class provides an interface for speaker phone devices to integrate into Qtopia.

    The QSpeakerPhoneAccessoryProvider class provides an interface for
    speaker phone devices to integrate into Qtopia.  Speaker phone devices
    inherit from this class and override setOnSpeaker() to
    implement the required functionality.

    \sa QSpeakerPhoneAccessory
*/

/*!
    Create a speaker phone device called \a id and attach it to \a parent.
*/
QSpeakerPhoneAccessoryProvider::QSpeakerPhoneAccessoryProvider
        ( const QString& id, QObject *parent )
    : QSpeakerPhoneAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this speaker phone provider.
*/
QSpeakerPhoneAccessoryProvider::~QSpeakerPhoneAccessoryProvider()
{
}

/*!
    \reimp
*/
void QSpeakerPhoneAccessoryProvider::setOnSpeaker( bool value )
{
    setValue( "onSpeaker", value );
    emit onSpeakerModified();
}
