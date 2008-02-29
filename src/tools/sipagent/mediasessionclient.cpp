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

#include "mediasessionclient.h"
#include "sdpparser.h"
#include <qtopiaservices.h>
#include <quuid.h>
#include <qdatastream.h>
#include <qtopiachannel.h>
#include <qtopialog.h>

enum MediaPhase
{
    Media_Inactive,
    Media_Negotiate,
    Media_Started,
    Media_Suspended
};

class MediaSessionClientPrivate
{
public:
    MediaSessionClientPrivate()
    {
        // Clear all values to their initial states.
        clear();

        // Create a dummy set of SDP parameters to indicate "hold".
        QStringList codecs;
        codecs += "PCMU/8000";
        codecs += "GSM/8000";
        codecs += "PCMA/8000";
        QList<int> payloadTypes;
        holdParameters = SdpParser::format
                ( "Hold", SdpParser::IPv4, "0.0.0.0", 0,        // No tr
                  codecs, payloadTypes );
    }

    QString id;
    QString remoteParameters;
    QString localParameters;
    QString holdParameters;
    MediaPhase mediaPhase;
    bool localOnHold;
    bool remoteOnHold;

    void clear()
    {
        id = QUuid::createUuid().toString();
        remoteParameters = QString();
        localParameters = QString();
        mediaPhase = Media_Inactive;
        localOnHold = false;
        remoteOnHold = false;
    }
};

// Determine if an SDP message indicates that the connection is on hold.
static bool isHoldRequest( const QString& sdp )
{
    SdpParser parser;
    parser.parse( sdp );
    return ( SdpParser::address( parser.connection( "audio" ) ) == "0.0.0.0" );
}

MediaSessionClient::MediaSessionClient( QObject *parent )
    : QObject( parent )
{
    d = new MediaSessionClientPrivate();

    QtopiaChannel *channel = new QtopiaChannel( "QPE/MediaSession", this );
    connect( channel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(received(QString,QByteArray)) );
}

MediaSessionClient::~MediaSessionClient()
{
    end();
    delete d;
}

/*!
    Determine if the remote side of the connection is currently on hold.
    The remote side may go on hold as a side-effect of the local side
    being placed on hold.  The onHold() method is a better indicator of
    whether the connection is currently in the hold state.

    \sa localOnHold(), onHold(), onHoldChanged(), remoteOnHoldChanged()
*/
bool MediaSessionClient::remoteOnHold() const
{
    return d->remoteOnHold;
}

/*!
    Determine if the local side of the connection is currently on hold.

    \sa remoteOnHold(), onHold(), onHoldChanged()
*/
bool MediaSessionClient::localOnHold() const
{
    return d->localOnHold;
}

/*!
    \fn bool MediaSessionClient::onHold() const

    Determine if the connection is currently on hold, either due to
    the local or remote side placing it on hold.

    \sa localOnHold(), remoteOnHold(), onHoldChanged(), remoteOnHoldChanged()
*/

/*!
    Set up a media session that was initiated locally.  The signal
    sendLocalParameters() will be emitted when the local parameters
    to use can be sent to the remote host.  If there is an existing
    session, then it will be terminated.
*/
void MediaSessionClient::setupLocal()
{
    end();
    localUnhold();
}

/*!
    Set up a media session that was initiated remotely.  The \a sdp
    parameters indicates the session description from the remote side.
    The signal sendLocalParameters() will be emitted when the
    local parameters for the session have been negotiated.  If there
    is an existing session, then it will be terminated.
*/
void MediaSessionClient::setupRemote( const QString& sdp )
{
    end();
    changeRemoteParameters( sdp );
}

/*!
    Change to the new remote parameters specified by \a sdp.
    If the session is not active, a new one will be started.
*/
void MediaSessionClient::changeRemoteParameters( const QString& sdp )
{
    if ( isHoldRequest( sdp ) ) {
        remoteHold();
    } else if ( d->mediaPhase == Media_Negotiate ) {
        // Locally-negotiated session - we now have both sets of values.
        d->remoteParameters = sdp;
        startMedia();
    } else {
        remoteSet( sdp );
    }
}

/*!
    Change the local hold state of this session to \a flag.
*/
void MediaSessionClient::setLocalOnHold( bool flag )
{
    if ( flag != d->localOnHold ) {
        d->localOnHold = flag;
        if ( flag ) {
            localHold();
        } else {
            localUnhold();
        }
    }
}

/*!
    End the current session.
*/
void MediaSessionClient::end()
{
    stopMedia();
    d->clear();
}

/*!
    Send DTMF \a tones to the remote side.  Ignored if the session
    is not currently sending media traffic.
*/
void MediaSessionClient::dtmf( const QString& tones )
{
    if ( d->mediaPhase == Media_Started ) {
        QtopiaServiceRequest e( "MediaSession",
                              "dtmf(QString,QString)" );
        e << d->id;
        e << tones;
        e.send();
    }
}

/*!
    \fn void MediaSessionClient::sendLocalParameters( const QString& sdp )

    Signal that is emitted when the media session wishes to send \a sdp
    as the local parameter block to the remote side.
*/

/*!
    \fn void MediaSessionClient::onHoldChanged()

    Signal that is emitted if the local or remote hold state
    may have changed.  Use localOnHold() and remoteOnHold() to
    determine the local or remote hold states.  Use onHold() to
    get a general indication as to whether the connection is on hold.

    \sa onHold(), localOnHold(), remoteOnHold()
*/

/*!
    \fn void MediaSessionClient::remoteOnHoldChanged()

    Signal that is emitted if the remote hold state
    may have changed.  Use remoteOnHold() to determine the remote
    hold state.  Use onHold() to get a general indication as to
    whether the connection is on hold.

    \sa onHold(), localOnHold(), remoteOnHold()
*/

