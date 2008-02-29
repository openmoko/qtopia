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

#include <qmodemcallbarring.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemCallBarring
    \mainclass
    \brief The QModemCallBarring class implements the call barring settings for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CLCK} command from 3GPP TS 27.007.

    QModemCallBarring implements the QCallBarring telephony interface.  Client
    applications should use QCallBarring instead of this class to access the
    modem's call barring settings.

    \sa QCallBarring
*/

/*!
    Construct a new modem call barring handler for \a service.
*/
QModemCallBarring::QModemCallBarring( QModemService *service )
    : QCallBarring( service->service(), service, QCommInterface::Server )
{
    this->service = service;
}

/*!
    Destroy this modem call barring handler.
*/
QModemCallBarring::~QModemCallBarring()
{
    // Nothing to do here.
}

class QBarringUserData : public QAtResult::UserData
{
public:
    QBarringUserData( QCallBarring::BarringType type )
    { this->type = type; }

    QCallBarring::BarringType type;
};

/*!
    \reimp
*/
void QModemCallBarring::requestBarringStatus( QCallBarring::BarringType type )
{
    QString str = typeToString( type );
    if ( str.isEmpty() ) {
        // This type of barring operation is not supported,
        // So report that nothing is barred.
        emit barringStatus( type, QTelephony::CallClassNone );
    } else {
        service->primaryAtChat()->chat
            ( "AT+CLCK=\"" + str + "\",2",
              this, SLOT(requestDone(bool,QAtResult)),
              new QBarringUserData( type ) );
    }
}

/*!
    \reimp
*/
void QModemCallBarring::setBarringStatus
        ( QCallBarring::BarringType type, const QString& password,
          QTelephony::CallClass cls, bool lock )
{
    QString str = typeToString( type );
    if ( str.isEmpty() ) {
        // This type of barring operating is not supported,
        // so report an error immediately to the client.
        emit setBarringStatusResult( QTelephony::OperationNotSupported );
    } else if ( lock ) {
        service->primaryAtChat()->chat
            ( "AT+CLCK=\"" + str + "\",1,\"" +
              QAtUtils::quote( password ) + "\"," +
              QString::number( (int)cls ),
              this, SLOT(setDone(bool,QAtResult)) );
    } else {
        service->primaryAtChat()->chat
            ( "AT+CLCK=\"" + str + "\",0,\"" +
              QAtUtils::quote( password ) + "\"," +
              QString::number( (int)cls ),
              this, SLOT(setDone(bool,QAtResult)) );
    }
}

/*!
    \reimp
*/
void QModemCallBarring::unlockAll( const QString& password )
{
    service->primaryAtChat()->chat
        ( "AT+CLCK=\"AB\",0,\"" + QAtUtils::quote( password ) + "\"",
          this, SLOT(unlockDone(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallBarring::unlockAllIncoming( const QString& password )
{
    service->primaryAtChat()->chat
        ( "AT+CLCK=\"AC\",0,\"" + QAtUtils::quote( password ) + "\"",
          this, SLOT(unlockDone(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallBarring::unlockAllOutgoing( const QString& password )
{
    service->primaryAtChat()->chat
        ( "AT+CLCK=\"AG\",0,\"" + QAtUtils::quote( password ) + "\"",
          this, SLOT(unlockDone(bool,QAtResult)) );
}

/*!
    \reimp
*/
void QModemCallBarring::changeBarringPassword
        ( QCallBarring::BarringType type, const QString& oldPassword,
          const QString& newPassword )
{
    QString str = typeToString( type );
    if ( str.isEmpty() ) {
        // This type of barring operating is not supported,
        // so report an error immediately to the client.
        emit changeBarringPasswordResult
            ( QTelephony::OperationNotSupported );
    } else {
        service->primaryAtChat()->chat
                ( "AT+CPWD=\"" + str + "\",\"" +
                  QAtUtils::quote( oldPassword ) + "\",\"" +
                  QAtUtils::quote( newPassword ) + "\"",
                  this, SLOT(cpwdDone(bool,QAtResult)) );
    }
}

/*!
    Convert \a type into its two-letter 3GPP TS 27.007 string form.
    This function is virtual to allow for the possibility of modems
    that support more call barring types than those specified in
    3GPP TS 27.007.  Returns an empty string if \a type is not supported.

    \sa QCallBarring::BarringType
*/
QString QModemCallBarring::typeToString( QCallBarring::BarringType type ) const
{
    switch ( type ) {
        case QCallBarring::OutgoingAll:                     return "AO";
        case QCallBarring::OutgoingInternational:           return "OI";
        case QCallBarring::OutgoingInternationalExceptHome: return "OX";
        case QCallBarring::IncomingAll:                     return "AI";
        case QCallBarring::IncomingWhenRoaming:             return "IR";
        case QCallBarring::IncomingNonTA:                   return "NT";
        case QCallBarring::IncomingNonMT:                   return "NM";
        case QCallBarring::IncomingNonSIM:                  return "NS";
        case QCallBarring::IncomingNonMemory:               return "NA";
        case QCallBarring::AllBarringServices:              return "AB";
        case QCallBarring::AllOutgoingBarringServices:      return "AG";
        case QCallBarring::AllIncomingBarringServices:      return "AC";
    }
    return QString();
}

void QModemCallBarring::setDone( bool, const QAtResult& result )
{
    emit setBarringStatusResult( (QTelephony::Result)result.resultCode() );
}

void QModemCallBarring::requestDone( bool, const QAtResult& result )
{
    QCallBarring::BarringType type
        = ((QBarringUserData *)result.userData())->type;
    QTelephony::CallClass cls = QTelephony::CallClassNone;
    QAtResultParser parser( result );
    while ( parser.next( "+CLCK:" ) ) {
        uint status = parser.readNumeric();
        uint classx = parser.readNumeric();
        if ( status != 0 )
            cls = (QTelephony::CallClass)( cls | classx );
    }
    emit barringStatus( type, cls );
}

void QModemCallBarring::unlockDone( bool, const QAtResult& result )
{
    emit unlockResult( (QTelephony::Result)result.resultCode() );
}

void QModemCallBarring::cpwdDone( bool, const QAtResult& result )
{
    emit changeBarringPasswordResult
        ( (QTelephony::Result)result.resultCode() );
}
