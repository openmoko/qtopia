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
#include "qheadsetaccessory.h"

// Constants
static const char* const QHEADSETACCESSORY_NAME        = "QHeadsetAccessory";
static const char* const QHEADSETACCESSORY_CHANNELS    = "channels";
static const char* const QHEADSETACCESSORY_MICPRES     = "microphonePresent";
static const char* const QHEADSETACCESSORY_MICVOL      = "microphoneVolume";
static const char* const QHEADSETACCESSORY_SPEAKERPRES = "speakerPresent";
static const char* const QHEADSETACCESSORY_SPEAKERVOL  = "speakerVolume";
static const char* const QHEADSETACCESSORY_CONTYPE     = "connectionType";

// ============================================================================
//
// QHeadsetAccessory
//
// ============================================================================

/*!
    \class QHeadsetAccessory
    \mainclass

    \brief The QHeadsetAccessory class provides access to a headset accessory on the device.

    QHeadsetAccessory can be used to control the microphone and speaker volume
    on the headset using setMicrophoneVolume() and setSpeakerVolume().

    Headset device implementations should inherit from
    QHeadsetAccessoryProvider.

    \sa QHeadsetAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    \enum QHeadsetAccessory::ConnectionType
    Defines the way the headset is connected to the device

    \value Internal The headset is internal to the device
    \value Wired The headset is connected via a wire or cable
    \value Bluetooth The headset is connected via Bluetooth
*/

/*!
    \fn void QHeadsetAccessory::microphoneVolumeModified();

    Signal that is emitted when the microphone volume is modified.
*/

/*!
    \fn void QHeadsetAccessory::speakerVolumeModified();

    Signal that is emitted when the speaker volume is modified.
*/

/*!
    Construct a new headset accessory object for provider \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory provider that supports the headset interface.  If there is more
    than one service that supports the headset interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QHeadsetAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QHeadsetAccessory::QHeadsetAccessory(
    const QString& id,
    QObject *parent,
    QAbstractIpcInterface::Mode mode )
:   QHardwareInterface( QHEADSETACCESSORY_NAME, id, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroys the headset accessory.
*/
QHeadsetAccessory::~QHeadsetAccessory()
{
}

/*!
    Returns the number of channels the headset supports.
*/
int QHeadsetAccessory::channels() const
{
    return value( QHEADSETACCESSORY_CHANNELS, 1 ).toInt();
}

/*!
    Returns true if the headset has a microphone; otherwise returns false.
*/
bool QHeadsetAccessory::microphonePresent() const
{
    return value( QHEADSETACCESSORY_MICPRES, false ).toBool();
}

/*!
    Returns the microphone volume as a percentage, the default value is 0.
*/
int QHeadsetAccessory::microphoneVolume() const
{
    return value( QHEADSETACCESSORY_MICVOL, 0 ).toInt();
}

/*!
    Returns true if the headset has a speaker; otherwise returns false.
*/
bool QHeadsetAccessory::speakerPresent() const
{
    return value( QHEADSETACCESSORY_SPEAKERPRES, false ).toBool();
}

/*!
    Returns the speaker volume as a percentage, the default value is 0.
*/
int QHeadsetAccessory::speakerVolume() const
{
    return value( QHEADSETACCESSORY_SPEAKERVOL, 0 ).toInt();
}

/*!
    Returns the type of connection used to connect the headset accessory
    to the handset.
*/
QHeadsetAccessory::ConnectionType QHeadsetAccessory::connectionType() const
{
    return static_cast<ConnectionType>(
        value( QHEADSETACCESSORY_CONTYPE,
               static_cast<int>( Internal ) ).toInt() );
}

/*!
    Sets the microphone volume to \a volume percent.
*/
void QHeadsetAccessory::setMicrophoneVolume( const int volume )
{
    invoke( SLOT(setMicrophoneVolume(int)), volume );
}

/*!
    Sets the speaker volume to \a volume percent.
*/
void QHeadsetAccessory::setSpeakerVolume( const int volume )
{
    invoke( SLOT(setSpeakerVolume(int)), volume );
}

// ============================================================================
//
// QHeadsetAccessoryProvider
//
// ============================================================================

/*!
    \class QHeadsetAccessoryProvider
    \mainclass

    \brief The QHeadsetAccessoryProvider class provides an interface for making a headset device visible to the accessory system.

    Headset implementations inherit from this class and set their
    attributes using the set methods.  They then override setMicrophoneVolume()
    and setSpeakerVolume() to update the headset hardware when the volume is
    modified.

    \ingroup hardware
*/

/*!
    Create a headset device provider called \a id and attaches it to \a parent.
*/
QHeadsetAccessoryProvider::QHeadsetAccessoryProvider
        ( const QString& id, QObject *parent )
    : QHeadsetAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the headset accessory provider.
*/
QHeadsetAccessoryProvider::~QHeadsetAccessoryProvider()
{
}

/*!
    Indicate the number of \a channels this headset accessory supports. This is
    typically called from the constructor of subclass implementations.
*/
void QHeadsetAccessoryProvider::setChannels( const int channels )
{
    setValue( QHEADSETACCESSORY_CHANNELS, channels );
}

/*!
    Indicate whether this headset accessory has a microphone \a present.
    This is typically called from the constructor of subclass implementations.
*/
void QHeadsetAccessoryProvider::setMicrophonePresent( const bool present )
{
    setValue( QHEADSETACCESSORY_MICPRES, present );
}

/*!
    Indicate whether this headset accessory has a speaker \a present.
    This is typically called from the constructor of subclass implementations.
*/
void QHeadsetAccessoryProvider::setSpeakerPresent( const bool present )
{
    setValue( QHEADSETACCESSORY_SPEAKERPRES, present );
}

/*!
    Indicate the \a connection type for this headset accessory.
    This is typically called from the constructor of subclass implementations.
*/
void QHeadsetAccessoryProvider::setConnectionType( const ConnectionType connection )
{
    setValue( QHEADSETACCESSORY_CONTYPE, static_cast<int>( connection ) );
}

/*!
    Sets the microphone volume to \a volume percent. The default implementation
    updates the volume value as seen by microphoneVolume() on the client.
    Headset implementations should override this function to provide
    device-specific functionality and then call this implementation
    to update the client's view of the volume value.
*/
void QHeadsetAccessoryProvider::setMicrophoneVolume( const int volume )
{
    setValue( QHEADSETACCESSORY_MICVOL, volume );
    emit microphoneVolumeModified();
}

/*!
    Sets the speaker volume to \a volume percent. The default implementation
    updates the volume value as seen by speakerVolume() on the client.
    Headset implementations should override this function to provide
    device-specific functionality and then call this implementation
    to update the client's view of the volume value.
*/
void QHeadsetAccessoryProvider::setSpeakerVolume( const int volume )
{
    setValue( QHEADSETACCESSORY_SPEAKERVOL, volume );
    emit speakerVolumeModified();
}
