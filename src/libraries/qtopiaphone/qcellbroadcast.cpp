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

#include <qcellbroadcast.h>

/*!
    \class QCellBroadcast
    \mainclass
    \brief The QCellBroadcast class provides access to cell broadcast messages on GSM networks according to 3GPP TS 03.41.
    \ingroup telephony

    The cell broadcast service permits a number of unacknowledged general
    messages to be broadcast to all receivers within a particular region.
    Cell broadcast messages are broadcast to defined geographical areas known
    as cell broadcast areas. These areas may comprise of one or more cells,
    or may comprise the entire network operator's coverage area.

    Cell broadcast messages contain a channel number which allows simple
    filtering to be performed at the modem to reduce the amount of traffic
    that the client phone software needs to deal with.  Client applications
    request the list of channels they are interested in with the
    setChannels() method.  As messages on the selected channels arrive,
    they are delivered to client applications using the broadcast() signal.

    \sa QCBSMessage, QCommInterface
*/

/*!
    Construct a new cell broadcast handling object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports cell broadcast.  If there is more
    than one service that supports cell broadcast, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QCellBroadcast objects for each.

    \sa QCommServiceManager::supports()
*/
QCellBroadcast::QCellBroadcast
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QCellBroadcast", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this cell broadcast handling object.
*/
QCellBroadcast::~QCellBroadcast()
{
}

/*!
    \property QCellBroadcast::channels
    \brief the current list of cell broadcast channels that are being listened to by the modem.
*/

/*!
    Returns the current list of cell broadcast channels that are being
    listened to by the modem.

    \sa setChannels()
*/
QList<int> QCellBroadcast::channels() const
{
    QVariant list = value( "channels" );
    if ( list.isNull() )
        return QList<int>();
    else
        return qVariantValue< QList<int> >( list );
}

/*!
    Sets the list of cell broadcast channels to \a list.  The result of
    performing the operation is returned by the setChanelsResult() signal.

    \sa setChannelsResult()
*/
void QCellBroadcast::setChannels( const QList<int>& list )
{
    invoke( SLOT(setChannels(QList<int>)), qVariantFromValue( list ) );
}

/*!
    \fn void QCellBroadcast::setChannelsResult( QTelephony::Result result )

    Signal that is emitted to report the \a result of a
    setChannels() request.

    \sa setChannels()
*/

/*!
    \fn void QCellBroadcast::broadcast( const QCBSMessage& message )

    Signal that is emitted when a new cell broadcast \a message arrives.

    \sa QCBSMessage
*/

Q_IMPLEMENT_USER_METATYPE_NO_OPERATORS(QList<int>)
