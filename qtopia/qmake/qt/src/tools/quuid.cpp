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

#include "quuid.h"

#include <qdatastream.h>

/*!
    \class QUuid quuid.h
    \brief The QUuid class defines a Universally Unique Identifier (UUID).

    \reentrant

    For objects or declarations that need to be uniquely identified,
    UUIDs (also known as GUIDs) are widely used in order to assign a
    fixed and easy to compare value to the object or declaration. The
    128-bit value of a UUID is generated by an algorithm that
    guarantees that the value is unique.

    In Qt, UUIDs are wrapped by the QUuid struct which provides
    convenience functions for handling UUIDs. Most platforms provide a
    tool to generate new UUIDs, for example, uuidgen and guidgen.
*/

/*!
    \fn QUuid::QUuid()

    Creates the null UUID {00000000-0000-0000-0000-000000000000}.
*/

/*!
    \fn QUuid::QUuid( uint l, ushort w1, ushort w2, uchar b1, uchar b2, uchar b3, uchar b4, uchar b5, uchar b6, uchar b7, uchar b8 )

    Creates a UUID with the value specified by the parameters, \a l,
    \a w1, \a w2, \a b1, \a b2, \a b3, \a b4, \a b5, \a b6, \a b7, \a
    b8.

    Example:
    \code
    // {67C8770B-44F1-410A-AB9A-F9B5446F13EE}
    QUuid IID_MyInterface( 0x67c8770b, 0x44f1, 0x410a, 0xab, 0x9a, 0xf9, 0xb5, 0x44, 0x6f, 0x13, 0xee )
    \endcode
*/

/*!
    \fn QUuid::QUuid( const QUuid &orig )

    Creates a copy of the QUuid \a orig.
*/
#ifndef QT_NO_QUUID_STRING
/*!
    Creates a QUuid object from the string \a text. The function can
    only convert a string in the format
    {HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH} (where 'H' stands for a hex
    digit), and will create the null UUID if the conversion fails.
*/
QUuid::QUuid( const QString &text )
{
    bool ok;
    if ( text.isEmpty() ) {
	*this = QUuid();
	return;
    }
    QString temp = text.upper();
    if ( temp[0] != '{' )
	temp = "{" + text;
    if ( text[(int)text.length()-1] != '}' )
	temp += "}";

    data1 = temp.mid( 1, 8 ).toULong( &ok, 16 );
    if ( !ok ) {
	*this = QUuid();
	return;
    }

    data2 = temp.mid( 10, 4 ).toUInt( &ok, 16 );
    if ( !ok ) {
	*this = QUuid();
	return;
    }
    data3 = temp.mid( 15, 4 ).toUInt( &ok, 16 );
    if ( !ok ) {
	*this = QUuid();
	return;
    }
    data4[0] = temp.mid( 20, 2 ).toUInt( &ok, 16 );
    if ( !ok ) {
	*this = QUuid();
	return;
    }
    data4[1] = temp.mid( 22, 2 ).toUInt( &ok, 16 );
    if ( !ok ) {
	*this = QUuid();
	return;
    }
    for ( int i = 2; i<8; i++ ) {
	data4[i] = temp.mid( 25 + (i-2)*2, 2 ).toUShort( &ok, 16 );
	if ( !ok ) {
	    *this = QUuid();
	    return;
	}
    }
}

/*!
    \internal
*/
QUuid::QUuid( const char *text )
{
    *this = QUuid( QString(text) );
}
#endif
/*!
    \fn QUuid QUuid::operator=(const QUuid &uuid )

    Assigns the value of \a uuid to this QUuid object.
*/

/*!
    \fn bool QUuid::operator==(const QUuid &other) const

    Returns TRUE if this QUuid and the \a other QUuid are identical;
    otherwise returns FALSE.
*/

/*!
    \fn bool QUuid::operator!=(const QUuid &other) const

    Returns TRUE if this QUuid and the \a other QUuid are different;
    otherwise returns FALSE.
*/
#ifndef QT_NO_QUUID_STRING
/*!
    \fn QUuid::operator QString() const

    Returns the string representation of the uuid.

    \sa toString()
*/

/*!
    QString QUuid::toString() const

    Returns the string representation of the uuid.
*/
QString QUuid::toString() const
{
    QString result;

    result = "{" + QString::number( data1, 16 ).rightJustify( 8, '0' ) + "-";
    result += QString::number( (int)data2, 16 ).rightJustify( 4, '0' ) + "-";
    result += QString::number( (int)data3, 16 ).rightJustify( 4, '0' ) + "-";
    result += QString::number( (int)data4[0], 16 ).rightJustify( 2, '0' );
    result += QString::number( (int)data4[1], 16 ).rightJustify( 2, '0' ) + "-";
    for ( int i = 2; i < 8; i++ )
	result += QString::number( (int)data4[i], 16 ).rightJustify( 2, '0' );

    return result + "}";
}
#endif

#ifndef QT_NO_DATASTREAM
/*!
    \relates QUuid
    Writes the uuid \a id to the datastream \a s.
*/
QDataStream &operator<<( QDataStream &s, const QUuid &id )
{
    s << (Q_UINT32)id.data1;
    s << (Q_UINT16)id.data2;
    s << (Q_UINT16)id.data3;
    for (int i = 0; i < 8; i++ )
	s << (Q_UINT8)id.data4[i];
    return s;
}

/*!
    \relates QUuid
    Reads uuid from from the stream \a s into \a id.
*/
QDataStream &operator>>( QDataStream &s, QUuid &id )
{
    Q_UINT32 u32;
    Q_UINT16 u16;
    Q_UINT8 u8;
    s >> u32;
    id.data1 = u32;
    s >> u16;
    id.data2 = u16;
    s >> u16;
    id.data3 = u16;
    for (int i = 0; i < 8; i++ ) {
	s >> u8;
	id.data4[i] = u8;
    }
    return s;
}
#endif

/*!
    Returns TRUE if this is the null UUID
    {00000000-0000-0000-0000-000000000000}; otherwise returns FALSE.
*/
bool QUuid::isNull() const
{
    return data4[0] == 0 && data4[1] == 0 && data4[2] == 0 && data4[3] == 0 &&
	   data4[4] == 0 && data4[5] == 0 && data4[6] == 0 && data4[7] == 0 &&
	   data1 == 0 && data2 == 0 && data3 == 0;
}
