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

#include "qspeakerphoneaccessory.h"

/*!
    \class QSpeakerPhoneAccessory
    \mainclass

    \brief The QSpeakerPhoneAccessory class provides access to the speaker phone.

    QSpeakerPhoneAccessory can be used to switch the speaker phone mode on and
    off using setOnSpeaker().  The usual way to turn on the speaker phone
    within a client application is as follows:

    \code
    QSpeakerPhoneAccessory sp;
    sp.setOnSpeaker( true );
    \endcode

    Speaker phone device implementations should inherit from
    QSpeakerPhoneAccessoryProvider.

    \sa QSpeakerPhoneAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    Construct a new speaker phone accessory object for provider \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the first available
    accessory provider that supports the speaker phone interface.  If there is more
    than one service that supports the speaker phone interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QSpeakerPhoneAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QSpeakerPhoneAccessory::QSpeakerPhoneAccessory
    ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( "QSpeakerPhoneAccessory", id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys the speaker phone accessory.
*/
QSpeakerPhoneAccessory::~QSpeakerPhoneAccessory()
{
}

/*!
    Returns true if audio is currently being played via the speaker; otherwise returns false.
*/
bool QSpeakerPhoneAccessory::onSpeaker() const
{
    return value( "onSpeaker", false ).toBool();
}

/*!
    Sets the speaker phone state to \a value.
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
    \mainclass

    \brief The QSpeakerPhoneAccessoryProvider class provides an interface for speaker phone devices to integrate into Qtopia.

    Speaker phone devices inherit from this class and override setOnSpeaker()
    to implement the required functionality.

    \sa QSpeakerPhoneAccessory

    \ingroup hardware
*/

/*!
    Create a speaker phone device provider called \a id and attaches it to \a parent.
*/
QSpeakerPhoneAccessoryProvider::QSpeakerPhoneAccessoryProvider
        ( const QString& id, QObject *parent )
    : QSpeakerPhoneAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys speaker phone provider.
*/
QSpeakerPhoneAccessoryProvider::~QSpeakerPhoneAccessoryProvider()
{
}

/*!
    Sets the speaker phone state to \a value.  The default implementation
    updates the speaker phone state as seen by onSpeaker() on the client.
    Speaker phone implementations should override this function to provide
    device-specific functionality and then call this implementation to update
    the client's view of the speaker phone state.
*/
void QSpeakerPhoneAccessoryProvider::setOnSpeaker( bool value )
{
    setValue( "onSpeaker", value );
    emit onSpeakerModified();
}
