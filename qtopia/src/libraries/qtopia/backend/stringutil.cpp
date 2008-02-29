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

#include <qtopia/stringutil.h>
#include <qregexp.h>
#include <qstringlist.h>

namespace Qtopia
{



/*
  Very, very simple Latin-1 only collation guaranteed to displease anyone
  who actually uses the non-ASCII characters.
 */

static const char  collationHack[] = {
(char) 0x00, //C-@
(char) 0x01, //C-A
(char) 0x02, //C-B
(char) 0x03, //C-C
(char) 0x04, //C-D
(char) 0x05, //C-E
(char) 0x06, //C-F
(char) 0x07, //C-G
(char) 0x08, //C-H
(char) 0x09, //C-I
(char) 0x0a, //C-J
(char) 0x0b, //C-K
(char) 0x0c, //C-L
(char) 0x0d, //C-M
(char) 0x0e, //C-N
(char) 0x0f, //C-O
(char) 0x10, //C-P
(char) 0x11, //C-Q
(char) 0x12, //C-R
(char) 0x13, //C-S
(char) 0x14, //C-T
(char) 0x15, //C-U
(char) 0x16, //C-V
(char) 0x17, //C-W
(char) 0x18, //C-X
(char) 0x19, //C-Y
(char) 0x1a, //C-Z
(char) 0x1b, //C-[
(char) 0x1c, //C-backslash
(char) 0x1d, //C-]
(char) 0x1e, //C-^
(char) 0x1f, //C-_
(char) 0x20,
(char) 0x21,
(char) 0x22,
(char) 0x23,
(char) 0x24,
(char) 0x25,
(char) 0x26,
(char) 0x27,
(char) 0x28,
(char) 0x29,
(char) 0x2a,
(char) 0x2b,
(char) 0x2c,
(char) 0x2d,
(char) 0x2e,
(char) 0x2f,
(char) 0x80, //0
(char) 0x81, //1
(char) 0x82, //2
(char) 0x83, //3
(char) 0x84, //4
(char) 0x85, //5
(char) 0x86, //6
(char) 0x87, //7
(char) 0x88, //8
(char) 0x89, //9
(char) 0x3a,
(char) 0x3b,
(char) 0x3c,
(char) 0x3d,
(char) 0x3f,
(char) 0x40,
(char) 0x41,
(char) 0x42,
(char) 0x43,
(char) 0x44,
(char) 0x45,
(char) 0x46,
(char) 0x47,
(char) 0x48,
(char) 0x49,
(char) 0x4a,
(char) 0x4b,
(char) 0x4c,
(char) 0x4d,
(char) 0x4e,
(char) 0x4f,
(char) 0x50,
(char) 0x51,
(char) 0x52,
(char) 0x53,
(char) 0x54,
(char) 0x55,
(char) 0x56,
(char) 0x57,
(char) 0x58,
(char) 0x59,
(char) 0x5a,
(char) 0x5b,
(char) 0x5c,
(char) 0x5d,
(char) 0x5e,
(char) 0x5f,
(char) 0x60,
(char) 0x61,
(char) 0x62,
(char) 0x63,
(char) 0x64,
(char) 0x65,
(char) 0x66,
(char) 0x67,
(char) 0x68,
(char) 0x69,
(char) 0x6a,
(char) 0x6b,
(char) 0x6c,
(char) 0x6d,
(char) 0x6e,
(char) 0x6f,
(char) 0x70,
(char) 0x71,
(char) 0x72,
(char) 0x73,
(char) 0x74,
(char) 0x75,
(char) 0x76,
(char) 0x77,
(char) 0x78,
(char) 0x79,
(char) 0x7a,
(char) 0x7b,
(char) 0x7c,
(char) 0x7d,
(char) 0x7e,
(char) 0x7f,
(char) 0x80, //C-M-@
(char) 0x81, //C-M-A
(char) 0x82, //C-M-B
(char) 0x83, //C-M-C
(char) 0x84, //C-M-D
(char) 0x85, //C-M-E
(char) 0x86, //C-M-F
(char) 0x87, //C-M-G
(char) 0x88, //C-M-H
(char) 0x89, //C-M-I
(char) 0x8a, //C-M-J
(char) 0x8b, //C-M-K
(char) 0x8c, //C-M-L
(char) 0x8d, //C-M-M
(char) 0x8e, //C-M-N
(char) 0x8f, //C-M-O
(char) 0x90, //C-M-P
(char) 0x91, //C-M-Q
(char) 0x92, //C-M-R
(char) 0x93, //C-M-S
(char) 0x94, //C-M-T
(char) 0x95, //C-M-U
(char) 0x96, //C-M-V
(char) 0x97, //C-M-W
(char) 0x98, //C-M-X
(char) 0x99, //C-M-Y
(char) 0x9a, //C-M-Z
(char) 0x9b, //C-M-[
(char) 0x9c, //C-M-backslash
(char) 0x9d, //C-M-]
(char) 0x9e, //C-M-^
(char) 0x9f, //C-M-_
(char) 0xa0,
(char) 0xa1,
(char) 0xa2,
(char) 0xa3,
(char) 0xa4,
(char) 0xa5,
(char) 0xa6,
(char) 0xa7,
(char) 0xa8,
(char) 0xa9,
(char) 0xaa,
(char) 0xab,
(char) 0xac,
(char) 0xad,
(char) 0xae,
(char) 0xaf,
(char) 0xb0,
(char) 0xb1,
(char) 0xb2,
(char) 0xb3,
(char) 0xb4,
(char) 0xb5,
(char) 0xb6,
(char) 0xb7,
(char) 0xb8,
(char) 0xb9,
(char) 0xba,
(char) 0xbb,
(char) 0xbc,
(char) 0xbd,
(char) 0xbe,
(char) 0xbf,
(char) 0xc0,
(char) 0xc1,
(char) 0xc2,
(char) 0xc3,
(char) 0xc4,
(char) 0xc5,
(char) 0xc6,
(char) 0xc7,
(char) 0xc8,
(char) 0xc9,
(char) 0xca,
(char) 0xcb,
(char) 0xcc,
(char) 0xcd,
(char) 0xce,
(char) 0xcf,
(char) 0xd0,
(char) 0xd1,
(char) 0xd2,
(char) 0xd3,
(char) 0xd4,
(char) 0xd5,
(char) 0xd6,
(char) 0xd7,
(char) 0xd8,
(char) 0xd9,
(char) 0xda,
(char) 0xdb,
(char) 0xdc,
(char) 0xdd,
(char) 0xde,
(char) 0xdf,
(char) 0xe0,
(char) 0xe1,
(char) 0xe2,
(char) 0xe3,
(char) 0xe4,
(char) 0xe5,
(char) 0xe6,
(char) 0xe7,
(char) 0xe8,
(char) 0xe9,
(char) 0xea,
(char) 0xeb,
(char) 0xec,
(char) 0xed,
(char) 0xee,
(char) 0xef,
(char) 0xf0,
(char) 0xf1,
(char) 0xf2,
(char) 0xf3,
(char) 0xf4,
(char) 0xf5,
(char) 0xf6,
(char) 0xf7,
(char) 0xf8,
(char) 0xf9,
(char) 0xfa,
(char) 0xfb,
(char) 0xfc,
(char) 0xfd,
(char) 0xfe,
(char) 0xff,
};





static void hackString ( QString &s )
{
    int len = s.length();
    const QChar* uc = s.unicode();
    for ( int i = 0; i < len; i++ ) {
	if ( !uc++->row() )
	    s[i] = collationHack[s[i].cell()];
    }
}

QString buildSortKey( const QString & s )
{
    QString res = s;
    hackString( res );
    return res;
}

QString buildSortKey( const QString & s1, const QString & s2 )
{
    QString res = s1 + QChar( '\0' ) + s2;
    hackString( res );
    return res;
}

QString buildSortKey( const QString & s1, const QString & s2, 
		      const QString & s3 )
{
    QString res = s1 + QChar( '\0' ) +  s2 + QChar( '\0' ) + s3;
    hackString( res );
    return res;
}

static inline QChar coll( QChar u )
{
    return u.row() ? u : QChar(collationHack[ u.cell() ]);
}


int compare( const QString & s1, const QString & s2 )
{
    const QChar* u1 = s1.unicode();
    const QChar* u2 = s2.unicode();
    
    if ( u1 == u2 )
	return 0;
    if ( u1 == 0 )
	return 1;
    if ( u2 == 0 )
	return -1;
    int l=QMIN(s1.length(),s2.length());
    while ( l-- && coll(*u1) == coll(*u2) )
	u1++,u2++;
    if ( l==-1 )
	return ( s1.length()-s2.length() );
    return u1->unicode() - u2->unicode();
}

QString simplifyMultiLineSpace( const QString &multiLine )
{
    QString result;
    QStringList lines =  QStringList::split("\n", multiLine);
    for ( QStringList::Iterator it = lines.begin(); it != lines.end(); ++it ) {
	if ( it != lines.begin() )
	    result += "\n";
	result += (*it).simplifyWhiteSpace();
    }
    return result;
}

QString escapeString( const QString& plain )
{
    QString tmp(plain);
    int pos = tmp.length();
    const QChar *uc = plain.unicode();
    while ( pos-- ) {
	unsigned char ch = uc[pos].latin1();
	if ( ch == '&' )
	    tmp.replace( pos, 1, "&amp;" );
	else if ( ch == '<' )
	    tmp.replace( pos, 1, "&lt;" );
	else if ( ch == '>' )
	    tmp.replace( pos, 1, "&gt;" );
	else if ( ch == '\"' )
	    tmp.replace( pos, 1, "&quot;" );
	// DON'T escape newlines here
    }
    return tmp;
}

QString plainString( const char* escaped, unsigned int length )
{
    return plainString( QString::fromUtf8( escaped, length ) );
}

QString plainString( const QCString& string )
{
    // We first have to pass it through a ::fromUtf8()
    return plainString( string.data(), string.length() );
}

QString plainString( const QString& string )
{
    QString tmp( string );
    int pos = -1;
    while ( (pos = tmp.find( "&", pos +1 ) ) != -1 ) {
	if ( tmp.find( "&amp;", pos ) == pos )
	    tmp.replace( pos, 5, "&" );
	else if ( tmp.find( "&lt;", pos ) == pos )
	    tmp.replace( pos, 4, "<" );
	else if( tmp.find( "&gt;", pos ) == pos )
	    tmp.replace( pos, 4, ">" );
	else if ( tmp.find( "&quot;", pos ) == pos )
	    tmp.replace( pos, 6, "\"" );
    }
    return tmp;
}

} // namespace QPC
