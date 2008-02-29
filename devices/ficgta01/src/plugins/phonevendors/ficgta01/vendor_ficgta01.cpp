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

#include "vendor_ficgta01_p.h"
#include <qmodemindicators.h>
#include <qatutils.h>
#include <qatresultparser.h>
#include <QProcess>
#include <QTimer>
#include <stdio.h>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>

#include <alsa/asoundlib.h>

#include <qmodemcallvolume.h>
#include <qmodemsiminfo.h>

static bool supportsStk = false;

Ficgta01CallProvider::Ficgta01CallProvider( QModemService *service )
    : QModemCallProvider( service )
{
    service->primaryAtChat()->registerNotificationType
        ( "%CPI:", this, SLOT(cpiNotification(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "%CNAP:", this, SLOT(cnapNotification(QString)) );
}

Ficgta01CallProvider::~Ficgta01CallProvider()
{
}

QModemCallProvider::AtdBehavior Ficgta01CallProvider::atdBehavior() const
{
    // When ATD reports OK, it indicates that it is back in command
    // mode and a %CPI notification will indicate when we are connected.
    return AtdOkIsDialingWithStatus;
}

void Ficgta01CallProvider::abortDial( uint id, QPhoneCall::Scope scope )
{
    // Use the ATH command to abort outgoing calls, instead of AT+CHLD=1.
    //atchat()->chat( "ATH" );

    // Use default behaviour of CR followed by AT+CHLD - seems to work better.
    QModemCallProvider::abortDial( id, scope );
}

void Ficgta01CallProvider::cpiNotification( const QString& msg )
{
    // Call progress notification for the FICGTA01 device.
    // %CPI: <id>,<msgType>,<ibt>,<tch>,<dir>,<mode>,<number>,<ton>,<alpha>
    // where <id> is the call identifier, and <msgType> is one of:
    // 0 = SETUP, 1 = DISCONNECT, 2 = ALERT, 3 = PROCEED,
    // 4 = SYNCHRONIZATION, 5 = PROGRESS, 6 = CONNECTED,
    // 7 = RELEASE, 8 = REJECT
    uint posn = 5;
    uint identifier = QAtUtils::parseNumber( msg, posn );

    uint status = QAtUtils::parseNumber( msg, posn );
    QModemCall *call = callForIdentifier( identifier );

    if ( status == 6 && call &&
         ( call->state() == QPhoneCall::Dialing ||
           call->state() == QPhoneCall::Alerting ) ) {

        // This is an indication that a "Dialing" connection
        // is now in the "Connected" state.
        call->setConnected();

    } else if ( ( status == 1 || status == 7 ) && call &&
                ( call->state() == QPhoneCall::Dialing ||
                  call->state() == QPhoneCall::Alerting ) ) {

        // We never managed to connect.
        hangupRemote( call );

    } else if ( status == 2 && call &&
                call->state() == QPhoneCall::Dialing ) {

        // Call is moving from Dialing to Alerting.
        call->setState( QPhoneCall::Alerting );

    } else if ( ( status == 1 || status == 7 ) && call &&
                ( call->state() == QPhoneCall::Connected ||
                  call->state() == QPhoneCall::Hold ) ) {

        // This is an indication that the connection has been lost.
        hangupRemote( call );

    } else if ( ( status == 1 || status == 7 ) && call &&
                call->state() == QPhoneCall::Incoming ) {

        // This is an indication that an incoming call was missed.
        call->setState( QPhoneCall::Missed );

    } else if ( ( status == 2 || status == 4 ) && !call ) {

        // This is a newly waiting call.  Treat it the same as "RING".
        QAtUtils::skipField( msg, posn );
        QAtUtils::skipField( msg, posn );
        QAtUtils::skipField( msg, posn );
        uint mode = QAtUtils::parseNumber( msg, posn );
        QString callType;
        if ( mode == 1 || mode == 6 || mode == 7 )
            callType = "Data";  // No tr
        else if ( mode == 2 || mode == 8 )
            callType = "Fax";   // No tr
        else
            callType = "Voice"; // No tr
        QString number = QAtUtils::nextString( msg, posn );
        uint type = QAtUtils::parseNumber( msg, posn );
        ringing( QAtUtils::decodeNumber( number, type ), callType, identifier );

    }
}

void Ficgta01CallProvider::cnapNotification( const QString& msg )
{
    // Calling name presentation from the network.
    uint posn = 6;
    QAtUtils::skipField( msg, posn );	    // pres_mode
    QAtUtils::skipField( msg, posn );	    // dcs
    QAtUtils::skipField( msg, posn );	    // name_length
    QString name = QAtUtils::nextString( msg, posn );
    QModemCall *call = incomingCall();
    if ( call )
        call->emitNotification( QPhoneCall::CallingName, name );
}

Ficgta01SimToolkit::Ficgta01SimToolkit( QModemService *service )
    : QModemSimToolkit( service )
{
    supportsStk = false;
    lastCommand.setType( QSimCommand::NoCommand );
    mainMenu = lastCommand;
    lastResponseWasExit = false;

    service->primaryAtChat()->registerNotificationType
        ( "%SATA:", this, SLOT(sataNotification(QString)) );
    service->primaryAtChat()->registerNotificationType
	( "%SATN:", this, SLOT(satnNotification(QString)) );
}

Ficgta01SimToolkit::~Ficgta01SimToolkit()
{
}

void Ficgta01SimToolkit::initialize()
{
    // We don't need to do anything here, because SIM toolkit initialization
    // happens during the detection code.
    QModemSimToolkit::initialize();
}

void Ficgta01SimToolkit::begin()
{
    if ( !supportsStk ) {

	// SIM toolkit functionality is not available.
	emit beginFailed();

    } else if ( lastCommand.type() == QSimCommand::SetupMenu ) {

	// We just fetched the main menu, so return what we fetched.
	emit command( lastCommand );

    } else if ( mainMenu.type() == QSimCommand::SetupMenu ) {

	// We have a cached main menu from a previous invocation.
	lastCommand = mainMenu;
	lastCommandBytes = mainMenuBytes;
	emit command( mainMenu );

    } else {

	// The SIM toolkit is in an unknown state, so we cannot proceed.
	// If the FICGTA01 could perform a proper STK reset, we might have
	// been able to do something.
	emit beginFailed();

    }
}

void Ficgta01SimToolkit::sendResponse( const QSimTerminalResponse& resp )
{
    if ( resp.command().type() == QSimCommand::SelectItem &&
         resp.result() == QSimTerminalResponse::BackwardMove ) {
        lastResponseWasExit = true;
    } else {
        lastResponseWasExit = false;
    }
    service()->primaryAtChat()->chat
        ( "AT%SATR=\"" + QAtUtils::toHex( resp.toPdu() ) + "\"" );
}

void Ficgta01SimToolkit::sendEnvelope( const QSimEnvelope& env )
{
    service()->primaryAtChat()->chat
        ( "AT%SATE=\"" + QAtUtils::toHex( env.toPdu() ) + "\"" );
}

void Ficgta01SimToolkit::sataNotification( const QString& msg )
{
    // SIM toolkit command indication.
    QByteArray bytes = QAtUtils::fromHex( msg.mid(6) );
    if ( bytes.size() > 0 ) {

        lastCommandBytes = bytes;
        lastCommand = QSimCommand::fromPdu( bytes );
        if ( lastCommand.type() == QSimCommand::SetupMenu ) {
            // Cache the main menu, because we won't get it again!
            mainMenuBytes = bytes;
            mainMenu = lastCommand;
        }
        qLog(AtChat)<< "SIM command of type" << (int)(lastCommand.type());
        emitCommandAndRespond( lastCommand );

    } else if ( lastResponseWasExit &&
                mainMenu.type() == QSimCommand::SetupMenu ) {

        // We exited from a submenu and we got an empty "%SATA"
        // response.  This is the FICGTA01's way of telling us that we
        // now need to display the main menu.  It would have been
        // better if the FICGTA01 resent the menu to us itself.
        lastCommandBytes = mainMenuBytes;
        lastCommand = mainMenu;
        qLog(AtChat)<< "Simulating SIM command of type"<< (int)(lastCommand.type());
        emit command( lastCommand );

    }
}

void Ficgta01SimToolkit::satnNotification( const QString& )
{
    // Nothing to do here at present.  Just ignore the %SATN notifications.
}

Ficgta01PhoneBook::Ficgta01PhoneBook( QModemService *service )
    : QModemPhoneBook( service )
{
    qLog(AtChat)<<"Ficgta01PhoneBook::Ficgta01PhoneBook";
    // Turn on status notification messages for finding out when
    // the phone book is ready to use.
    service->primaryAtChat()->registerNotificationType
        ( "%CSTAT:", this, SLOT(cstatNotification(QString)) );
    service->primaryAtChat()->chat( "AT%CSTAT=1" );
}

Ficgta01PhoneBook::~Ficgta01PhoneBook()
{
}

bool Ficgta01PhoneBook::hasModemPhoneBookCache() const
{
    // TODO: make this return true once we know what %CSTAT looks like.
    return false;
}

bool Ficgta01PhoneBook::hasEmptyPhoneBookIndex() const
{
    return true;
}

void Ficgta01PhoneBook::cstatNotification( const QString& msg )
{
    uint statusPosn = 0;
    uint entityPosn = 8;
    QString entity = msg.mid( 8, 3);

    if( entity == "PHB") {
// phonebook

        uint status = msg.mid(13).toInt();
        if(status == 0) {

        }

    } else if (entity == "RDY" ) {
    } else if (entity == "SMS") {
    }

    // "%CSTAT: PHB, 0"
    // %CSTAT: <entity>,<status>
    // PHB (phone book)
    // SMS
    // RDY (Ready when both PHB and SMS have reported they are ready)
    //
    //
}

Ficgta01PinManager::Ficgta01PinManager( QModemService *service )
    : QModemPinManager( service )
{
}

Ficgta01PinManager::~Ficgta01PinManager()
{
}

bool Ficgta01PinManager::emptyPinIsReady() const
{
    return true;
}


// Known bands, by mask.
typedef struct
{
    const char *name;
    int         value;

} BandInfo;
static BandInfo const bandInfo[] = {
    {"GSM 900",             1},
    {"DCS 1800",            2},
    {"PCS 1900",            4},
    {"E-GSM",               8},
    {"GSM 850",             16},
    {"Tripleband 900/1800/1900", 15},
};
#define numBands    ((int)(sizeof(bandInfo) / sizeof(BandInfo)))

Ficgta01BandSelection::Ficgta01BandSelection( QModemService *service )
    : QBandSelection( service->service(), service, Server )
{
    this->service = service;
}

Ficgta01BandSelection::~Ficgta01BandSelection()
{
}

void Ficgta01BandSelection::requestBand()
{
    service->primaryAtChat()->chat
        ( "AT%BAND?", this, SLOT(bandQuery(bool,QAtResult)) );
}

void Ficgta01BandSelection::requestBands()
{
    QStringList list;
    for ( int index = 0; index < numBands; ++index ) {
        list += QString( bandInfo[index].name );
    }
    emit bands( list );

//     service->primaryAtChat()->chat
//         ( "AT%BAND=?", this, SLOT(bandList(bool,QAtResult)) );
}

void Ficgta01BandSelection::setBand( QBandSelection::BandMode mode, const QString& value )
{
    if ( mode == Automatic ) {
        service->primaryAtChat()->chat
            ( "AT%BAND=0", this, SLOT(bandSet(bool,QAtResult)) );
    } else {
        int bandValue = 0;
        QStringList names = value.split(", ");
        foreach ( QString name, names ) {
            bool seen = false;
            for ( int index = 0; index < numBands; ++index ) {
                if ( name == bandInfo[index].name ) {
                    bandValue |= bandInfo[index].value;
                    seen = true;
                    break;
                }
            }
            if ( !seen ) {
                // The band name is not valid.
                emit setBandResult( QTelephony::OperationNotSupported );
                return;
            }
        }
        if ( !bandValue ) {
            // No band names supplied.
            emit setBandResult( QTelephony::OperationNotSupported );
            return;
        }
        service->primaryAtChat()->chat
            ( "AT%BAND=1," + QString::number( bandValue ),
              this, SLOT(bandSet(bool,QAtResult)) );
    }
}

// Convert a band value into a name.  Returns an empty list if unknown.
static QStringList bandValueToName( int bandValue )
{
    QStringList bands;
    for ( int index = 0; index < numBands; ++index ) {
        if ( ( bandValue & bandInfo[index].value ) == bandInfo[index].value ) {
            bandValue &= ~bandInfo[index].value;
            bands += QString( bandInfo[index].name );
        }
    }
    return bands;
}

void Ficgta01BandSelection::bandQuery( bool, const QAtResult& result )
{

    QAtResultParser parser( result );
    int bandValue;
        qLog(AtChat)<<"bandQuery";
    if ( parser.next( "%BAND:" ) ) {
        bandValue = (int)parser.readNumeric();
    } else {
        // Command did not work, so assume "Auto".
        bandValue = 4;
    }
    for ( int index = 0; index < numBands; ++index ) {
        if ( bandValue == bandInfo[index].value ) {
            emit band( Manual, bandInfo[index].name );
            return;
        }
    }
    emit band( Automatic, QString() );



//     QAtResultParser parser( result );
//     int bandValue;
//     qLog(AtChat)<<"bandQuery";
//     if ( parser.next( "%BAND:" ) ) {
//         if ( parser.readNumeric() != 0 ) {
//             bandValue = (int)parser.readNumeric();
//             QStringList bands = bandValueToName( bandValue );
//             if ( bands.size() > 0 ) {
//                 emit band( Manual, bands.join(", ") );
//                 return;
//             }

//         }
//     }

//     emit band( Automatic, QString() );

}

void Ficgta01BandSelection::bandList( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    QStringList bandNames;
    if ( parser.next( "%BAND:" ) ) {

        parser.readList();  // Skip list of supported modes.
        QList<QAtResultParser::Node> list = parser.readList();
        foreach ( QAtResultParser::Node node, list ) {

            if ( node.isNumber() ) {
                bandNames += bandValueToName( (int)node.asNumber() );
                qLog(AtChat)<<  (int)node.asNumber();
            } else if ( node.isRange() ) {
                int first = (int)node.asFirst();
                int last = (int)node.asLast();
                qLog(AtChat)<<"isRange"<<first<<last;
                while ( first <= last ) {
                    qLog(AtChat)<< bandValueToName( first ) << first;
                    bandNames += bandValueToName( first ).join(" | ");
                    ++first;
                }
            }
        }
    }
     emit bands( bandNames );
}

void Ficgta01BandSelection::bandSet( bool, const QAtResult& result )
{
    emit setBandResult( (QTelephony::Result)result.resultCode() );
}

Ficgta01ModemService::Ficgta01ModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
{

    connect( this, SIGNAL(resetModem()), this, SLOT(reset()) );


    // Turn on dynamic signal quality notifications.
    // Register for "%CSQ" notifications to get signal quality updates.
     primaryAtChat()->registerNotificationType
         ( "%CSQ:", this, SLOT(csq(QString)), true );
     chat("AT%CSQ=1");


     // Turn on SIM toolkit support in the modem.  This must be done
    // very early in the process, to ensure that it happens before
    // the first AT+CFUN command.
    chat( "AT%SATC=1,\"FFFFFFFFFF\"", this, SLOT(configureDone(bool)) );

    // Enable %CPRI for ciphering indications.
//    chat( "AT%CPRI=1" );

    // Make the modem send unsolicited reports at any time
    // the "user is not typing".  i.e. don't intersperse unsolicited
    // notifications and command echo as it will confuse QAtChat.
    chat( "AT%CUNS=1" );

    // Enable the reporting of timezone and local time information.
     primaryAtChat()->registerNotificationType
          ( "%CTZV:", this, SLOT(ctzv(QString)), true );
     chat( "AT%CTZV=1" );


// Turn on call progress indications, with phone number information.
    chat( "AT%CPI=2" );

// blah
    chat("AT+CFUN=1");
}

Ficgta01ModemService::~Ficgta01ModemService()
{
}

void Ficgta01ModemService::initialize()
{
#if 0
    if ( !supports<QSimToolkit>() )
        addInterface( new Ficgta01SimToolkit( this ) );
#endif

    if ( !supports<QPhoneBook>() )
        addInterface( new Ficgta01PhoneBook( this ) );

    if ( !supports<QPinManager>() )
        addInterface( new Ficgta01PinManager( this ) );


    if ( !supports<QBandSelection>() )
        addInterface( new Ficgta01BandSelection( this ) );

    if ( !supports<QSimInfo>() )
        addInterface( new Ficgta01SimInfo( this ) );

    if ( !callProvider() )
        setCallProvider( new Ficgta01CallProvider( this ) );

   if ( !supports<QVibrateAccessory>() )
        addInterface( new Ficgta01VibrateAccessory( this ) );

    if ( !supports<QCallVolume>() )
        addInterface( new Ficgta01CallVolume(this));


   QModemService::initialize();
}

void Ficgta01ModemService::csq( const QString& msg )
{
    // Automatic signal quality update, in the range 0-31.
    uint posn = 6;
    uint rssi = QAtUtils::parseNumber( msg, posn );
    indicators()->setSignalQuality( (int)rssi, 31 );
}



void Ficgta01ModemService::ctzv( const QString& msg )
{
    // Process a %CTZV notification from the modem.
    // TODO: format on this is ->"yy/mm/dd,hh:mm:ss+/-tz"

    qLog(AtChat)<<"ctzv"<<msg;


//    Q_UNUSED(msg);
}

void Ficgta01ModemService::configureDone( bool ok )
{
    supportsStk = ok;
}

 void Ficgta01ModemService::reset()
 {
      qLog(AtChat)<<" Ficgta01ModemService::reset()";

//     // Turn on "%CNAP" notifications, which supply the caller's
//     // name on an call.  Only supported on some networks.
//     chat( "AT%CNAP=1" );

      chat("AT%NRG=0"); //force auto operations
//      chat("AT%COPS=0");

 }

void Ficgta01ModemService::sendSuspendDone()
{
    //   QtopiaChannel::send("QPE/GreenphoneModem", "sleep()");
    suspendDone();
}

void Ficgta01ModemService::suspend()
{
    qLog(AtChat)<<" Ficgta01ModemService::suspend()";
    // Turn off cell id information on +CREG and +CGREG as it will
    // cause unnecessary wakeups when moving between cells.
    chat( "AT+CREG=1" );
    chat( "AT+CGREG=1" );

    chat( "AT%CREG=1" );
    chat( "AT%CGREG=1" );

    // Turn off cell broadcast location messages.
//     chat( "AT%CSQ=1" );

    // Turn off signal quality notifications while the system is suspended.
     QTimer::singleShot( 500, this, SLOT(sendSuspendDone()) );
}

void Ficgta01ModemService::wake()
{
    qLog(AtChat)<<" Ficgta01ModemService::wake()";

//reset modem

//    chat( "AT%CWUP=1" );

    // Turn cell id information back on.
    chat( "AT+CREG=2" );
    chat( "AT+CGREG=2" );

    // Turn cell broadcast location messages back on again.

    // Re-enable signal quality notifications when the system wakes up again.
   // Turn on dynamic signal quality notifications.
    chat( "AT%CSQ=1" );

    wakeDone();
}

 Ficgta01VibrateAccessory::Ficgta01VibrateAccessory
        ( QModemService *service )
    : QVibrateAccessoryProvider( service->service(), service )
{
    setSupportsVibrateOnRing( true );
    setSupportsVibrateNow( false );
}

Ficgta01VibrateAccessory::~Ficgta01VibrateAccessory()
{
}

void Ficgta01VibrateAccessory::setVibrateOnRing( const bool value )
{
    qLog(AtChat)<<"setVibrateOnRing";
    setVibrateNow(value);
}

void Ficgta01VibrateAccessory::setVibrateNow( const bool value )
{
    qLog(AtChat)<<"setVibrateNow";
    int result;
    QString cmd;
    if ( value ) { //turn on
        QFile trigger( "/sys/class/leds/gta01:vibrator/trigger");
        trigger.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out(&trigger);
        out<<"timer";
        trigger.close();

        QFile delayOn( "/sys/class/leds/gta01:vibrator/delay_on");
        delayOn.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out1(&delayOn);
        out1<<"500";
        delayOn.close();

        QFile delayOff("/sys/class/leds/gta01:vibrator/delay_off");
        delayOff.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out2(&delayOff);
        out2<<"1000";
        delayOff.close();

    } else { //turn off
        QFile trigger( "/sys/class/leds/gta01:vibrator/trigger");
        trigger.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out(&trigger);
        out<<"none";
        trigger.close();
    }

    QVibrateAccessoryProvider::setVibrateNow( value );
}


Ficgta01CallVolume::Ficgta01CallVolume(Ficgta01ModemService *service)
    : QModemCallVolume(service)
{
   this->service = service;


    QtopiaIpcAdaptor *adaptor
            = new QtopiaIpcAdaptor( "QPE/Ficgta01Modem", this );

    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setSpeakerVolumeRange(int, int)),
              this, SLOT(setSpeakerVolumeRange(int,int)) );
    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setMicVolumeRange(int, int)),
              this, SLOT(setMicVolumeRange(int,int)) );

    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setOutputVolume(int)),
              this, SLOT(setSpeakerVolume(int)) );
    QtopiaIpcAdaptor::connect
            ( adaptor, MESSAGE(setMicVolume(int)),
              this, SLOT(setMicrophoneVolume(int)) );

}

