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

#include "commondecodestream.h"

CommonDecodeStream::CommonDecodeStream
        ( QObject *parent, int payloadType, int packetSize )
    : MediaStream( parent )
{
    _payloadType = payloadType;
    _packetSize = packetSize;
    open( WriteOnly );
}

CommonDecodeStream::~CommonDecodeStream()
{
}

int CommonDecodeStream::payloadType() const
{
    return _payloadType;
}

qint64 CommonDecodeStream::writeData( const char *data, qint64 len )
{
    if ( len >= _packetSize ) {
        int declen = decode( (unsigned char *)data, frameout );
        writeNext( (char *)frameout, declen * 2 );
    }
    return len;
}
