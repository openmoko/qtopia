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

#include "sipagentmedia.h"
#include "sdpparser.h"

#include "rtpsinkstream.h"
#include "gsmencodestream.h"
#include "pcmuencodestream.h"
#include "pcmaencodestream.h"
#include "audioinputstream.h"
#include "dtmfstream.h"

#include "rtpsourcestream.h"
#include "gsmdecodestream.h"
#include "pcmudecodestream.h"
#include "pcmadecodestream.h"
#include "audiooutputstream.h"

#include "wavoutputstream.h"

//#define DEBUG_AUDIO_DATA 1

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qnetworkstate.h>
#include <qnetworkdevice.h>
#include <dissipate2/sipprotocol.h>
#include <qtopialog.h>

class SipAgentMediaSession
{
public:
    SipAgentMediaSession( const QString& id );
    ~SipAgentMediaSession();

    void shutdown();
    void enable();
    void disable();

    QString id;
    QObject *mediaParent;
    AudioOutputStream *audioOut;
    AudioInputStream *audioIn;
    DtmfStream *dtmf;

private:
    void init();
};

SipAgentMediaSession::SipAgentMediaSession( const QString& id )
{
    this->id = id;
    init();
}

SipAgentMediaSession::~SipAgentMediaSession()
{
    delete mediaParent;
}

void SipAgentMediaSession::shutdown()
{
    delete mediaParent;
    init();
}

void SipAgentMediaSession::enable()
{
    qLog(Sip) << "SipAgentMediaSession::enable";
    if ( audioIn )
        audioIn->start();
    if ( audioOut )
        audioOut->start();
}

void SipAgentMediaSession::disable()
{
    qLog(Sip) << "SipAgentMediaSession::disable";
    if ( audioIn )
        audioIn->stop();
    if ( audioOut )
        audioOut->stop();
}

void SipAgentMediaSession::init()
{
    mediaParent = new QObject();
    audioOut = 0;
    audioIn = 0;
    dtmf = 0;
}

SipAgentMedia::SipAgentMedia( QObject *parent )
    : MediaSessionService( parent )
{
    currentSession = 0;

    networkState = new QNetworkState( this );
    QObject::connect
        ( networkState, SIGNAL(defaultGatewayChanged(QString,QNetworkInterface)),
          this, SLOT(networkChanged(QString,QNetworkInterface)) );
    QNetworkInterface iface = QNetworkDevice( networkState->gateway() ).address();
    if ( iface.isValid() ) {
        localAddress = iface.addressEntries().at(0).ip();
    } else {
        localAddress = QHostAddress();
    }
}

SipAgentMedia::~SipAgentMedia()
{
    QMap<QString, SipAgentMediaSession *>::Iterator it;
    for ( it = sessions.begin(); it != sessions.end(); ++it ) {
        delete it.value();
    }
    sessions.clear();
}