Ficgta01CallVolume::~Ficgta01CallVolume()
{

}

bool Ficgta01CallVolume::hasDelayedInit() const
{
    return true;
}

void Ficgta01CallVolume::setSpeakerVolume( int volume )
{
    int boundedVolume = qBound(value("MinimumSpeakerVolume").toInt(), volume,
                               value("MaximumSpeakerVolume").toInt());

    setValue( "SpeakerVolume", boundedVolume );
    emit speakerVolumeChanged(boundedVolume);
}

void Ficgta01CallVolume::setMicrophoneVolume( int volume )
{
    int boundedVolume = qBound(value("MinimumMicrophoneVolume").toInt(), volume,
                               value("MaximumMicrophoneVolume").toInt());

    setValue( "MicrophoneVolume", boundedVolume );
    emit microphoneVolumeChanged(boundedVolume);
}


void Ficgta01CallVolume::setSpeakerVolumeRange(int min,int max)
{
    setValue( "MinimumSpeakerVolume", min );
    setValue( "MaximumSpeakerVolume", max );
}

void Ficgta01CallVolume::setMicVolumeRange(int min,int max)
{
    setValue( "MinimumMicrophoneVolume", min );
    setValue( "MaximumMicrophoneVolume", max );
}


// Number of milliseconds between polling attempts on AT+CIMI command.
#ifndef CIMI_TIMEOUT
#define CIMI_TIMEOUT    2000
#endif

