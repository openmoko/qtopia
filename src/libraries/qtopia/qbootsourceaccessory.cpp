/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include "qbootsourceaccessory.h"

// Constants
static const char* const QBOOTSOURCEACCESSORY_NAME     = "QBootSourceAccessory";
static const char* const QBOOTSOURCEACCESSORY_SOURCE   = "source";

// ============================================================================
//
// QBootSourceAccessory
//
// ============================================================================

/*!
    \class QBootSourceAccessory
    \mainclass

    \brief The QBootSourceAccessory class provides an interface for querying the event which triggered the last boot sequence.

    QBootSourceAccessory can be used to query the event which triggered the last
    boot using bootSource().  For example the following code checks if the
    device was booted by plugging in the charger:

    \code
        QBootSourceAccessory bsa;
        if (bsa.bootSource() == QBootSourceAccessory::Charger) {
            ...
        }
    \endcode

    Boot source implementations should inherit from QBootSourceAccessoryProvider.

    \sa QBootSourceAccessoryProvider, QHardwareInterface

    \ingroup hardware
*/

/*!
    \enum QBootSourceAccessory::Source
    Defines the events which can trigger a boot sequence

    \value Unknown Boot was triggered by an unknown event
    \value PowerKey Boot was triggered by pressing the power-on key
    \value Charger Boot was triggered by plugging in the battery charger
    \value Alarm Boot was triggered by an RTC alarm
    \value Watchdog Boot was triggered by the expiration of the watchdog timer
    \value Software Boot was triggered by software, e.g. via a software reboot.
*/

/*!
    \fn void QBootSourceAccessory::bootSourceModified()

    This signal is emitted when bootSource() changes.
*/

/*!
    Construct a new boot source accessory object for the given provider \a id
    and attaches it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory provider that supports the boot source interface.  If there is more
    than one service that supports the boot source interface, the caller
    should enumerate them with QHardwareManager::providers()
    and create separate QBootSourceAccessory objects for each.

    \sa QHardwareManager::providers()
*/
QBootSourceAccessory::QBootSourceAccessory
        ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( QBOOTSOURCEACCESSORY_NAME, id, parent, mode )
{
    proxy( SIGNAL(bootSourceModified()) );
}

/*!
    Destroys the boot source accessory.
*/
QBootSourceAccessory::~QBootSourceAccessory()
{
}

/*!
    Returns the event which triggered the last boot sequence.
*/
QBootSourceAccessory::Source QBootSourceAccessory::bootSource() const
{
    return static_cast<QBootSourceAccessory::Source>(
        value( QBOOTSOURCEACCESSORY_SOURCE, QBootSourceAccessory::Unknown ).toInt());
}

// ============================================================================
//
// QBootSourceAccessoryProvider
//
// ============================================================================

/*!
    \class QBootSourceAccessoryProvider
    \mainclass

    \brief The QBootSourceAccessoryProvider class provides an interface for integrating device specific boot source detection code into Qtopia.

    To use this class either inherit from it and call setBootSource() to
    indicate the event which triggered the last boot sequence, or instantiate
    this class directly and connect an appropriate signal to the
    setBootSource() slot.
    
    \sa QBootSourceAccessory

    \ingroup hardware
*/

/*!
    Create a boot source accessory provider called \a id and attaches it to \a parent.
*/
QBootSourceAccessoryProvider::QBootSourceAccessoryProvider
        ( const QString& id, QObject *parent )
    : QBootSourceAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroys the boot source accessory provider.
*/
QBootSourceAccessoryProvider::~QBootSourceAccessoryProvider()
{
}

/*!
    Sets the boot source attribute to \a source.
*/
void QBootSourceAccessoryProvider::setBootSource( QBootSourceAccessory::Source source )
{
    setValue( QBOOTSOURCEACCESSORY_SOURCE, source );
    emit bootSourceModified();
}