void SipAgentMedia::negotiate( const QString& id, const QString& remotesdp )
{
    qLog(Sip) << "SipAgentMedia::negotiate(" << id << "," << remotesdp << ")";

    // Get the current session object for the id, or create a new one.
    SipAgentMediaSession *session;
    if ( sessions.contains( id ) ) {
        session = sessions.value( id );
    } else {
        session = new SipAgentMediaSession( id );
        sessions.insert( id, session );
    }
    session->shutdown();

    // Collect up the full list of codecs that we support.
    QStringList ourCodecs = configuredCodecs();

    // Negotiate codecs with the remote side.
    QStringList codecs;
    if ( remotesdp.isEmpty() ) {
        // No remote yet, so send our full list of capabilities.
        codecs = ourCodecs;
    } else {
        // Find the codecs that both sides support.
        SdpParser parser( remotesdp );
        codecs = commonCodecs( ourCodecs, parser.codecs( "audio" ) );
    }

    // Create the RTP source for local playback.
    QObject *mediaParent = session->mediaParent;
    RtpSourceStream *source = new RtpSourceStream( mediaParent );

    // Get the local IP address to use.
    SdpParser::Protocol ipprotocol;
    QString ipaddress;
    if ( localAddress.protocol() == QAbstractSocket::IPv4Protocol ) {
        ipprotocol = SdpParser::IPv4;
        ipaddress = localAddress.toString();
    } else if ( localAddress.protocol() == QAbstractSocket::IPv6Protocol ) {
        ipprotocol = SdpParser::IPv6;
        ipaddress = localAddress.toString();
    } else {
        // Qtopia network is not up yet, so use libdissipate2 to get the addr.
        ipprotocol = SdpParser::IPv4;
        ipaddress = Sip::getLocalAddress();
    }

    // Construct the SDP packet that describes the local capabilities.
    QString localsdp;
    QList<int> payloadTypes;
    localsdp = SdpParser::format( "-",
                                  ipprotocol,
                                  ipaddress,
                                  source->port(),
                                  codecs,
                                  payloadTypes );

    // Create the codec streams.
    source->addCodec( new GsmDecodeStream( mediaParent ) );
    source->addCodec( new PcmuDecodeStream( mediaParent ) );
    source->addCodec( new PcmaDecodeStream( mediaParent ) );

    // Create a wav output stream to save the data to "/tmp/audioout.wav".
    MediaStream *prev = source;
#ifdef DEBUG_AUDIO_DATA
    WavOutputStream *wav = new WavOutputStream
            ( "/tmp/audioout.wav", mediaParent );
    wav->start( 8000, 1 );
    prev->setNext( wav );
    prev = wav;
#endif

    // Create the audio output stream.
    AudioOutputStream *audioOut = new AudioOutputStream( mediaParent );
    prev->setNext( audioOut );
    session->audioOut = audioOut;

    // Send the local SDP parameters to the client.  Negotiation is done.
    qLog(Sip) << "SipAgentMedia::negotiateResult("
              << id << "," << localsdp << ")";
    QtopiaIpcEnvelope e( "QPE/MediaSession", "negotiateResult(QString,QString)" );
    e << id;
    e << localsdp;
}

void SipAgentMedia::start( const QString& id, const QString& localsdp,
                           const QString& remotesdp )
{
    qLog(Sip) << "SipAgentMedia::start("
              << id << "," << localsdp << "," << remotesdp << ")";

    // Find the session.
    SipAgentMediaSession *session;
    if ( ! sessions.contains( id ) )
        return;
    session = sessions.value( id );

    // Do we need to set up an upload path for this session?
    // We need remote SDP parameters, and valid destination port details.
    if ( ! remotesdp.isEmpty() ) {
        SdpParser remoteParser( remotesdp );
        QString conn = remoteParser.connection( "audio" );
        int port = remoteParser.port( "audio" );
        if ( port > 0 && ! conn.isEmpty() ) {

            // Negotiate the common codec that we will be using.
            SdpParser localParser( localsdp );
            QStringList codecs = commonCodecs( localParser.codecs( "audio" ),
                                               remoteParser.codecs( "audio" ) );

            // Set up the RTP sink for the destination.
            QObject *mediaParent = session->mediaParent;
            RtpSinkStream *sink = new RtpSinkStream( mediaParent );
            sink->setDestination( SdpParser::address( conn ), port );

            // Create the codec for encoding our local audio.
            MediaStream *stream;
            if ( codecs[0] == "PCMU/8000" )
                stream = new PcmuEncodeStream( mediaParent );
            else if ( codecs[0] == "PCMA/8000" )
                stream = new PcmaEncodeStream( mediaParent );
            else if ( codecs[0] == "GSM/8000" )
                stream = new GsmEncodeStream( mediaParent );
            else {
                // Shouldn't happen due to how commonCodecs() works!
                stream = new PcmuEncodeStream( mediaParent );
            }
            stream->setNext( sink );

            // Create the DTMF generator.
            DtmfStream *dtmf = new DtmfStream( mediaParent );
            session->dtmf = dtmf;
            dtmf->setNext( stream );

            // Create a debug stream to write the data to "/tmp/audioin.wav".
            MediaStream *audioNext = dtmf;
#ifdef DEBUG_AUDIO_DATA
            WavOutputStream *wav = new WavOutputStream
                ( "/tmp/audioin.wav", mediaParent );
            wav->start( 8000, 1 );
            wav->setNext( audioNext );
            audioNext = wav;
#endif

            // Create the audio input stream to feed the codec.
            AudioInputStream *audioIn = new AudioInputStream( mediaParent );
            audioIn->setNext( audioNext );
            session->audioIn = audioIn;
            qLog(Sip) << "SipAgentMedia::start - audio in created";
        }
    }

    // Enable sending and receiving of audio data.
    if ( currentSession )
        currentSession->disable();
    currentSession = session;
    session->enable();
}

