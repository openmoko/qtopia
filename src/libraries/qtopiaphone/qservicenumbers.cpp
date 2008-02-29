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

#include <qservicenumbers.h>

/*!
    \class QServiceNumbers
    \mainclass
    \brief The QServiceNumbers class provides access to GSM service numbers such as voice mail and SMS service center.
    \ingroup telephony

    The value of service numbers may be queried with requestServiceNumber().  The telephony
    service will respond by emitting the serviceNumber() signal.

    The value of service numbers may be changed with setServiceNumber().  The telephony
    service will respond by emitting the setServiceNumberResult() signal, indicating
    whether the request succeeded or failed.

    \sa QCommInterface
*/

/*!
    \enum QServiceNumbers::NumberId
    This enum defines the service number to query or set with QServiceNumbers.

    \value VoiceMail Query or set the voice mail number on the SIM.
    \value SmsServiceCenter Query or set the SMS service center number.
    \value SubscriberNumber Query or set the subscriber's number in the SIM.
*/

/*!
    Construct a new service number object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.

    If \a service is empty, this class will use the first available
    service that supports service numbers.  If there is more
    than one service that supports service numbers, the caller
    should enumerate them with QCommServiceManager::supports()
    and create separate QServiceNumbers objects for each.

    \sa QCommServiceManager::supports()
*/
QServiceNumbers::QServiceNumbers
        ( const QString& service, QObject *parent, QCommInterface::Mode mode )
    : QCommInterface( "QServiceNumbers", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this service number object.
*/
QServiceNumbers::~QServiceNumbers()
{
}

/*!
    Request the current value of service number \a id.  The service responds
    by emitting the serviceNumber() signal.

    \sa serviceNumber(), setServiceNumber()
*/
void QServiceNumbers::requestServiceNumber( QServiceNumbers::NumberId id )
{
    invoke( SLOT(requestServiceNumber(QServiceNumbers::NumberId)),
            qVariantFromValue( id ) );
}

/*!
    Sets the value of service number \a id to \a number.  The service
    responds by emitting the setServiceNumberResult() signal.

    \sa setServiceNumberResult(), requestServiceNumber(), serviceNumber()
*/
void QServiceNumbers::setServiceNumber
        ( QServiceNumbers::NumberId id, const QString& number )
{
    invoke( SLOT(setServiceNumber(QServiceNumbers::NumberId,QString)),
            qVariantFromValue( id ), number );
}

/*!
    \fn void QServiceNumbers::serviceNumber( QServiceNumbers::NumberId id, const QString& number )

    Signal that is emitted in response to requestServiceNumber(),
    to report the \a number associated with \a id.  If \a number
    is empty, then the number is not set.

    \sa requestServiceNumber(), setServiceNumber()
*/

/*!
    \fn void QServiceNumbers::setServiceNumberResult( QServiceNumbers::NumberId id, QTelephony::Result result )

    Signal that is emitted to report the \a result of calling
    setServiceNumber() for \a id.

    \sa setServiceNumber()
*/

Q_IMPLEMENT_USER_METATYPE_ENUM(QServiceNumbers::NumberId)
