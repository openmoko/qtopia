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
#include <QSettings>

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

    setUseMissedTimer(false);
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
    return true;
}

bool Ficgta01PhoneBook::hasEmptyPhoneBookIndex() const
{
    return true;
}

void Ficgta01PhoneBook::cstatNotification( const QString& msg )
{
    QString entity = msg.mid( 8, 3);

    if( entity == "PHB") {
// phonebook

        uint status = msg.mid(13).toInt();
        if(status == 1) {
            phoneBooksReady();
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
/*    {"GSM 850",             16}, */
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

    // Register a wakeup command to ping the modem if we haven't
    // sent anything during the last 5 seconds.  This command may
    // not get a response, but the modem should then become responsive
    // to the next command that is sent afterwards.
    primaryAtChat()->registerWakeupCommand( "ATE0", 5000 );

    // Turn on dynamic signal quality notifications.
    // Register for "%CSQ" notifications to get signal quality updates.
     primaryAtChat()->registerNotificationType
         ( "%CSQ:", this, SLOT(csq(QString)) );
     chat("AT%CSQ=1");
     QTimer::singleShot( 2500, this, SLOT(firstCsqQuery()) );


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

    chat("AT+COPS=0");


}

Ficgta01ModemService::~Ficgta01ModemService()
{
}

void Ficgta01ModemService::initialize()
{
    if ( !supports<QPinManager>() )
        addInterface( new Ficgta01PinManager( this ) );

    if ( !supports<QPhoneBook>() )
        addInterface( new Ficgta01PhoneBook( this ) );


    if ( !supports<QBandSelection>() )
        addInterface( new Ficgta01BandSelection( this ) );

    if ( !supports<QSimInfo>() ) {
        QModemSimInfo* simInfo = new QModemSimInfo( this );
        simInfo->setSimNotInsertedReason( QModemSimInfo::Reason_SimFailure );
        addInterface( simInfo );
    }

    if ( !callProvider() )
        setCallProvider( new Ficgta01CallProvider( this ) );

   if ( !supports<QVibrateAccessory>() )
        addInterface( new Ficgta01VibrateAccessory( this ) );

    if ( !supports<QCallVolume>() )
        addInterface( new Ficgta01CallVolume(this));

    if ( !supports<QPreferredNetworkOperators>() )
        addInterface( new Ficgta01PreferredNetworkOperators(this));


   QModemService::initialize();
}

void Ficgta01ModemService::csq( const QString& msg )
{
    // Automatic signal quality update, in the range 0-31.
    if ( msg.contains( QChar(',') ) ) {
        uint posn = 6;
        uint rssi = QAtUtils::parseNumber( msg, posn );
        indicators()->setSignalQuality( (int)rssi, 31 );
    }
}

void Ficgta01ModemService::firstCsqQuery()
{
    // Perform an initial AT%CSQ? which should cause the modem to
    // respond with a %CSQ notification.  This is needed to shut
    // off AT+CSQ polling in qmodemindicators.cpp when the modem is
    // quiet and not sending periodic %CSQ notifications at startup.
    chat( "AT%CSQ?" );
}

void Ficgta01ModemService::ctzv( const QString& msg )
{
    // Timezone information from the network.  Format is "yy/mm/dd,hh:mm:ss+/-tz".
    // There is no dst indicator according to the spec, but we parse an extra
    // argument just in case future modem firmware versions fix this oversight.
    // If there is no extra argument, the default value of zero will be used.
    uint posn = 7;
    QString time = QAtUtils::nextString( msg, posn );
    int dst = ((int)QAtUtils::parseNumber( msg, posn )) * 60;
    int zoneIndex = time.length();
    while ( zoneIndex > 0 && time[zoneIndex - 1] != QChar('-') &&
            time[zoneIndex - 1] != QChar('+') )
        --zoneIndex;
    int zoneOffset;
    if ( zoneIndex > 0 && time[zoneIndex - 1] == QChar('-') ) {
        zoneOffset = time.mid(zoneIndex - 1).toInt() * 15;
    } else if ( zoneIndex > 0 && time[zoneIndex - 1] == QChar('+') ) {
        zoneOffset = time.mid(zoneIndex).toInt() * 15;
    } else {
        // Unknown timezone information.
        return;
    }
    QString timeString;
    if (zoneIndex > 0)
        timeString = time.mid(0, zoneIndex - 1);
    else
        timeString = time;
    QDateTime t = QDateTime::fromString(timeString, "yy/MM/dd,HH:mm:ss");
    if (!t.isValid())
        t = QDateTime::fromString(timeString, "yyyy/MM/dd,HH:mm:ss"); // Just in case.
    QDateTime utc = QDateTime(t.date(), t.time(), Qt::UTC);
    utc = utc.addSecs(-zoneOffset * 60);
    indicators()->setNetworkTime( utc.toTime_t(), zoneOffset, dst );
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
     chat( "AT%CNAP=1" );

     //begin really ugky hack
     QSettings cfg("Trolltech", "PhoneProfile");
     cfg.beginGroup("Profiles");

    if( !cfg.value("PlaneMode",false).toBool()) {
         chat("AT%NRG=0"); //force auto operations
     }

//      chat("AT%COPS=0");

}

void Ficgta01ModemService::sendSuspendDone()
{
    suspendDone();
}

void Ficgta01ModemService::suspend()
{
    qLog(AtChat)<<" Ficgta01ModemService::suspend()";
    // Turn off cell id information on +CREG and +CGREG as it will
    // cause unnecessary wakeups when moving between cells.
    chat( "AT+CREG=0" );
    chat( "AT+CGREG=0" );

    chat( "AT%CREG=0" );
    chat( "AT%CGREG=0" );

    // Turn off cell broadcast location messages.
    chat( "AT%CSQ=0", this, SLOT(sendSuspendDone()) );
}

void Ficgta01ModemService::wake()
{
    qLog(AtChat)<<" Ficgta01ModemService::wake()";

//reset modem

//  chat( "AT%CWUP=1" );
    chat("\r");
    chat("ATE0\r");
    // Turn cell id information back on.
     chat( "AT+CREG=2" );
     chat( "AT+CGREG=2" );

     //   chat( "AT%CREG=2" );
     //   chat( "AT%CGREG=2" );
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
    qLog(AtChat) << __FUNCTION__ << value;
    setVibrateNow(value);
}

void Ficgta01VibrateAccessory::setVibrateNow( const bool value )
{
    qLog(AtChat) << __FUNCTION__ << value;
    QString cmd;

    if ( value ) { //turn on
        QFile trigger( "/sys/class/leds/neo1973:vibrator/trigger");
        trigger.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out(&trigger);
        out<<"timer";
        trigger.close();

        QFile delayOn( "/sys/class/leds/neo1973:vibrator/delay_on");
        delayOn.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out1(&delayOn);
        out1<<"500";
        delayOn.close();

        QFile delayOff("/sys/class/leds/neo1973:vibrator/delay_off");
        delayOff.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        QTextStream out2(&delayOff);
        out2<<"1000";
        delayOff.close();

    } else { //turn off
        QFile trigger( "/sys/class/leds/neo1973:vibrator/trigger");
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

Ficgta01PreferredNetworkOperators::Ficgta01PreferredNetworkOperators( QModemService *service )
    : QModemPreferredNetworkOperators( service )
{
    // We have to delete an entry before we can write operator details into it.
    setDeleteBeforeUpdate( true );

    // Quote operator numbers when modifying preferred operator entries.
    setQuoteOperatorNumber( true );
}

Ficgta01PreferredNetworkOperators::~Ficgta01PreferredNetworkOperators()
{
}