void SipAgentMedia::stop( const QString& id )
{
    qLog(Sip) << "SipAgentMedia::stop(" << id << ")";

    // Find the session.
    SipAgentMediaSession *session;
    if ( ! sessions.contains( id ) )
        return;
    session = sessions.value( id );

    // Disable sending and receiving of audio data.
    session->disable();

    // Remove the session and shut it down.
    if ( currentSession == session )
        currentSession = 0;
    sessions.remove( id );
    delete session;
}

void SipAgentMedia::suspend( const QString& id )
{
    qLog(Sip) << "SipAgentMedia::suspend(" << id << ")";
    // In this implementation, stop and suspend are identical.
    // This may change if we ever add support for video.
    stop( id );
}

void SipAgentMedia::dtmf( const QString& id, const QString& tones )
{
    // Find the session.
    SipAgentMediaSession *session;
    if ( ! sessions.contains( id ) )
        return;
    session = sessions.value( id );

    // Generate the DTMF tones.
    if ( session->dtmf )
        session->dtmf->dtmf( tones );
}

QStringList SipAgentMedia::commonCodecs( const QStringList& ourCodecs,
                                         const QStringList& theirCodecs )
{
    QStringList::ConstIterator it;
    QStringList codecs;
    for ( it = theirCodecs.begin(); it != theirCodecs.end(); ++it ) {
        QString name = (*it).toUpper();
        if ( ourCodecs.contains( name ) )
            codecs += name;
    }
    if ( codecs.isEmpty() ) {
        // Nothing in common!  Should at least be able to do PCMU!
        qLog(Sip) << "SipAgentMedia: no codecs in common; using PCMU/8000";
        codecs += "PCMU/8000";
    }
    return codecs;
}

bool SipAgentMedia::supportsCodecs( const QStringList& codecs )
{
    QStringList::ConstIterator it;
    QStringList ourCodecs = configuredCodecs();
    for ( it = codecs.begin(); it != codecs.end(); ++it ) {
        QString name = (*it).toUpper();
        if ( ourCodecs.contains( name ) )
            return true;
    }
    return false;
}

void SipAgentMedia::networkChanged( QString , const QNetworkInterface& newAddress )
{
    if ( !newAddress.isValid())
        return;

    QHostAddress addr = newAddress.addressEntries().at(0).ip();
    if ( ! ( addr == localAddress ) ) {

        // The local IP address has changed, so we need to renegotiate
        // all active sessions with the remote host.
        localAddress = addr;
        QMap<QString, SipAgentMediaSession *>::Iterator it;
        for ( it = sessions.begin(); it != sessions.end(); ++it ) {
            renegotiate( (*it)->id );
        }

    }
}

QStringList SipAgentMedia::configuredCodecs()
{
    // Build a list of all codecs that we support.
    QStringList codecs;
    codecs += "PCMU/8000";
    codecs += "GSM/8000";
    codecs += "PCMA/8000";

    // Find the configured codec and move it to the head of the list.
    QSettings config( "Trolltech", "SIPAgent" );
    config.beginGroup( "Media" );
    QString codec = config.value( "Codec", QString( "PCMU/8000" ) ).toString();
    if ( codecs.contains( codec ) ) {
        codecs.removeAll( codec );
        codecs.prepend( codec );
    }
    return codecs;
}
