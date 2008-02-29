/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QUUID_H
#define QUUID_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#include <string.h>

#if defined(Q_OS_WIN32)
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    ulong   Data1;
    ushort  Data2;
    ushort  Data3;
    uchar   Data4[ 8 ];
} GUID, *REFGUID, *LPGUID;
#endif
#endif

struct Q_EXPORT QUuid
{
    QUuid()
    {
	memset( this, 0, sizeof(QUuid) );
    }
    QUuid( uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8 )
    {
	data1 = l;
	data2 = w1;
	data3 = w2;
	data4[0] = b1;
	data4[1] = b2;
	data4[2] = b3;
	data4[3] = b4;
	data4[4] = b5;
	data4[5] = b6;
	data4[6] = b7;
	data4[7] = b8;
    }
    QUuid( const QUuid &uuid )
    {
	memcpy( this, &uuid, sizeof(QUuid) );
    }
#ifndef QT_NO_QUUID_STRING
    QUuid( const QString & );
    QUuid( const char * );
    QString toString() const;
    operator QString() const { return toString(); }
#endif
    bool isNull() const;

    QUuid &operator=(const QUuid &orig )
    {
	memcpy( this, &orig, sizeof(QUuid) );
	return *this;
    }

    bool operator==(const QUuid &orig ) const
    {
	uint i;
	if ( data1 != orig.data1 || data2 != orig.data2 || 
	     data3 != orig.data3 )
	    return FALSE;

	for( i = 0; i < 8; i++ )
	    if ( data4[i] != orig.data4[i] )
		return FALSE;
	
	return TRUE;
    }

    bool operator!=(const QUuid &orig ) const
    {
	return !( *this == orig );
    }

#if defined(Q_OS_WIN32)
    // On Windows we have a type GUID that is used by the platform API, so we
    // provide convenience operators to cast from and to this type.
    QUuid( const GUID &guid )
    {
	memcpy( this, &guid, sizeof(GUID) );
    }

    QUuid &operator=(const GUID &orig )
    {
	memcpy( this, &orig, sizeof(QUuid) );
	return *this;
    }

    operator GUID() const
    {
	GUID guid = { data1, data2, data3, { data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7] } };
	return guid;
    }

    bool operator==( const GUID &guid ) const
    {
	uint i;
	if ( data1 != guid.Data1 || data2 != guid.Data2 || 
	     data3 != guid.Data3 )
	    return FALSE;

	for( i = 0; i < 8; i++ )
	    if ( data4[i] != guid.Data4[i] )
		return FALSE;
	
	return TRUE;
    }

    bool operator!=( const GUID &guid ) const
    {
	return !( *this == guid );
    }
#endif

    uint    data1;
    ushort  data2;
    ushort  data3;
    uchar   data4[ 8 ];
};

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QUuid & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QUuid & );
#endif

#endif //QUUID_H
