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

#include <qmodemsimgenericaccess.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>

/*!
    \class QModemSimGenericAccess
    \mainclass
    \brief The QModemSimGenericAccess class implements SIM generic access for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CSIM} command from 3GPP TS 27.007
    to access the SIM.

    QModemSimGenericAccess implements the QSimGenericAccess telephony interface.
    Client applications should use QSimGenericAccess instead of this class to
    send generic SIM commands.

    \sa QSimGenericAccess
*/

/*!
    Construct a new modem-based SIM generic access object for \a service.
*/
QModemSimGenericAccess::QModemSimGenericAccess( QModemService *service )
    : QSimGenericAccess( service->service(), service, Server )
{
    this->service = service;
}

/*!
    Destroy this modem-based SIM generic access object.
*/
QModemSimGenericAccess::~QModemSimGenericAccess()
{
}

class QCSimUserData : public QAtResult::UserData
{
public:
    QCSimUserData( const QString& reqid )
    { this->reqid = reqid; }

    QString reqid;
};

/*!
    \reimp
*/
void QModemSimGenericAccess::command
        ( const QString& reqid, const QByteArray& data )
{
    QString cmd = "AT+CSIM=" + QString::number( data.size() * 2 ) + "," +
                  QAtUtils::toHex( data );
    service->chat( cmd, this, SLOT(csim(bool,QAtResult)),
                   new QCSimUserData( reqid ) );
}

void QModemSimGenericAccess::csim( bool ok, const QAtResult& result )
{
    QByteArray data;
    QString reqid = ((QCSimUserData *)result.userData())->reqid;
    if ( ok ) {
        QAtResultParser parser( result );
        if ( parser.next( "+CSIM:" ) ) {
            uint posn = 0;
            QString line = parser.line();
            QAtUtils::parseNumber( line, posn );    // Skip length.
            if ( ((int)posn) < line.length() && line[posn] == ',' )
                ++posn;
            data = QAtUtils::fromHex( line.mid( (int)posn ) );
        }
    }
    emit response( reqid, (QTelephony::Result)result.resultCode(), data );
}
