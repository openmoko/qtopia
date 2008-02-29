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

#include "commonencodestream.h"

CommonEncodeStream::CommonEncodeStream
        ( QObject *parent, int payloadType, int payloadSize )
    : MediaStream( parent )
{
    _payloadType = payloadType;
    _payloadSize = payloadSize;
    size = 0;
    open( WriteOnly );
}

CommonEncodeStream::~CommonEncodeStream()
{
}

void CommonEncodeStream::setNext( MediaStream *value )
{
    MediaStream::setNext( value );

    // Tell the RTP sink on the end of the stream chain about our settings.
    if ( value ) {
        value->setPayloadType( _payloadType );
        value->setPayloadSize( _payloadSize );
    }
}

/*!
    Write \a len bytes from \a data to this encoder.  The length must be
    a multiple of 2, and the data should consist of 16-bit samples at 8000 Hz,
    one channel.  The samples should be in host byte order.
*/
qint64 CommonEncodeStream::writeData( const char *data, qint64 len )
{
    int requiredBytes = _payloadSize * 2;
    int temp = (int)len;
    int enclen;
    while ( temp > 1 ) {
        if ( size == 0 && (((long)data) & 1) == 0 && temp >= requiredBytes ) {

            // Encode a full packet direct from the supplied input buffer.
            enclen = encode( (short *)data, frameout );
            writeNext( ( const char *)frameout, enclen );
            data += requiredBytes;
            temp -= requiredBytes;

        } else if ( ( size * 2 + temp ) >= requiredBytes ) {

            // Fill the framein buffer and then encode it.
            int copy = requiredBytes - size * 2;
            memcpy( framein + size, data, copy );
            enclen = encode( framein, frameout );
            writeNext( ( const char *)frameout, enclen );
            data += copy;
            temp -= copy;
            size = 0;

        } else {

            // Not enough bytes yet - copy as many as we can.
            memcpy( framein + size, data, temp );
            size += temp / 2;
            data += temp;
            temp = 0;

        }
    }
    return len;
}
