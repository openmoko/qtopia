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

#include <qabstractipcinterfacegroup.h>
#include <qmap.h>
#include <QValueSpaceObject>

/*!
    \class QAbstractIpcInterfaceGroup
    \brief The QAbstractIpcInterfaceGroup class provides a convenient wrapper to create the interfaces within a server group.
    \ingroup communication

    The QAbstractIpcInterfaceGroup class provides a convenient wrapper to
    create the interfaces within a server group.

    It also provides discovery mechanisms that allows one interface
    in a group to quickly locate another at runtime.

    \sa QAbstractIpcInterface
*/

class QAbstractIpcInterfaceGroupPrivate
{
public:
    QAbstractIpcInterfaceGroupPrivate( const QString& groupName )
    {
        this->groupName = groupName;
    }

    QString groupName;
    QMap<QString, QAbstractIpcInterface *> interfaces;
    QStringList suppressed;
};

/*!
    Create a new server interface group called \a groupName and attach
    it to \a parent.

    A call to the constructor should be followed by a call to
    initialize() to complete the initialization process.

    \sa initialize()
*/
QAbstractIpcInterfaceGroup::QAbstractIpcInterfaceGroup
        ( const QString& groupName, QObject *parent )
    : QObject( parent )
{
    d = new QAbstractIpcInterfaceGroupPrivate( groupName );
}

/*!
    Destroy this server interface group.
*/
QAbstractIpcInterfaceGroup::~QAbstractIpcInterfaceGroup()
{
    delete d;
}

/*!
    Return the name of this server interface group.
*/
QString QAbstractIpcInterfaceGroup::groupName() const
{
    return d->groupName;
}

/*!
    Initialize this server interface group by creating all of the interfaces
    that it supports.  In subclasses, interfaces should be created using
    the following pattern:

    \code
    if ( !supports<T>() )
        addInterface( new MyT(this) );
    \endcode

    where \c{T} is the name of the interface being created and \c{MyT}
    is the name of the class that implements the interface.

    At the end of a subclass initialize(), it must call its immediate
    superclass.  The superclass follows the same pattern to create
    missing interfaces that were not explicitly overridden.

    The base implementation calls QAbstractIpcInterface::groupInitialized()
    on all of the interfaces that were created by subclasses.

    \sa supports(), addInterface(), QAbstractIpcInterface::groupInitialized()
*/
void QAbstractIpcInterfaceGroup::initialize()
{
    // Notify all interfaces that the group has been initialized.
    QMap<QString, QAbstractIpcInterface *>::ConstIterator iter;
    for ( iter = d->interfaces.begin(); iter != d->interfaces.end(); ++iter ) {
        iter.value()->groupInitialized( this );
    }

    // Flush all of the changes to the value space so this
    // server interface group becomes visible to clients.
    QValueSpaceObject::sync();
}

/*!
    \fn bool QAbstractIpcInterfaceGroup::supports<T>() const

    Determine if this server interface group supports an interface with
    the name \c{T}.
*/

/*!
    \fn T *QAbstractIpcInterfaceGroup::interface<T>() const

    Get the object that implements the interface \c{T} on this
    server interface group, or null if there is no such interface.
*/

/*!
    Add \a interface to the list of interfaces that is supported by
    this server interface group.
*/
void QAbstractIpcInterfaceGroup::addInterface
        ( QAbstractIpcInterface *interface )
{
    if ( interface ) {
        QString name = interface->interfaceName();
        interface->setObjectName( name );
        d->interfaces.insert( name, interface );
        connect( interface, SIGNAL(destroyed()),
                 this, SLOT(interfaceDestroyed()) );
    }
}

/*!
    \fn void QAbstractIpcInterfaceGroup::suppressInterface<T>()

    Suppress the interface \c{T} from being created during initialize().
    This causes the supports() function to return true for \c{T} even
    if no interface object for \c{T} has been added yet.  The main use
    of this method is to stop a parent class from creating a default
    implementation of \c{T} when the subclass does not create their own.
*/

void QAbstractIpcInterfaceGroup::interfaceDestroyed()
{
    d->interfaces.remove( sender()->objectName() );
}

bool QAbstractIpcInterfaceGroup::_supports( const char *interfaceName ) const
{
    QString name = QString( interfaceName );
    if ( d->suppressed.contains( name ) )
        return true;
    return d->interfaces.contains( name );
}

QAbstractIpcInterface *QAbstractIpcInterfaceGroup::_interface
        ( const char *interfaceName ) const
{
    QMap<QString, QAbstractIpcInterface *>::ConstIterator iter;
    iter = d->interfaces.find( QString( interfaceName ) );
    if ( iter != d->interfaces.end() )
        return iter.value();
    else
        return 0;
}

void QAbstractIpcInterfaceGroup::_suppressInterface
        ( const char *interfaceName )
{
    d->suppressed += QString( interfaceName );
}