/*!
    \fn void MediaSessionClient::sessionFailed()

    Signal that is emitted when the session fails to initialize because
    there were no codecs in common between the local and remote, or
    because hardware resources are not available to handle the
    requested session.
*/

void MediaSessionClient::received( const QString& msg, const QByteArray& data )
{
    if ( msg == "negotiateResult(QString,QString)" ) {
        QDataStream stream( data );
        QString id, sdp;
        stream >> id;
        stream >> sdp;
        qLog(Sip) << "MediaSessionClient::negotiateResult("
                  << id << "," << sdp << ")";
        if ( id == d->id ) {
            d->localParameters = sdp;
            if ( d->remoteParameters.isEmpty() ) {
                // We don't have remote parameters yet, so we are negotiating
                // a setup request from the local to the remote.
            qLog(Sip) << "sendLocalParameters()";
                emit sendLocalParameters( sdp );
            } else {
                // We have both sets of parameters, so start up the media.
                emit sendLocalParameters( sdp );
                startMedia();
            }
        }
    } else if ( msg == "startFailed(QString)" ) {
        QDataStream stream( data );
        QString id;
        stream >> id;
        if ( id == d->id ) {
            // Shut everything down because we couldn't create a session.
            stopMedia();
            emit sessionFailed();
        }
    } else if ( msg == "renegotiate(QString)" ) {
        QDataStream stream( data );
        QString id;
        stream >> id;
        if ( id == d->id ) {
            // Force an "unhold" to renegotiate the session parameters.
            if ( ! onHold() )
                localUnhold();
        }
    }
}

void MediaSessionClient::startMedia()
{
    if ( d->mediaPhase == Media_Negotiate ) {
        d->mediaPhase = Media_Started;
        QtopiaServiceRequest e( "MediaSession",
                              "start(QString,QString,QString)" );
        e << d->id << d->localParameters << d->remoteParameters;
        e.send();
    }
}

void MediaSessionClient::stopMedia()
{
    if ( d->mediaPhase != Media_Inactive ) {

        // Stop the current negotiation or media session.
        QtopiaServiceRequest e( "MediaSession", "stop(QString)" );
        e << d->id;
        e.send();
        d->mediaPhase = Media_Inactive;

    }
}

void MediaSessionClient::suspendMedia()
{
    qLog(Sip) << "MediaSessionClient::suspendMedia()";
    if ( d->mediaPhase == Media_Negotiate || d->mediaPhase == Media_Started ) {

        // Suspend the current media session.
        QtopiaServiceRequest e( "MediaSession", "suspend(QString)" );
        e << d->id;
        e.send();
        d->mediaPhase = Media_Suspended;

    }
}

// The remote has requested to be put on hold.
void MediaSessionClient::remoteHold()
{
    d->remoteOnHold = true;
    if ( d->mediaPhase == Media_Started ) {
        qLog(Sip) << "MediaSessionClient::remoteHold() - doing suspend";
        suspendMedia();
        d->remoteParameters = QString();
        emit sendLocalParameters( d->holdParameters );
    }
    emit onHoldChanged();
    emit remoteOnHoldChanged();
}

// Set the remote parameters (if not hold).
void MediaSessionClient::remoteSet( const QString& sdp )
{
    if ( d->localOnHold ) {

        // The local side is still on hold, so fail the request.
        d->remoteOnHold = false;
        d->remoteParameters = QString();
        emit sendLocalParameters( d->holdParameters );
        emit onHoldChanged();

    } else {

        // Bail out if the remote just told us to do what we are already doing.
        if ( d->mediaPhase == Media_Started && sdp == d->remoteParameters )
            return;

        // Suspend the current media session.
        qLog(Sip) << "MediaSessionClient::remoteSet() - doing suspend, "
                  << sdp << "," << d->remoteParameters;
        suspendMedia();

        // Negotiate a new session.
        d->remoteParameters = sdp;
        d->mediaPhase = Media_Negotiate;
        QtopiaServiceRequest e( "MediaSession", "negotiate(QString,QString)" );
        e << d->id << sdp;
        e.send();

        // Notify higher layers that the remote has come off hold.
        d->remoteOnHold = false;
        emit onHoldChanged();

    }
    emit remoteOnHoldChanged();
}

// The local side has requested to be put on hold.
void MediaSessionClient::localHold()
{
    d->localOnHold = true;
    if ( d->mediaPhase == Media_Started || d->mediaPhase == Media_Negotiate ) {
        qLog(Sip) << "MediaSessionClient::localHold() - doing suspend";
        suspendMedia();
        d->remoteParameters = QString();
        emit sendLocalParameters( d->holdParameters );
    }
    emit onHoldChanged();
}

// The local side has requested to be taken off hold.
void MediaSessionClient::localUnhold()
{
    // Modify the local hold state.
    d->localOnHold = false;

    qLog(Sip) << "MediaSessionClient::localUnhold() - doing suspend";
    // Force a previous negotiation to end if there was one.
    suspendMedia();

    // Negotiate a new session.  It may be rejected by the
    // remote side if it is still on hold itself.  Until then,
    // we assume that the remote is not on hold.
    qLog(Sip) << "MediaSessionClient::negotiate("
              << d->id << ", \"\" )";
    d->remoteOnHold = false;
    d->mediaPhase = Media_Negotiate;
    d->remoteParameters = QString();
    QtopiaServiceRequest e( "MediaSession", "negotiate(QString,QString)" );
    e << d->id << QString();
    e.send();

    // Notify higher layers of potential changes in the hold state.
    emit onHoldChanged();
}
