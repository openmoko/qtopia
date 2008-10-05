/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <qmodemnetworkregistration.h>
#include <qmodemservice.h>
#include <qatchat.h>
#include <qatutils.h>
#include <qatresult.h>
#include <qatresultparser.h>

/*!
    \class QModemNetworkRegistration
    \inpublicgroup QtCellModule

    \brief The QModemNetworkRegistration class provides access to network registration features of AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+COPS} and \c{AT+CREG} commands from 3GPP TS 27.007.

    QModemNetworkRegistration implements the QNetworkRegistration telephony interface.
    Client applications should use QNetworkRegistration instead of this class to
    access the modem's network registration settings.

    \sa QNetworkRegistration
*/

class QModemNetworkRegistrationPrivate
{
public:
    QModemNetworkRegistrationPrivate( QModemService *service )
    {
        this->service = service;
        supportsOperatorTechnology = false;
        operatorId = -1;
    }

    QModemService *service;
    bool supportsOperatorTechnology;
    int operatorId;
};

/*!
    Construct an AT-based modem network registration object for \a service.
*/
QModemNetworkRegistration::QModemNetworkRegistration( QModemService *service )
    : QNetworkRegistrationServer( service->service(), service )
{
    d = new QModemNetworkRegistrationPrivate( service );
    service->primaryAtChat()->registerNotificationType
        ( "+CREG:", this, SLOT(cregNotify(QString)), true );
    service->connectToPost( "cfunDone", this, SLOT(cfunDone()) );
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );
}

/*!
    Destroy this AT-based modem network registration object.
*/
QModemNetworkRegistration::~QModemNetworkRegistration()
{
    delete d;
}

/*!
    Returns true if the modem supports the operator technology flag
    to the \c{AT+COPS} command; otherwise returns false.

    \sa setSupportsOperatorTechnology()
*/
bool QModemNetworkRegistration::supportsOperatorTechnology() const
{
    return d->supportsOperatorTechnology;
}

/*!
    Sets the flag to \a value that determines if the modem supports the
    operator technology flag on the \c{AT+COPS} command.

    Normally the value is detected automatically based on the response
    to the \c{AT+COPS?} or \c{AT+COPS=?} command.  This function can be
    used in a modem vendor plug-in to explicitly set the flag at start up.
    The default value, before auto-detection, is false.

    \sa supportsOperatorTechnology()
*/
void QModemNetworkRegistration::setSupportsOperatorTechnology( bool value )
{
    d->supportsOperatorTechnology = value;
}

class QSetOperatorUserData : public QAtResult::UserData
{
public:
    QSetOperatorUserData
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& name, const QString& technology )
    { this->mode = mode;
      this->id = id;
      this->name = name;
      this->technology = technology; }

    QTelephony::OperatorMode mode;
    QString id;
    QString name;
    QString technology;
};

/*!
    Sets the current network operator information to \a mode, \a id,
    and \a technology.  The server will respond with the
    setCurrentOperatorResult() signal when the request completes.

    The setCurrentOperatorCommand() method is used to create the
    AT command for changing the operator.
*/
void QModemNetworkRegistration::setCurrentOperator
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology )
{
    QString cmd = setCurrentOperatorCommand( mode, id, technology );
    d->service->secondaryAtChat()->chat
        ( cmd, this, SLOT(setDone(bool,QAtResult)),
          new QSetOperatorUserData
                ( mode, id, operatorNameForId( id ), technology ) );
}

/*!
    \reimp
*/
void QModemNetworkRegistration::requestAvailableOperators()
{
    d->service->secondaryAtChat()->chat
        ( "AT+COPS=?", this, SLOT(availDone(bool,QAtResult)) );
}

/*!
    Issue commands to reset the modem's network registration functionality.
    The default implementation issues \c{AT+CREG=2} and \c{AT+COPS=3,0}
    and then calls queryRegistration().

    This may be overridden for modems that need different commands to
    initialize network registration.  The queryRegistration() function
    must be called at the end of the modem-specific initialization.
*/
void QModemNetworkRegistration::resetModem()
{
    d->service->chat( "AT+CREG=2" );
    d->service->retryChat( "AT+COPS=3,0" );
    queryRegistration();
}

