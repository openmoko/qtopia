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
    \brief The QBootSourceAccessory class provides access to the boot source which triggered the boot sequence.

    The QBootSourceAccessory class provides access to the boot source which triggered the boot sequence.
    Boot source implementations should inherit from QBootSourceAccessoryProvider.

    \sa QBootSourceAccessoryProvider, QHardwareInterface
*/


/*!
    \fn void QBootSourceAccessory::bootSourceModified()

    Signal that is emitted when bootSource() is changed.
*/

/*!
    Construct a new boot source accessory object for \a id and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a id is empty, this class will use the default
    accessory that supports the boot source interface.  If there is more
    than one service that supports the boot source interface, the caller
    should enumerate them with QHardwareManager::supports()
    and create separate QBootSourceAccessory objects for each.

    \sa QHardwareManager::supports()
*/
QBootSourceAccessory::QBootSourceAccessory
        ( const QString& id, QObject *parent, QAbstractIpcInterface::Mode mode )
    : QHardwareInterface( QBOOTSOURCEACCESSORY_NAME, id, parent, mode )
{
    proxy( SIGNAL( bootSourceModified() ) );
}

/*!
    Destroy this boot source accessory.
*/
QBootSourceAccessory::~QBootSourceAccessory()
{
}

/*!
    Determines the event which triggered the boot sequence.
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
    \brief The QBootSourceAccessoryProvider class provides an interface for boot source device to integrate into Qtopia.

    The QBootSourceAccessoryProvider class provides an interface for boot source
    devices to integrate into Qtopia.  Boot source devices inherit from this and
    call setBootSource() to indicate the event which triggered the boot sequence.

    \sa QBootSourceAccessory
*/

/*!
    Create a boot source accessory called \a id and attach it to \a parent.
*/
QBootSourceAccessoryProvider::QBootSourceAccessoryProvider
        ( const QString& id, QObject *parent )
    : QBootSourceAccessory( id, parent, QAbstractIpcInterface::Server )
{
}

/*!
    Destroy this boot source accessory provider.
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

