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

#define DEFAULT_CHARSET QLatin1String("UCS2")

Ficgta01CallProvider::Ficgta01CallProvider( QModemService *service )
    : QModemCallProvider( service )
{
    service->primaryAtChat()->registerNotificationType
        ( "%CPI:", this, SLOT(cpiNotification(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "%CNAP:", this, SLOT(cnapNotification(QString)) );

    setUseMissedTimer(false);
    setUseDetectTimer(false);
    setTreatAcceptCommandFailedAsMissed(false);
}

Ficgta01CallProvider::~Ficgta01CallProvider()
{
}

QModemCallProvider::AtdBehavior Ficgta01CallProvider::atdBehavior() const
{
    // When ATD reports OK or NO CARRIER, it indicates that it is
    // back in command mode. We just want to ignore these in QModemCall
    // as we will have %CPI that is going to give us the right state.
    return AtdOkIgnore;
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
    // %CPI: <cId>,<msgType>,<ibt>,<tch>,[<dir>],[<mode>][,<number>,<type>[,<alpha>]] 
    // where <cId> is the call identifier, and <msgType> is one of:
    // 0 = SETUP, 1 = DISCONNECT, 2 = ALERT, 3 = PROCEED,
    // 4 = SYNCHRONIZATION, 5 = PROGRESS, 6 = CONNECTED,
    // 7 = RELEASE, 8 = REJECT
    // dir: 0 = mobile originated, 1 = mobile terminated, 2 = network initiaited mobile
    // originated call, 3 = redialing mobile originated
    uint posn = 5;
    uint identifier = QAtUtils::parseNumber( msg, posn );

    uint status = QAtUtils::parseNumber( msg, posn );
    QAtUtils::skipField( msg, posn );
    QAtUtils::skipField( msg, posn );
    uint direction = QAtUtils::parseNumber( msg, posn );
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

    } else if ( ( status == 2 || status == 4 || status == 0) && !call && direction == 1) {

        // This is a newly waiting call.  Treat it the same as "RING".
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

        // We got everything we need, indicate the call to the outside world
        announceCall();
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

/*
 * Reimplementation because we don't want the standard notifications
 * as we have a CPI which should give us everything that we need.
 * We don't need:
 *    +CRING (CPI gives us the calltype)
 *    +CLIP  (CPI gives us the number)
 *    RING
 */
void Ficgta01CallProvider::resetModem()
{
    // disable all of these and do not call the base class
    atchat()->chat( "AT+CRC=0" );
    service()->retryChat( "AT+CLIP=0" );
    service()->retryChat( "AT+COLP=0" );

    // enable callwaiting support
    service()->retryChat( "AT+CCWA=1" );
}

// We need to set GSM as codec for that to work on the TI Calypso see
// https://docs.openmoko.org/trac/ticket/2038 for more information
QString Ficgta01CallProvider::dialServiceCommand(const QDialOptions& options) const
{
    QTextCodec* codec = QAtUtils::codec(DEFAULT_CHARSET);
    QString cmd = QString::fromLatin1("AT+CUSD=1,%1,15");
    return cmd.arg(QAtUtils::quote(options.number(), codec));
}

// Use the enable the echo suppression for TI Calypso before dialing
QString Ficgta01CallProvider::dialVoiceCommand(const QDialOptions& options) const
{
	Ficgta01ModemService::echoCancellation(atchat());
	return QModemCallProvider::dialVoiceCommand(options);
}

// Use the enable the echo suppression for TI Calypso before accepting calls
QString Ficgta01CallProvider::acceptCallCommand( bool otherActiveCalls ) const
{
	Ficgta01ModemService::echoCancellation(atchat());
	return QModemCallProvider::acceptCallCommand(otherActiveCalls);
}

Ficgta01PhoneBook::Ficgta01PhoneBook( QModemService *service )
    : QModemPhoneBook( service )
    , m_phoneBookWasReady(false)
{
    qLog(AtChat)<<"Ficgta01PhoneBook::Ficgta01PhoneBook";
    connect(this, SIGNAL(queryFailed(const QString&)),
            SLOT(slotQueryFailed(const QString&)));
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

void Ficgta01PhoneBook::sendPhoneBooksReady()
{
    m_phoneBookWasReady = true;
    phoneBooksReady();
}

void Ficgta01PhoneBook::slotQueryFailed(const QString& book)
{
    // We didn't say we are ready, ignore this
    if (!m_phoneBookWasReady)
        return;

    qLog(Modem) << "Phonebook query failed, ask to retry. " << book;
    phoneBooksReady();
}

Ficgta01PinManager::Ficgta01PinManager( QModemService *service )
    : QModemPinManager( service )
{
    setShouldSendSimReady(false);
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

DummyCellBroadcast::DummyCellBroadcast(QModemService* service)
    : QCellBroadcast(service->service(), service, QCommInterface::Server)
{
}

void DummyCellBroadcast::setChannels(const QList<int>&)
{
}

Ficgta01ModemService::Ficgta01ModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux,
          QObject *parent )
    : QModemService( service, mux, parent )
    , m_vibratorService( 0 )
    , m_phoneBook( 0 )
    , m_phoneBookIsReady( false )
    , m_smsIsReady( false )
{
    connect( this, SIGNAL(resetModem()), this, SLOT(reset()) );

    // Register a wakeup command to ping the modem if we haven't
    // sent anything during the last 5 seconds.  This command may
    // not get a response, but the modem should then become responsive
    // to the next command that is sent afterwards.
    primaryAtChat()->registerWakeupCommand( QChar(0x1a), 5000 );

    // Attempt to reset the modem
    chat("AT%CWUP=1");
    chat("AT+CFUN=1");


    // Turn on dynamic signal quality notifications.
    // Register for "%CSQ" notifications to get signal quality updates.
     primaryAtChat()->registerNotificationType
         ( "%CSQ:", this, SLOT(csq(QString)) );
     chat("AT%CSQ=1");
     QTimer::singleShot( 2500, this, SLOT(firstCsqQuery()) );

    // Modem dead detection for https://docs.openmoko.org/trac/ticket/1192
    primaryAtChat()->registerNotificationType
        ( "+CME ERROR: 512", this, SLOT(modemDied()));


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

// Enable the work-around for the bouncy rubber calypso problem
    csqTimer = new QTimer( this );
    csqTimer->setSingleShot( true );
    connect ( csqTimer, SIGNAL(timeout()), this, SLOT(csqTimeOut()) );

    // Setup the default text codec to UCS2 for none English langs
    setDefaultCharset(DEFAULT_CHARSET);
    chat( "AT+CSCS=\""+ DEFAULT_CHARSET + "\"" );

    // Turn on status notification messages for finding out when
    // the SIM/phonebook/SMS is ready to use.
    primaryAtChat()->registerNotificationType
        ( "%CSTAT:", this, SLOT(cstatNotification(QString)) );
    primaryAtChat()->chat( "AT%CSTAT=1" );
}

Ficgta01ModemService::~Ficgta01ModemService()
{
    delete csqTimer;
}

void Ficgta01ModemService::initialize()
{
    if ( !supports<QPinManager>() )
        addInterface( new Ficgta01PinManager( this ) );

    if ( !supports<QPhoneBook>() ) {
        m_phoneBook = new Ficgta01PhoneBook( this );
        addInterface( m_phoneBook );
    }


    if ( !supports<QBandSelection>() )
        addInterface( new Ficgta01BandSelection( this ) );

    if ( !supports<QSimInfo>() ) {
        QModemSimInfo* simInfo = new QModemSimInfo( this );
        simInfo->setSimNotInsertedReason( QModemSimInfo::Reason_SimFailure );
        addInterface( simInfo );
    }

    if ( !callProvider() )
        setCallProvider( new Ficgta01CallProvider( this ) );

   if ( !supports<QVibrateAccessory>() ) {
        m_vibratorService = new Ficgta01VibrateAccessory( this );
        addInterface( m_vibratorService );
    }

    if ( !supports<QCallVolume>() )
        addInterface( new Ficgta01CallVolume(this));

    if ( !supports<QPreferredNetworkOperators>() )
        addInterface( new Ficgta01PreferredNetworkOperators(this));

    // CBMs create an issue on suspend/resume disable for now #1530
    if ( !supports<QCellBroadcast>() )
        addInterface( new DummyCellBroadcast(this) );


   QModemService::initialize();
}

void Ficgta01ModemService::csq( const QString& msg )
{
    // Automatic signal quality update, in the range 0-31.
    if ( msg.contains( QChar(',') ) ) {
        uint posn = 6;
        uint rssi = QAtUtils::parseNumber( msg, posn );

        // An rssi of "99" indicates invalid (i.e. loss of signal).
        // We wish to be careful about reporting that, because the
        // silly calypso will report that each and every time it
        // changes cell sites, followed almost immediately by a new
        // (valid) signal report.  So we defer reporting the invalid
        // signal, and wait to see if we get an new one first.
        qLog(AtChat)<< "percentCSQ event, rssi: " << (int)(rssi);
        if ( rssi == 99 ) {
            if ( !csqTimer->isActive() )
	        csqTimer->start( 8000 );  // 8 second one-shot timer
        } else {
            if ( csqTimer->isActive() ) {
                csqTimer->stop();
                indicators()->setSignalQuality( (int)rssi, 31 );
            }
        }
    }
}

void Ficgta01ModemService::csqTimeOut()
{
    // Timeout on a signal quality notification, it must have been real
    qLog(AtChat)<< "percentCSQ timer expired; reporting loss";
    indicators()->setSignalQuality( -1, 31 );
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

    // Set a default CBM state, disable all, setChannels on QCellBroadcast
    // would enable them again. This should mean no (empty) CBM is accepted.
    chat("AT+CSCB=0");

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

    // Don't need these, just clutter (nothing uses these
    // proprietary commands yet -- MJW)
    //chat( "AT%CREG=0" );
    //chat( "AT%CGREG=0" );

    // Turn off cell broadcast location messages.

    // Make sure the (useless) CIEV notifications are disabled too.
    chat( "AT+CMER=0,0,0,0,0" );

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

// Modem state notification. It will tell us once the SIM is ready
// to give us access to the phonebook and SMS. To delay certain access
// we will also send simready from here...
void Ficgta01ModemService::cstatNotification( const QString& msg )
{
    // %CSTAT: PHB, 0
    // %CSTAT: <entity>,<status>
    // PHB (phone book)
    // SMS
    // RDY (Ready when both PHB and SMS have reported they are ready)
    QString entity = msg.mid(8, 3);
    uint status = msg.mid(13).toInt();

    // We are already ready, ignore
    if (m_phoneBookIsReady && m_smsIsReady)
        return;

    if (entity == "PHB")
        m_phoneBookIsReady = status;
    else if (entity == "SMS")
        m_smsIsReady = status;
    else if (entity == "RDY") {
        m_smsIsReady = status;
        m_phoneBookIsReady = status;
    }

    if  (m_smsIsReady || m_phoneBookIsReady) {
        post("simready"); 

        if (m_phoneBook)
            m_phoneBook->sendPhoneBooksReady();
    }
}


// Modem dead detection for https://docs.openmoko.org/trac/ticket/1192
// The modem will not work with us anymore. We would need to make the AT
// command queue to freeze, fail all commands, we would need to reset the
// modem, force reinit (send the commands from all the c'tors again), make sure
// every application logic gets restarted. This will be a major effort. So the
// 2nd best thing is to detect the error and inform the user that he should
// restart his device. Currently I have no idea how often the error occurs
// and assume that it is not often at all.
//
// We do something unusual and open a QWidget from this plugin. This is okay
// as we will have a QtopiaApplication for the QCOP communication anyway. So
// there will be a GUI connection.
void Ficgta01ModemService::modemDied()
{
    static bool claimedDead = false;
    if (claimedDead)
        return;

    claimedDead = true;

    if (m_vibratorService)
        m_vibratorService->setVibrateNow(true);
    QMessageBox::information(0, tr("Modem Died"),
                             tr("The firmware of the modem appears to have crashed. "
                                "No further interaction with the modem will be possible. "
                                "Please restart the device."), QMessageBox::Ok);
    if (m_vibratorService)
        m_vibratorService->setVibrateNow(false);
}

void Ficgta01ModemService::echoCancellation(QAtChat * atChat)
{
    Ficgta01ModemHiddenFeatures* hs = new Ficgta01ModemHiddenFeatures(atChat);
    atChat->chat( "AT@ST=\"-26\"" ); // audio side tone: set to minimum
    atChat->chat( "AT%N028B" );      // Long Echo Cancellation: active, -6db
    atChat->chat( "AT%N0125" );      // Noise reduction: active, -6db
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

Ficgta01ModemHiddenFeatures::Ficgta01ModemHiddenFeatures(QAtChat* atChat) :
    atPrefix( "AT%N" )
{
    m_atChat = atChat;
}

Ficgta01ModemHiddenFeatures::~Ficgta01ModemHiddenFeatures()
{
}

void Ficgta01ModemHiddenFeatures::sendHiddenFeatureCommand(int code)
{
    m_atChat->chat(atPrefix + QString::number(code, 16).toUpper().rightJustified(4, '0'));
}

void Ficgta01ModemHiddenFeatures::enableAEC(int type, bool longAEC = true)
{
    if (type <= 0 && type >= -18 && (-type) % 6 == 0) {
        int code = 0x0083 + (((-type) / 6) * 0x8);
        
        if (longAEC)
            code += 0x0200;
            
         sendHiddenFeatureCommand(code);
    }
}

void Ficgta01ModemHiddenFeatures::enableNoiseReduction(int type)
{
    if (type <= 0 && type >= -18 && (-type) % 6 == 0) {
        int code = 0x0105 + (((-type) / 6) * 0x20);
            
        sendHiddenFeatureCommand(code);
    }
}

void Ficgta01ModemHiddenFeatures::enableNoiseReductionAEC()
{
    sendHiddenFeatureCommand(0x0187);
}

void Ficgta01ModemHiddenFeatures::disableNoiseReductionAEC()
{
    sendHiddenFeatureCommand(0x0001);
}

