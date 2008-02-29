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
#include "qhardwareinterface_p.h"
#include "qhardwaremanager.h"

// ============================================================================
//
// QHardwareManager
//
// ============================================================================

/*!
    \class QHardwareManager
    \brief The QHardwareManager class finds available hardware accessories.
    \ingroup qtopiaemb

    QHardwareManager is part of the accessory system which
    provides information about the available physical accessories.
    QHardwareManager is responsible for searching for the available
    hardware accessories.

    \sa QHardwareInterface

    \ingroup hardware
*/

/*!
    \fn void QHardwareManager::accessoriesChanged();

    Signal that is emitted when the hardware accessories have changed.
*/

/*!
    \fn void QHardwareManager::accessoryAdded( const QString& id );

    Signal that is emitted when accessory \a id is added.
*/

/*!
    \fn void QHardwareManager::accessoryRemoved( const QString& id );

    Signal that is emitted when accessory \a id is removed.
*/

/*!
    Creates a QHardwareManager object and attaches it to \a parent.
*/
QHardwareManager::QHardwareManager( QObject *parent )
    : QAbstractIpcInterfaceGroupManager( HARDWAREINTERFACE_VALUEPATH, parent )
{
    connect( this, SIGNAL(groupsChanged()),
             this, SIGNAL(accessoriesChanged()) );
    connect( this, SIGNAL(groupAdded(QString)),
             this, SIGNAL(accessoryAdded(QString)) );
    connect( this, SIGNAL(groupRemoved(QString)),
             this, SIGNAL(accessoryRemoved(QString)) );
}

/*!
    \internal
*/
QHardwareManager::~QHardwareManager()
{
}

/*!
    Returns a list of all the accessory IDs which are available on the device.
*/
QStringList QHardwareManager::accessoryIds() const
{
    return groups();
}

/*!
    Returns a list of the accessory types which are supported by a \a id.
*/
QStringList QHardwareManager::accessoryTypes( const QString& id ) const
{
    return interfaces( id );
}

