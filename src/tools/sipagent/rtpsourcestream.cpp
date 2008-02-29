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

#include "rtpsourcestream.h"
#include <qudpsocket.h>
#include <qtopialog.h>

RtpSourceStream::RtpSourceStream( QObject *parent )
    : MediaStream( parent )
{
    cached = 0;
    cachedPayloadType = -1;

    // RTP needs to run on an even port number, so keep binding until
    // we find an even port.  Hopefully this won't take too long!
    QList<QUdpSocket *> failed;
    for (;;) {
        socket = new QUdpSocket( this );
        socket->bind();
        qLog(Sip) << "listening on" << socket->localPort();
        if ( (socket->localPort() % 2) == 0 )
            break;
        else
            failed.append( socket );
    }

    // Clean up the sockets that bound to odd ports.
    QList<QUdpSocket *>::Iterator it;
    for ( it = failed.begin(); it != failed.end(); ++it ) {
        delete (*it);
    }

    // Hook up the readyToRead signal on our even socket.
    connect( socket, SIGNAL(readyRead()), this, SLOT(readyToRead()) );

    // Open this stream for writing.
    open( WriteOnly );
}

RtpSourceStream::~RtpSourceStream()
{
}

int RtpSourceStream::port() const
{
    return socket->localPort();
}

void RtpSourceStream::addCodec( MediaStream *value )
{
    if ( value ) {
        connect( value, SIGNAL(destroyed()), this, SLOT(codecDestroyed()) );
        codecs.append( value );
        value->setNext( next() );
    }
}

void RtpSourceStream::setNext( MediaStream *value )
{
    // Pass the value to all of the codecs, as it is really their "next".
    QList<MediaStream *>::Iterator it;
    for ( it = codecs.begin(); it != codecs.end(); ++it ) {
        (*it)->setNext( value );
    }

    // Remember the value for the next call to addCodec().
    MediaStream::setNext( value );
}

qint64 RtpSourceStream::writeData( const char *, qint64 len )
{
    // Should never be called for a source.
    return len;
}

void RtpSourceStream::codecDestroyed()
{
    codecs.removeAll( (MediaStream *)sender() );
    cached = 0;
    cachedPayloadType = -1;
}

void RtpSourceStream::readyToRead()
{
    int count=0;

    while ( socket->hasPendingDatagrams() ) {
        if(count > 20) break;
        count++;
        qint64 len = socket->readDatagram( buffer, sizeof( buffer ) );
        if ( len >= 12 ) {
            int type = ( buffer[1] & 0x7F );
            qLog(SipAudioData) << "RtpSourceStream::readyToRead ["
                               << socket->localPort()
                               << "]: rtp packet" << type;
            if ( type == cachedPayloadType ) {
                // Same as last time, so quickly call what we already found.
                if(count < 2)
                    cached->write( buffer + 12, len - 12 );
            } else {
                // Different payload type, so search for an appropriate codec.
                bool handled = false;
                QList<MediaStream *>::ConstIterator it;
                for ( it = codecs.begin(); it != codecs.end(); ++it ) {
                    if ( (*it)->payloadType() == type ) {
                        (*it)->write( buffer + 12, len - 12 );
                        handled = true;
                        cached = (*it);
                        cachedPayloadType = type;
                        break;
                    }
                }
                if ( !handled ) {
                    qLog(Sip) << "rtp payload type" << type << "is unknown";
                }
            }
        } else {
            qLog(Sip) << "short rtp datagram received";
        }
    }
}
