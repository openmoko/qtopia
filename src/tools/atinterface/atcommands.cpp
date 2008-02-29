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

#include "atcommands.h"
#include "atsessionmanager.h"
#include "atoptions.h"
#include "atindicators.h"
#include "atcustom.h"
#include <qslotinvoker.h>
#include <qgsm0710multiplexer.h>
#include <qatutils.h>
#include <qcommservicemanager.h>
#include <version.h>
#ifdef QTOPIA_CELL
#include <qsiminfo.h>
#include <qsmssender.h>
#include <qsmsreader.h>
#include <qphonerffunctionality.h>
#include <QPhoneBook>
#endif
#include <qvaluespace.h>
#include <qtopialog.h>
#include <qmap.h>
#include <qpair.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <qtopiaservices.h>
#include <QContactModel>
#include <QUniqueId>

#ifdef QTOPIA_BLUETOOTH
// This class was copied from src/server/bluetooth/hf/qbluetoothhfservice.cpp
class atinterface_HandsfreeIpcAdaptor : public QtopiaIpcAdaptor
{
    Q_OBJECT
    friend class AtCommands;

public:
    atinterface_HandsfreeIpcAdaptor(AtCommands *parent);

public slots:
    void setSpeakerVolume(int vol);
    void setMicrophoneVolume(int vol);

signals:
    void speakerVolumeChanged(int volume);
    void microphoneVolumeChanged(int volume);

private:
    AtCommands *m_parent;
};

atinterface_HandsfreeIpcAdaptor::atinterface_HandsfreeIpcAdaptor(AtCommands *parent)
    : QtopiaIpcAdaptor("QPE/BluetoothHandsfree", parent), m_parent(parent)
{
    publishAll(QtopiaIpcAdaptor::SignalsAndSlots);
}

void atinterface_HandsfreeIpcAdaptor::setSpeakerVolume(int vol)
{
    m_parent->setSpeakerVolume(vol);
}

void atinterface_HandsfreeIpcAdaptor::setMicrophoneVolume(int vol)
{
    m_parent->setMicrophoneVolume(vol);
}
#endif

struct ContactRecord
{
public:
    QUniqueId id;
    QContact::PhoneType type;
};

class AtCommandsPrivate
{
public:
    AtCommandsPrivate()
    {
        cmdsPosn = 0;
        result = QAtResult::OK;
        underlying = 0;
        multiplexer = 0;
        extendedError = QAtResult::OK;
        indicators = 0;
#ifdef QTOPIA_CELL
        callSettings = new QCallSettings();
        requestingCallWaiting = false;
        settingCallWaiting = false;
#endif
        netReg = new QNetworkRegistration( "modem" );   // No tr
        if ( !netReg->available() ) {
            // May be a VoIP-only phone, so use the default network reg object.
            delete netReg;
            netReg = new QNetworkRegistration();
        }
        requestingAvailableOperators = false;
        settingCurrentOperator = false;
        serviceNumbers = new QServiceNumbers();
        pendingQuery = (QServiceNumbers::NumberId)(-1);
        pendingSet = (QServiceNumbers::NumberId)(-1);
#ifdef QTOPIA_BLUETOOTH
        m_adaptor = 0;
#endif
        dataCallRequested = false;
        sendRelease = false;
        keyPressTime = 0;
        keyPauseTime = 0;
#ifdef QTOPIA_CELL
        sendingSms = false;
        smsSender = new QSMSSender( "modem" );
        smsMessageReference = 1;
        smsReader = new QSMSReader( "modem" );
        phonerf = new QPhoneRfFunctionality( "modem" ); // No tr
        settingPhoneRf = false;
        phoneBook = new QPhoneBook( "modem" );
        phoneBookQueried = false;
        limitsReqBy = "";
        contactModel = 0;
        availableMemoryContacts = 50000; //we set an upper limit for contact
#endif
    }
    ~AtCommandsPrivate()
    {
        delete netReg;
        delete serviceNumbers;
#ifdef QTOPIA_BLUETOOTH
        if (m_adaptor)
            delete m_adaptor;
#endif
#ifdef QTOPIA_CELL
        delete callSettings;
        delete smsSender;
        delete phonerf;
        delete smsReader;
        delete phoneBook;
#endif
    }

    AtFrontEnd *frontEnd;
    AtSessionManager *manager;
    QAtResult::ResultCode result;
    QStringList cmds;
    int cmdsPosn;
    QMap<QString, QSlotInvoker *> invokers;
    QSerialIODevice *underlying;
    QGsm0710MultiplexerServer *multiplexer;
    QAtResult::ResultCode extendedError;
    AtPhoneIndicators *indicators;
#ifdef QTOPIA_CELL
    QCallSettings *callSettings;
    bool requestingCallWaiting;
    bool settingCallWaiting;
#endif
    QNetworkRegistration *netReg;
    bool requestingAvailableOperators;
    bool settingCurrentOperator;
    QServiceNumbers *serviceNumbers;
    QServiceNumbers::NumberId pendingQuery;
    QServiceNumbers::NumberId pendingSet;
#ifdef QTOPIA_BLUETOOTH
    atinterface_HandsfreeIpcAdaptor *m_adaptor;
#endif
    bool dataCallRequested;
    bool sendRelease;
    int keyPressTime;
    int keyPauseTime;
    QList< QPair<int, int> > keys;
#ifdef QTOPIA_CELL
    QString smsNumber;
    QString smsMessageId;
    bool sendingSms;
    QSMSSender *smsSender;
    int smsMessageReference;
    QSMSReader *smsReader;
    QPhoneRfFunctionality *phonerf;
    bool settingPhoneRf;

    //phone book related
    QPhoneBook* phoneBook;
    QPair<uint,uint> phoneBookIndex;
    QPhoneBookEntry entryToWrite;
    bool phoneBookQueried;
    QByteArray limitsReqBy;
    QContactModel* contactModel;
    QHash<int, ContactRecord> contactIndices;
    int availableMemoryContacts;
    QString pbSearchText;
#endif
};

AtCommands::AtCommands( AtFrontEnd *frontEnd, AtSessionManager *manager )
    : QObject( frontEnd )
{
    qLog(ModemEmulator) << "AT command session started";

    d = new AtCommandsPrivate();
    d->frontEnd = frontEnd;
    d->manager = manager;

    connect( frontEnd, SIGNAL(commands(QStringList)),
             this, SLOT(commands(QStringList)) );
    connect( frontEnd, SIGNAL(extra(QString,bool)),
             this, SLOT(extraLine(QString,bool)) );

    AtCallManager *calls = manager->callManager();
    connect( calls, SIGNAL(stateChanged(int,AtCallManager::CallState,QString,QString)),
             this, SLOT(stateChanged(int,AtCallManager::CallState,QString,QString)) );
    connect( calls, SIGNAL(deferredResult(AtCommands*,QAtResult::ResultCode)),
             this, SLOT(deferredResult(AtCommands*,QAtResult::ResultCode)) );
    connect( calls, SIGNAL(ring(QString,QString)),
             this, SLOT(ring(QString,QString)) );
    connect( calls, SIGNAL(callWaiting(QString,QString)),
             this, SLOT(callWaiting(QString,QString)) );
    connect( calls, SIGNAL(noCarrier()), this, SLOT(noCarrier()) );

#ifdef QTOPIA_CELL
    connect( d->callSettings, SIGNAL(callWaiting(QTelephony::CallClass)),
             this, SLOT(callWaitingState(QTelephony::CallClass)) );
#endif

    connect( d->netReg, SIGNAL(registrationStateChanged()),
             this, SLOT(registrationStateChanged()) );
    connect( d->netReg, SIGNAL(availableOperators(QList<QNetworkRegistration::AvailableOperator>)),
             this, SLOT(availableOperators(QList<QNetworkRegistration::AvailableOperator>)) );

    connect( d->serviceNumbers,
             SIGNAL(serviceNumber(QServiceNumbers::NumberId,QString)),
             this,
             SLOT(serviceNumber(QServiceNumbers::NumberId,QString)) );

    connect( d->serviceNumbers,
             SIGNAL(setServiceNumberResult(QServiceNumbers::NumberId,QTelephony::Result)),
             this,
             SLOT(setServiceNumberResult(QServiceNumbers::NumberId,QTelephony::Result)) );

#ifdef QTOPIA_CELL
    connect( d->smsSender,
             SIGNAL(finished(QString,QTelephony::Result)),
             this,
             SLOT(smsFinished(QString,QTelephony::Result)) );

    connect( d->phonerf,
             SIGNAL(setLevelResult(QTelephony::Result)),
             this,
             SLOT(setLevelResult(QTelephony::Result)) );

    connect( d->phoneBook, SIGNAL(limits(QString,QPhoneBookLimits)),
             this, SLOT(phoneBookLimits(QString,QPhoneBookLimits)) );

    connect( d->phoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
             this, SLOT(phoneBookEntries(QString,QList<QPhoneBookEntry>)) );
#endif

    // Add handlers for the common V.250 commands.  Many of these are
    // ignored because they have no meaning for us, or they will be
    // handled by other settings on the phone, not AT commands.
    add( "A", this, SLOT(ata()) );
    add( "D", this, SLOT(atd(QString)) );
    add( "E", this, SLOT(ate(QString)) );
    add( "H", this, SLOT(ath()) );
    add( "I", this, SLOT(ati(QString)) );
    add( "L", this, SLOT(ignore()) );
    add( "M", this, SLOT(ignore()) );
    add( "O", this, SLOT(ato()) );
    add( "P", this, SLOT(ignore()) );
    add( "Q", this, SLOT(atq(QString)) );
    add( "S0", this, SLOT(ignore()) );
    add( "S3", this, SLOT(ats3(QString)) );
    add( "S4", this, SLOT(ats4(QString)) );
    add( "S5", this, SLOT(ats5(QString)) );
    add( "S6", this, SLOT(ignore()) );
    add( "S7", this, SLOT(ignore()) );
    add( "S8", this, SLOT(ignore()) );
    add( "S10", this, SLOT(ignore()) );
    add( "T", this, SLOT(ignore()) );
    add( "V", this, SLOT(atv(QString)) );
    add( "X", this, SLOT(ignore()) );
    add( "Z", this, SLOT(atz()) );
    add( "&C", this, SLOT(ignore()) );
    add( "&D", this, SLOT(ignore()) );
    add( "&F", this, SLOT(atampf()) );
    add( "&W", this, SLOT(atampw()) );
    add( "+GCAP", this, SLOT(atgcap()) );
    add( "+GCI", this, SLOT(ignore()) );
    add( "+GMI", this, SLOT(atgmi()) );
    add( "+GMM", this, SLOT(atgmm()) );
    add( "+GMR", this, SLOT(atgmr()) );
    add( "+GOI", this, SLOT(ignore()) );
    add( "+GSN", this, SLOT(atgsn()) );

    // Handle GSM 27.007 commands.
    add( "+CBC", this, SLOT(atcbc(QString)) );
    add( "+CBST", this, SLOT(atcbst(QString)) );
#ifdef QTOPIA_CELL
    add( "+CCWA", this, SLOT(atccwa(QString)) );
#endif
    add( "+CDIS", this, SLOT(notAllowed()) );
    add( "+CEER", this, SLOT(atceer()) );
    add( "+CFUN", this, SLOT(atcfun(QString)) );
    add( "+CGDATA", this, SLOT(atcgdata(QString)) );
    add( "+CGDCONT", this, SLOT(atcgdcont(QString)) );
    add( "+CGMI", this, SLOT(atcgmi()) );
    add( "+CGMM", this, SLOT(atcgmm()) );
    add( "+CGMR", this, SLOT(atcgmr()) );
    add( "+CGSN", this, SLOT(atcgsn()) );
    add( "+CHLD", this, SLOT(atchld(QString)) );
    add( "+CHUP", this, SLOT(atchup()) );
    add( "+CIMI", this, SLOT(atcimi()) );
    add( "+CIND", this, SLOT(atcind(QString)) );
    add( "+CKPD", this, SLOT(atckpd(QString)) );
    add( "+CLCC", this, SLOT(atclcc()) );
    add( "+CLIP", this, SLOT(atclip(QString)) );
    add( "+CMEC", this, SLOT(atcmec(QString)) );
    add( "+CMEE", this, SLOT(atcmee(QString)) );
    add( "+CMER", this, SLOT(atcmer(QString)) );
#ifdef QTOPIA_CELL
    add( "+CMGF", this, SLOT(atcmgf(QString)) );
    add( "+CMGS", this, SLOT(atcmgs(QString)) );
    add( "+CMMS", this, SLOT(atcmms(QString)) );
#endif
    add( "+CMUX", this, SLOT(atcmux(QString)) );
    add( "+CNUM", this, SLOT(atcnum()) );
    add( "+COPS", this, SLOT(atcops(QString)) );
    add( "+CPAS", this, SLOT(atcpas(QString)) );
#ifdef QTOPIA_CELL
    add( "+CPBF", this, SLOT(atcpbf(QString)) );
    add( "+CPBR", this, SLOT(atcpbr(QString)) );
    add( "+CPBS", this, SLOT(atcpbs(QString)) );
    add( "+CPBW", this, SLOT(atcpbw(QString)) );
#endif
    add( "+CPIN", this, SLOT(atcpin(QString)) );
#ifdef QTOPIA_CELL
    add( "+CPMS", this, SLOT(atcpms(QString)) );
#endif
    add( "+CRC", this, SLOT(atcrc(QString)) );
    add( "+CREG", this, SLOT(atcreg(QString)) );
    add( "+CRES", this, SLOT(atcres()) );
    add( "+CSAS", this, SLOT(atcsas()) );
#ifdef QTOPIA_CELL
    add( "+CSCA", this, SLOT(atcsca(QString)) );
#endif
    add( "+CSCS", this, SLOT(atcscs(QString)) );
#ifdef QTOPIA_CELL
    add( "+CSDH", this, SLOT(atcsdh(QString)) );
    add( "+CSMP", this, SLOT(atcsmp(QString)) );
    add( "+CSMS", this, SLOT(atcsms(QString)) );
#endif
    add( "+CSQ", this, SLOT(atcsq(QString)) );
    add( "+CSTA", this, SLOT(atcsta(QString)) );
    add( "+VTD", this, SLOT(atvtd(QString)) );
    add( "+VTS", this, SLOT(atvts(QString)) );

#ifndef QTOPIA_AT_STRICT
    // Extension commands for features that should be in the
    // GSM specifications, but aren't.
    add( "*QBC", this, SLOT(atqbc(QString)) );  // Battery charge notifications.
    add( "*QCAM", this, SLOT(atqcam(QString)) );// Call status monitoring.
    add( "*QSQ", this, SLOT(atqsq(QString)) );  // Signal quality notifications.
#endif

#ifdef QTOPIA_BLUETOOTH
    // Add commands that are specific to Bluetooth hands-free.
    if ( options()->hasStartupOption( "handsfree" ) ) {
        add( "+BLDN", this, SLOT(atbldn()) );
        add( "+BRSF", this, SLOT(atbrsf(QString)) );
        add( "+VGM", this, SLOT(atvgm(QString)) );
        add( "+VGS", this, SLOT(atvgs(QString)) );
        add( "+NREC", this, SLOT(atnrec(QString)) );
        add( "+BVRA", this, SLOT(atbvra(QString)) );
        add( "+BINP", this, SLOT(atbinp(QString)) );
        add( "+BTRH", this, SLOT(atbtrh(QString)) );

        d->m_adaptor = new atinterface_HandsfreeIpcAdaptor(this);
    }
#endif
}

AtCommands::~AtCommands()
{
    qLog(ModemEmulator) << "AT command session stopped";
    delete d;
}

AtFrontEnd *AtCommands::frontEnd() const
{
    return d->frontEnd;
}

AtSessionManager *AtCommands::manager() const
{
    return d->manager;
}

AtOptions *AtCommands::options() const
{
    return d->frontEnd->options();
}

/*
    Add \a name to the list of commands that are processed by this
    AT interface.  When the command arrives, \a slot on \a target
    will be invoked, with a QString argument corresponding to the
    command parameters.

    The slot should perform whatever processing is necessary and
    then call done() to terminate the results.  If the command
    will be sending lines of data, it should call send() before done().

    The command name should be in upper case and should not start
    with \c AT.  For example, \c{+CGMI}, \c{S0}, \c{D} etc.
*/
void AtCommands::add( const QString& name, QObject *target, const char *slot )
{
    d->invokers.insert( name, new QSlotInvoker( target, slot, this ) );
}

void AtCommands::send( const QString& line )
{
    d->frontEnd->send( line );
}

void AtCommands::done( QAtResult::ResultCode result )
{
    // Record the final result code for the command we just executed.
    d->result = result;

    // Arrange for the next command in sequence to be processed.
    // Do it upon the next event loop entry to prevent the
    // stack from growing too much.
    QTimer::singleShot( 0, this, SLOT(processNextCommand()) );
}

void AtCommands::doneWithExtendedError( QAtResult::ResultCode result )
{
    d->extendedError = result;
    done( result );
}

void AtCommands::commands( const QStringList& cmds )
{
    // Start by assuming that the final result will be OK.
    // If it is ever set to non-OK, then the commands stop.
    d->result = QAtResult::OK;

    // Save the commands.
    d->cmds = cmds;
    d->cmdsPosn = 0;

    // Process the first command.
    processNextCommand();
}

void AtCommands::processNextCommand()
{
    // If we have a non-OK result, or no further commands, then stop now.
    if ( d->result != QAtResult::OK || d->cmdsPosn >= d->cmds.size() ) {
        d->cmdsPosn = d->cmds.size();
        d->frontEnd->send( d->result );
        return;
    }

    // Remove one command from the queue.
    QString name = d->cmds[(d->cmdsPosn)++];
    QString params = d->cmds[(d->cmdsPosn)++];

    // Print the full command to the debug output stream.
    qLog(ModemEmulator) << "AT" + name + params;

    // Determine how to dispatch the command.
    QMap<QString, QSlotInvoker *>::Iterator iter;
    iter = d->invokers.find( name );
    if ( iter == d->invokers.end() ) {
        // Process the AT+CLAC command according to GSM 27.007, section 8.37.
        // We also supply AT* as a synonym for quicker debugging.
        if ( name == "+CLAC"
        #ifndef QTOPIA_AT_STRICT
            || name == "*"
        #endif
            ) {
            for ( iter = d->invokers.begin();
                  iter != d->invokers.end(); ++iter ) {
                send( "AT" + iter.key() );
            }
            send( "AT+CLAC" );
        #ifndef QTOPIA_AT_STRICT
            send( "AT*" );
        #endif
            done( QAtResult::OK );
            return;
        }

        // We don't know how to process this command, so error out.
        // This will stop any further commands from being processed.
        d->cmdsPosn = d->cmds.size();
        d->frontEnd->send( QAtResult::Error );
        return;
    }

    // Notify the call manager which handler will be talking to it
    // to properly deliver deferred results back to this handler.
    d->manager->callManager()->setHandler( this );

    // Dispatch the command to the associated slot.
    QList<QVariant> args;
    args += QVariant( params );
    iter.value()->invoke( args );
}

