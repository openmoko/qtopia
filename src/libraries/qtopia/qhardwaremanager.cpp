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

#include <QValueSpaceItem>

// ============================================================================
//
// QHardwareManager
//
// ============================================================================

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

    The following example detects whenever a new battery accessory becomes available:

    \code
        QHardwareManager* manager = new QHardwareManager("QBatteryAccessory");
        connect( manager, SIGNAL(providerAdded(const QString&)),
            this, SLOT(newBatteryAdded(const QString&)));
    \endcode

    The types of hardware accessories are the names of the accessory classes 
    which implement the accessories. Any class that is subclassing QHardwareInterface 
    is considered to be a Qtopia accessory.
    
    \sa QHardwareInterface, QBatteryAccessory, QSignalSource, QVibrateAccessory
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

struct QHardwareManagerPrivate 
{
    QString interface;
    QStringList accessories;
    QValueSpaceItem *vs;
    
    void updateAccessories()
    {
        accessories = vs->subPaths();
        qSort(accessories.begin(), accessories.end());
    }
};

/*!
    Creates a QHardwareManager object and attaches it to \a parent. \a interface is
    the name/type of the accessory that this object is monitoring.

    The following code assumes that Qtopia is running on a device
    which exposes the battery level via the internal modem.

    \code 
        QHardwareManager manager( "QBatteryAccessory" );
        QStringList providers = manager.providers();
        providers.contains( "modem" );  //always true
    \endcode

    Another way to achieve the same as the above example would be:

    \code 
        QStringList providers = QHardwareManager::providers<QBatteryAccessory>();
        providers.contains( "modem" );  //always true
    \endcode
*/
QHardwareManager::QHardwareManager( const QString& interface, QObject *parent )
    : QAbstractIpcInterfaceGroupManager( HARDWAREINTERFACE_VALUEPATH, parent )
{
    d = new QHardwareManagerPrivate;
    d->interface = interface;
    while ( d->interface.endsWith("/") )
        d->interface.chop(1);
    d->vs = new QValueSpaceItem(HARDWAREINTERFACE_VALUEPATH+"/" + interface );
    connect( d->vs, SIGNAL(contentsChanged()), this, SLOT(changed()) );
    d->updateAccessories();
}

/*!
  Destroys this object.
*/
QHardwareManager::~QHardwareManager()
{
    delete d;
}

/*!
  Returns the interface that this object is monitoring.
  */
QString QHardwareManager::interface() const
{
    return d->interface;
}

/*!
    Returns the list of providers that support the interface
    that this object was initialized with.
  */
QStringList QHardwareManager::providers() const
{
    return d->accessories;
}

/*!
  \internal
  */
void QHardwareManager::changed()
{
    QStringList oldAccessories = providers();
    d->updateAccessories();
    QStringList newAccessories = providers();

    int old = 0;
    int acc = 0;

    while(old < oldAccessories.count() || acc < newAccessories.count()) {
        if(old == oldAccessories.count()) {
            emit providerAdded(newAccessories.at(acc));
            acc++;
        } else if(acc == newAccessories.count()) {
            emit providerRemoved(oldAccessories.at(old));
            old++;
        } else {
            const QString &oldAcc = oldAccessories.at(old);
            const QString &newAcc = newAccessories.at(acc);

            if(oldAcc == newAcc) {
                old++;
                acc++;
            } else if(oldAcc < newAcc) {
                emit providerRemoved(oldAcc);
                old++;
            } else { // newAcc < oldAcc
                emit providerAdded(newAcc);
                acc++;
            }
        }
    }
}

/*!
  \fn QStringList QHardwareManager::providers<T>()

  Returns a list of providers that support the given interface of type \c{T}.
  The following example demonstrates how to get the list of providers
  which support the QBatteryAccessory interface:

  \code
    QStringList providers = QHardwareManager::providers<QBatteryAccessory>();
  \endcode
  */
