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

#include <qmodemrffunctionality.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>

/*!
    \class QModemRfFunctionality
    \mainclass
    \brief The QModemRfFunctionality class provides a method to get or set the level of RF activity on an AT-based modem.
    \ingroup telephony::modem

    Thhis class uses the \c{AT+CFUN} command from 3GPP TS 27.007.

    QModemRfFunctionality implements the QPhoneRfFunctionality telephony interface.  Client
    applications should use QPhoneRfFunctionality instead of this class to
    access the modem's RF functionality level.

    \sa QPhoneRfFunctionality
*/

/*!
    Create an AT-based phone RF functionality object for \a service.
*/
QModemRfFunctionality::QModemRfFunctionality( QModemService *service )
    : QPhoneRfFunctionality( service->service(), service, Server )
{
    this->service = service;
}

/*!
    Destroy this AT-based phone RF functionality object.
*/
QModemRfFunctionality::~QModemRfFunctionality()
{
}

/*!
    \reimp
*/
void QModemRfFunctionality::forceLevelRequest()
{
    service->primaryAtChat()->chat
        ( "AT+CFUN?", this, SLOT(cfun(bool,QAtResult)) );
}

class QCFunUserData : public QAtResult::UserData
{
public:
    QCFunUserData( QPhoneRfFunctionality::Level level )
    { this->level = level; }

    QPhoneRfFunctionality::Level level;
};

/*!
    \reimp
*/
void QModemRfFunctionality::setLevel( QPhoneRfFunctionality::Level level )
{
    service->primaryAtChat()->chat
        ( "AT+CFUN=" + QString::number( (int)level ),
          this, SLOT(cfunSet(bool,QAtResult)),
          new QCFunUserData( level ) );
}

void QModemRfFunctionality::cfun( bool ok, const QAtResult& result )
{
    QAtResultParser parser( result );
    if ( parser.next( "+CFUN:" ) ) {
        Level level = (Level)parser.readNumeric();
        setValue( "level", qVariantFromValue( level ) );
        emit levelChanged();
    }

    if ( !ok )
        emit levelQueryFailed();
}

void QModemRfFunctionality::cfunSet( bool ok, const QAtResult& result )
{
    Level level = ((QCFunUserData *)result.userData())->level;

    // Report the results to the client applications.
    if ( ok ) {
        setValue( "level", qVariantFromValue( level ) );
        emit levelChanged();
    }
    emit setLevelResult( (QTelephony::Result)result.resultCode() );

    // Turn on extended error reporting, which may have been disabled
    // by the CFUN change.
    service->primaryAtChat()->chat( "AT+CMEE=1" );

    // Force a PIN query at this point, because we may need to supply it again.
    if ( level != Minimum )
        service->post( "pinquery" );
}
