/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "md5hash.h"

#include <stdlib.h>
#include <string.h>
#include "md5.h"

QString MD5::hash( const QString &str )
{
    /* MD5 wants pointers to unsigned char - without a
       lot of horrible and hazardous casts its not possible
       to get to and from a QByteArray, so lets just copy
       to and from the buffers MD5 wants */
    unsigned char outbuf[17];
    ::memset( outbuf, 0, 17 );

    unsigned char *inbuf;
    int inbufSize = str.length() * sizeof(QChar);
    inbuf = (unsigned char *)::malloc( inbufSize );
    Q_CHECK_PTR( inbuf );
    ::memcpy( inbuf, str.unicode(), inbufSize );

    MD5Context context;

    MD5Init( &context);
    MD5Update( &context, inbuf, inbufSize );
    MD5Final( outbuf, &context );

    // creates a deep copy
    QByteArray qbuffer( (const char *)outbuf );

    ::free( inbuf );
    return QString( qbuffer.toBase64() );
}