void QModemNetworkRegistration::cregNotify( const QString& msg )
{
    uint posn = 6;
    uint stat = QAtUtils::parseNumber( msg, posn );
    QString lac = QAtUtils::nextString( msg, posn );
    QString ci = QAtUtils::nextString( msg, posn );
    if ( !lac.isEmpty() && !ci.isEmpty() ) {
        // We have location information after the state value.
        updateRegistrationState( (QTelephony::RegistrationState)stat,
                                 lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
    } else {
        // We don't have any location information.
        updateRegistrationState( (QTelephony::RegistrationState)stat );
    }

    // Query for the operator name if home or roaming.
    if ( stat == 1 || stat == 5 )
        queryCurrentOperator();
}

void QModemNetworkRegistration::cregQuery( bool, const QAtResult& result )
{
    // Parse and update the registration state.
    QAtResultParser parser( result );
    parser.next( "+CREG:" );
    parser.readNumeric();  // Skip unsolicited result flag which is unused.
    uint stat = parser.readNumeric();
    QString lac = parser.readString();
    QString ci = parser.readString();
    if ( !lac.isEmpty() && !ci.isEmpty() ) {
        // We have location information after the state value.
        updateRegistrationState( (QTelephony::RegistrationState)stat,
                                 lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
    } else {
        // We don't have any location information.
        updateRegistrationState( (QTelephony::RegistrationState)stat );
    }

    // Once we do this, the modem is considered initialized.
    updateInitialized( true );

    // Query for the operator name if home or roaming.
    if ( stat == 1 || stat == 5 )
        queryCurrentOperator();
}

void QModemNetworkRegistration::copsDone( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    parser.next( "+COPS:" );
    uint mode = parser.readNumeric();
    uint format = parser.readNumeric();
    QString id, name;
    if ( format == 2 ) {
        name = QString::number( parser.readNumeric() );
        id = "2" + name;
    } else {
        name = parser.readString();
        if ( d->operatorId != -1 )
            id = "2" + QString::number( d->operatorId );
        else
            id = QString::number( format ) + name;
    }
    uint tech = parser.readNumeric();
    QTelephony::OperatorMode mmode;
    switch ( mode ) {
        default: case 0: mmode = QTelephony::OperatorModeAutomatic; break;
        case 1: mmode = QTelephony::OperatorModeManual; break;
        case 2: mmode = QTelephony::OperatorModeDeregister; break;
        case 4: mmode = QTelephony::OperatorModeManualAutomatic; break;
    }
    QString technology;
    switch ( tech ) {
        case 0: default:        technology = "GSM"; break;
        case 1:                 technology = "GSMCompact"; break;
        case 2:                 technology = "UTRAN"; break;
    }
    d->operatorId = -1;
    updateCurrentOperator( mmode, id, name, technology );
}

void QModemNetworkRegistration::copsNumericDone( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    parser.next( "+COPS:" );
    parser.readNumeric();       // Skip mode.
    uint format = parser.readNumeric();
    if ( format == 2 )
        d->operatorId = (int)( parser.readNumeric() );
    else
        d->operatorId = -1;
}

void QModemNetworkRegistration::setDone( bool ok, const QAtResult& result )
{
    if ( ok ) {
        QSetOperatorUserData *data = (QSetOperatorUserData *)result.userData();
        updateCurrentOperator( data->mode, data->id, data->name, data->technology );
        queryCurrentOperator();
    }
    emit setCurrentOperatorResult
            ( (QTelephony::Result)( result.resultCode() ) );
}

void QModemNetworkRegistration::availDone( bool, const QAtResult& result )
{
    QList<QNetworkRegistration::AvailableOperator> list;
    QAtResultParser parser( result );
    while ( parser.next( "+COPS:" ) ) {
        QList<QAtResultParser::Node> nodes = parser.readList();
        while ( !nodes.isEmpty() ) {
            parseAvailableOperator( list, nodes );
            nodes = parser.readList();
        }
    }
    emit availableOperators( list );
}

/*!
    Query the current network registration state with \c{AT+CREG?}.
    This is typically used at the end of the initialization process
    that is performed in resetModem().
*/
void QModemNetworkRegistration::queryRegistration()
{
    d->service->primaryAtChat()->chat
        ( "AT+CREG?", this, SLOT(cregQuery(bool,QAtResult)) );
}

void QModemNetworkRegistration::cfunDone()
{
    queryCurrentOperator();
}

/*!
    Returns the AT command to use to effect a setCurrentOperator()
    request for \a mode, \a id, and \a technology.

    This implementation formats a \c{AT+COPS} command according to
    3GPP TS 27.007.  That is, \c{AT+COPS=mode,[format[,oper[,AcT]]]}.
    The \c{AcT} field is only present if supportsOperatorTechnology()
    returns true.  Modem vendor plug-ins should override this method
    if the syntax of their modem's \c{AT+COPS} command is different.

    \sa setCurrentOperator()
*/
QString QModemNetworkRegistration::setCurrentOperatorCommand
        ( QTelephony::OperatorMode mode, const QString& id,
          const QString& technology )
{
    QString cmd = "AT+COPS=";                       // No tr
    switch ( mode ) {
        case QTelephony::OperatorModeAutomatic:         cmd += "0"; break;
        case QTelephony::OperatorModeManual:            cmd += "1"; break;
        case QTelephony::OperatorModeDeregister:        cmd += "2"; break;
        case QTelephony::OperatorModeManualAutomatic:   cmd += "4"; break;
    }
    QString name = operatorNameForId( id );
    if ( mode == QTelephony::OperatorModeManual ||
         mode == QTelephony::OperatorModeManualAutomatic ) {
        if ( id.startsWith( "2" ) ) {
            // Numeric operator identifier.
            cmd += ",2," + name;
        } else {
            // Short or long operator identifier.
            cmd += "," + id.left(1) + ",\"" + QAtUtils::quote( name ) + "\"";
        }
        if ( supportsOperatorTechnology() ) {
            if ( technology == "GSM" )             // No tr
                cmd += ",0";
            else if ( technology == "GSMCompact" ) // No tr
                cmd += ",1";
            else if ( technology == "UTRAN" )      // No tr
                cmd += ",2";
        }
    }
    return cmd;
}

/*!
    Convert an operator identifier \a id into the corresponding operator name.
*/
QString QModemNetworkRegistration::operatorNameForId( const QString& id )
{
    if ( id.length() > 0 )
        return id.mid(1);
    else
        return QString();
}

void QModemNetworkRegistration::parseAvailableOperator
            ( QList<QNetworkRegistration::AvailableOperator>&list,
              const QList<QAtResultParser::Node>& values )
{
    QNetworkRegistration::AvailableOperator op;
    if ( values.size() < 4 )
        return;     // Invalid operator definition.
    if ( !values[0].isNumber() )
        return;     // Invalid mode value.
    if ( !values[1].isString() )
        return;     // Invalid long alphanumeric value.
    if ( !values[2].isString() )
        return;     // Invalid short alphanumeric value.
    if ( !values[3].isString() )
        return;     // Invalid numeric operator value.
    op.availability =
        (QTelephony::OperatorAvailability)( values[0].asNumber() );
    op.name = values[1].asString();
    if ( op.name.isEmpty() )
        op.name = values[2].asString();
    if ( op.name.isEmpty() )
        op.name = values[3].asString();
    op.shortName = values[2].asString();
    if ( op.shortName.isEmpty() )
        op.shortName = op.name;
    QString id = values[3].asString();
    if ( !id.isEmpty() ) {
        op.id = "2" + id;
    } else {
        id = values[2].asString();
        if ( !id.isEmpty() ) {
            op.id = "1" + id;
        } else {
            op.id = "0" + values[1].asString();
        }
    }
    op.technology = "GSM";      // No tr
    if ( values.size() > 4 && values[4].isNumber() ) {
        uint tech = values[4].asNumber();
        if ( tech == 1 )
            op.technology = "GSMCompact";       // No tr
        else if ( tech == 2 )
            op.technology = "UTRAN";            // No tr
        else if ( tech != 0 )
            op.technology = QString();          // Unknown technology.

        // Remember that we saw operator technology information so that
        // when a setCurrentOperator() request arrives, we can add the
        // required extra field to the AT+COPS command.
        d->supportsOperatorTechnology = true;
    }
    list += op;
}

void QModemNetworkRegistration::queryCurrentOperator()
{
    // We want both the numeric and the alphanumeric versions of
    // the operator names, so query for both.
    d->operatorId = -1;
    d->service->secondaryAtChat()->chat( "AT+COPS=3,2" );
    d->service->secondaryAtChat()->chat
        ( "AT+COPS?", this, SLOT(copsNumericDone(bool,QAtResult)) );
    d->service->secondaryAtChat()->chat( "AT+COPS=3,0" );
    d->service->secondaryAtChat()->chat
        ( "AT+COPS?", this, SLOT(copsDone(bool,QAtResult)) );
}