class Ficgta01SimInfoPrivate
{
public:
    Ficgta01ModemService *service;
    QTimer *checkTimer;
    int count;
};

Ficgta01SimInfo::Ficgta01SimInfo( Ficgta01ModemService *service )
    : QSimInfo( service->service(), service, QCommInterface::Server )
{
    d = new Ficgta01SimInfoPrivate();
    d->service = service;
    d->checkTimer = new QTimer( this );
    d->checkTimer->setSingleShot( true );
    connect( d->checkTimer, SIGNAL(timeout()), this, SLOT(requestIdentity()) );
    d->count = 0;

    connect( service, SIGNAL(simInserted()), this, SLOT(simInserted()) );
    connect( service, SIGNAL(simRemoved()), this, SLOT(simRemoved()) );

    // Perform an initial AT+CIMI request to get the SIM identity.
    QTimer::singleShot( 0, this, SLOT(requestIdentity()) );
}

Ficgta01SimInfo::~Ficgta01SimInfo()
{
    delete d;
}

void Ficgta01SimInfo::simInserted()
{
    if ( !d->checkTimer->isActive() )
        requestIdentity();
}

void Ficgta01SimInfo::simRemoved()
{
    setIdentity( QString() );
}

void Ficgta01SimInfo::requestIdentity()
{
    d->service->primaryAtChat()->chat
        ( "AT+CIMI", this, SLOT(cimi(bool,QAtResult)) );
}

