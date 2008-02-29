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

#include <qcomminterface.h>

/*!
    \class QCommInterface
    \mainclass
    \brief The QCommInterface class provides facilities for implementing client and server communications objects

    Configuration values for the interface are placed under
    \c{/Communications/interfaceName/service} in the value space,
    where \c interfaceName and \c service are supplied in the
    constructor.

    Communications interfaces are normally created in an override of
    QAbstractIpcInterfaceGroup::initialize().  The groupInitialized()
    method will be called at the end of the service initialization process.

    \sa QAbstractIpcInterface, QCommServiceManager

    \ingroup ipc
    \ingroup telephony
*/

/*!
    Construct a new communications object for the interface called
    \a interfaceName on \a service and attach it to \a parent.
    If \a mode is \c Server, then the object is constructed in server
    mode and \a service must not be empty.  If \a mode is \c Client,
    then the object is constructed in client mode and \a service may
    be empty to indicate the default service that implements this
    type of interface.

    \sa service()
*/
QCommInterface::QCommInterface
        ( const QString& interfaceName, const QString& service,
          QObject *parent, QAbstractIpcInterface::Mode mode )
    : QAbstractIpcInterface( "/Communications", interfaceName,
                             service, parent, mode )
{
}

/*!
    Destroy this communications object.
*/
QCommInterface::~QCommInterface()
{
}

/*!
    Get the name of the service associated with this communications object.
    The service name is the same as groupName().
*/
QString QCommInterface::service() const
{
    return groupName();
}
