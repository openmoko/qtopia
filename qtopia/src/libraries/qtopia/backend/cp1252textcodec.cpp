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

#include "cp1252textcodec_p.h"

Q_UINT16 values[128] = { 
            0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
            0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
            0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
            0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178, 
            0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 
            0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
            0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 
            0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
            0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 
            0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 
            0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 
            0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF, 
            0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 
            0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 
            0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 
            0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF };

CP1252TextCodec::CP1252TextCodec()
{
}

CP1252TextCodec::~CP1252TextCodec()
{
}

const char *CP1252TextCodec::name() const
{
    return "CP-1252";
}

int CP1252TextCodec::mibEnum() const
{
    return 2001;
}

QString CP1252TextCodec::toUnicode( const char *chars, int len ) const
{
    QString str;
    
    for( int i = 0; i < len; ++i )
    {
        if( (unsigned char)chars[i] < 128 )
            str += QChar( chars[i] );
        else
            str += QChar( values[(unsigned char)chars[i] - 128] );
    }
    
    return str;
}

QCString CP1252TextCodec::fromUnicode( const QString &uc, int &lenInOut ) const
{
    QCString str;
    
    for( int i = 0; i < lenInOut; ++i )
    {
        if( (unsigned char)uc[i] < (unsigned char)128 )
            str += uc[i];
        else
        {
            QChar c = uc[i];
            for( unsigned char j = 0; j < 128; ++j )
            {
                if( values[j] == (Q_UINT16)c )
                {
                    str += ( j + 128 );
                    break;
                }
                
                str += '?';
            }
        }
    }
    
    return str;
}

int CP1252TextCodec::heuristicContentMatch( const char *chars, int len ) const
{
    QString t = toUnicode(chars, len);
    int l = t.length();
    QCString mb = fromUnicode(t, l);
    int i = 0;
    while( i < len ) 
    {
        if( chars[i] == mb[i] )
            i++;
        else
            break;
    }
    return i;
}

int CP1252TextCodec::heuristicNameMatch( const char * hint ) const
{
    int nameMatch = simpleHeuristicNameMatch( name(), hint );
    int fullMatch = simpleHeuristicNameMatch( "ISO-8859-1-Windows-3.1-Latin-1", hint );
    int subsetMatch = simpleHeuristicNameMatch( "ISO-8859-1", hint );
    
    return QMAX( nameMatch, QMAX( fullMatch, subsetMatch ) );
}