void AtCommands::channelOpened( int /*channel*/, QSerialIODevice *device )
{
    // Wrap the device in a new front end, and disallow GSM 07.10 multiplexing.
    AtFrontEnd *front = new AtFrontEnd( options()->startupOptions, device );
    front->setDevice( device );
    front->setCanMux( false );

    // Let the session manager know about the front end so that
    // it can wrap it further with higher-level command handlers.
    emit d->manager->newSession( front );
}

void AtCommands::muxTerminated()
{
    // Restore the underlying device for AT command processing.
    d->frontEnd->setDevice( d->underlying );
    d->underlying = 0;

    // Destroy the multiplexer as it is no longer required.
    d->multiplexer->deleteLater();
    d->multiplexer = 0;
}

void AtCommands::sendSignalQuality( int value )
{
    if ( value < 0 )
        send( "+CSQ: 99,99" );
    else
        send( "+CSQ: " + QString::number( value * 31 / 100 ) + ",99" );
}

void AtCommands::signalQualityChanged( int value )
{
    if ( options()->qsq )
        sendSignalQuality( value );
}

void AtCommands::sendBatteryCharge( int value )
{
    if ( value < 0 )
        send( "+CBC: 0,100" );
    else
        send( "+CBC: 0," + QString::number( value ) );
}

void AtCommands::batteryChargeChanged( int value )
{
    if ( options()->qbc )
        sendBatteryCharge( value );
}

/*
    Handler that ignores the command and just responds \c OK.
*/
void AtCommands::ignore()
{
    done();
}

/*
    Handler that errors out the command with "not allowed".
*/
void AtCommands::notAllowed()
{
    done( QAtResult::OperationNotAllowed );
}

/*!
    \page modem-emulator.html
    \title Modem Emulator

    The modem emulator component in Qtopia allows external devices, such
    as laptops and Bluetooth hands-free kits, to send AT commands to a
    Qtopia Phone to cause it to perform operations on the external device's
    behalf.

    The following sections describe the AT commands that are supported
    by Qtopia via the Modem Emulator interface.

    \list
        \o \l{Modem Emulator - Call Control}{Call Control Commands}
        \o \l{Modem Emulator - Control and Status}{Control and Status Commands}
        \o \l{Modem Emulator - Phonebook Operations}{Phonebook Commands}
        \o \l{Modem Emulator - Identification}{Identification Commands}
        \o \l{Modem Emulator - Network}{Network Commands}
        \o \l{Modem Emulator - Supplementary Services}{Supplementary Service Commands}
        \o \l{Modem Emulator - GPRS}{GPRS Commands}
        \o \l{Modem Emulator - Ignored Commands}{Ignored Commands}
    \endlist

    The modem emulator is accessed by sending QCop service messages to
    the \l{ModemEmulatorService}{ModemEmulator} service as devices that
    need AT command support are connected and disconnected.

    For phones with a standard serial cable for external devices to
    access the phone, the \c{ExternalAccessDevice} option can be
    set in the \c Phone.conf file to specify this device.  See
    \l{GSM Modem Integration#modem-emulator}{GSM Modem Integration: Modem Emulator}
    for more information.

    \nextpage{Modem Emulator - Call Control}
*/

/*!
    \group ModemEmulator::CallControl
    \title Modem Emulator - Call Control

    The AT commands in this section are used for dialing, accepting,
    and manipulating voice and data calls.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Control and Status}
    \previouspage{Modem Emulator}
*/

/*!
    \group ModemEmulator::ControlAndStatus
    \title Modem Emulator - Control and Status
    \section1 Control and Status

    The AT commands in this section are used for controlling state
    information in the modem and for reporting the current status
    of the state information.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Phonebook Operations}
    \previouspage{Modem Emulator - Call Control}
*/

/*!
    \group ModemEmulator::PhoneBook
    \title Modem Emulator - Phonebook Operations
    \section1 Phonebook Operations

    The AT commands in this section are used for accessing the SIM phone book.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Identification}
    \previouspage{Modem Emulator - Control and Status}

*/

/*!
    \group ModemEmulator::Identification
    \title Modem Emulator - Identification

    The AT commands in this section are used for retrieving identification
    information for the modem.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Network}
    \previouspage{Modem Emulator - Phonebook Operations}
*/

/*!
    \group ModemEmulator::Network
    \title Modem Emulator - Network

    The AT commands in this section are used for accessing network
    services on GSM and similar networks.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Supplementary Services}
    \previouspage{Modem Emulator - Identification}
*/

/*!
    \group ModemEmulator::SupplementaryServices
    \title Modem Emulator - Supplementary Services

    The AT commands in this section are used for accessing GSM supplementary
    services such as call waiting, caller-id, etc.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - GPRS}
    \previouspage{Modem Emulator - Network}
*/

/*!
    \group ModemEmulator::GPRS
    \title Modem Emulator - GPRS

    The AT commands in this section are used for setting up GPRS connections.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Short Message Service}
    \previouspage{Modem Emulator - Supplementary Services}
*/

/*!
    \group ModemEmulator::ShortMessageService
    \title Modem Emulator - Short Message Service

    The AT commands in this section are used for accessing the Short Message
    Service (SMS) subsystem.

    \generatelist{relatedinline}
    \nextpage{Modem Emulator - Ignored Commands}
    \previouspage{Modem Emulator - GPRS}
*/

/*!
    \group ModemEmulator::Ignored
    \title Modem Emulator - Ignored Commands

    The following AT commands are ignored and will always return \c{OK}.
    They exist for compatibility with Recommendation V.250.

    \list
        \o \c{ATL} Monitor speaker loudness.
        \o \c{ATM} Monitor speaker mode.
        \o \c{ATP} Select pulse dialing.
        \o \c{ATS0} Automatic answer.
        \o \c{ATS6} Pause before blind dialing.
        \o \c{ATS7} Connection completion timeout.
        \o \c{ATS8} Comma dial modifier time.
        \o \c{ATS10} Automatic disconnect delay.
        \o \c{ATT} Select tone dialing.
        \o \c{ATX} Result code selection and call progress monitoring control.
        \o \c{AT&C} Circuit 109 (Received line signal detector) behavior.
        \o \c{AT&D} Circuit 108 (Data terminal ready) behavior.
        \o \c{AT+GCI} Country of installation.
        \o \c{AT+GOI} Request global object identification.
    \endlist

    \previouspage{Modem Emulator - Short Message Service}
*/

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATA Answer Incoming Call}
    \compat

    The \c{ATA} command answers an incoming voice or data call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATA}
         \o \list
               \o \c{CONNECT [<rate>]}: Data call has connected at \c rate.
               \o \c{OK}: Voice call has connected.
               \o \c{NO CARRIER}: Connection could not be established.
               \o \c{ERROR}: Command issues when already connected.
            \endlist
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ata()
{
    QAtResult::ResultCode result = d->manager->callManager()->accept();
    if ( result != AtCallManager::Defer )
        done( result );
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATD Dial Call}
    \compat

    The \c{ATD} command initiates a dial for a voice or data call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATDdialstring[i][;]}
         \o \list
               \o \c{CONNECT [<rate>]}: Data call has connected at \c rate.
               \o \c{OK}: Voice call has connected.
               \o \c{NO CARRIER}: Connection could not be established.
               \o \c{BUSY}: Called party is busy.
               \o \c{ERROR}: Command issues when already connected.
            \endlist
    \endtable

    The command name is followed by a string of digits and control
    characters, according to the following table:

    \table
    \row \o \c{0-9, *, #, +, A, B, C, D}
         \o Digits to be dialed.  In GSM networks, \c{D} is ignored,
            in accordance with 3GPP TS 27.007.
    \row \o \c{,} \o Insert a pause into the dialing sequence.
    \row \o \c{T}, \c{P} \o Select tone or pulse dialing.  These are ignored.
    \row \o \c{!}
         \o Insert a hook flash into the dialing sequence.  This is ignored
            in GSM networks.
    \row \o \c{W} \o Wait for dial tone.  This is ignored in GSM networks.
    \row \o \c{@} \o Wait for quiet answer.  This is ignored in GSM networks.
    \row \o \c{;}
         \o This must be last character in the dialing string, and indicates
            a voice call rather than a data call.  The system will issue
            \c{OK} and immediately return to command mode.
    \row \o \c{i}
         \o Allow the local user's caller ID information to be presented
            to the called party.
    \row \o \c{I}
         \o Suppress the local user's caller ID information from being
            presented to the called party.
    \endtable

    If the dialing sequence begins with \c{>}, then the rest of the
    sequence, up until the semi-colon, is interpreted as a name in a
    direct-dialing phonebook.  This will only work with GSM modems that
    support the feature.  Dialing by explicit number is recommended.

    If the system has been configured with support for VoIP, then VoIP
    calls can be placed with the \c{ATD} command by using the full
    URI of the called party.  For example: \c{ATDsip:fred@jones.com;}.

    Conforms with: Recommendation V.250, 3GPP TS 27.007.
*/
void AtCommands::atd( const QString& params )
{
    bool isDataCall = false;
    if ( params.endsWith( "#" ) ) {
        if ( d->netReg->registrationState() != QTelephony::RegistrationHome &&
                d->netReg->registrationState() != QTelephony::RegistrationRoaming ) {
            //no network -> cannot make data calls
            done( QAtResult::NoDialtone );
            return;
        }
        isDataCall = true;
    }
    QAtResult::ResultCode result = d->manager->callManager()->dial( params );
    if ( result != AtCallManager::Defer )
        done( result );
    else if ( isDataCall ) //deferred data call
       d->dataCallRequested = true; //remember that we are in data call
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATE Command Echo}
    \compat

    The \c{ATE} command can be used to turn command echo on (\c{ATE1})
    or off (\c{ATE0}).  If no parameter is supplied (i.e. \c{ATE}),
    it is the same as \c{ATE0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATE[<n>]} \o \c{OK}, \c{ERROR}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ate( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        options()->echo = false;
        done();
    } else if ( params == "1" ) {
        options()->echo = true;
        done();
    } else {
        done( QAtResult::Error );
    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATH Hangup Call}
    \compat

    The \c{ATH} command hangs up the current connected, dialing, or
    incoming call.  The \c{AT+CHLD} command is recommended instead for
    voice calls.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATH[<n>]} \o \c{OK}, \c{ERROR}
    \endtable

    The parameter \c{<n>} is ignored in this implementation.  If it is
    present, it should be zero according to Recommendation V.250.

    Conforms with: Recommendation V.250.
