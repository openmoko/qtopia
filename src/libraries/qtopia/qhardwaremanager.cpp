/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#include "qhardwareinterface_p.h"
#include "qhardwaremanager.h"

// ============================================================================
//
// QHardwareManager
//
// ============================================================================

struct QHardwareManagerPrivate
{
    QString interface;
};

/*!
    \class QHardwareManager
    \mainclass
    \brief The QHardwareManager class finds available accessory providers for a 
    given hardware interface.

    QHardwareManager is part of the \l{Qtopia Accessory System}{Qtopia Accessory System},
    which provides information about the available hardware accessories.

    A QHardwareMonitor can be used to monitor the availability of a given
    type of hardware accessory, providing a list of providers through providers() 
    and emitting signals providerAdded() and  providerRemoved() when that list changes.
    The type of hardware accessory monitored is given as a parameter to
    constructing the QHardwareMonitor.

    The following example responds whenever a new QSignalSource becomes available:

    \code
        QHardwareManager* manager = new QHardwareManager( "QSignalSource" );
        connect( manager, SIGNAL(providerAdded(QString)), 
            this, SLOT(newBatteryAdded(QString)));
    \endcode

    The types of hardware accessories are the names of the accessory classes 
    which implement the accessories. Any class that is subclassing QHardwareInterface 
    is considered to be a Qtopia accessory.

    \sa QHardwareInterface, QPowerSource, QSignalSource, QVibrateAccessory
    \ingroup hardware
*/

/*!
    \fn void QHardwareManager::providerAdded( const QString& id );

    This signal is emitted when provider \a id is added.
*/

/*!
    \fn void QHardwareManager::providerRemoved( const QString& id );

    This signal is emitted when provider \a id is removed.
*/

/*!
    Creates a QHardwareManager object and attaches it to \a parent. \a interface is the
    name of the accessory interface that this object is monitoring.

    The following code assumes that Qtopia is running on a device that
    has a modem as power source (for more details see QPowerStatus and QPowerSource).
    \code
        QHardwareManager manager( "QPowerSource" );
        QStringList providers = manager.providers();
        
        //Qtopia always has a virtual power source with id DefaultBattery
        providers.contains( "DefaultBattery" ); //always true
        providers.contains( "modem" ); //true
    \endcode

    Another way to achieve the same as the above example would be:

    \code
        QStringList providers = QHardwareManager::providers<QPowerSource>();
        providers.contains( "DefaultBattery" ); //always true
        providers.contains( "modem" ); //true
    \endcode
*/
QHardwareManager::QHardwareManager( const QString& interface, QObject *parent )
    : QAbstractIpcInterfaceGroupManager( HARDWAREINTERFACE_VALUEPATH, interface, parent )
{
    d = new QHardwareManagerPrivate;
    d->interface = interface;
    connect( this, SIGNAL(groupAdded(QString)),
             this, SIGNAL(providerAdded(QString)) );
    connect( this, SIGNAL(groupRemoved(QString)),
             this, SIGNAL(providerRemoved(QString)) );
}

/*!
    Destroys the QHardwareManager object.
*/
QHardwareManager::~QHardwareManager()
{
}

/*!
  Returns the interface that this object is monitoring.
*/
QString QHardwareManager::interface() const
{
    return d->interface;
}

/*!
    Returns a list of providers that support the interface that this object was initialized
    with.
*/
QStringList QHardwareManager::providers() const
{
    return groups();
}

/*!
  \fn QStringList QHardwareManager::providers<T>()

  Returns a list of providers which support the given interface of type \c{T}. 
  The following example demonstrates how to get the list of providers that supports
  the QPowerSource interface.

  \code
        QStringList providers = QHardwareManager::providers<QPowerSource>();
  \endcode
 */
