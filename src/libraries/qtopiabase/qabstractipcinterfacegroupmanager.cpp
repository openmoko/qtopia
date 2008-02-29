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

#include <qabstractipcinterfacegroupmanager.h>
#include <qvaluespace.h>
#include <QMap>
#include <QSet>
#include <qdebug.h>

/*!
    \class QAbstractIpcInterfaceGroupManager
    \brief The QAbstractIpcInterfaceGroupManager class provides a method to discover the active IPC interfaces and server interface groups.
    \ingroup communication

    The QAbstractIpcInterfaceGroupManager class provides a method to discover
    the IPC interfaces and server interface groups that are active within
    the system.

    Each server interface group has a unique name, such as \c{GSM},
    \c{VoIP}, etc.  Within each group is a list of interfaces for
    functionality areas, which have names such as
    \c{QNetworkRegistration}, \c{QSMSSender}, etc.

    Interface names correspond to class names elsewhere in the Qtopia
    Communications and Telephony API's.  To use an interface, the caller
    creates an instance of the corresponding class.  The caller can specify
    an explicit group name, if more than one group implements the same
    interface, or leave the group name empty to use the default group
    for that interface.

    See the documentation for QAbstractIpcInterface for more information
    on writing and using interface classes.

    \sa QAbstractIpcInterface
*/

class QAbstractIpcInterfaceGroupManagerPrivate
{
public:
    QAbstractIpcInterfaceGroupManagerPrivate( const QString& location )
    {
        valueSpaceLocation = location;
        item = new QValueSpaceItem( location );
        cacheInitialized = false;
    }
    ~QAbstractIpcInterfaceGroupManagerPrivate()
    {
        delete item;
    }

    QString valueSpaceLocation;
    QValueSpaceItem *item;
    bool cacheInitialized;
    QSet<QString> groups;
    QMap<QString,QStringList> interfaces;

    void initCache()
    {
        if ( !cacheInitialized ) {
            cacheInitialized = true;
            rebuildCache();
        }
    }

    void rebuildCache();
};

void QAbstractIpcInterfaceGroupManagerPrivate::rebuildCache()
{
    groups.clear();
    interfaces.clear();
    QStringList interfaces = item->subPaths();
    foreach ( QString interface, interfaces ) {
        QValueSpaceItem iface( *item, interface );
        QStringList grps = iface.subPaths();
        foreach ( QString group, grps ) {
            this->interfaces[group].append( interface );
        }
        groups += QSet<QString>::fromList( grps );
    }
}

/*!
    Construct a new server interface group manager and attach it to \a parent.

    The \a valueSpaceLocation parameter specifies the location the
    value space to place all information about this interface.
    Subclasses such as QCommServiceManager will set this to a particular
    value meeting their requirements.
*/
QAbstractIpcInterfaceGroupManager::QAbstractIpcInterfaceGroupManager
        ( const QString& valueSpaceLocation, QObject *parent )
    : QObject( parent )
{
    d = new QAbstractIpcInterfaceGroupManagerPrivate( valueSpaceLocation );
    connect( d->item, SIGNAL(contentsChanged()),
             this, SLOT(groupsChangedInner()) );
}

/*!
    Destroy this server interface group manager.
*/
QAbstractIpcInterfaceGroupManager::~QAbstractIpcInterfaceGroupManager()
{
    delete d;
}

/*!
    Get the list of all groups that are currently active within the system.
*/
QStringList QAbstractIpcInterfaceGroupManager::groups() const
{
    d->initCache();
    return d->groups.toList();
}

/*!
    Get the list of interfaces that are supported by \a group.
*/
QStringList QAbstractIpcInterfaceGroupManager::interfaces
        ( const QString& group ) const
{
    d->initCache();
    QMap<QString,QStringList>::ConstIterator it;
    it = d->interfaces.find( group );
    if ( it != d->interfaces.end() )
        return it.value();
    else
        return QStringList();
}

/*!
    \fn QStringList QAbstractIpcInterfaceGroupManager::supports<T>() const

    Get the list of groups that supports the interface \c{T}.  The following
    example demonstrates how to get the list of groups that supports the
    QNetworkRegistration interface:

    \code
    QCommServiceManager manager;
    QStringList list = manager.supports<QNetworkRegistration>();
    \endcode
*/

/*!
    \fn bool QAbstractIpcInterfaceGroupManager::supports<T>( const QString& group ) const

    Determine if \a group supports the interface \c{T}.  The following
    example demonstrates how to determine if the \c modem group supports
    the QNetworkRegistration interface:

    \code
    QCommServiceManager manager;
    if ( manager.supports<QNetworkRegistration>( "modem" ) )
        ...
    \endcode
*/

/*!
    \fn int QAbstractIpcInterfaceGroupManager::priority<T>( const QString& group ) const

    Get the priority of the interface \c{T} within \a group.  The priority
    determines which group will be selected by default if an explicit
    group name is not supplied when constructing an interface object.

    Returns the default priority of zero if the interface or group name
    does not exist.

    See the documentation for QAbstractIpcInterface for more information on how
    priorities affect the choice of a default interface implementation.

    \sa QAbstractIpcInterface
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupsChanged()

    Signal that is emitted when the list of groups changes, if the interfaces
    on a group has changed, or if the priority assignments have changed.

    \sa groupAdded(), groupRemoved()
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupAdded( const QString& group )

    Signal that is emitted when \a group is added.  A group is considered
    to have been added when its first interface is constructed.
*/

/*!
    \fn void QAbstractIpcInterfaceGroupManager::groupRemoved( const QString& group )

    Signal that is emitted when \a group is removed.  A group is
    considered to have been removed when its last interface is deleted.
*/

/*!
    \internal
*/
void QAbstractIpcInterfaceGroupManager::connectNotify( const char *signal )
{
    // Don't cache the group data until someone actually needs it.
    if ( QLatin1String( signal ) == SIGNAL(groupAdded(QString)) ||
         QLatin1String( signal ) == SIGNAL(groupRemoved(QString)) ) {
        d->initCache();
    }
    QObject::connectNotify( signal );
}

void QAbstractIpcInterfaceGroupManager::groupsChangedInner()
{
    if ( d->cacheInitialized ) {
        // Check for changes in the list of services.
        QSet<QString> current = d->groups;
        d->rebuildCache();
        QSet<QString> list = d->groups;
        foreach ( QString newGroup, list ) {
            if ( !current.contains( newGroup ) ) {
                emit groupAdded( newGroup );
            }
        }
        foreach ( QString oldGroup, current ) {
            if ( !list.contains( oldGroup ) ) {
                emit groupRemoved( oldGroup );
            }
        }
    }
    emit groupsChanged();
}

QStringList QAbstractIpcInterfaceGroupManager::supports( const QString& iface ) const
{
    QValueSpaceItem item( *d->item, iface );
    return item.subPaths();
}

bool QAbstractIpcInterfaceGroupManager::supports
        ( const QString& group, const QString& iface ) const
{
    return supports( iface ).contains( group );
}

int QAbstractIpcInterfaceGroupManager::priority
    ( const QString& group, const QString& iface ) const
{
    return d->item->value( iface + "/" + group + "/priority", (int)0 ).toInt();
}
