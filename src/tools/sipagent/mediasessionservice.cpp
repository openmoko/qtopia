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

#include "mediasessionservice.h"
#include <qtopiaipcenvelope.h>

/*!
    \service MediaSessionService MediaSession
    \brief Provides the Qtopia MediaSession service.

    The \i MediaSession service enables applications such as VoIP clients
    to access audio and video streaming sessions without needing to provide
    the media transport themselves.

    Session descriptions are passed to and from the \i MediaSession service
    in SDP (Session Description Protocol, RFC 2327) format, from which codec
    and RTP (Real-Time Protocol, RFC 3550) transport parameters can be
    extracted.

    The \i MediaSession service handles media transport, including the
    streaming of microphone/camera input to the network, and network
    traffic to the speaker/screen.  Higher level session operations,
    such as those performed by SIP (Session Initiation Protocol, RFC 3261),
    are handled by the VoIP client prior to invoking the \i MediaSession
    service.

    When initiating a session locally (e.g. for dial out), the client starts
    by calling negotiate() with a unique identifier for the session, and
    an empty set of remote SDP parameters.  The media session service responds
    by calling negotiateResult() with the local SDP parameters that describes
    its capabilities.  The client sends these parameters to the remote side,
    which responds with its SDP parameters.  These local and remote SDP
    parameters are then passed to start() to begin media traffic.

    The media session server responds by calling negotiateResult() with
    the local SDP parameters that the server will be using.  Once the result
    is received by the client, it will call start() with both the local and
    remote parameters to start media transport.  When the session ends,
    the client calls stop().

    When initiating a session from a remote source (e.g. for an incoming call),
    the client starts by calling negotiate() with a unique identifier for
    the session, and the remote SDP parameters that were supplied in the
    incoming call request.  The media session service responds by calling
    negotiateResult() with the local SDP parameters that best matches the
    remote request.  The client sends these parameters to the remote side,
    and then calls start() to begin media traffic.

    During negotiation, the media server may need to bind to local ports
    to set up the local side of the session (e.g. binding to a local UDP
    port for RTP traffic).  Any packets that are received prior to calling
    start() should be discarded.

    The client can abandon the session after negotiate() and before start()
    by calling stop().  This may happen if the remote side refuses to accept
    the local SDP parameters (e.g. because the connection is on hold).

    The companion MediaSessionClient class provides a convenient interface
    for accessing the \i MediaSession service.  It can also handle holds by
    shutting down the media session temporarily while a call is held
    by either party.

    \sa MediaSessionClient
*/

/*!
    \internal
*/
MediaSessionService::MediaSessionService( QObject *parent )
    : QtopiaAbstractService( "MediaSession", parent )
{
    publishAll();
}

/*!
    \internal
*/
MediaSessionService::~MediaSessionService()
{
}

/*!
    \fn void MediaSessionService::negotiate( const QString& id, const QString& remotesdp )

    Negotiate a new media session called \a id.  The \a remotesdp argument
    indicates the SDP parameters for the remote side of the session,
    or is empty for a locally-initiated session.  The service will
    respond by calling negotiateResult().

    This slot corresponds to the QCop service message
    \c{MediaSession::negotiate(QString,QString)}.
*/

/*!
    \fn void MediaSessionService::start( const QString& id, const QString& localsdp, const QString& remotesdp )

    Start the media session called \a id, with the local SDP parameters in
    \a localsdp, and the remote SDP parameters in \a remotesdp.

    The \a remotesdp parameter can be empty if we are starting a streaming
    download that does not have an upload counterpart.  It is still necessary
    to call negotiate() to get the local parameters to set up the download.

    This slot corresponds to the QCop service message
    \c{MediaSession::start(QString,QString,QString)}.
*/

/*!
    \fn void MediaSessionService::stop( const QString& id )

    Stop the media session called \a id.  This may be called after
    negotiate() or start().  If the media session service was
    displaying a user interface (for example, to show incoming video
    from the other party), it should be removed from the screen.

    This slot corresponds to the QCop service message
    \c{MediaSession::stop(QString)}.
*/

/*!
    \fn void MediaSessionService::suspend( const QString& id )

    Suspend the media session called \a id.  This happens when the session
    is placed on hold.  The session may be resumed at some future point by
    a call to negotiate(), or stopped permanently with a call to stop().

    When a session is suspended, it should detach from the microphone and
    camera to allow other applications or media sessions to use it.  If the
    media session service was displaying a user interface (for example,
    to show incoming video from the other party), it may be retained
    on-screen showing the last-received frame or some kind of hold image.

    It is valid for the media session service to treat suspend() the
    same as stop().

    This slot corresponds to the QCop service message
    \c{MediaSession::suspend(QString)}.
*/

/*!
    \fn void MediaSessionService::dtmf( const QString& id, const QString& tones )

    Send DTMF \a tones on the media session called \a id.  If the underlying
    transport has an out of band mechanism for DTMF, that should be used.
    Otherwise the tones should be generated locally and transmitted as
    audio samples.

    This slot corresponds to the QCop service message
    \c{MediaSession::dtmf(QString,QString)}.
*/

/*!
    Answer a negotiate() request for the session called \a id.
    The \a localsdp argument indicates the SDP parameters for the local
    side of the session.

    This method corresponds to the QCop message
    \c{negotiateResult(QString,QString)} on the \c{QPE/MediaSession} channel.
*/
void MediaSessionService::negotiateResult
            ( const QString& id, const QString& localsdp )
{
    QtopiaIpcEnvelope e( "QPE/MediaSession", "negotiate(QString,QString)" );
    e << id << localsdp;
}

/*!
    Indicate to clients that the session called \a id failed to start,
    either because there were no codecs in common between the local
    and the remote, or hardware resources are not available to handle
    the requested session.
*/
void MediaSessionService::startFailed( const QString& id )
{
    QtopiaIpcEnvelope e( "QPE/MediaSession", "startFailed(QString)" );
    e << id;
}

/*!
    Indicate to clients that the session called \a id should be
    renegotiated because the network configuration has changed.
*/
void MediaSessionService::renegotiate( const QString& id )
{
    QtopiaIpcEnvelope e( "QPE/MediaSession", "renegotiate(QString)" );
    e << id;
}
