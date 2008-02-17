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

#include <qmodemconfiguration.h>
#include <qmodemservice.h>
#include <qatresult.h>

/*!
    \class QModemConfiguration
    \mainclass
    \brief The QModemConfiguration class provides information about the modem such as its manufacturer and serial number.
    \ingroup telephony::modem

    The following read-only configuration values are available:

    \table
    \header \o Name \o Description \o AT command
    \row \o \c{manufacturer} \o Name of the modem's manufacturer \o \c{AT+CGMI}
    \row \o \c{model} \o Name of the modem's modem \o \c{AT+CGMM}
    \row \o \c{revision} \o Name of the modem's revision \o \c{AT+CGMR}
    \row \o \c{serial} \o Serial number for the modem \o \c{AT+CGSN}
    \row \o \c{extraVersion} \o Extra manufacturer-specific version information.
    \endtable

    Modem vendor plug-ins may inherit this class and override request()
    if they use different AT commands from those listed above.

    The \c{extraVersion} value is intended to return additional version
    information beyond that supplied by \c{revision}.

    QModemConfiguration implements the QTelephonyConfiguration telephony interface.
    Client applications should use the QTelephonyConfiguration class instead of this
    class to access the modem's configuration values.

    \sa QTelephonyConfiguration
*/

/*!
    Create a modem configuration object for \a service.
*/
QModemConfiguration::QModemConfiguration( QModemService *service )
    : QTelephonyConfiguration( service->service(), service, Server )
{
    this->service = service;
}

/*!
    Destroy this modem configuration object.
*/
QModemConfiguration::~QModemConfiguration()
{
}

/*!
    \reimp
*/
void QModemConfiguration::update( const QString&, const QString& )
{
    // All values are read-only.
}

/*!
    \reimp
*/
void QModemConfiguration::request( const QString& name )
{
    if ( name == "manufacturer" ) {
        service->primaryAtChat()->chat
            ( "AT+CGMI", this, SLOT(cgmi(bool,QAtResult)) );
    } else if ( name == "model" ) {
        service->primaryAtChat()->chat
            ( "AT+CGMM", this, SLOT(cgmm(bool,QAtResult)) );
    } else if ( name == "revision" ) {
        service->primaryAtChat()->chat
            ( "AT+CGMR", this, SLOT(cgmr(bool,QAtResult)) );
    } else if ( name == "serial" ) {
        service->primaryAtChat()->chat
            ( "AT+CGSN", this, SLOT(cgsn(bool,QAtResult)) );
    } else {
        // Not supported - return an empty string.
        emit notification( name, QString() );
    }
}

void QModemConfiguration::cgmi( bool, const QAtResult& result )
{
    emit notification( "manufacturer", fixResponse( result.content(), "+CGMI:" ) );
}

void QModemConfiguration::cgmm( bool, const QAtResult& result )
{
    emit notification( "model", fixResponse( result.content(), "+CGMM:" ) );
}

void QModemConfiguration::cgmr( bool, const QAtResult& result )
{
    emit notification( "revision", fixResponse( result.content(), "+CGMR:" ) );
}

void QModemConfiguration::cgsn( bool, const QAtResult& result )
{
    emit notification( "serial", fixResponse( result.content(), "+CGSN:" ) );
}

QString QModemConfiguration::fixResponse( const QString& value, const QString& prefix )
{
    QString result = value.trimmed();
    if ( result.startsWith( prefix ) )
        return result.mid( prefix.length() ).trimmed();
    else
        return result;
}
