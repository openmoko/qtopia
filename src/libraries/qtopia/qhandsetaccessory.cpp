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
#include "qhandsetaccessory.h"

// Constants
static const char* const QHANDSETACCESSORY_NAME         = "QHandsetAccessory";
static const char* const QHANDSETACCESSORY_MODE         = "mode";
static const char* const QHANDSETACCESSORY_SPEAKERPHONE = "speakerPhone";
static const char* const QHANDSETACCESSORY_TTY          = "tty";

// ============================================================================
//
// QHandsetAccessory
//
// ============================================================================

/*!
    \class QHandsetAccessory
    \mainclass

    \brief The QHandsetAccessory class provides access to a handset accessory on the device.

    Handset device implementations should inherit from
    QHandsetAccessoryProvider.

    \sa QHandsetAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    \enum QHandsetAccessory::Mode
    Defines the mode for a handset accessory

    \value Internal The handset is internal to the device
    \value External The handset is external to the device
*/

/*!
    Construct a new handset accessory object for provider \a id and attaches
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory provider that supports the handset interface.  If there is more
    than one service that supports the handset interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QHandsetAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QHandsetAccessory::QHandsetAccessory(
    const QString& id,
    QObject *parent,
    QAbstractIpcInterface::Mode mode )
:   QHardwareInterface( QHANDSETACCESSORY_NAME, id, parent, mode )
{
}

/*!
    Destroys the handset accessory.
*/
QHandsetAccessory::~QHandsetAccessory()
{
}

/*!
    Returns the mode of the handset accessory.
*/
QHandsetAccessory::Mode QHandsetAccessory::mode() const
{
    return static_cast<Mode>(
        value( QHANDSETACCESSORY_MODE,
               static_cast<int>( Internal ) ).toInt() );
}

/*!
    Returns true if the handset accessory is a speaker phone; otherwise returns false.
*/
bool QHandsetAccessory::speakerPhone() const
{
    return value( QHANDSETACCESSORY_SPEAKERPHONE, false ).toBool();
}

/*!
    Returns true if the handset accessory is a TTY handset; otherwise returns false.
*/
bool QHandsetAccessory::tty() const
{
    return value( QHANDSETACCESSORY_TTY, false ).toBool();
}

// ============================================================================
//
// QHandsetAccessoryProvider
//
// ============================================================================

/*!
    \class QHandsetAccessoryProvider
    \mainclass

    \brief The QHandsetAccessoryProvider class provides an interface for handset devices to integrate into Qtopia.

    Handset devices inherit from this and call setMode(), setSpeakerPhone()
    and setTty() to indicate the level of functionality that is supported.

    \sa QHandsetAccessory

    \ingroup hardware
*/

/*!
    Create a handset accessory provider called \a id and attaches it to \a parent.
*/
QHandsetAccessoryProvider::QHandsetAccessoryProvider(
    const QString& id,
    QObject *parent )
:   QHandsetAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the handset accessory provider.
*/
QHandsetAccessoryProvider::~QHandsetAccessoryProvider()
{
}

/*!
    Sets the mode of the handset accessory to \a mode. This is typically called
    from the constructor of subclass implementations.
*/
void QHandsetAccessoryProvider::setMode( const Mode mode )
{
    setValue( QHANDSETACCESSORY_MODE, static_cast<int>( mode ) );
}

/*!
    Sets the speaker phone attribute to \a speakerPhone. This is typically called
    from the constructor of subclass implementations.
*/
void QHandsetAccessoryProvider::setSpeakerPhone( const bool speakerPhone )
{
    setValue( QHANDSETACCESSORY_SPEAKERPHONE, speakerPhone );
}

/*!
    Sets the TTY attribute to \a tty. This is typically called from the
    constructor of subclass implementations.
*/
void QHandsetAccessoryProvider::setTty( const bool tty )
{
    setValue( QHANDSETACCESSORY_TTY, tty );
}

