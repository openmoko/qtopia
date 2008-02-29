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

#include "rtpsinkstream.h"
#include <qudpsocket.h>
#include <qtimer.h>
#include <qtopialog.h>

RtpSinkStream::RtpSinkStream( QObject *parent )
    : MediaStream( parent )
{
    _payloadType = 0;
    _payloadSize = 160;     // Usual size of a 20ms long 8000 Hz frame.
    socket = 0;
    port = 0;
    sequence = 0;
    timestamp = 0;
    source = 10;
    buffer.resize( 1024 );

    open( WriteOnly );
}

RtpSinkStream::~RtpSinkStream()
{
}

/*!
    Set the destination of the RTP sink to \a port on \a hostName.
    The readyToWrite() signal will be emitted when the RTP sink
    is ready to accept data.
*/
void RtpSinkStream::setDestination( const QString& hostName, int port )
{
    if ( socket ) {
        delete socket;
        socket = 0;
    }
    this->port = (quint16)port;
    if ( address.setAddress( hostName ) ) {
        makeSocket();
    } else {
        qLog(Sip) << "rtp sink lookup on" << hostName;
        QHostInfo::lookupHost( hostName, this, SLOT(lookupDone(QHostInfo)) );
    }
}

/*!
    Get the payload type to use in RTP packet headers.
*/
int RtpSinkStream::payloadType() const
{
    return _payloadType;
}

/*!
    Set the payload \a type to use in RTP packet headers.
*/
void RtpSinkStream::setPayloadType( int type )
{
    _payloadType = type;
}

/*!
    Get the size of the RTP packet payload.
*/
int RtpSinkStream::payloadSize() const
{
    return _payloadSize;
}

/*!
    Set the \a size of the RTP packet payload.  This is used to
    adjust the timestamp field of in the RTP packet header.
*/
void RtpSinkStream::setPayloadSize( int size )
{
    _payloadSize = size;
}

/*!
    Get the synchronization source value for the RTP packets.
*/
int RtpSinkStream::synchronizationSource() const
{
    return source;
}

/*!
    Set the synchronization \a source value for the RTP packets.
    The default will normally be sufficient.
*/
void RtpSinkStream::setSynchronizationSource( int source )
{
    this->source = source;
}

/*!
    Write \a len bytes from \a data to the RTP destination.
    All of the bytes will be incorporated into a single RTP packet.
    It is the responsibility of the previous media stream object
    to divide the stream into logical packets.
*/
qint64 RtpSinkStream::writeData( const char *data, qint64 len )
{
    qLog(SipAudioData) << "RtpSinkStream::writeData to remote port" << port;
    if ( socket ) {

        // Resize the buffer if necessary.
        if ( ( len + 12 ) > buffer.size() ) {
            buffer.resize( (int)(len + 12) );
        }

        // Populate the header fields.
        char *buf = buffer.data();
        buf[0]  = (char)0x80;
        buf[1]  = (char)_payloadType;
        buf[2]  = (char)(sequence >> 8);
        buf[3]  = (char)sequence;
        buf[4]  = (char)(timestamp >> 24);
        buf[5]  = (char)(timestamp >> 16);
        buf[6]  = (char)(timestamp >> 8);
        buf[7]  = (char)timestamp;
        buf[8]  = (char)(source >> 24);
        buf[9]  = (char)(source >> 16);
        buf[10] = (char)(source >> 8);
        buf[11] = (char)source;

        // Update the sequence number and timestamp for the next packet.
        ++sequence;
        timestamp += _payloadSize;

        // Copy the frame data into place and then send the datagram.
        memcpy( buf + 12, data, (int)len );
        socket->writeDatagram( buf, len + 12, address, port );

    }
    return len;
}

/*!
    \fn void RtpSinkStream::readyToWrite()

    Signal that is emitted after calling setDestination() to indicate
    that host lookup has been performed and data can now be written
    to the RTP sink.  Any data sent before this signal is emitted may be lost.
*/

/*!
    \fn void RtpSinkStream::lookupFailed()

    Signal that is emitted when hostname lookup fails.  This indicates
    that the RTP stream could not be initialized.
*/

void RtpSinkStream::lookupDone( const QHostInfo& info )
{
    if ( info.error() == QHostInfo::NoError ) {
        address = info.addresses()[0];
        makeSocket();
    } else {
        emit lookupFailed();
    }
}

void RtpSinkStream::makeSocket()
{
    if ( !socket ) {
        socket = new QUdpSocket( this );

        // Emit the "readyToWrite" signal upon the next entry to the
        // event loop because we don't want it to be emitted from
        // within a call to setDestination().
        QTimer::singleShot( 0, this, SIGNAL(readyToWrite()) );
    }
}
