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

#include <qtelephonyconfiguration.h>

/*!
    \class QTelephonyConfiguration
    \mainclass
    \brief The QTelephonyConfiguration class provides access to configuration parameters on specific telephony services.
    \ingroup telephony

    This class is intended for use by settings applications that need to communicate
    changes from the user to the back end telephony handlers.  It should not be used for
    information that is normally available through other interfaces.

    Telephony services that support GSM modems should provide the configuration
    parameters specified in the QModemConfiguration class documentation.

    Telephony services that support Voice-Over-IP are free to implement
    whatever configuration parameters they wish.  These configuration parameters
    will typically be used by a service-specific settings UI program to
    modify the configuration of the Voice-Over-IP implementation.

    \sa QModemConfiguration
*/

/*!
    Construct a new telephony configuration object for \a service and attach
    it to \a parent.  The object will be created in client mode if
    \a mode is Client, or server mode otherwise.
*/
QTelephonyConfiguration::QTelephonyConfiguration
        ( const QString& service, QObject *parent,
          QCommInterface::Mode mode )
    : QCommInterface( "QTelephonyConfiguration", service, parent, mode )
{
    proxyAll( staticMetaObject );
}

/*!
    Destroy this configuration object.
*/
QTelephonyConfiguration::~QTelephonyConfiguration()
{
}

/*!
    Update the configuration item \a name to \a value.

    \sa request()
*/
void QTelephonyConfiguration::update
        ( const QString& name, const QString& value )
{
    invoke( SLOT(update(QString,QString)), name, value );
}

/*!
    Request the current value of the configuration item \a name.
    The value will be returned via the notification() signal.

    \sa notification(), update()
*/
void QTelephonyConfiguration::request( const QString& name )
{
    invoke( SLOT(request(QString)), name );
}

/*!
    \fn void QTelephonyConfiguration::notification( const QString& name, const QString& value )

    Signal that is emitted by the configuration service to indicate that
    the value of \a name is now \a value.  This is usually emitted in response
    to a call on request(), but it may also be emitted if the configuration
    \a name corresponds to an unsolicited value.

    \sa request()
*/
