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

// Local includes
#include "qhardwareinterface.h"
#include "qhardwareinterface_p.h"

// Qt includes
#include <QSettings>

// ============================================================================
//
// QHardwareInterface
//
// ============================================================================

/*!
    \class QHardwareInterface
    \brief The QHardwareInterface class is a base-class for accessory classes.
    \ingroup qtopiaemb

    QHardwareInterface is part of the accessory system which
    provides information about the available physical accessories.
    QHardwareInterface is used as a base-class for hardware accessory classes.

    Default accessories are defined in
    \c {<Qtopia Runtime Prefix>/etc/default/Trolltech/HardwareAccessories.conf}.
    The file contains a list of interface names and the ID of the default
    accessory, for example:

    \code
    [Defaults]
    QBatteryAccessory = StandardBattery
    \endcode

    \sa QHardwareManager
*/

/*!
    Creates a QHardwareInterface object with identity \a name which
    belongs to a \a id and operates in \a mode. The object is attached to
    \a parent.

    If \a id is empty the default accessory for \a name will be
    automatically selected.
*/
QHardwareInterface::QHardwareInterface( const QString& name,
                                        const QString& id,
                                        QObject* parent,
                                        QAbstractIpcInterface::Mode mode )
: QAbstractIpcInterface( HARDWAREINTERFACE_VALUEPATH,
                         name,
                         id,
                         parent,
                         mode )
{
    if ( mode == QAbstractIpcInterface::Server ) {
        QSettings defaults( "Trolltech", "HardwareAccessories" );
        defaults.beginGroup( "Defaults" );
        if ( defaults.value( name ) == id )
            setPriority( 1 );
        defaults.endGroup();
    }
}

/*!
    \internal
*/
QHardwareInterface::~QHardwareInterface()
{
}

