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

#include "mediastream.h"

/*!
    \class MediaStream
    \brief The MediaStream class provides an infrastructure for handling audiovisual data streams.

    The MediaStream class provides an infrastructure for handling audiovisual
    data streams.

    MediaStream objects are chained together with the setNext() function.
    Data is written to the first object in the stream, and it flows through
    the linked objects to the end.

    Media stream objects at the start of a chain are referred to as a \i source.
    A source might be a network socket for streaming data from the network,
    or an audio device to stream data from a microphone.

    Media stream objects at the end of a chain are referred to as a \i sink.
    A sink might be a network socket for streaming data to the network,
    or an audio device to stream data to a speaker.

    Media stream objects in the middle of a chain typically convert the
    data between formats; e.g. to encode or decode according to a codec.

    \sa QIODevice
*/

/*!
    Construct a new media stream object and attach it to \a parent.
    This will normally be followed by a calls to setNext() and open().
*/
MediaStream::MediaStream( QObject *parent )
    : QIODevice( parent )
{
    _next = 0;
}

/*!
    Destroy this media stream object.
*/
MediaStream::~MediaStream()
{
}

/*!
    \fn MediaStream *MediaStream::next() const

    Get the next object in the media stream.
*/

/*!
    Set the next object in the media stream to \a value.
*/
void MediaStream::setNext( MediaStream *value )
{
    if ( value == _next )
        return;
    if ( _next )
        disconnect( _next, SIGNAL(destroyed()), this, SLOT(nextDestroyed()) );
    if ( value )
        connect( value, SIGNAL(destroyed()), this, SLOT(nextDestroyed()) );
    _next = value;
}

/*!
    Get the payload type to use in RTP packet headers from the
    RTP sink at the end of this media stream.
*/
int MediaStream::payloadType() const
{
    if ( _next )
        return _next->payloadType();
    else
        return 0;
}

/*!
    Set the payload \a type to use in RTP packet headers in the
    RTP sink at the end of this media stream.
*/
void MediaStream::setPayloadType( int type )
{
    if ( _next )
        _next->setPayloadType( type );
}

/*!
    Get the payload size to use in RTP packet headers from the
    RTP sink at the end of this media stream.
*/
int MediaStream::payloadSize() const
{
    if ( _next )
        return _next->payloadSize();
    else
        return 160;
}

/*!
    Set the payload \a size to use in RTP packet headers in the
    RTP sink at the end of this media stream.
*/
void MediaStream::setPayloadSize( int size )
{
    if ( _next )
        _next->setPayloadSize( size );
}

/*!
    Determine if this QIODevice is sequential.  Always returns true.
*/
bool MediaStream::isSequential() const
{
    return true;
}

/*!
    Open this media stream in \a mode.  The only allowable mode
    is QIODevice::WriteOnly.  Media streams can never be read from.
*/
bool MediaStream::open( QIODevice::OpenMode mode )
{
    // Media streams can only be written to, never read from.
    if ( mode != WriteOnly )
        return false;
    setOpenMode( mode | Unbuffered );
    return true;
}

/*!
    Close this media stream.
*/
void MediaStream::close()
{
    setOpenMode( NotOpen );
}

/*!
    Flush the remaining data in this media stream.  The default
    implementation calls flush() on the next object in the stream.
*/
void MediaStream::flush()
{
    if ( _next )
        _next->flush();
}

/*!
    Read up to \a maxlen bytes from the stream and copy it to \a data.

    This implementation always returns -1 to indicate an error.  Media
    streams can only be written to, never read from.
*/
qint64 MediaStream::readData( char *, qint64 )
{
    return -1;
}

/*!
    Write \a len bytes from \a data to the next object in the media stream.
    Does nothing if there is no next object.  This is usually called by
    the subclass writeData() implementation when a complete packet is ready
    for the next object in the stream.
*/
qint64 MediaStream::writeNext( const char *data, qint64 len )
{
    if ( _next )
        return _next->write( data, len );
    else
        return len;
}

void MediaStream::nextDestroyed()
{
    _next = 0;
}