*/
void AtCommands::ath()
{
    QAtResult::ResultCode result = d->manager->callManager()->hangup();
    if ( result != AtCallManager::Defer )
        done( result );
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{ATI Identification Information}
    \compat

    The \c{ATI} command returns identification information about the
    modem's manufacturer and revision.  \c{ATI0} can also be used for
    this purpose.  All other \c{ATIn} commands will return an empty string.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATI[<n>]} \o \c{<manufacturer> <revision>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ati( const QString& params )
{
    // Only ATI and ATI0 should return the identification.
    // All other values should return an empty string.
    if ( params.isEmpty() || params == "0" )
        send( QString(QTOPIA_AT_MANUFACTURER) + " " QTOPIA_AT_REVISION );
    done();
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{ATO Return to Online Data State}
    \compat

    The \c{ATO} command returns to the online data state if a data call
    is currently in progress.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATO[<n>]} \o \c{OK}, \c{ERROR}
    \endtable

    The parameter \c{<n>} is ignored in this implementation.  If it is
    present, it should be zero according to Recommendation V.250.

    Conforms with: Recommendation V.250.
*/
void AtCommands::ato()
{
    QAtResult::ResultCode result = d->manager->callManager()->online();
    if ( result != AtCallManager::Defer )
        done( result );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATQ Result Code Suppression}
    \compat

    The \c{ATQ} command can be used to suppress (\c{ATQ1})
    or not suppress (\c{ATQ0}) the reporting of result codes.
    If no parameter is supplied (i.e. \c{ATQ}), it is the
    same as \c{ATQ0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATQ[<n>]} \o \c{OK}, \c{ERROR}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atq( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        options()->suppressResults = false;
        done();
    } else if ( params == "1" ) {
        options()->suppressResults = true;
        done();
    } else {
        done( QAtResult::Error );
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS3 Command Line Termination Character}
    \compat

    The \c{ATS3} command can be used to query or alter the character
    character used to terminate AT command lines.  There is usually no
    reason to set this to something other than the default of 13.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS3=<n>]} \o \c{OK}
    \row \o \c{ATS3?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ats3( const QString& params )
{
    int value = soption( params, options()->terminator, 0, 127 );
    if ( value != -1 )
        options()->terminator = (char)value;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS4 Response Formatting Character}
    \compat

    The \c{ATS4} command can be used to query or alter the response
    formatting (line feed) character used to format AT response lines.
    There is usually no reason to set this to something other than the
    default of 10.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS4=<n>]} \o \c{OK}
    \row \o \c{ATS4?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ats4( const QString& params )
{
    int value = soption( params, options()->response, 0, 127 );
    if ( value != -1 )
        options()->response = (char)value;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATS5 Command Line Editing Character (Backspace)}
    \compat

    The \c{ATS5} command can be used to query or alter the backspace
    character that is used to edit AT command lines.  There is
    usually no reason to set this to something other than the
    default of 8.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATS5=<n>]} \o \c{OK}
    \row \o \c{ATS5?} \o \c{<n><CR><LF>OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::ats5( const QString& params )
{
    int value = soption( params, options()->backspace, 0, 127 );
    if ( value != -1 )
        options()->backspace = (char)value;
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATV Verbose Result Codes}
    \compat

    The \c{ATV} command can be used to turn on (\c{ATV1})
    or off (\c{ATV0}) the use of verbose result codes such
    as \c{OK}, \c{ERROR}, \c{NO CARRIER}, etc.  If no parameter
    is supplied (i.e. \c{ATV}), it is the same as \c{ATV0}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATV[<n>]} \o \c{OK}, \c{ERROR}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atv( const QString& params )
{
    if ( params.isEmpty() || params == "0" ) {
        options()->verboseResults = false;
        done();
    } else if ( params == "1" ) {
        options()->verboseResults = true;
        done();
    } else {
        done( QAtResult::Error );
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{ATZ Initialize Modem}
    \compat

    The \c{ATZ} command initializes the modem and returns all status
    settings to their default values.  In the current implementation,
    this command is identical to \c{AT&F}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{ATZ[<n>]} \o \c{OK}
    \endtable

    The parameter \c{<n>} is ignored in this implementation.

    Conforms with: Recommendation V.250.
*/
void AtCommands::atz()
{
    options()->factoryDefaults();
    options()->load();
    done();
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT&F Reset to Factory Defaults}
    \compat

    The \c{AT&F} command resets all status settings to their factory
    default values.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT&F} \o \c{OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atampf()
{
    options()->factoryDefaults();
    done();
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT&W Write Settings}
    \compat

    The \c{AT&W} command saves all status settings so they can be restored the
    next time the modem is initialized.  In the current implementation,
    this command is ignored.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT&W} \o \c{OK}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atampw()
{
    options()->save();
    done();
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+BLDN Bluetooth Last Dialed Number}
    \compat

    The \c{AT+BLDN} command is used by Bluetooth hands-free (HF) devices
    to request that the last number dialed be re-dialed.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BLDN} \o \c{OK}
    \row \o \c{AT+BLDN}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atbldn()
{
#ifdef QTOPIA_BLUETOOTH
    QValueSpaceItem item( "/Communications/Calls" );
    QString lastNumber = item.value("LastDialedCall").toString();

    QAtResult::ResultCode result = d->manager->callManager()->dial( lastNumber + ";" );
    if ( result != AtCallManager::Defer )
        done( result );
#else
    done( QAtResult::Error );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BRSF Bluetooth Retrieve Supported Features}
    \compat

    The \c{AT+BRSF} command is used by Bluetooth hands-free (HF) devices
    to report the features that are supported by the hands-free unit,
    and to request the phone's audio gateway (AG) feature set.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BRSF=<HF supported features bitmap>}
         \o \c{+BRSF: <AG supported features bitmap>}
    \row \o \c{AT+BRSF=<HF supported features bitmap>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \endtable

    Set command is used by the hands-free device to send a bitmap
    of its supported features to the phone's audio gateway.  The audio
    gateway in turn responds with a separate bitmap of its supported
    features.

    \table
    \row \o \c{<HF supported features bitmap>}
         \o Decimal integer containing the following bits:
            \list
                \o 0 - EC / NR function
                \o 1 - Call waiting and 3-way calling
                \o 2 - CLI presentation capability.
                \o 3 - Voice recognition activation.
                \o 4 - Remote volume control
                \o 5 - Enhanced call status
                \o 6 - Enhanced call control
                \o 7-31 - Reserved
            \endlist
    \row \o \c{<AG supported features bitmap>}
         \o Decimal integer containing the following bits:
            \list
                \o 0 - Three way calling
                \o 1 - EC / NR function
                \o 2 - Voice recognition
                \o 3 - In-band ring
                \o 4 - Attach a number to voice tag
                \o 5 - Ability to reject a call
                \o 6 - Enhanced call status
                \o 7 - Enhanced call control
                \o 8 - Extended error result codes
                \o 9-31 - Reserved
            \endlist
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atbrsf(const QString &params)
{
#ifdef QTOPIA_BLUETOOTH
    switch ( mode( params ) ) {
        case Set:
        {
            uint posn = 1;
            uint bitmap = QAtUtils::parseNumber( params, posn );
            Q_UNUSED(bitmap);   // TODO - save this somewhere for later.
            send( "+BRSF: 225" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
#else
    Q_UNUSED(params);
    done( QAtResult::Error );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CBC Battery Charge}
    \compat

    The \c{AT+CBC} command can be used to query the current battery charge.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBC} \o \c{+CBC: <bcs>, <bcl>}
    \row \o \c{AT+CBC=?} \o \c{+CBC: (0-3),(0-100)}
    \endtable

    Execution command returns battery connection status \c{<bcs>} and
    battery charge level \c{<bcl>} of the MT.

    \table
    \row \o \c{<bcs>}
         \o \list
                \o 0 MT is powered by the battery
                \o 1 MT has a battery connected, but is not powered by it
                \o 2 MT does not have a battery connected
                \o 3 Recognized power fault, calls inhibited
            \endlist
    \row \o \c{<bcl>}
         \o \list
                \o 0 battery is exhausted, or MT does not have a battery
                   connected.
                \o 1...100 battery has 1-100 percent of capacity remaining.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcbc( const QString& params )
{
    needIndicators();
    switch ( mode( params ) ) {

        case CommandOnly:
        case Get:
        {
            // Report the current battery charge value.
            sendBatteryCharge( d->indicators->batteryCharge() );
            done();
        }
        break;

        case Support:
        {
            // Report the supported values.
            send( "+CBC: (0-3),(0-100)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CBST Select Bearer Service Type}
    \compat

    The \c{AT+CBST} command is used to select the GSM data bearer.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBST=[<speed>[,<name>[,<ce>]]]} \o \c{OK}
    \row \o \c{AT+CBST?} \o \c{+CBST: <speed>,<name>,<ce>}
    \row \o \c{AT+CBST=?} \o \c{+CBST: (list of supported <speed>s), (list of supported <name>s), (list of supported <ce>s)}
    \endtable

    Set command selects bearer \c{<name>} with data rate \c{<speed>}, and
    the connection element \c{<ce>} to be used when data calls are
    originated.  See 3GPP TS 27.007 for more information on the valid
    values for these parameters.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcbst( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            if ( options()->cbstSpeed != -1 ) {
                send( "+CBST: " +
                      QString::number( options()->cbstSpeed ) + "," +
                      QString::number( options()->cbstName ) + "," +
                      QString::number( options()->cbstCe ) );
            } else {
                // Hasn't been set yet, so return a reasonable set of defaults.
                send( "+CBST: 0,0,1" );
            }
            done();
        }
        break;

        case Support:
        {
            // Say that we support everything.  The modem will reject
            // values that aren't supported when the dial happens.
            send( "+CBST: (0-134),(0-7),(0-3)" );
            done();
        }
        break;

        case Set:
        {
            // Just save the information away.  It is up to the modem
            // to accept or reject the information when the dial is done.
            uint posn = 1;
            options()->cbstSpeed =
                (int)( QAtUtils::parseNumber( params, posn ) );
            options()->cbstName =
                (int)( QAtUtils::parseNumber( params, posn ) );
            options()->cbstCe =
                (int)( QAtUtils::parseNumber( params, posn ) );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::SupplementaryServices
    \bold{AT+CCWA Call Waiting}
    \compat

    The \c{AT+CCWA} command allows control of the Call Waiting supplementary
    service according to 3GPP TS 22.083.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CCWA=[<n>[,<mode>[,<class>]]]}
         \o when \c{<mode>}=2 and command successful:
            \code
            +CCWA: <status>,<class1>
            +CCWA: <status>,<class2>
            ...
            \endcode
    \row \o \c{AT+CCWA?} \o \c{+CCWA: <n>}
    \row \o \c{AT+CCWA=?} \o \c{+CCWA: (0,1)}
    \endtable

    Activation, deactivation, and status query are supported.
    When querying the status of a network service (\c{<mode>}=2) the
    response line for the "not active" case (\c{<status>}=0) should be
    returned only if the service is not active for any \c{<class>}.

    Parameter \c{<n>} is used to disable/enable the presentation
    of an unsolicited result code \c{+CCWA: <number>,<type>,<class>,[<alpha>][,<CLI validity>[,<subaddr>,<satype>[,<priority>]]]} to the TE when the call
    waiting service is enabled.

    Test command returns the supported unsolicited presentation values.

    \table
    \row \o \c{<n>}
         \o Unsolicited presentation status value.
            \list
                \o 0 Disable
                \o 1 Enable
            \endlist
    \row \o \c{<mode>}
         \o Mode of call waiting operation to perform.
            \list
                \o 0 Disable call waiting
                \o 1 Enable call waiting
                \o 2 Query status
            \endlist
    \row \o \c{<class>}
         \o Sum of integers representing a class of information (default 7).
            \list
                \o 1 voice (telephony)
                \o 2 data (refers to all bearer services)
                \o 4 fax
                \o 8 short message service
                \o 16 data circuit sync
                \o 32 data circuit async
                \o 64 dedicated packet access
                \o 128 dedicated PAD access
            \endlist
    \row \o \c{<status>}
         \o \list
                \o 0 not active
                \o 1 active
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<CLI validity>}
         \o \list
                \o 0 CLI valid
                \o 1 CLI has been withheld by the originator
                \o 2 CLI is not available due to interworking problems or
                     limitations of originating network.
            \endlist
    \row \o \c{<subaddr>}
         \o String type subaddress of format specified by \c{<satype>}.
    \row \o \c{<satype>}
         \o Type of subaddress octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<priority>}
         \o Digit indicating eMLPP priority level of incoming call
            (refer 3GPP TS 22.067).
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atccwa( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( options()->ccwa ? "+CCWA: 1" : "+CCWA: 0" );
            done();
        }
        break;

        case Support:
        {
            send( "+CCWA: (0,1)" );
            done();
        }
        break;

        case Set:
        {
            uint posn = 1;
            uint n = QAtUtils::parseNumber( params, posn );
            if ( n == 0 ) {
                options()->ccwa = false;
            } else if ( n == 1 ) {
                options()->ccwa = true;
            } else {
                // Invalid value for "n".
                done( QAtResult::OperationNotAllowed );
                return;
            }
            if ( posn < (uint)(params.length()) ) {
                uint mode = QAtUtils::parseNumber( params, posn );
                uint classx = 7;
                if ( posn < (uint)(params.length()) ) {
                    classx = QAtUtils::parseNumber( params, posn );
                }
                switch ( mode ) {

                    case 0:
                    {
                        // Disable call waiting for the specified call classes.
                        if ( d->callSettings->available() ) {
                            d->settingCallWaiting = true;
                            d->callSettings->setCallWaiting
                                ( false, (QTelephony::CallClass)classx );
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report not supported.
                            done( QAtResult::OperationNotSupported );
                        }
                        return;
                    }
                    // Not reached.

                    case 1:
                    {
                        // Enable call waiting for the specified call classes.
                        if ( d->callSettings->available() ) {
                            d->settingCallWaiting = true;
                            d->callSettings->setCallWaiting
                                ( true, (QTelephony::CallClass)classx );
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report not supported.
                            done( QAtResult::OperationNotSupported );
                        }
                        return;
                    }
                    // Not reached.

                    case 2:
                    {
                        // Query the current call waiting classes.
                        if ( d->callSettings->available() ) {
                            d->requestingCallWaiting = true;
                            d->callSettings->requestCallWaiting();
                            return;
                        } else {
                            // We don't have call settings support on this
                            // system at all, so report no classes enabled.
                            send( "+CCWA: 0,7" );
                        }
                    }
                    break;

                    default:
                    {
                        // Invalid mode parameter.
                        done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    // Not reached.
                }
            }
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CEER Extended error report}
    \compat

    The \c{AT+CEER} command provides extended error information.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CEER} \o \c{+CEER: <report>}
    \endtable

    Execution command causes the TA to return one or more lines of
    information text \c{<report>}, determined by the MT manufacturer,
    which should offer the user of the TA an extended report of the reason
    for the last failed operation.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atceer()
{
    if ( d->extendedError == QAtResult::OK ) {
        send( "+CEER: Error 0" );
    } else {
        QAtResult verbose;
        verbose.setResultCode( d->extendedError );
        send( "+CEER: " + verbose.verboseResult() );
    }
    done();
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CFUN Set Phone Functionality}
    \compat

    The \c{AT+CFUN} command returns information about the current
    functionality level of the phone, and allows the level to be
    modified.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CFUN=<fun>[,<rst>]} \o \c{OK}, \c{ERROR}
    \row \o \c{AT+CFUN?} \o \c{+CFUN: <fun>}
    \row \o \c{AT+CFUN=?} \o \c{+CFUN: (0-4),(0-1)}
    \endtable

    Set command selects the level of functionality \c{<fun>} in the MT.
    Read command returns the current level.  Test command returns the
    values that are supported by the MT.

    \table
    \row \o \c{<fun>}
         \o \list
                \o 0 minimum functionality
                \o 1 full functionality
                \o 2 disable phone transmit RF circuits only
                \o 3 disable phone receive RF circuits only
                \o 4 disable phone both transmit and receive RF circuits
            \endlist
    \row \o \c{<rst>}
         \o \list
                \o 0 do not reset the MT before setting it to \c{<fun>}
                \o 1 reset the MT before setting it to \c{<fun>} (not supported at present)
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcfun( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
        #ifdef QTOPIA_CELL
            send( "+CFUN: " + QString::number( (int)(d->phonerf->level()) ) );
        #else
            send( "+CFUN: 1" );
        #endif
            done();
        }
        break;

        case Set:
        {
        #ifdef QTOPIA_CELL
            // Process the <fun> parameter.  We ignore the <rst> parameter.
            uint posn = 1;
            uint fun = QAtUtils::parseNumber( params, posn );
            if ( d->phonerf->available() && fun <= 4 ) {
                d->settingPhoneRf = true;
                d->phonerf->setLevel( (QPhoneRfFunctionality::Level)fun );
            } else {
                done( QAtResult::OperationNotAllowed );
            }
        #else
            done( QAtResult::OperationNotAllowed );
        #endif
        }
        break;

        case Support:
        {
            send( "+CFUN: (0-4),(0-1)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::GPRS
    \bold{AT+CGDATA Enter Data State}
    \compat

    The \c{AT+CGDATA} command is used to enter the GPRS data state.
    In this implementation, it is identical to \c{ATD*99***1#}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGDATA=[<L2P>[,<cid>[,<cid>[,...]]]]}
         \o \c{CONNECT}, \c{ERROR}
    \row \o \c{AT+CGDATA=?} \o \c{+CGDCONT:}
    \endtable
*/
void AtCommands::atcgdata( const QString& params )
{
    switch ( mode( params ) ) {

        case CommandOnly:
        case Set:
        {
            if ( options()->contextSet ) {
                atd( "*99***1#" );
            } else {
                // GPRS context hasn't been set yet.
                done ( QAtResult::OperationNotSupported );
            }
        }
        break;

        case Support:
        {
            send( "+CGDATA: " );
            done();
        }
        break;

        default:
        {
            done ( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::GPRS
    \bold{AT+CGDCONT Define PDP context}
    \compat

    The \c{AT+CGDCONT} command is used to select the Packet Data Protocols
    and Packet Domain.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGDCONT=[<cid>[,<PDP_type>[,<APN>]]]} \o \c{OK} \c{ERROR}
    \row \o \c{AT+CGDCONT?} \o \c{+CGDCONT: <cid>,<PDP_type>,<APN>}
    \row \o \c{AT+CGDCONT=?} \o \c{+CGDCONT: (1),"IP", }
    \endtable
*/
void AtCommands::atcgdcont( const QString& params )
{
    switch ( mode( params ) ) {
        case Get:
            {
                send( "+CGDCONT: 1,\"IP\",\"" + QAtUtils::quote( options()->apn ) + "\"" );
                done();
            }
            break;
        case Set:
            {
                uint posn = 1;
                if ( QAtUtils::parseNumber( params, posn ) != 1 ) {
                    // Only 1 context id is supported.
                    done( QAtResult::OperationNotSupported );
                    break;
                }
                if ( posn >= (uint)( params.length() ) ) {
                    // Special form that clears context information.
                    options()->contextSet = false;
                    done();
                    break;
                }
                QString pdp = QAtUtils::nextString( params, posn );
                if ( pdp != "IP" ) {
                    // We only support "IP" at present.
                    done( QAtResult::OperationNotSupported );
                    break;
                }
                QString apn = QAtUtils::nextString( params, posn );
                if ( apn.startsWith( QChar('"') ) )
                    apn = apn.mid( 1 );
                if ( apn.endsWith( QChar('"') ) )
                    apn = apn.left( apn.length()-1 );
                options()->apn = apn;
                options()->contextSet = true;
                done();
            }
            break;
        case Support:
            {
                send( "+CGDCONT: (1),\"IP\"," );
                done();
            }
            break;
        default:
            {
                done ( QAtResult::OperationNotAllowed );
            }
            break;
    }
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMI Read Manufacturer Information}
    \compat

    The \c{AT+CGMI} command returns information about the manufacturer
    of the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMI} \o \c{<manufacturer>}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcgmi()
{
    send( QTOPIA_AT_MANUFACTURER );
    done();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMM Read Model Information}
    \compat

    The \c{AT+CGMM} command returns information about the model
    of the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMM} \o \c{<model>}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcgmm()
{
    send( QTOPIA_AT_MODEL );
    done();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGMR Read Revision Information}
    \compat

    The \c{AT+CGMR} command returns information about the revision
    of the phone's software.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGMR} \o \c{<revision>}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcgmr()
{
    send( QTOPIA_AT_REVISION );
    done();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+CGSN Read Product Serial Number}
    \compat

    The \c{AT+CGSN} command returns information about the serial number
    of the phone.  Note: the return value may be a static string for all
    phones of a type, so it should not be relied upon to be unique.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CGSN} \o \c{<sn>}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcgsn()
{
    send( QTOPIA_AT_SERIAL_NUMBER );
    done();
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CLCC List Current Calls}
    \compat

    The \c{AT+CLCC} command lists the calls that are currently
    active within the system.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLCC}
         \o \code
            +CLCC: <id1>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]
            +CLCC: <id2>,<dir>,<stat>,<mode>,<mpty>[,<number>,<type>[,<alpha>[,<priority>]]]
            ...
            \endcode
    \endtable

    \table
    \row \o <id> \o Integer identifier for the call.
    \row \o <dir> \o Direction of the call: 0 = outgoing MO, 1 = incoming MT.
    \row \o <stat>
         \o State of the call:
            \list
                \o 0 active
                \o 1 held
                \o 2 dialing (MO call)
                \o 3 alerting (MO call)
                \o 4 incoming (MT call)
                \o 5 waiting (MT call)
            \endlist
    \row \o <mode>
         \o Bearer/teleservice:
            \list
                \o 0 voice
                \o 1 data
                \o 2 fax
                \o 3 voice followed by data, voice mode
                \o 4 alternating voice/data, voice mode
                \o 5 alternating voice/fax, voice mode
                \o 6 voice followed by data, data mode
                \o 7 alternating voice/data, data mode
                \o 8 alternating voice/fax, fax mode
                \o 9 unknown - used to indicate video calls
            \endlist
    \row \o <mpty> \o Multiparty indicator: 1 = multi-party, 0 = no multi-party.
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<priority>}
         \o Digit indicating eMLPP priority level of incoming call
            (refer 3GPP TS 22.067).
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atclcc()
{
    QStringList list = d->manager->callManager()->formatCallList();
    foreach ( QString line, list ) {
        send( line );
    }
    done();
}

/*!
    \ingroup ModemEmulator::SupplementaryServices
    \bold{AT+CLIP Calling line identification presentation}
    \compat

    The \c{AT+CLIP} command allows control of the calling line identification
    presentation supplementary service.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CLIP=[<n>]} \o \c{OK}
    \row \o \c{AT+CLIP?} \o \c{+CLIP: <n>,<m>}
    \row \o \c{AT+CLIP=?} \o \c{+CLIP: (0,1)}
    \endtable

    Set command enables or disables the presentation of the CLI at the TE.
    It has no effect on the execution of the supplementary service in the
    network.  When \c{<n>} is 1, the unsolicited response
    \c{+CLIP: <number>,<type>[,<subaddr>,<satype>[,[<alpha>][,<CLI validitity>]]}
    is sent to the TE after every \c{RING} or \c{+CRING} response.

    Read command gives the status of \c{<n>}, plus the provision status
    of the CLIP service (\c{<m>}).

    \table
    \row \o \c{<n>}
         \o Unsolicited presentation status value.
            \list
                \o 0 Disable
                \o 1 Enable
            \endlist
    \row \o \c{<m>}
         \o Provision status of CLIP in the network:
            \list
                \o 0 CLIP not provisioned
                \o 1 CLIP provisioned
                \o 2 unknown (e.g. no network, etc.)
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<subaddr>}
         \o String type subaddress of format specified by \c{<satype>}.
    \row \o \c{<satype>}
         \o Type of subaddress octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<CLI validity>}
         \o \list
                \o 0 CLI valid
                \o 1 CLI has been withheld by the originator
                \o 2 CLI is not available due to interworking problems or
                     limitations of originating network.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atclip( const QString& params )
{
    // The ",1" indicates that CLIP is always provisioned, even if it isn't.
    // The underlying system handles the case of no CLIP better than us.
    flagCommand( "+CLIP: ", options()->clip, params, ",1" );
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CHLD Call Hold And Multiparty}
    \compat

    The \c{AT+CHLD} command is used to control call hold, release,
    and multiparty states.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CHLD=[<n>]} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CHLD=?} \o \c{+CHLD: (0-4,11-19,21-29)}
    \endtable

    The parameter \c{<n>} is an integer value in accordance to
    3GPP TS 22.030:

    \table
    \header \o \c{<n>} \o Description
    \row \o 0 \o Release all held calls or set the busy state
                 for the waiting call.
    \row \o 1 \o Release all active calls.
    \row \o 1x \o Release only call \c{x}.
    \row \o 2 \o Put active calls on hold and activate the waiting or held call.
    \row \o 2x \o Put active calls on hold and activate call \c{x}.
    \row \o 3 \o Add the held calls to the active conversation.
    \row \o 4 \o Add the held calls to the active conversation, and then
                 detach the local subscriber from the conversation.
    \endtable

    GSM modems typically cannot support more than 7 or 8 calls at once.
    This implementation can support up to 99 calls using two-digit
    call identifiers for \c{x} between 10 and 99.  Test command only
    reports 9 call identifiers for backwards compatibility with
    existing client software.

    Conforms with: 3GPP TS 27.007, 22.030.
*/
void AtCommands::atchld( const QString& params )
{
    switch ( mode( params ) ) {

        case Support:
        {
            // Report all of the arguments that we support.  We actually
            // support more than the 9 call identifiers listed here, but
            // it isn't easy to express it in a GSM-compatible fashion.
            send( "+CHLD: (0-4,11-19,21-29)" );
            done();
        }
        break;

        case Set:
        {
            uint posn = 1;
            uint arg = QAtUtils::parseNumber( params, posn );
            uint callID = 0;
            if ( arg >= 11 && arg <= 19 ) {
                // Hang up a specific active call: single-digit call identifier.
                callID = arg - 10;
                arg = 1;
            } else if ( arg >= 110 && arg <= 199 ) {
                // Hang up a specific active call: two-digit call identifier.
                callID = arg - 100;
                arg = 1;
            } else if ( arg >= 21 && arg <= 29 ) {
                // Put current calls on hold and activate a specific call.
                callID = arg - 20;
                arg = 2;
            } else if ( arg >= 210 && arg <= 299 ) {
                // Put calls on hold and activate: two-digit call identifier.
                callID = arg - 200;
                arg = 2;
            }
            QAtResult::ResultCode result = AtCallManager::Defer;
            switch ( arg ) {

                // The following values are from GSM 22.030, section 6.5.5.1.

                case 0:
                {
                    // Release all held calls or set busy for a waiting call.
                    result = d->manager->callManager()->hangupHeldCalls();
                }
                break;

                case 1:
                {
                    // Hangup all active calls or just the selected call.
                    if ( !callID )
                        result = d->manager->callManager()->hangup();
                    else
                        result = d->manager->callManager()->hangup( callID );
                }
                break;

                case 2:
                {
                    // Place all calls on hold and activate one or more that
                    // were held or waiting.
                    if ( !callID )
                        result = d->manager->callManager()->activateHeldCalls();
                    else
                        result = d->manager->callManager()->activate( callID );
                }
                break;

                case 3:
                {
                    // Adds the held calls to the current conversation (join).
                    result = d->manager->callManager()->join();
                }
                break;

                case 4:
                {
                    // Connect held and active calls, then hangup (transfer).
                    result = d->manager->callManager()->transfer();
                }
                break;

                default:
                {
                    result = QAtResult::OperationNotAllowed;
                }
                break;
            }
            if ( result != AtCallManager::Defer )
                done( result );
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CHUP Hangup Call}
    \compat

    The \c{AT+CHUP} command is an alias for \c{ATH}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CHUP} \o \c{OK}, \c{ERROR}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atchup()
{
    ath();
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CIMI Request International Mobile Subscriber Identity}
    \compat

    The \c{AT+CIMI} command retrieves the IMSI value from the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CIMI} \o \c{<IMSI>}
    \row \o \c{AT+CIMI} \o \c{+CME ERROR: 10}
    \endtable

    An error will be returned if there is no SIM present, or the SIM
    is still being initialized and the IMSI is not available yet.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcimi()
{
#ifdef QTOPIA_CELL
    QSimInfo simInfo;
    QString imsi = simInfo.identity();
    if ( imsi.isEmpty() ) {
        done( QAtResult::SimNotInserted );
    } else {
        send( imsi );
        done();
    }
#else
    done( QAtResult::SimNotInserted );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CIND Indicator Control}
    \compat

    The \c{AT+CIND} command is used to get the current indicator values.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CIND=[<ind>,[<ind>[,...]]]} \o \c{+CME ERROR: 3}
    \row \o \c{AT+CIND?} \o \c{+CIND: <ind>[,<ind>[,...]]}
    \row \o \c{AT+CIND=?} \o \c{+CIND: (<descr>,(list of <ind>'s))[,...]}
    \endtable

    Set command is used to set the indicator values.  In this implementation,
    indicators cannot be set and the set command will always return an error.

    Read command returns the current status of the indicators.

    Test command returns pairs, where the string value \c{<descr>} is a
    description of the indicator, and the compound value is the allowable
    values for the indicator.

    The following indicators are supported:

    \table
    \header \o Name \o Description \o Range
    \row \o \c{battchg} \o Battery charge level \o 0-5
    \row \o \c{signal} \o Signal quality \o 0-5
    \row \o \c{service} \o Service availability \o 0-1
    \row \o \c{message} \o Message received \o 0-1
    \row \o \c{call} \o Call in progress \o 0-1
    \row \o \c{roam} \o Roaming indicator \o 0-1
    \row \o \c{smsfull}
         \o SMS memory state \o
            \list
                \o 0 space is available
                \o 1 memory has just become full
                \o 2 memory is full and a message was just rejected
            \endlist
    \row \o \c{callsetup}
         \o Call setup state \o
            \list
                \o 0 no call setup in progress
                \o 1 incoming call setup in progress
                \o 2 outgoing call setup in progress
                \o 3 outgoing call setup in the "alerting" phase
            \endlist
    \row \o \c{callhold}
         \o Call hold state \o
            \list
                \o 0 no calls are held
                \o 1 there are both active and held calls
                \o 2 there are held calls, but no active calls
            \endlist
    \endtable

    The \c{callsetup} and \c{callhold} indicators are from the
    Bluetooth Hands-Free Profile version 1.5.  The rest are from
    3GPP TS 27.007.

    Conforms with: 3GPP TS 27.007, Bluetooth Hands-Free Profile 1.5.
*/
void AtCommands::atcind( const QString& params )
{
    needIndicators();
    switch ( mode( params ) ) {

        case Get:
        {
            QString result;
            for ( int ind = 0; ind < d->indicators->numIndicators(); ++ind ) {
                if ( !result.isEmpty() )
                    result += ",";
                result += QString::number( d->indicators->value( ind ) );
            }
            send( "+CIND: " + result );
            done();
        }
        break;

        case Support:
        {
            QString result;
            for ( int ind = 0; ind < d->indicators->numIndicators(); ++ind ) {
                if ( !result.isEmpty() )
                    result += ",";
                result += "(\"" + QAtUtils::quote( d->indicators->name( ind ) )
                       + "\",(0-"
                       + QString::number( d->indicators->maxValue( ind ) )
                       + "))";
            }
            send( "+CIND: " + result );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CKPD Keypad Control}
    \compat

    The \c{AT+CKPD} command is used to send keypad events to the phone.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CKPD=<keys>,[<time>[,<pause>]]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CKPD=?} \o \c{+CKPD: (0123456789*#),(0-255),(0-255)}
    \endtable

    Execution command is used to send keypad events to the phone.
    The following \c{<keys>} are defined by 3GPP TS 27.007:

    \table
    \header \o Character \o Description
    \row \o \c{#} \o Hash (number sign)
    \row \o \c{%} \o Percent sign
    \row \o \c{*} \o Star
    \row \o \c{0} ... \c{9} \o Digit keys
    \row \o \c{:C} \o Send \c{C} as a literal character
    \row \o \c{;CCC...;} \o Send \c{CCC...} as literal characters
    \row \o \c{<} \o Left arrow key
    \row \o \c{>} \o Right arrow key
    \row \o \c{@} \o Alpha key for switching input method modes
    \row \o \c{C/c} \o Clear
    \row \o \c{D/d} \o Volume down
    \row \o \c{E/e} \o Connection end (END or hangup key)
    \row \o \c{F/f} \o Function key
    \row \o \c{L/l} \o Phone lock
    \row \o \c{M/m} \o Menu
    \row \o \c{Q/q} \o Quiet/mute
    \row \o \c{S/s} \o Connection start (SEND or call key)
    \row \o \c{U/u} \o Volume up
    \row \o \c{V/v} \o Down arrow key
    \row \o \c{W/w} \o Pause character
    \row \o \c{Y/y} \o Delete last character (same as \c{C/c})
    \row \o \c{[} \o Soft key 1 (same as \c{F/f})
    \row \o \c{]} \o Soft key 2 (same as \c{C/c})
    \row \o \c{^} \o Up arrow key
    \endtable

    The following keys from 3GPP TS 27.007 are not currently supported:

    \table
    \header \o Character \o Description
    \row \o \c{A/a} \o Channel A
    \row \o \c{B/b} \o Channel B
    \row \o \c{P/p} \o Power
    \row \o \c{R/r} \o Recall last number
    \row \o \c{T/t} \o Store/memory
    \row \o \c{X/x} \o Auxillary key
    \endtable

    The following additional non-27.007 keys are supported:

    \table
    \header \o Character \o Description
    \row \o \c{H/h} \o Key on headphones
    \endtable

    \table
    \row \o \c{time}
         \o Time that the key should be held down, in tenths of a second.
            The default is 1.
    \row \o \c{pause}
         \o The time to pause between keys, in tenths of a second.
            The default is 1.
    \endtable

    Conforms with: 3GPP TS 27.007
*/
void AtCommands::atckpd( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            uint posn = 1;
            QString keys = QAtUtils::nextString( params, posn );
            if ( posn < (uint)(params.length()) )
                d->keyPressTime = QAtUtils::parseNumber( params, posn );
            else
                d->keyPressTime = 1;
            if ( posn < (uint)(params.length()) )
                d->keyPauseTime = QAtUtils::parseNumber( params, posn );
            else
                d->keyPauseTime = 1;
            QList< QPair<int,int> > codes;
            for ( int index = 0; index < keys.length(); ++index ) {
                int ch = keys[index].unicode();
                int unicode, keycode;
                unicode = 0xFFFF;
                keycode = 0;
                switch ( ch ) {

                    // Keys with Unicode equivalents.
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                    case '*': case '#': case '%':
                    {
                        unicode = keycode = ch;
                    }
                    break;

                    // Primary mappings.
                    case '@':           keycode = Qt::Key_Mode_switch; break;
                    case '<':           keycode = Qt::Key_Left; break;
                    case '>':           keycode = Qt::Key_Right; break;
                    case 'c': case 'C': keycode = Qt::Key_Back; break;
                    case 'd': case 'D': keycode = Qt::Key_VolumeDown; break;
                    case 'e': case 'E': keycode = Qt::Key_Hangup; break;
                    case 'f': case 'F': keycode = Qt::Key_Context1; break;
                    case 'l': case 'L': keycode = Qt::Key_F29; break;
                    case 'm': case 'M': keycode = Qt::Key_Select; break;
                    case 'q': case 'Q': keycode = Qt::Key_VolumeMute; break;
                    case 's': case 'S': keycode = Qt::Key_Call; break;
                    case 'u': case 'U': keycode = Qt::Key_VolumeUp; break;
                    case 'v': case 'V': keycode = Qt::Key_Down; break;
                    case '^':           keycode = Qt::Key_Up; break;

                    // Aliases.
                    case 'y': case 'Y': keycode = Qt::Key_Back; break;
                    case '[':           keycode = Qt::Key_Context1; break;
                    case ']':           keycode = Qt::Key_Back; break;

                    // Key on headphones.
                    case 'h': case 'H': keycode = Qt::Key_F28; break;

                    // Literal key.
                    case ':':
                    {
                        ++index;
                        if ( index < keys.length() ) {
                            unicode = keycode = keys[index].unicode();
                        } else {
                            done( QAtResult::OperationNotAllowed );
                            return;
                        }
                    }
                    break;

                    // Literal key sequence.
                    case ';':
                    {
                        ++index;
                        while ( index < keys.length() &&
                                keys[index] != QChar(';') ) {
                            unicode = keycode = keys[index].unicode();
                            codes.append( QPair<int,int>( unicode, keycode ) );
                            ++index;
                        }
                        if ( index >= keys.length() ) {
                            // No trailing ';' at the end of the sequence.
                            done( QAtResult::OperationNotAllowed );
                            return;
                        }
                    }
                    continue;   // Keys have already been added to the list.
                }
                codes.append( QPair<int,int>( unicode, keycode ) );
            }
            d->keys = codes;
            d->sendRelease = false;
            sendNextKey();
        }
        break;

        case Support:
        {
            send( "+CKPD: (0123456789*#),(0-255),(0-255)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMEC Mobile Termination Control Mode}
    \compat

    The \c{AT+CMEC} command is used to select the modes that can
    control the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMEC=[<keyp>,[<disp>[,<ind>]]]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CMEC?} \o \c{+CMEC: 0,0,0}
    \row \o \c{AT+CMEC=?} \o \c{+CMEC: (0),(0),(0)}
    \endtable

    In this implementation, keypad and display control are not supported,
    and indicators can only be read, never written.  It is an error to use
    the set command with non-zero parameters.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcmec( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            // All three parameters must be zero, or the command will fail.
            // If a parameter is not supplied, parseNumber() defaults to zero.
            uint posn = 1;
            if ( QAtUtils::parseNumber( params, posn ) != 0 ||
                 QAtUtils::parseNumber( params, posn ) != 0 ||
                 QAtUtils::parseNumber( params, posn ) != 0 ) {
                done( QAtResult::OperationNotAllowed );
            } else {
                done();
            }
        }
        break;

        case Get:
        {
            send( "+CMEC: 0,0,0" );
            done();
        }
        break;

        case Support:
        {
            send( "+CMEC: (0),(0),(0)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMEE Report Mobile Termination Error}
    \compat

    The \c{AT+CMEE} command is used to select the error reporting mode
    for the MT.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMEE=[<n>]} \o \c{OK}, \c{+CME ERROR: 4}
    \row \o \c{AT+CMEE?} \o \c{+CMEE: <n>}
    \row \o \c{AT+CMEE=?} \o \c{+CMEE: (0-2)}
    \endtable

    Set command disables or enables the use of result code
    \c{+CME ERROR: <err>} as an indication of an error relating
    to the functionality of the MT.  When enabled, MT related errors
    cause the \c{+CME ERROR: <err>} final result code instead of the
    regular \c{ERROR} result code.  \c{ERROR} is still returned
    normally when the error is related to syntax, invalid parameters,
    or TA functionality.

    \table
    \row \o \c{<n>}
         \o \list
                \o 0 disable \c{+CME ERROR: <err>} result code and use
                     \c{ERROR} instead.  This is the default value.
                \o 1 enable \c{+CME ERROR: <err>} result code and use
                     numeric \c{<err>} values.
                \o 2 enable \c{+CME ERROR: <err>} result code and use
                     verbose \c{<err>} values.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcmee( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( "+CMEE: " + QString::number( options()->extendedErrors ) );
            done();
        }
        break;

        case Support:
        {
            send( "+CMEE: (0-2)" );
            done();
        }
        break;

        case Set:
        {
            int value = params.mid(1).toInt();
            if ( value < 0 || value > 2 ) {
                done( QAtResult::OperationNotSupported );
            } else {
                options()->extendedErrors = value;
                done();
            }
        }
        break;

        default:
        {
            done( QAtResult::OperationNotSupported );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMER Mobile Termination Event Reporting}
    \compat

    The \c{AT+CMER} command enables or disables unsolicited result
    codes related to MT events.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMER=[<mode>[,<keyp>[,<disp>[,<ind>[,<bfr>]]]]]}
         \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CMER?} \o \c{+CMER: 1,0,0,<ind>,0}
    \row \o \c{AT+CMER=?} \o \c{+CMER: (1),(0),(0),(0-2),(0)}
    \endtable

    Set command enables or disables unsolicited result codes for
    indicators.  This implementation does not support the other event
    types and \c{<mode>} must always be 1.

    \table
    \row \o \c{<ind>}
         \o \list
                \o 0 no indicator event reporting.
                \o 1 indicator event reporting using result code
                     \c{+CIEV: <ind>,<value>}.  Only those indicator
                     changes that are not caused by \c{AT+CIND} set
                     command are reported.
                \o 2 indicator event reporting using result code
                     \c{+CIEV: <ind>,<value>}.  All indicator changes
                     are reported.
            \endlist
    \endtable

    In this implementation, \c{AT+CIND} set command is not supported,
    so \c{<ind>} values 1 and 2 are identical.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcmer( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( "+CMER: 1,0,0," +
                  QString::number( options()->cind ) + ",0" );
            done();
        }
        break;

        case Set:
        {
            // Search for the indicator setting, but ignore everything else.
            uint posn = 1;
            int index = 0;
            int value;
            while ( posn < (uint)(params.length()) ) {
                value = (int)(QAtUtils::parseNumber( params, posn ));
                if ( index == 3 ) {
                    needIndicators();
                    if ( value < 0 || value > 2 ) {
                        done( QAtResult::OperationNotAllowed );
                        return;
                    }
                    options()->cind = value;
                    break;
                }
                ++index;
            }
            done();
        }
        break;

        case Support:
        {
            send( "+CMER: (1),(0),(0),(0-2),(0)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGF Message Format}
    \compat

    The \c{AT+CMGF} command can be used to set the SMS message format
    to PDU (0) or text (1).  The default is PDU.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGF=<mode>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CMGF?} \o \c{+CMGF: <mode>}
    \row \o \c{AT+CMGF=?} \o \c{+CMGF: (0,1)}
    \endtable
*/
void AtCommands::atcmgf( const QString& params )
{
    // Ignore the command if the SMS subsystem is not yet ready.
    if ( d->smsReader->ready() )
        flagCommand( "+CMGF: ", options()->messageFormat, params );
    else
        done( QAtResult::USimBusy );
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMGS Send Message}
    \compat

    The \c{AT+CMGS} command can be used to send SMS messages in
    either PDU (\c{AT+CMGF=0}) or text (\c{AT+CMGF=1}) mode.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMGS=<da>[,<toda>]<CR>message<ctrl-Z/ESC>}
         \o \c{+CMGS: <mr>}, \c{+CME ERROR: <err>} - if in text mode.
    \row \o \c{AT+CMGS=<length><CR>pdu<ctrl-Z/ESC>}
         \o \c{+CMGS: <mr>}, \c{+CME ERROR: <err>} - if in pdu mode.
    \row \o \c{AT+CMGS=?} \o \c{OK}
    \endtable

    Execution command sends the message to the network.  The message
    reference value \c{<mr>} is returned to the TE upon successful message
    delivery.
*/
void AtCommands::atcmgs( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            // Extract the destination number if we are sending in text mode.
            // If we are sending in PDU mode, we ignore the length as it will
            // be implicit in the data that follows.
            if ( options()->messageFormat ) {
                uint posn = 1;
                d->smsNumber = QAtUtils::nextString( params, posn );
                uint type = QAtUtils::parseNumber( params, posn );
                if ( type == 0 )
                    type = 129;
                d->smsNumber = QAtUtils::decodeNumber( d->smsNumber, type );
            }

            // Request an extra line of data for the text/pdu.
            d->sendingSms = true;
            frontEnd()->requestExtra();
        }
        break;

        case Support:
        {
            // Just need to say "OK" to indicate that we support this command.
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CMMS More Messages to Send}
    \compat

    The \c{AT+CMMS} command can be used to inform the modem that several
    SMS messages will be sent in quick succession, and the link should be
    held open for more efficient transmission.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMMS?} \c \c{+CMMS: 0}
    \row \o \c{AT+CMGS=?} \o \c{+CMMS: (0-2)}
    \endtable

    This implementation does not do anything special for this command.
    The underlying modem will hold the link open when it judges that it
    would be advantageous to do so.
*/
void AtCommands::atcmms( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( "+CMMS: 0" );
            done();
        }
        break;

        case Set:
        {
            // Allow the user to set anything - we don't do anything special.
            done();
        }
        break;

        case Support:
        {
            send( "+CMMS: (0-2)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CMUX Multiplexing Mode}
    \compat

    The \c{AT+CMUX} enables multiplexing according to 3GPP TS 27.010.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CMUX=[<mode>[,<subset>[,<port_speed>[,<frame_size>[,...]]]]]}
         \o \c{OK}, \c{+CME ERROR: 4}
    \row \o \c{AT+CMUX=?} \o \c{+CMUX: (0),(0),(1-6),(1-32768),(1-255),(0-100),(2-255),(1-255),(1-7)}
    \endtable

    Set command enables multiplexing mode according to the supplied
    parameters.  This implementation supports basic mode, UIH frames,
    any port speed, and any frame size.  Set command will report an
    error if some other mode is requested, or the command is used
    on a channel that already has multiplexing enabled.

    \table
    \row \o \c{<mode>}
         \o Multiplexer transparency mechanism:
            \list
                \o 0 Basic mode
                \o 1 Advanced mode; not supported
            \endlist
    \row \o \c{<subset>}
         \o Subset of \c{<mode>} to use:
            \list
                \o 0 UIH frames used only
                \o 1 UI frames used only; not supported
                \o 2 I frames used only; not supported
            \endlist
    \row \o \c{<port_speed>} \o Transmission rate; ignored.
    \row \o \c{<frame_size>} \o Maximum frame size, 1-32768.  Default is 31.
    \endtable

    Test command returns the parameter values that are supported,
    including additional parameters from 3GPP TS 27.007 which are
    ignored by this implementation.

    Conforms with: 3GPP TS 27.007, 27.010.
*/
void AtCommands::atcmux( const QString& params )
{
    if ( ! d->frontEnd->canMux() ) {
        // Cannot use GSM 07.10 multiplexing on a multiplexed channel.
        done( QAtResult::OperationNotSupported );
    } if ( params.startsWith( "=?" ) ) {
        // Indicate that we support Basic, UIH frames, all frame sizes.
        send( "+CMUX: (0),(0),(1-6),(1-32768),(1-255),(0-100),"
              "(2-255),(1-255),(1-7)" );
        done();
    } else if ( params.startsWith( "=" ) ) {
        // Parse the first four parameters and validate.  Ignore the rest.
        uint posn = 1;
        uint mode = QAtUtils::parseNumber( params, posn );
        uint subset = QAtUtils::parseNumber( params, posn );
        QAtUtils::parseNumber( params, posn );  // Skip port speed: not used.
        uint frameSize = QAtUtils::parseNumber( params, posn );
        if ( !frameSize )
            frameSize = 31;     // Use the default frame size if not set.
        if ( mode != 0 || subset != 0 || frameSize > 32768 ) {
            done( QAtResult::OperationNotSupported );
            return;
        }

        // Send the OK response.  But make sure that done() and
        // processNextCommand() are not called because we are about
        // to shut down the underlying device for use by this object.
        d->frontEnd->send( QAtResult::OK );
        d->cmdsPosn = d->cmds.size();

        // Create a multiplexing wrapper around the underlying device.
        QGsm0710MultiplexerServer *mux;
        d->underlying = d->frontEnd->device();
        d->frontEnd->setDevice( 0 );
        d->multiplexer = mux = new QGsm0710MultiplexerServer
                ( d->underlying, frameSize, false, this );
        connect( mux, SIGNAL(opened(int,QSerialIODevice*)),
                 this, SLOT(channelOpened(int,QSerialIODevice*)) );
        connect( mux, SIGNAL(terminated()), this, SLOT(muxTerminated()) );
    } else {
        // Incorrectly formatted AT+CMUX command.
        done( QAtResult::OperationNotSupported );
    }
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CNUM Subscriber Number}
    \compat

    The \c{AT+CNUM} command retrieves the MSISDN's related to the subscriber
    from the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CNUM}
         \o \code
            +CNUM: [<alpha>],<number>,<type>[,<speed>,<service>[,<itc>]]
            ...
            \endcode
    \endtable

    Execution command returns the MSISDN's related to the subscriber from
    the SIM.  If the subscriber has different MSISDN's for different
    services, each MSISDN is returned on a separate line.

    \table
    \row \o \c{<alpha>}
         \o String indicating the name of a phonebook entry
            corresponding to \c{<number>}.  Usually this is empty.
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \row \o \c{<speed>}
         \o GSM bearer speed value.  See 3GPP TS 27.007 for more information.
    \row \o \c{<service>}
         \o \list
                \o Asynchronous modem
                \o Synchronous modem
                \o PAD access
                \o Packet access
                \o Voice
                \o Fax
            \endlist
    \row \o \c{<itc>}
         \o \list
                \o 0 3.1 kHz
                \o 1 UDI
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcnum()
{
    queryNumber( QServiceNumbers::SubscriberNumber );
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+COPS Operator Selection}
    \compat

    The \c{AT+COPS} command is used to select operators, to request the
    current operator details, and to request a list of the available operators.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+COPS=[<mode>[,<format>[,<oper>[,<AcT>]]]]}
         \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+COPS?} \o \c{+COPS: <mode>[,<format>,<oper>[,<AcT>]]}
    \row \o \c{AT+COPS=?} \o \c{+COPS: [list of supported (<stat>,long <oper>,short <oper>,numeric <oper>[,<AcT>])][,,(list of supported <mode>s),(list of supported <format>s)]}
    \endtable

    Set command forces an attempt to select and register the network operator.
    \c{<mode>} is used to select whether the selection is done automatically
    by the MT or is forced by this command to use operator \c{<oper>}.
    If the selected operator is not available, no other operator shall
    be selected (except for \c{<mode>}=4).  The selected operator name
    format shall apply to further read commands.  \c{<mode>}=2 forces an
    attempt to deregister from the network.

    Read command returns the current mode, the currently selected operator
    and the current access technology (AcT).  If no operator is selected,
    \c{<format>}, \c{<oper>}, and \c{<AcT>} are omitted.

    Test command returns a set of five parameters, each representing an
    operator present in the network.  Each set consists of a \c{<stat>}
    integer indicating the availability of the operator, long and short
    alphanumeric names for the operator, the numeric name for the operator,
    and the access technology.

    \table
    \row \o \c{<mode>}
         \o \list
                \o 0 automatic
                \o 1 manual
                \o 2 deregister from the network
                \o 3 set only \c{<format>}
                \o 4 manual/automatic; if manual selection fails, use automatic
            \endlist
    \row \o \c{<format>}
         \o \list
                \o 0 long format alphanumeric \c{<oper>}
                \o 1 short format alphanumeric \c{<oper>}
                \o 2 numeric \c{<oper>}
            \endlist
    \row \o \c{<oper>} \o String type indicating the name of the operator
                          according to \c{<format>}.
    \row \o \c{<stat>}
         \o \list
                \o 0 unknown
                \o 1 available
                \o 2 current
                \o 3 forbidden
            \endlist
    \row \o \c{<AcT>}
         \o \list
                \o 0 GSM
                \o 1 GSM Compact
                \o 2 UTRAN
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcops( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            // Send the operator details, which we always send in
            // long alphanumeric format, regardless of the format setting.
            // If the technology type is "GSM", we omit the "AcT" parameter
            // as older software conformant with 3GPP TS 07.07 may not know
            // what to do with the extra parameter.
            QString status = "+COPS: ";
            status += QString::number
                ( (int)(d->netReg->currentOperatorMode()) );
            if ( !d->netReg->currentOperatorName().isEmpty() ) {
                status += ",0,\"";
                status += QAtUtils::quote( d->netReg->currentOperatorName() );
                status += "\"";
                QString tech = d->netReg->currentOperatorTechnology();
                if ( tech == "GSMCompact" )        // No tr
                    status += ",1";
                else if ( tech == "UTRAN" )        // No tr
                    status += ",2";
            }
            send( status );
            done();
        }
        break;

        case Support:
        {
            // Request the available operators and report them.
            if ( d->netReg->available() ) {
                d->requestingAvailableOperators = true;
                d->netReg->requestAvailableOperators();
            } else {
                // No service, so no network operators to report.
                send( "+COPS: ,,(0-4),(0-2)" );
                done();
            }
        }
        break;

        case Set:
        {
            // Set the current operator details.
            uint posn = 1;
            uint mode = QAtUtils::parseNumber( params, posn );
            uint format = QAtUtils::parseNumber( params, posn );
            QString oper = QAtUtils::nextString( params, posn );
            uint tech = QAtUtils::parseNumber( params, posn );
            if ( mode == 3 ) {
                // Set format for read command.  In this implementation,
                // we only support "long format alphanumeric" in read.
                // Ignore the format.  Since most users of this command
                // use AT+COPS=3,<format> to set long format alphanumeric
                // anyway, this is not expected to be a problem.
                done();
            } else if ( mode <= 4 ) {
                // Determine the operator id and technology strings.
                QString id, technology;
                if ( !oper.isEmpty() ) {
                    id = QString::number( format ) + oper;
                    if ( tech == 0 )
                        technology = "GSM";             // No tr
                    else if ( tech == 1 )
                        technology = "GSMCompact";      // No tr
                    else if ( tech == 2 )
                        technology = "UTRAN";           // No tr
                    else {
                        // Unknown technology value.
                        done( QAtResult::OperationNotAllowed );
                        return;
                    }
                }
                if ( d->netReg->available() ) {
                    d->settingCurrentOperator = true;
                    d->netReg->setCurrentOperator
                        ( (QTelephony::OperatorMode)mode, id, technology );
                } else {
                    // No service available, so cannot change operators.
                    done( QAtResult::OperationNotSupported );
                }
            } else {
                // Invalid mode value.
                done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CPAS Phone Activity Status}
    \compat

    The \c{AT+CPAS} returns information about the mode the phone is
    currently operating in.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPAS} \o \c{+CPAS: <pas>}
    \row \o \c{AT+CPAS=?} \o \c{+CPAS: (0-5)}
    \endtable

    Execution command returns the activity status \c{<pas>} of the MT.
    It can be used to interrogate the MT before requesting action
    from the phone.

    Test command returns the values that are supported by the MT.

    \table
    \row \o \c{<pas>}
         \o \list
                \o 0 ready
                \o 1 unavailable
                \o 2 unknown
                \o 3 ringing
                \o 4 call in progress
                \o 5 asleep
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcpas( const QString& params )
{
    switch ( mode( params ) ) {

        case CommandOnly:
        {
            if ( d->manager->callManager()->ringing() )
                send( "+CPAS: 3" );
            else if ( d->manager->callManager()->callInProgress() )
                send( "+CPAS: 4" );
            else
                send( "+CPAS: 0" );
            done();
        }
        break;

        case Support:
        {
            send( "+CPAS: (0-5)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBF Find phonebook entries}
    \compat

    The \c{AT+CPBF} command allows for searching the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row    \o \c{AT+CPBF=<findtext>}   \o \c{[+CPBF: <index1>,<number>,<type>,<text>[<hidden>][[...]<CR><LF>
                                               +CPBF: <index2>,<number>,<type>,<text>[<hidden>]]]}, \c{+CME ERROR: <err>}
    \row    \o \c{AT+CPBF=?}            \o \c{+CPBF: [<nlength>],[<tlength>]}, \c{+CME ERROR: <err>}
    \endtable
   
    Test command returns the maximum length of \c{<number>} and \c{<text>} fields.

    The set command returns a set of four parameters, each representing an entry in the phonebook memory. If the memory
    has several entries which alphanumeric field start with \c{<findtext>} each entry is returned on a separate line. 
    The search is caseinsensitive.

    \table
    \row \o \c{<index1>, <index2>} \o integer number indicating the location number of a phonebook entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145 
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    Conforms with: 3GPP TS 27.007.
    */
void AtCommands::atcpbf( const QString& params )
{
    switch( mode( params ) ) 
    {
        case Support:
        {
            QString status;
            if ( options()->phoneStore == "ME" ) {
                status = "+CPBF: 60,60"; //fixed values
                send( status );
                done();
            } else {
                d->limitsReqBy = "cpbfSupport";
                d->phoneBookQueried = true;
                d->phoneBook->requestLimits( options()->phoneStore );
            }
        }
        break;
        case Set:
        {
            uint pos = 1;
            if ( params[1] != QChar('"') ) {
                done( QAtResult::OperationNotAllowed );
                return;
            }

            d->pbSearchText = nextString( params, pos);

            if ( options()->phoneStore == "ME" ) {
                if ( !d->contactModel )
                    initializeMemoryPhoneBook();
                //find contact with given name
                QModelIndex start = d->contactModel->index(0, 0);
                QModelIndexList list = d->contactModel->match( start, 
                        0 /*don't care what role*/, 
                        QVariant( d->pbSearchText ), -1 );

                if ( list.count() == 0 ) {
                    done( QAtResult::NotFound );
                    return;
                }
                QSet<QUniqueId> foundIds;
                foreach( QModelIndex i, list )
                    foundIds.insert( d->contactModel->id( i ) );

                QString status = "+CPBF: %1,\"%2\",%3,\"%4\"";
                QString temp;
                QList<int> usedIndices = d->contactIndices.uniqueKeys();
                QList<int>::const_iterator iter;
                for ( iter = usedIndices.constBegin(); 
                        iter!= usedIndices.constEnd();
                        iter++ ) 
                {
                    ContactRecord rec = d->contactIndices.value( *iter );
                    if ( foundIds.contains( rec.id ) ) {
                        QContact cnt = d->contactModel->contact( rec.id );
                        //QContactModel::match() matches beginning of firstname and lastname 
                        //whereas we need a matching on label()
                        if ( !cnt.label().startsWith( d->pbSearchText, Qt::CaseInsensitive ) )
                            continue;
                        temp = status.arg( *iter );
                        QString number = cnt.phoneNumber( rec.type ).left( 60 );
                        temp = temp.arg( number );
                        if ( number[0] == QChar('+') )
                            temp = temp.arg( 145 );
                        else
                            temp = temp.arg( 129 );
                        temp = temp.arg( quote( cnt.label().left( 60 )) );
                        send( temp );
                    }
                }
                done();
            } else {
                d->limitsReqBy = "cpbfSet";
                d->phoneBookQueried = true;
                d->phoneBook->getEntries( options()->phoneStore );
            }
        }
        break;
        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}


/*!
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBR Read phonebook entries}
    \compat

    The \c{AT+CPBR} command allows for reading the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPR=<index1>[,<index2>]} \o \c{[+CPBR: <index1>,<number>,<type>,<text>[[...]<CR><LF>
                                                   +CPBR: <index2>,<number>,<type>,<text>]]}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBR=?} \o \c{+CPBR (list of supported <index>s), [<nlength>], [<tlength>]}, \c{+CME ERROR: <err>}
    \endtable

    Test command returns the available location indices, the maximum length of phone number
    fields and the maximum length of text fields. 
    
    The set command returns a set of four parameters, each representing an entry in the phonebook memory. If the memory
    has several entries each entry is returned on a separate line.

    \table
    \row \o \c{<index>, <index1>, <index2>} \o integer number indicating the location number of a phonebook entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145 
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcpbr( const QString& params )
{
    if ( !d->contactModel && options()->phoneStore == "ME" )
        initializeMemoryPhoneBook();
    switch( mode( params ) )
    {
        case Set:
            {
                uint pos = 1;
                uint index1 = QAtUtils::parseNumber( params, pos);
                uint index2 = index1;
                if ( pos < (uint)params.length() ) //second param
                    index2 = QAtUtils::parseNumber( params, pos); //0 if not passed
                if ( index1 < 1 || index1 > index2 ) {
                    done( QAtResult::InvalidIndex );
                    return;
                }
                
                if ( options()->phoneStore == "ME" ) {
                    //get limits so that we can check for invalid indices
                    if ( 1 > index1 || d->availableMemoryContacts < (int) index2 )
                    {
                       done( QAtResult::InvalidIndex ); 
                       return;
                    }

                    const QString raw = "+CPBR: %1,\"%2\",%3,\"%4\"";
                    QString status;
                    QString name;
                    QString number;
                    for ( uint i = index1; i<=index2; i++ ) {
                        if ( !d->contactIndices.contains( i ) )
                            continue;
                        status = raw.arg( i );
                        QContact c = d->contactModel->contact( d->contactIndices.value( i ).id );
                        //number/text field length: 60 chars
                        number = c.phoneNumber( d->contactIndices.value( i ).type ).left( 60 );
                        status = status.arg( number );
                        if ( number[0] == QChar('+') )
                            status = status.arg( 145 );
                        else
                            status = status.arg( 129 );
                        name = c.label();
                        name = name.left(60);
                        status = status.arg( quote(name) );
                        send( status );
                    }
                    done();
 
                } else {
                    d->limitsReqBy = "cpbrSet";
                    d->phoneBookIndex.first = index1;
                    d->phoneBookIndex.second = index2;
                    d->phoneBookQueried = true;
                    d->phoneBook->requestLimits( options()->phoneStore );
                }
            }
            break;
        case Support:
            {
                if ( options()->phoneStore == "ME" ) {
                    QString status = "+CPBR: (1,%1),60,60";
                    status = status.arg( d->availableMemoryContacts );
                    send( status );
                    done();
                } else {
                    d->limitsReqBy = "cpbrSupport";
                    d->phoneBookQueried = true;
                    d->phoneBook->requestLimits( options()->phoneStore );
                }
            }
            break;
        default:
            {
                done( QAtResult::OperationNotAllowed );
            }
            break;
    }
}

/*!
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBS Select phonebook memory storage}
    \compat

    The \c{AT+CPBS} command allows for querying and selecting the
    phone book storage.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPS=<storage>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBS?} \o \c{+CPBS: <storage>,[<used>,<total>]}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBS=?} \o \c{+CPBS (list of <storage>s)}
    \endtable
   
    The set command sets the selected phonebook memroy to <storage>. A list of available phonebook memories
    can be obtained when using the test command.
    
    The Read command returns the currently selected storage and the used/total number of locations in the selected
    memory.

    This implementation is not entirely conform with 3GPP TS 27.007. It is not 
    possible to set a password for phone book stores.
*/
void AtCommands::atcpbs( const QString& params )
{
    switch( mode( params ) ) 
    {
        case CommandOnly:    
        case Get:
        {
            if ( options()->phoneStore == "ME" ) {
                if ( !d->contactModel && options()->phoneStore == "ME" )
                    initializeMemoryPhoneBook();
                QString status = "+CPBS: \"ME\",%1,%2";
                status = status.arg( d->contactIndices.size() ).arg( d->availableMemoryContacts );
                send( status );
                done();
            } else {
                /*if ( options()->phoneStorePw.isEmpty() )
                    d->phoneBook->clearPassword( options()->phoneStore );
                else
                    d->phoneBook->setPassword( options()->phoneStore, options()->phoneStorePw );*/
                d->limitsReqBy = "cpbs";
                d->phoneBookQueried = true;
                d->phoneBook->requestLimits( options()->phoneStore );
            }
        }
        break;
        case Support:
        {
            QString result = "+CPBS: (";
            QStringList storages = d->phoneBook->storages();
            storages.removeAll( "MT" ); //no support for MT atm
            if ( !storages.contains( "ME" ) )
                storages.append( "ME" );
            for( int i = 0; i<storages.count(); i++ ) {
                result.append( "\"" + QAtUtils::quote(storages[i]) + "\"," );
            }

            if ( storages.count() )
                result.chop( 1 );//remove last comma
            result.append( QLatin1String(")") );
            
            send( result ); 
            done();
        }
        break;

        case Set:
        {
            uint posn = 1;
            QString store = QAtUtils::nextString( params, posn );
            if ( !d->phoneBook->storages().contains( store ) ) {
                done( QAtResult::OperationNotAllowed );
                return;
            }
            if ( !QAtUtils::nextString( params, posn ).isEmpty() )
            {
                //QPhoneBook cannot report an error when the user provides a wrong password
                done( QAtResult::OperationNotSupported );
                return;
            }
            options()->phoneStore = store;
            done();
            if ( store == "ME" ) {
                QTimer::singleShot( 5, this, SLOT(initializeMemoryPhoneBook()) );
            }
        }
        break;

        default: 
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
        
    }
}
/*!
    \ingroup ModemEmulator::PhoneBook
    \bold{AT+CPBR Write phonebook entry}
    \compat

    The \c{AT+CPBW} command allows for writing of the phonebook entries
    from the current phonebook memory storage selected with \c{+CPBS}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CBPW=[<index>],[<number>[,<type>,<text>]]} \o \c{+CME ERROR: <err>}
    \row \o \c{AT+CPBW=?} \o \c{+CPBW (list of supported <index>s), [<nlength>], (list of supported <type>s), [<tlength>]}, \c{+CME ERROR: <err>}
    \endtable

    Test command returns the available location indices, the maximum length of phone number
    fields, the supported number formats of the storage and the maximum length of text fields. 
    
    The set command expects a set of four parameters. \c{<index>} determines the location where the current write operation takes place.
    In addition the entry field contains the phone number \c{<number>} (in the format \c{<type>}), 
        and the text associated with the given number. If these fields are omitted the phonebook entry at \c{<index>} is deleted. If
    \c{<index>} is omitted but \c{<number>} is given the entry is saved at the first free location in the phonebook.

    \table
    \row \o \c{<index>} \o integer number indicating the location number of a phonebook memory entry
    \row \o \c{<number>} \o string type phone number of format \c{<type>}
    \row \o \c{<type>}  \o string type, phone number of format \c{<type>}; default 145 
    when dialling string includes "+", otherwise 129
    \row \o \c{<text>} \o string type, character set as specified by selected character set \c{+CSCS}
    \endtable

    \table
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcpbw( const QString& params )
{
    if ( !d->contactModel && options()->phoneStore == "ME" )
        initializeMemoryPhoneBook();
    QStringList notSupportedStores;
    notSupportedStores << "DC" << "EN" << "MC" << "RC" << "LD" << "MT";
    switch( mode( params ) )
    {
        case Set:
            {
                if ( notSupportedStores.contains(options()->phoneStore) ) {
                    done( QAtResult::OperationNotAllowed );
                    return;
                }
                uint pos = 1;
                //number of parameters determines operation
                QStringList pList = params.split( "," );
                d->entryToWrite = QPhoneBookEntry();
                switch( pList.count() ) {
                    case 1:
                        {
                            //just index passed ->delete entry - AT+CPBW=3
                            uint index = QAtUtils::parseNumber( params, pos );
                            if ( options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( true, index );
                                return;
                            } else {
                                d->limitsReqBy = "cpbwSet-Remove";
                                d->entryToWrite.setIndex( index );
                            }
                        }
                        break;
                    case 2:
                        {
                            //add number w/o text at index  - AT+CPBW=2,"0544444"
                            uint index = QAtUtils::parseNumber( params, pos );
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9') 
                                        && number.at(i) != '+' ) {
                                    done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }
                           
                            if ( options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( false, index, number );
                                return;
                            } else { 
                                d->limitsReqBy = "cpbwSet-Number@Index";
                                d->entryToWrite.setIndex( index );
                                d->entryToWrite.setNumber( number );
                            }
                        }
                        break;
                    case 3: 
                        {
                            //add entry at next available spot - AT+CPBW="4444",129,"text"
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9') 
                                        && number.at(i) != '+' ) {
                                    done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }

                            uint type = QAtUtils::parseNumber( params, pos );
                            if ( type != 129 && type != 145 ) {
                                done( QAtResult::OperationNotAllowed );
                                return;
                            }
                            QString text = nextString( params, pos );
                            //test for letters
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9') 
                                        && number.at(i) != '+' ) {
                                    done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }
                            
                            if ( options()->phoneStore == "ME" ) {
                                //find next available phone book index
                                uint index = 0;
                                for( int i = 1; i <= d->availableMemoryContacts; i++ )
                                {
                                    if ( !d->contactIndices.contains( i ) ) {
                                        index = i;
                                        break;
                                    }
                                }
                                if ( index == 0 ) { //could not find free index
                                    done( QAtResult::MemoryFull );
                                    return;
                                }
                                writeMemoryPhoneBookEntry( false, index, number, text );
                                return;
                            } else {
                                d->limitsReqBy = "cpbwSet-Number@nextIndex";
                                d->entryToWrite.setNumber( number );
                                d->entryToWrite.setText( text );
                            }
                        }
                        break;
                    case 4:
                        {
                            //add entry incl text at index - AT+CPBW=1,"4444",129,"text"
                            uint index = QAtUtils::parseNumber( params, pos );
                            QString number = QAtUtils::nextString( params, pos );
                            //test for letters in dial string
                            for ( int i = 0; i<number.length() ; i++ ) {
                                if ( (number.at(i) < '0' || number.at(i) > '9') 
                                        && number.at(i) != '+' ) {
                                    done( QAtResult::InvalidCharsInDialString );
                                    return;
                                }
                            }
                            uint type = QAtUtils::parseNumber( params, pos );
                            if ( type != 129 && type != 145 ) {
                                done( QAtResult::OperationNotAllowed );
                                return;
                            }
                            QString text = nextString( params, pos );

                            if ( options()->phoneStore == "ME" ) {
                                writeMemoryPhoneBookEntry( false, index, number, text );
                                return;
                            } else {
                                d->limitsReqBy = "cpbwSet-NumberText@Index";
                                d->entryToWrite.setIndex( index );
                                d->entryToWrite.setNumber( number );
                                d->entryToWrite.setText( text );
                            }
                        }
                        break;
                    default:
                        done( QAtResult::OperationNotAllowed );
                        return;
                }
                
                d->phoneBookQueried = true;
                d->phoneBook->requestLimits( options()->phoneStore );

            }
            break;
        case Support:
            {
                if ( notSupportedStores.contains(options()->phoneStore) ) {
                    done( QAtResult::OperationNotAllowed );
                    return;
                }
                if ( options()->phoneStore == "ME" ) {
                    QString status = "+CPBW: (1-%1),60,(129,145),60";
                    status = status.arg( d->availableMemoryContacts );
                    send( status ); 
                    done();
                } else {
                    d->limitsReqBy = "cpbwSupport";
                    d->phoneBookQueried = true;
                    d->phoneBook->requestLimits( options()->phoneStore );
                }
            }
            break;
        default:
            {
                done( QAtResult::OperationNotAllowed );
            }
            break;
    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CPIN Enter PIN}
    \compat

    The \c{AT+CPIN} command allows for querying and entering the PIN.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPIN=<pin>[,<newpin]} \o \c{+CME ERROR: 3}
    \row \o \c{AT+CPIN?} \o \c{+CPIN: READY}
    \row \o \c{AT+CPIN=?} \o \c{OK}
    \endtable

    For security reasons, this implementation does not allow real access
    to the PIN through this interface.  It is assumed that the user has
    already entered the PIN directly on the phone using some other means.
    If the user hasn't, then requests to use restricted features will be
    denied or delayed.  Read command will always return \c{READY} and
    set command will always fail.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcpin( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            // Report that we are ready.  If we aren't, then the rest of the
            // system will deny or delay operations until it is actually ready.
            send( "+CPIN: READY" );
            done();
        }
        break;

        case Support:
        {
            // Nothing to do, according to GSM 27.007, section 8.3.
            done();
        }
        break;

        case Set:
        {
            // Attempt to enter or change the pin: disallow it.  See above.
            done( QAtResult::OperationNotAllowed );
        }
        break;

        default: break;

    }
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CPMS Preferred Message Storage}
    \compat

    The \c{AT+CPMS} command can be used to set the preferred message storage
    for reading and writing SMS messages.  The only message storage that
    is supported by this implementation is \c{SM}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CPMS=<mem1>[,<mem2>[,<mem3>]]} \o \c{+CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>,<used3>,<total3>} or \c{+CME ERROR: <err>}
    \row \o \c{AT+CPMS?} \o \c{+CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>,<used3>,<total3>}
    \row \o \c{AT+CPMS=?} \o \c{+CPMS: ((list of supported <mem1>s),(list of supported <mem2>s),(list of supported <mem3>s))}
    \endtable
*/
void AtCommands::atcpms( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            // Bail out if the SIM is not ready yet.
            if ( !d->smsReader->ready() ) {
                done( QAtResult::USimBusy );
                break;
            }

            // All three parameters must be set to "SM".
            uint posn = 1;
            if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                done( QAtResult::SMSOperationNotAllowed );
                break;
            }
            if ( posn < (uint)( params.length() ) ) {
                if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                    done( QAtResult::SMSOperationNotAllowed );
                    break;
                }
                if ( posn < (uint)( params.length() ) ) {
                    if ( QAtUtils::nextString( params, posn ) != "SM" ) {
                        done( QAtResult::SMSOperationNotAllowed );
                        break;
                    }
                }
            }

            // Report the used and total messages for the "SM" store.
            QString data = QString::number( d->smsReader->usedMessages() ) +
                           "," +
                           QString::number( d->smsReader->totalMessages() );
            send( "+CPMS: " + data + "," + data + "," + data );
            done();
        }
        break;

        case Get:
        {
            // Bail out if the SIM is not ready yet.
            if ( !d->smsReader->ready() ) {
                done( QAtResult::USimBusy );
                break;
            }

            // Report the used and total messages for the "SM" store.
            QString data = "\"SM\"," +
                           QString::number( d->smsReader->usedMessages() ) +
                           "," +
                           QString::number( d->smsReader->totalMessages() );
            send( "+CPMS: " + data + "," + data + "," + data );
            done();
        }
        break;

        case Support:
        {
            send( "+CPMS: ((\"SM\"),(\"SM\"),(\"SM\"))" );
            done();
        }
        break;

        default: break;

    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CRC Cellular Result Codes}
    \compat

    The \c{AT+CRC} command enables or disables the \c{+CRING}
    unsolicited response.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRC=[<mode>]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CRC?} \o \c{+CRC: <mode>}
    \row \o \c{AT+CRC=?} \o \c{+CRC: (0,1)}
    \endtable

    Set command controls whether or not the extended format of incoming
    call indication is used.  When enabled, an incoming call is indicated
    to the TE with unsolicited result code \c{+CRING: <type>} instead of
    the normal \c{RING}, where \c{<type>} is one of the following values:

    \table
    \row \o \c{ASYNC} \o Asynchronous data
    \row \o \c{FAX} \o Fascimile
    \row \o \c{VOICE} \o Normal voice
    \endtable

    GPRS network requests are not supported in this implementation.
    They are handled internally within the phone.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcrc( const QString& params )
{
    flagCommand( "+CRC: ", options()->cring, params );
}

// Convert an integer into a 4-four hex string, for lac and ci values.
static QString asHex( int value )
{
    static char const hexchars[] = "0123456789ABCDEF";
    return QString( QChar( hexchars[(value >> 12) & 0x0F] ) ) +
           QString( QChar( hexchars[(value >>  8) & 0x0F] ) ) +
           QString( QChar( hexchars[(value >>  4) & 0x0F] ) ) +
           QString( QChar( hexchars[ value        & 0x0F] ) );
}

/*!
    \ingroup ModemEmulator::Network
    \bold{AT+CREG Network Registration}
    \compat

    The \c{AT+CREG} command control the presentation of unsolicited
    network registration changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CREG=[<n>]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CREG?} \o \c{+CREG: <n>,<stat>[,<lac>,<ci>]}
    \row \o \c{AT+CREG=?} \o \c{+CREG: (0-2)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{+CREG: <stat>[,<lac>,<ci>]} when there is a change in the
    network registration status.

    Read command returns the presentation value and the current
    network registration status.

    \table
    \row \o \c{<n>}
         \o \list
                \o 0 disable network registration result code
                \o 1 enable network registration unsolicited result code
                     \c{+CREG: <stat>}
                \o 2 enable network registration unsolicited result code
                     \c{+CREG: <stat>[,<lac>,<ci>]}
            \endlist
    \row \o \c{<stat>}
         \o \list
                \o 0 not registered and not currently searching for a network
                \o 1 registered, home network
                \o 2 not registered and searching for a network
                \o 3 registration denied
                \o 4 unknown
                \o 5 registered, roaming
            \endlist
    \row \o \c{<lac>} \o String type, indicating the two byte location
                         area code in hexadecimal format.
    \row \o \c{<ci>} \o String type, indicating the two byte cell ID
                        in hexadecimal format.
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcreg( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            if ( options()->creg == 2 &&
                 d->netReg->locationAreaCode() != -1 ) {
                QString hexlac = asHex( d->netReg->locationAreaCode() );
                QString hexci = asHex( d->netReg->cellId() );
                send( "+CREG: " + QString::number( options()->creg ) + "," +
                      QString::number
                        ( (int)(d->netReg->registrationState()) ) + "," +
                      hexlac + "," + hexci );
            } else {
                send( "+CREG: " + QString::number( options()->creg ) + "," +
                      QString::number
                        ( (int)(d->netReg->registrationState()) ) );
            }
            done();
        }
        break;

        case Support:
        {
            send( "+CREG: (0-2)" );
            done();
        }
        break;

        case Set:
        {
            uint posn = 1;
            uint n = QAtUtils::parseNumber( params, posn );
            if ( n <= 2 ) {
                options()->creg = (int)n;
                done();
            } else {
                // Invalid value for "n".
                done( QAtResult::OperationNotAllowed );
            }
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CRES Restore Settings}
    \compat

    The \c{AT+CRES} command restores SMS settings from EEPROM.  This is
    not supported in this implementation.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CRES[=<profile>]} \o \c{OK}
    \row \o \c{AT+CRES=?} \o \c{OK}
    \endtable
*/
void AtCommands::atcres()
{
    // Restore SMS settings from EEPROM.  We don't support this.
    done();
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSAS Save Settings}
    \compat

    The \c{AT+CSAS} command saved SMS settings to EEPROM.  This is
    not supported in this implementation.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSAS[=<profile>]} \o \c{OK}
    \row \o \c{AT+CSAS=?} \o \c{OK}
    \endtable
*/
void AtCommands::atcsas()
{
    // Save SMS settings to EEPROM.  We don't support this.
    done();
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSCA Service Center Address}
    \compat

    The \c{AT+CSCA} command reads or modifies the SMS service center address
    on the SIM.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSCA=<sca>[,<tosca>]} \o \c{OK}
    \row \o \c{AT+CSCA?} \o \c{+CSCA: <sca>,<tosca>}
    \row \o \c{AT+CSCA=?} \o \c{OK}
    \endtable

    Set command updates the SMS service center address on the SIM.
    Read command reports the current SMS service center address on the SIM.
*/
void AtCommands::atcsca( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            uint posn = 1;
            QString number = QAtUtils::nextString( params, posn );
            uint type = QAtUtils::parseNumber( params, posn );
            if ( type == 0 )
                type = 129;
            number = QAtUtils::decodeNumber( number, type );
            setNumber( QServiceNumbers::SmsServiceCenter, number );
        }
        break;

        case Get:
        {
            queryNumber( QServiceNumbers::SmsServiceCenter );
        }
        break;

        case Support:
        {
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSCS Select TE Character Set}
    \compat

    The \c{AT+CSCS} command selects the character set to use in commands
    that take or return string arguments.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSCS=<chset>} \o \c{OK}, \c{+CME ERROR: 4}
    \row \o \c{AT+CSCS?} \o \c{+CSCS: <chset>}
    \row \o \c{AT+CSCS=?} \o \c{+CSCS:} (list of supported \c{<chset>}s)
    \endtable

    Set command informs the TA which character set \c{<chset>} is used
    by the TE.  TA is then able to convert character strings correctly
    between TE and MT character sets.  \c{+CME ERROR: 4} will be returned
    if the character set is not supported.

    Read command shows the current setting, and test command displays
    the available character sets.

    The following character sets are supported by this implementation:

    \table
    \header \o Name \o Description
    \row \o \c{GSM} \o GSM 7-bit default alphabet from 3GPP TS 23.038.
                       This is the default value.
    \row \o \c{HEX} \o Hexadecimal encoding of GSM 7-bit default alphabet.
    \row \o \c{UCS2} \o 16-bit universal multiple-octet coded character set.
                        UCS2 character strings are converted to hexadecimal.
    \row \o \c{8859-1} \o ISO-8859 Latin 1 character set.
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atcscs( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            // Note: don't use AtCommands::quote for this.  Charset changes
            // must always be in plain ASCII.
            send( "+CSCS: \"" + QAtUtils::quote( options()->charset ) + "\"" );
            done();
        }
        break;

        case Support:
        {
            send( "+CSCS: (\"GSM\",\"HEX\",\"UCS2\",\"8859-1\")" );
            done();
        }
        break;

        case Set:
        {
            // Note: don't use AtCommands::nextString for this.
            // Charset changes must always be in plain ASCII.
            uint posn = 1;
            QString value = QAtUtils::nextString( params, posn ).toUpper();
            if ( value == "GSM" || value == "HEX" ||
                  value == "UCS2" || value == "8859-1" ) {
                options()->setCharset( value );
                done();
            } else {
                done( QAtResult::OperationNotSupported );
            }
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

#ifdef QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSDH Show Text Mode Parameters}
    \compat

    The \c{AT+CSDH} command controls whether extended text mode parameters
    are shown in response to \c{AT+CMGL} and \c{AT+CMGR} commands in
    text mode.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSDH=<show>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT+CSDH?} \o \c{+CSDH: <show>}
    \row \o \c{AT+CSDH=?} \o \c{+CSDH: (0,1)}
    \endtable
*/
void AtCommands::atcsdh( const QString& params )
{
    flagCommand( "+CSDH: ", options()->csdh, params );
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSMP Set Text Mode Parameters}
    \compat

    The \c{AT+CSMP} command sets the additional parameters that are passed
    in the header of an SMS message when sending messages in text mode
    (\c{AT+CMGF=1}).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSMP=[<fo>[,<vp>[,<pid>[,<dcs>]]]]} \o \c{OK}
    \row \o \c{AT+CSMP?} \o \c{+CSMP: 1,167,0,0}
    \row \o \c{AT+CSMP=?} \o \c{OK}
    \endtable

    This implementation always sends SMS messages with the default
    GSM values, irrespective of what is set with \c{AT+CSMP}.
    Applications should use PDU mode for sending messages with
    specific header values.
*/
void AtCommands::atcsmp( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            // Return the standard defaults.
            send( "+CSMP: 1,167,0,0" );
            done();
        }
        break;

        case Support:
        {
            // The spec says to just respond with OK, so that is all we do.
            done();
        }
        break;

        case Set:
        {
            // Accept anything and then just ignore it.
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ShortMessageService
    \bold{AT+CSMS Select Message Service}
    \compat

    The \c{AT+CSMS} command selects the short message service to use.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSMS=<service>} \o \c{+CSMS: <mt>,<mo>,<bm>}
    \row \o \c{AT+CSMS?} \o \c{+CSMS: <service>,<mt>,<mo>,<bm>}
    \row \o \c{AT+CSMS=?} \o \c{+CSMS:} (list of supported \c{<service>}s)
    \endtable

    This implementation says that it supports service 0 and 1,
    mobile-terminated messages, mobile-originated messages, and
    cell broadcast messages, irrespective of the underlying
    modem's message support.  The underlying modem will select the
    correct service automatically.
*/
void AtCommands::atcsms( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( "+CSMS: " + QString::number( options()->smsService ) +
                  ",1,1,1" );
            done();
        }
        break;

        case Support:
        {
            send( "+CSMS: (0,1)" );
            done();
        }
        break;

        case Set:
        {
            if ( params == "=0" )
                options()->smsService = 0;
            else if ( params == "=1" )
                options()->smsService = 1;
            else {
                done( QAtResult::OperationNotAllowed );
                return;
            }
            send( "+CSMS: 1,1,1" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

#endif // QTOPIA_CELL

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+CSQ Signal Quality}
    \compat

    The \c{AT+CSQ} command can be used to query the current signal quality.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSQ} \o \c{+CBC: <rssi>, <ber>}
    \row \o \c{AT+CSQ=?} \o \c{+CBC: (0-31),(0-7)}
    \endtable

    Execution command returns received signal strength indication
    \c{<rssi>} and channel bit error rate \c{<ber>} from the MT.

    \table
    \row \o \c{<rssi>}
         \o \list
                \o 0 -113 dBm or less
                \o 1 -111 dBm
                \o 2...30 -109 to -53 sBm
                \o 31 -51 dBm or greater
                \o 99 not known or not detectable.
            \endlist
    \row \o \c{<ber>}
         \o \list
                \o 0...7 RXQUAL values as defined in 3GPP TS 45.008.
                \o 99 not known or not detectable.
            \endlist
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcsq( const QString& params )
{
    needIndicators();
    switch ( mode( params ) ) {

        case CommandOnly:
        case Get:
        {
            // Report the current signal quality value.
            sendSignalQuality( d->indicators->signalQuality() );
            done();
        }
        break;

        case Support:
        {
            // Report the supported values.
            send( "+CSQ: (0-31),(0-7)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+CSTA Select type of address}
    \compat

    The \c{AT+CSTA} command selects the type of number for dialing
    with the \c{ATD} command.  This implementation only supports 129.
    If a dial string starts with \c{+}, then 145 will be implicitly
    selected.  No other dial number types are supported.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+CSTA=[<type>]} \o \c{OK}, \c{+CME ERROR: 3}
    \row \o \c{AT+CSTA?} \o \c{+CSTA: 129}
    \row \o \c{AT+CSTA=?} \o \c{+CSTA: (129)}
    \endtable

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atcsta( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            // The only value supported by this implementation is 129.
            if ( params != "=129" )
                done( QAtResult::OperationNotSupported );
            else
                done();
        }
        break;

        case Get:
        {
            send( "+CSTA: 129" );
            done();
        }
        break;

        case Support:
        {
            send( "+CSTA: (129)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+GCAP Request Capabilities}
    \compat

    The \c{AT+GCAP} command requests the capabilities of the modem.
    It responds with a list of functionality areas, such as GSM,
    Fax, etc, that the modem supports.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GCAP} \o \c{+GCAP: <functionality-list>}
    \endtable

    The following functionality values may be returned:

    \table
    \header \o Value \o Description
    \row \o \c{+CGSM} \o GSM commands according to 3GPP TS 27.007 and 27.005.
    \row \o \c{+FCLASS} \o Fax commands.
    \row \o \c{+VOIP} \o VoIP calls can be dialed with \c{ATD<uri>;}.
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atgcap()
{
    QCommServiceManager mgr;
    QStringList services = mgr.services();
    QStringList values;
    if ( services.contains( "modem" ) )
        values += "+CGSM";
    if ( services.contains( "voip" ) )
        values += "+VOIP";
    // Detect fax support here when it is done and add "+FCLASS".
    if ( values.size() > 0 )
        send( "+GCAP: " + values.join(", ") );
    done();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMI Read Manufacturer Information}
    \compat

    \bold{AT+GMI Read Manufacturer Information}

    The \c{AT+GMI} command is an alias for \c{AT+CGMI}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GMI} \o \c{<manufacturer>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atgmi()
{
    atcgmi();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMM Read Model Information}
    \compat

    \bold{AT+GMM Read Model Information}

    The \c{AT+GMM} command is an alias for \c{AT+CGMM}.

    \table
   \header \o Command \o Possible Responses
    \row \o \c{AT+GMM} \o \c{<model>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atgmm()
{
    atcgmm();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GMR Read Revision Information}
    \compat

    The \c{AT+GMR} command is an alias for \c{AT+CGMR}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GMR} \o \c{<revision>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atgmr()
{
    atcgmr();
}

/*!
    \ingroup ModemEmulator::Identification
    \bold{AT+GSN Read Model Information}
    \compat

    The \c{AT+GSN} command is an alias for \c{AT+CGSN}.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+GSN} \o \c{<sn>}
    \endtable

    Conforms with: Recommendation V.250.
*/
void AtCommands::atgsn()
{
    atcgsn();
}

#ifndef QTOPIA_AT_STRICT

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QBC Enable Unsolicited Battery Charge Reporting}
    \compat

    The \c{AT*QBC} command can be used to enable or disable the
    unsolicited reporting of battery charge changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QBC=<n>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT*QBC?} \o \c{*QBC: <n>}
    \row \o \c{AT*QBC=?} \o \c{*QBC: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QBC: <bcs>,<bcl>} when \c{<n>}=1 and there is a change in the
    battery charge information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<bcs>}
         \o \list
                \o 0 MT is powered by the battery
                \o 1 MT has a battery connected, but is not powered by it
                \o 2 MT does not have a battery connected
                \o 3 Recognized power fault, calls inhibited
            \endlist
    \row \o \c{<bcl>}
         \o \list
                \o 0 battery is exhausted, or MT does not have a battery
                   connected.
                \o 1...100 battery has 1-100 percent of capacity remaining.
            \endlist
    \endtable
*/
void AtCommands::atqbc( const QString& params )
{
    needIndicators();
    flagCommand( "*QBC: ", options()->qbc, params );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QCAM Call Status Monitoring}
    \compat

    The \c{AT*QCAM} command can be used to enable or disable the
    unsolicited reporting of call status changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QCAM=<n>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT*QCAM?} \o \c{*QCAM: <n>}
    \row \o \c{AT*QCAM=?} \o \c{*QCAM: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QCAV: <id>,<state>,<calltype>[,<number>,<type>]} when \c{<n>}=1
    and there is a change in the call status information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<id>} \o Identifier for the call.
    \row \o \c{<state>}
         \o State of the call:
            \list
                \o 0 idle
                \o 1 calling
                \o 2 connecting
                \o 3 active
                \o 4 hold
                \o 5 waiting
                \o 6 alerting
                \o 7 busy
            \endlist
    \row \o \c{<calltype>}
         \o Type of call:
            \list
                \o 1 voice
                \o 2 data
                \o 4 fax
                \o 32 video
            \endlist
    \row \o \c{<number>}
         \o String type phone number of calling address in format
            specified by \c{<type>}.
    \row \o \c{<type>}
         \o Type of address octet in integer format (refer 3GPP TS 24.008).
    \endtable
*/
void AtCommands::atqcam( const QString& params )
{
    flagCommand( "*QCAM: ", options()->qcam, params );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT*QSQ Enable Unsolicited Signal Quality Reporting}
    \compat

    The \c{AT*QSQ} command can be used to enable or disable the
    unsolicited reporting of battery charge changes.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT*QSQ=<n>} \o \c{OK}, \c{+CME ERROR: <err>}
    \row \o \c{AT*QSQ?} \o \c{*QBC: <n>}
    \row \o \c{AT*QSQ=?} \o \c{*QBC: (0,1)}
    \endtable

    Set command controls the presentation of an unsolicited result code
    \c{*QSQ: <rssi>,<ber>} when \c{<n>}=1 and there is a change in the
    battery charge information.

    Read command returns the current state of the result code presentation
    value \c{<n>}.  The default value is 0.

    \table
    \row \o \c{<rssi>}
         \o \list
                \o 0 -113 dBm or less
                \o 1 -111 dBm
                \o 2...30 -109 to -53 sBm
                \o 31 -51 dBm or greater
                \o 99 not known or not detectable.
            \endlist
    \row \o \c{<ber>}
         \o \list
                \o 0...7 RXQUAL values as defined in 3GPP TS 45.008.
                \o 99 not known or not detectable.
            \endlist
    \endtable
*/
void AtCommands::atqsq( const QString& params )
{
    needIndicators();
    flagCommand( "*QSQ: ", options()->qsq, params );
}

#endif // !QTOPIA_AT_STRICT

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+VGM Gain of Microphone}
    \compat

    The \c{AT+VGM} command is used by a Bluetooth hands-free (HF) device
    to report its current microphone gain level setting to the phone's
    audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VGM=<gain>} \o \c{OK}
    \row \o \c{AT+VGM=<gain>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \row \o \c{AT+VGM?} \o \c{+VGM: <gain>}
    \row \o \c{AT+VGM=?} \o \c{+VGM: (0-15)}
    \endtable

    \table
    \row \o \c{<gain>}
         \o Decimal integer indicating the microphone gain between 0 and
            15, where 15 is the maximum gain.
    \endtable

    Set command reports the HF's current gain level setting.  Get command
    reports the previous setting.  Test command returns the valid gain levels.

    The audio gateway may also send unsolicited \c{+VGM: <gain>} messages
    whenever the gain changes on the AG.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atvgm(const QString &params)
{
#ifdef QTOPIA_BLUETOOTH
    switch ( mode( params ) ) {
        case Set:
        {
            uint posn = 1;
            uint volume = QAtUtils::parseNumber( params, posn );
            emit d->m_adaptor->microphoneVolumeChanged(volume);
            done();
        }
        break;

        case Get:
        {
            // TODO: retrieve the current value and report it.
            done();
        }
        break;

        case Support:
        {
            send( "+VGM: (0-15)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
#else
    Q_UNUSED(params);
    done( QAtResult::Error );
#endif
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+VGS Gain of Speaker}
    \compat

    The \c{AT+VGS} command is used by a Bluetooth hands-free (HF) device
    to report its current speaker gain level setting to the phone's
    audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VGS=<gain>} \o \c{OK}
    \row \o \c{AT+VGS=<gain>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit.
    \row \o \c{AT+VGS?} \o \c{+VGS: <gain>}
    \row \o \c{AT+VGS=?} \o \c{+VGS: (0-15)}
    \endtable

    \table
    \row \o \c{<gain>}
         \o Decimal integer indicating the speaker gain between 0 and
            15, where 15 is the maximum gain.
    \endtable

    Set command reports the HF's current gain level setting.  Get command
    reports the previous setting.  Test command returns the valid gain levels.

    The audio gateway may also send unsolicited \c{+VGS: <gain>} messages
    whenever the gain changes on the AG.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atvgs(const QString &params)
{
#ifdef QTOPIA_BLUETOOTH
    switch ( mode( params ) ) {
        case Set:
        {
            uint posn = 1;
            uint volume = QAtUtils::parseNumber( params, posn );
            emit d->m_adaptor->speakerVolumeChanged(volume);
            done();
        }
        break;

        case Get:
        {
            // Should never happen 
            done();
        }
        break;

        case Support:
        {
            send( "+VGS: (0-15)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
#else
    Q_UNUSED(params);
    done( QAtResult::Error );
#endif
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+NREC Control Noise Reduction and Echo Cancellation}
    \compat

    The \c{AT+NREC} command is used by a Bluetooth hands-free (HF) device
    to turn off the Noise Reduction and Echo Cancellation capabilities of 
    the phone's audio gateway (AG).

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+NREC=0} \o \c{OK}
    \row \o \c{AT+NREC=0}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atnrec(const QString &params)
{
    Q_UNUSED(params);
    done( QAtResult::Error );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BVRA Control Bluetooth Voice Recognition}
    \compat

    The \c{AT+BVRA} command is used by a Bluetooth hands-free (HF) device
    to turn on and off the Voice Recognition capabilities of the phone's
    audio gateway (AG). 

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BVRA=<num>} \o \c{OK}
    \row \o \c{AT+BVRA=<num>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    The AG can also initiate Bluetooth Voice Recognition events by sending the
    {+BVRA=<num>} unsolicited result code.

    \table
    \row \o \c{<num>}
         \o Decimal integer. 0 indicates voice recognition functionality should be ended.  1 indicates
            voice recognition functionality should be started.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atbvra(const QString &params)
{
    Q_UNUSED(params);
    done( QAtResult::Error );
}


/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BINP Bluetooth: Attach Phone Number to a Voice Tag}
    \compat

    The \c{AT+BINP} command can be used by the Bluetooth hands-free (HF) device
    to request Phone Number from the AG.  

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BINP=1} \o +BINP:<PhoneNumber>, \c{OK}
    \row \o \c{AT+BINP=1}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    Each time this command is sent, the Audio Gateway shall respond with a different Phone Number.

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atbinp(const QString &params)
{
    Q_UNUSED(params);
    done( QAtResult::Error );
}

/*!
    \ingroup ModemEmulator::ControlAndStatus
    \bold{AT+BTRH Bluetooth Respond and Hold}
    \compat

    The \c{AT+BTRH} command can be used by the Bluetooth hands-free (HF) device
    to perform the Respond and Hold feature.  

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+BTRH?} \o [+BTRH:<number>], \c{OK}
    \row \o \c{AT+BTRH = <num>} \o +BTRH:<num>, \c{OK}
    \row \o \c{AT+BTRH = <num>}
         \o \c{ERROR} if the device is not a Bluetooth hands-free unit or functionality is not supported.
    \endtable

    \table
    \header \o Num Value \o Description
    \row \o \c{0} \o Put the incoming call on hold.
    \row \o \c{1} \o Accept the call which was held.
    \row \o \c{2} \o Reject the call which was held.
    \endtable

    Conforms with: Bluetooth Hands-Free Profile 1.5
*/
void AtCommands::atbtrh(const QString &params)
{
#ifdef QTOPIA_BLUETOOTH
    switch ( mode( params ) ) {
        case Set:
        {
            done(QAtResult::Error);
        }
        break;

        case Get:
        {
            // Should never happen 
            done();
        }
        break;

        case Support:
        {
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
#else
    Q_UNUSED(params);
    done( QAtResult::Error );
#endif
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+VTD Tone Duration}
    \compat

    The \c{AT+VTD} command can be used to define the length of tones
    emitted as a result of the \c{AT+VTS} command.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VTD=<n>} \o \c{+CME ERROR: 3}
    \row \o \c{AT+VTD?} \o \c{+VTD: <n>}
    \row \o \c{AT+VTD=?} \o \c{+VTD: (0-255)}
    \endtable

    According to 3GPP TS 27.007, section C.2.12, the tone duration can only
    be queried, and never set.  This implementation always returns zero to
    indicate "manufacturer specific".

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atvtd( const QString& params )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( "+VTD: 0" );
            done();
        }
        break;

        case Support:
        {
            send( "+VTD: (0-255)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

/*!
    \ingroup ModemEmulator::CallControl
    \bold{AT+VTS DTMF and Tone Generation}
    \compat

    The \c{AT+VTS} command can be used to generate DTMF tones during
    a voice call.

    \table
    \header \o Command \o Possible Responses
    \row \o \c{AT+VTS=<tones>} \o \c{OK}
    \row \o \c{AT+VTS=?} \o \c{+VTS: (0-9,*,#,A,B,C,D)}
    \endtable

    The \c{<tones>} parameter is a string containing the digits to be sent
    as DTMF tones.  The dual tone frequencies and tone duration parameters
    from 3GPP TS 27.007 are not supported.

    Conforms with: 3GPP TS 27.007.
*/
void AtCommands::atvts( const QString& params )
{
    switch ( mode( params ) ) {

        case Set:
        {
            uint posn = 1;
            QString tones = QAtUtils::nextString( params, posn );
            if ( posn < (uint)(params.length()) ) {
                // We only support the single-argument form of tone generation.
                done( QAtResult::OperationNotSupported );
            } else {
                d->manager->callManager()->tone( tones );
                done();
            }
        }
        break;

        case Support:
        {
            send( "+VTS: (0-9,*,#,A,B,C,D)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;

    }
}

// Process an S option.
int AtCommands::soption( const QString& params, int prev, int min, int max )
{
    if ( params == "?" ) {
        // Report the current value as a three digit decimal value.
        QString result;
        prev &= 0xFF;
        result = QString( QChar( ( ( prev / 100 ) % 10 ) + '0' ) ) +
                 QString( QChar( ( ( prev /  10 ) % 10 ) + '0' ) ) +
                 QString( QChar( (   prev         % 10 ) + '0' ) );
        send( result );
        done();
    } else if ( params == "=" ) {
        // Set the value to zero unless zero is not an allowable value.
        if ( min != 0 ) {
            done( QAtResult::Error );
            return -1;
        }
        done();
        return 0;
    } else if ( params.startsWith( "=" ) ) {
        // Set the value to the parameter.
        int value = params.mid(1).toInt();
        if ( value < min || value > max ) {
            done( QAtResult::Error );
            return -1;
        }
        done();
        return value;
    } else if ( params.isEmpty() ) {
        // Do nothing.
        done();
    } else {
        // Syntax error in option.
        done( QAtResult::Error );
    }
    return -1;
}

// Determine the mode that an extension command is operating in.
AtCommands::Mode AtCommands::mode( const QString& params ) const
{
    if ( params.isEmpty() )
        return CommandOnly;
    if ( params[0] == QChar('=') ) {
        if ( params.length() > 1 && params[1] == QChar('?') ) {
            if ( params.length() != 2 )
                return Error;
            else
                return Support;
        } else {
            return Set;
        }
    }
    if ( params[0] == QChar('?') ) {
        if ( params.length() != 1 )
            return Error;
        else
            return Get;
    }
    return Error;
}

void AtCommands::flagCommand( const QString& prefix, bool& flag,
                              const QString& params, const QString& extra )
{
    switch ( mode( params ) ) {

        case Get:
        {
            send( prefix + ( flag ? "1" : "0" ) + extra );
            done();
        }
        break;

        case Set:
        {
            if ( params == "=0" )
                flag = false;
            else if ( params == "=1" )
                flag = true;
            else {
                done( QAtResult::OperationNotAllowed );
                return;
            }
            done();
        }
        break;

        case Support:
        {
            send( prefix + "(0,1)" );
            done();
        }
        break;

        default:
        {
            done( QAtResult::OperationNotAllowed );
        }
        break;
    }
}

// Make sure that we have indicator support whenever someone
// uses a command that may be affected by indicators.
void AtCommands::needIndicators()
{
    if ( d->indicators )
        return;
    d->indicators = new AtPhoneIndicators( this );
    connect( d->indicators, SIGNAL(indicatorChanged(int,int)),
             this, SLOT(indicatorChanged(int,int)) );
    connect( d->indicators, SIGNAL(signalQualityChanged(int)),
             this, SLOT(signalQualityChanged(int)) );
    connect( d->indicators, SIGNAL(batteryChargeChanged(int)),
             this, SLOT(batteryChargeChanged(int)) );
    connect( d->manager->callManager(), SIGNAL(setOnCall(bool)),
             d->indicators, SLOT(setOnCall(bool)) );
    connect( d->manager->callManager(),
             SIGNAL(setCallSetup(AtCallManager::CallSetup)),
             d->indicators, SLOT(setCallSetup(AtCallManager::CallSetup)) );
    connect( d->manager->callManager(),
             SIGNAL(setCallHold(AtCallManager::CallHoldState)),
             d->indicators, SLOT(setCallHold(AtCallManager::CallHoldState)) );

}

void AtCommands::queryNumber( QServiceNumbers::NumberId id )
{
    d->pendingQuery = id;
    d->serviceNumbers->requestServiceNumber( id );
}

void AtCommands::setNumber( QServiceNumbers::NumberId id, const QString& value )
{
    d->pendingSet = id;
    d->serviceNumbers->setServiceNumber( id, value );
}
 
// Parse the next string from a parameter list and convert according to codecs.
QString AtCommands::nextString( const QString& buf, uint& posn )
{
    return QAtUtils::decode( QAtUtils::nextString( buf, posn ),
                             options()->codec );
}

// Encode a string according to the prevailing codec, and escape specials.
// The result is suitable to be passed to AtFrontEnd::send for transmission.
QString AtCommands::quote( const QString& str )
{
    return QAtUtils::quote
        ( QString::fromLatin1( options()->codec->fromUnicode( str ) ) );
}

// State of a phone call has changed.
void AtCommands::stateChanged
        ( int callID, AtCallManager::CallState state,
          const QString& number, const QString& type )
{
    if ( options()->qcam ) {
        QString line;
        line = "*QCAV: " + QString::number( callID ) + "," +
               QString::number( (int)state ) + "," +
               QString::number( AtCallManager::numCallType( type ) );
        if ( state != AtCallManager::CallIdle && !number.isEmpty() ) {
            line += "," + QAtUtils::encodeNumber( number );
        }
        send( line );
    }
}

// Deferred result code from the call manager.
void AtCommands::deferredResult
        ( AtCommands *handler, QAtResult::ResultCode result )
{
    if ( handler != this )
        return;

    //if we have a connect we forward the raw data traffic
    if ( d->dataCallRequested && result == QAtResult::Connect ) {
        d->dataCallRequested = false;
        d->frontEnd->setState( AtFrontEnd::OnlineData );
    }
    done( result );
}


// Ring indication for an incoming call.
void AtCommands::ring( const QString& number, const QString& type )
{
    if ( options()->cring ) {
        // The client wants more information about the call type.
        QString typeString = AtCallManager::strCallType( type );
        send( "+CRING: " + typeString );
    } else {
        // Send an ordinary ring indication.
        send( "RING" );
    }
    if ( options()->clip && !number.isEmpty() ) {
        // Send the caller id information.
        send( "+CLIP: " + QAtUtils::encodeNumber( number ) );
    }
}

// Call waiting indication for an incoming call.
void AtCommands::callWaiting( const QString& number, const QString& type )
{
    if ( options()->ccwa ) {
        QString classId = QString::number
            ( AtCallManager::numCallType( type ) );
        send( "+CCWA: " + QAtUtils::encodeNumber( number ) + "," + classId );
    }
}

// Report "NO CARRIER" for the active call.
void AtCommands::noCarrier()
{
    send( "NO CARRIER" );
}

// Report than an indicator has changed value.
void AtCommands::indicatorChanged( int ind, int value )
{
    if ( options()->cind != 0 ) {
        send( "+CIEV: " + QString::number(ind + 1) + "," +
              QString::number(value) );
    }
}

#ifdef QTOPIA_CELL

void AtCommands::callWaitingState( QTelephony::CallClass cls )
{
    if ( d->requestingCallWaiting ) {
        d->requestingCallWaiting = false;
        if ( cls == QTelephony::CallClassNone ) {
            // The only time we report status=0 is for no call classes enabled.
            send( "+CCWA: 0,7" );
        } else {
            // Break the class mask up into individual bits and report them.
            int bit = 1;
            while ( bit <= 65536 ) {
                if ( (((int)cls) & bit) != 0 ) {
                    send( "+CCWA: 1," + QString::number(bit) );
                }
                bit <<= 1;
            }
        }
        done();
    }
}

void AtCommands::setCallWaitingResult( QTelephony::Result result )
{
    if ( d->settingCallWaiting ) {
        d->settingCallWaiting = false;
        done( (QAtResult::ResultCode)result );
    }
}

#endif // QTOPIA_CELL

void AtCommands::registrationStateChanged()
{
    // Report changes in the network registration state if necessary.
    if ( options()->creg == 2 &&
         d->netReg->locationAreaCode() != -1 ) {
        QString hexlac = asHex( d->netReg->locationAreaCode() );
        QString hexci = asHex( d->netReg->cellId() );
        send( "+CREG: " + QString::number( options()->creg ) + "," +
              QString::number
                ( (int)(d->netReg->registrationState()) ) + "," +
              hexlac + "," + hexci );
    } else if ( options()->creg != 0 ) {
        send( "+CREG: " +
              QString::number( (int)(d->netReg->registrationState()) ) );
    }
}

void AtCommands::availableOperators
        ( const QList<QNetworkRegistration::AvailableOperator>& opers )
{
    if ( d->requestingAvailableOperators ) {
        d->requestingAvailableOperators = false;
        QString status = "+COPS: ";
        QList<QNetworkRegistration::AvailableOperator>::ConstIterator it;
        for ( it = opers.begin(); it != opers.end(); ++it ) {
            if ( status.length() > 7 )
                status += ",";
            status += "(" + QString::number( (int)(*it).availability );
            status += ",\"" + QAtUtils::quote( (*it).name ) + "\",\"";
            status += QAtUtils::quote( (*it).shortName ) + "\",";
            if ( (*it).id.startsWith( "2" ) ) {
                // Back-end has supplied a numeric id for us to use.
                status += "\"" + (*it).id.mid(1) + "\"";
            } else {
                // No numeric id, so report zero.
                status += "\"0\"";
            }
            QString tech = (*it).technology;
            if ( tech == "GSMCompact" )        // No tr
                status += ",1";
            else if ( tech == "UTRAN" )        // No tr
                status += ",2";
            status += ")";
        }
        status += ",,(0-4),(0-2)";
        send( status );
        done();
    }
}

void AtCommands::setCurrentOperatorResult( QTelephony::Result result )
{
    if ( d->settingCurrentOperator ) {
        d->settingCurrentOperator = false;
        done( (QAtResult::ResultCode)result );
    }
}

void AtCommands::serviceNumber
        ( QServiceNumbers::NumberId id, const QString& number )
{
    if ( id != d->pendingQuery )
        return;
    d->pendingQuery = (QServiceNumbers::NumberId)(-1);
    if ( id == QServiceNumbers::SubscriberNumber ) {
        send( "+CNUM: ," + QAtUtils::encodeNumber( number ) );
    } else if ( id == QServiceNumbers::SmsServiceCenter ) {
        send( "+CSCA: " + QAtUtils::encodeNumber( number ) );
    }
    done();
}

void AtCommands::setServiceNumberResult
        ( QServiceNumbers::NumberId id, QTelephony::Result result )
{
    if ( id != d->pendingSet )
        return;
    d->pendingSet = (QServiceNumbers::NumberId)(-1);
    done( (QAtResult::ResultCode)result );
}

#ifdef QTOPIA_CELL
 
void AtCommands::extraLine( const QString& line, bool cancel )
{
    // Check that we actually expected this to happen.
    if ( !d->sendingSms )
        return;
    d->sendingSms = false;

    // If the operation was cancelled, just say "OK" and stop.
    if ( cancel ) {
        done();
        return;
    }

    // Compose the message to be sent.
    QSMSMessage msg;
    if ( options()->messageFormat ) {
        msg.setRecipient( d->smsNumber );
        msg.setText( QAtUtils::decode( line, options()->codec ) );
    } else {
        msg = QSMSMessage::fromPdu( QAtUtils::fromHex( line ) );
    }

    // If the SMS sender is not available, then fail the request
    // by pretending that we don't have network service to send it.
    if ( !d->smsSender->available() ) {
        done( QAtResult::SMSNoNetworkService );
        return;
    }

    // Send the message.
    d->smsMessageId = d->smsSender->send( msg );
}

void AtCommands::smsFinished( const QString& id, QTelephony::Result result )
{
    if ( id == d->smsMessageId ) {
        d->smsMessageId = QString();
        if ( result == QTelephony::OK ) {
            // Make up a message reference for the response.
            send( "+CMGS: " + QString::number( d->smsMessageReference++ ) );
        }
        done( (QAtResult::ResultCode)result );
    }
}

#endif // QTOPIA_CELL

#ifdef QTOPIA_BLUETOOTH
void AtCommands::setSpeakerVolume(int volume)
{
    char buf[64];
    sprintf(buf, "+VGS: %d", volume);
    send( buf );
}

void AtCommands::setMicrophoneVolume(int volume)
{
    char buf[64];
    sprintf(buf, "+VGM: %d", volume);
    send( buf );
}
#endif

#ifdef QTOPIA_CELL

void AtCommands::setLevelResult( QTelephony::Result result )
{
    if ( d->settingPhoneRf ) {
        d->settingPhoneRf = false;
        done( (QAtResult::ResultCode)result );
    }
}

#endif

void AtCommands::sendNextKey()
{
    if ( d->keys.size() == 0 ) {
        done();
        return;
    }
    if ( d->sendRelease ) {
        QtopiaServiceRequest release
            ( "VirtualKeyboard", "processKeyEvent(int,int,int,bool,bool)" );
        release << d->keys[0].first;
        release << d->keys[0].second;
        release << (int)0;
        release << false;
        release << false;
        release.send();
        d->keys = d->keys.mid(1);
        d->sendRelease = false;
        QTimer::singleShot( d->keyPauseTime * 100, this, SLOT(sendNextKey()) );
    } else {
        QtopiaServiceRequest press
            ( "VirtualKeyboard", "processKeyEvent(int,int,int,bool,bool)" );
        press << d->keys[0].first;
        press << d->keys[0].second;
        press << (int)0;
        press << true;
        press << false;
        press.send();
        d->sendRelease = true;
        QTimer::singleShot( d->keyPressTime * 100, this, SLOT(sendNextKey()) );
    }
}

#ifdef QTOPIA_CELL

void AtCommands::phoneBookLimits( const QString& /*store*/, const QPhoneBookLimits& limits )
{
    if (!d->phoneBookQueried)
        return;
    d->phoneBookQueried = false;
    QString status;
    if ( d->limitsReqBy == "cpbs" ) 
    {
        status += "+CPBS: ";
        status += "\"" + options()->phoneStore + "\",";
        status += QString::number(limits.used())+","
            +QString::number(limits.lastIndex()-limits.firstIndex()+1);
        send( status ); 
        done();
    }  
    else if ( d->limitsReqBy == "cpbrSupport" ) 
    {
        status = "+CPBR: ";
        status += "("+QString::number(limits.firstIndex()) + "," + 
                      QString::number(limits.lastIndex())+")," +
                      QString::number(limits.numberLength()) + "," +
                      QString::number(limits.textLength());
        send( status ); 
        done();
    } 
    else if ( d->limitsReqBy == "cpbrSet" ) 
    {
        //get limits so that we can check for invalid indices
        if ( limits.firstIndex() > d->phoneBookIndex.first ||
                limits.lastIndex() < d->phoneBookIndex.second )
        {
           done( QAtResult::InvalidIndex ); 
           return;
        }
        d->phoneBookQueried = true;
        d->phoneBook->getEntries( options()->phoneStore );
        return;
    } 
    else if ( d->limitsReqBy == "cpbwSupport" ) 
    {
        status = "+CPBW: (%1-%2),%3,(129,145),%4";
        status = status.arg(limits.firstIndex()).arg(limits.lastIndex());
        status = status.arg(limits.numberLength()).arg(limits.textLength());
        send( status ); 
        done();
    } 
    else if ( d->limitsReqBy == "cpbwSet-Remove" ) 
    {
        if ( d->entryToWrite.index() < limits.firstIndex() 
                || d->entryToWrite.index() > limits.lastIndex() ) {
            done( QAtResult::InvalidIndex );
            return;
        }
        d->phoneBook->remove( d->entryToWrite.index(), options()->phoneStore );
        done();                
    } 
    else if ( d->limitsReqBy == "cpbwSet-Number@Index"  
            || d->limitsReqBy == "cpbwSet-NumberText@Index" ) 
    {
        if ( d->entryToWrite.index() < limits.firstIndex() 
                || d->entryToWrite.index() > limits.lastIndex() ) {
            done( QAtResult::InvalidIndex );
            return;
        }

        if ( (uint)d->entryToWrite.number().length() > limits.numberLength() ) {
            done( QAtResult::DialStringTooLong );
            return;
        } else if ( (uint)d->entryToWrite.text().length() > limits.textLength() ) {
            done( QAtResult::TextStringTooLong );
            return;
        }
        d->phoneBook->update( d->entryToWrite, options()->phoneStore );
        done();
    } 
    else if ( d->limitsReqBy == "cpbwSet-Number@nextIndex" ) 
    {
        if ( (uint)d->entryToWrite.number().length() > limits.numberLength() ) {
            done( QAtResult::DialStringTooLong );
            return;
        } else if ( (uint)d->entryToWrite.text().length() > limits.textLength() ) {
            done( QAtResult::TextStringTooLong );
            return;
        }
        d->phoneBook->add( d->entryToWrite, options()->phoneStore );
        done();
    }
    else if ( d->limitsReqBy == "cpbfSupport" ) 
    {
        status = "+CPBF: %1,%2";
        status = status.arg(limits.numberLength()).arg(limits.textLength());
        send( status ); 
        done();
    }
}

/*!
   \internal

   Read/search the phone book entries of non memory phone storages (e.g. SM).
   */
void AtCommands::phoneBookEntries( const QString& /*store*/, const QList<QPhoneBookEntry>& entries )
{
    if (!d->phoneBookQueried)
        return;
    d->phoneBookQueried = false;

    QString prefix;
    bool isSearch = false;
    bool hasEntry = false; //have we found a matching entry for +CPBF
    if ( d->limitsReqBy == "cpbfSet" ) {
        prefix = "+CPBF: ";
        isSearch = true;
    } else if ( d->limitsReqBy == "cpbrSet" ) {
        prefix = "+CPBR: ";
    }
    
    
    //entries are not always sorted but all modems out there seem to 
    //return the results in a sorted order
    QMap<int,QPhoneBookEntry> sortedEntries;
    for ( int i = 0; i<entries.count(); i++ ) {
        sortedEntries.insert( entries[i].index(), entries[i] );
    }

    QMap<int,QPhoneBookEntry>::const_iterator iter;
    for ( iter = sortedEntries.constBegin(); 
            iter != sortedEntries.constEnd(); iter++ )
    {
        if ( !isSearch && 
                ((*iter).index() < d->phoneBookIndex.first 
                 || (*iter).index() > d->phoneBookIndex.second ) 
           )
            continue;
        else if ( isSearch )
            if ((*iter).text().startsWith( d->pbSearchText, Qt::CaseInsensitive ) )
                hasEntry = true; 
            else  
                continue;
            
        
        QString status = prefix + "%1,\"%2\",%3,\"%4\"";
        status = status.arg( (*iter).index() );
        status = status.arg( (*iter).number() );
        if ( (*iter).number().contains( "+" ) )
            status = status.arg(145);
        else 
            status = status.arg( 129 );
        status = status.arg( quote((*iter).text()) );//use +CSCS encoding
        send( status );
    }

    if ( isSearch && !hasEntry )
        done( QAtResult::NotFound );
    else
        done();
}

void AtCommands::initializeMemoryPhoneBook()
{
    if ( d->contactModel )
        return;

    QSet<QContact::PhoneType> typeRef;
    d->contactModel = new QContactModel( this );
    d->contactIndices.reserve( d->contactModel->rowCount() + 100 );
    //build ContactToIndex mapping for this session
    int index = 1;
    QUniqueId id;
    for ( int i = 0; i<d->contactModel->rowCount(); i++ ) {
        id = d->contactModel->id( i );
        if ( !d->contactModel->isSIMCardContact( id ) ) {
            QContact c = d->contactModel->contact( id );
            QMap<QContact::PhoneType,QString> numbers = c.phoneNumbers();
            QList<QContact::PhoneType> types = numbers.keys();
            QList<QContact::PhoneType>::const_iterator iter;
            for ( iter = types.constBegin() ;iter != types.constEnd(); iter++ ) {
                //don't show pager or fax
                if ( (*iter) & QContact::Fax  || (*iter) & QContact::Pager )
                    continue;
               
                ContactRecord store;
                store.type = *iter;
                store.id = id;
                d->contactIndices.insert( index++, store );
            }
        }
        
    }
    //contactmodel doesn't really have entry limit
    //set artifical limits for the sake of at+cpbs 
    d->availableMemoryContacts = qMin( d->availableMemoryContacts,
                index-(index%50)+100 );

}

void AtCommands::writeMemoryPhoneBookEntry( bool isDeletion, uint index, const QString& number, const QString& text )
{
    if ( index < 1 || (int) index > d->availableMemoryContacts ) {
        done( QAtResult::InvalidIndex );
        return;
    }
    if ( number.length() > 60 ) {
        done( QAtResult::DialStringTooLong );
        return;
    }

    if ( text.length() > 60 ) {
        done( QAtResult::TextStringTooLong );
        return;
    }

    //index already used by other contact
    if ( d->contactIndices.contains( index ) ) {
        QContact c =d->contactModel->contact( 
                d->contactIndices.value( index ).id );
        
        QString name = c.label();
        name = name.left( 60 ); //we display the first 60 chars only

        if ( isDeletion ) {
            //delete entry at given index
            c.setPhoneNumber( d->contactIndices.value(index).type, QString() );
            d->contactModel->updateContact( c );
            d->contactIndices.remove( index );
        } else if ( QString::compare(name, text, Qt::CaseInsensitive) == 0 ) {
            //same name -> replace phone number
            c.setPhoneNumber( d->contactIndices.value(index).type, number );
            d->contactModel->updateContact( c );
        } else {
            //not same name 
            //remove old phone number
            c.setPhoneNumber( d->contactIndices.value(index).type, QString() );
            d->contactModel->updateContact( c );
            d->contactIndices.remove( index );
            //add new contact
            QContact newContact;
            newContact.setPhoneNumber( QContact::HomePhone, number );
            if ( !text.isEmpty() ) {
                int space = text.indexOf( QChar(' ') );
                if ( space == -1 ) {  //not found
                    newContact.setFirstName( text );
                } else {
                    newContact.setFirstName( text.left( space ) );
                    newContact.setLastName( text.mid( space+1 ) );
                }
            }
            ContactRecord store;
            store.type = QContact::HomePhone;
            store.id = d->contactModel->addContact( newContact );
            if ( store.id.isNull() )
                qLog(ModemEmulator) << "Cannot add new memory contact.";
            else
                d->contactIndices.insert( index, store );
        }
    } else {
        QContact newContact;
        newContact.setPhoneNumber( QContact::HomePhone, number );
        if ( !text.isEmpty() ) {
            int space = text.indexOf( QChar(' ') );
            if ( space == -1 ) {  //not found
                newContact.setFirstName( text );
            } else {
                newContact.setFirstName( text.left( space ) );
                newContact.setLastName( text.mid( space+1 ) );
            }
        }
        ContactRecord store;
        store.type = QContact::HomePhone;
        store.id = d->contactModel->addContact( newContact );
        if ( store.id.isNull() )
            qLog(ModemEmulator) << "Cannot add new memory contact.";
        else
            d->contactIndices.insert( index, store );

    }
    done();
}

#endif // QTOPIA_CELL

#include "atcommands.moc"