void Ficgta01SimInfo::cimi( bool ok, const QAtResult& result )
{
    QString id = extractIdentity( result.content().trimmed() );
    if ( ok && !id.isEmpty() ) {
        // We have a valid SIM identity.
        setIdentity( id );
    } else {
        // No SIM identity, so poll again in a few seconds for the first two minutes.
        setIdentity( QString() );

        if ( d->count < 120000/CIMI_TIMEOUT ) {
            d->checkTimer->start( CIMI_TIMEOUT );
            d->count++;
        } else {
            d->count = 0;
            // post a message to modem service to stop SIM PIN polling
            d->service->post( "simnotinserted" );
            emit notInserted();
        }
        // If we got a definite "not inserted" or "sim failure" error,
        // then emit notInserted().
        if ( result.resultCode() == QAtResult::SimNotInserted
          || result.resultCode() == QAtResult::SimFailure)
            emit notInserted();
    }
}

// Extract the identity information from the content of an AT+CIMI response.
// It is possible that we got multiple lines, including some unsolicited
// notifications from the modem that are not yet recognised.  Skip over
// such garbage and find the actual identity.
QString Ficgta01SimInfo::extractIdentity( const QString& content )
{
    QStringList lines = content.split( QChar('\n') );
    foreach ( QString line, lines ) {
        if ( line.length() > 0 ) {
            uint ch = line[0].unicode();
            if ( ch >= '0' && ch <= '9' )
                return line;
        }
    }
    return QString();
}
