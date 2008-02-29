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

#ifndef QUNICODETABLES_P_H
#define QUNICODETABLES_P_H

#include "qstring.h"

#ifdef QT_NO_UNICODETABLES
# include <ctype.h>
#endif

class QUnicodeTables {
public:
    static const Q_UINT8 unicode_info[];
#ifndef QT_NO_UNICODETABLES
    static const Q_UINT16 decomposition_map[];
    static const Q_UINT16 decomposition_info[];
    static const Q_UINT16 ligature_map[];
    static const Q_UINT16 ligature_info[];
    static const Q_UINT8 direction_info[];
    static const Q_UINT8 combining_info[];
    static const Q_UINT16 case_info[];
    static const Q_INT8 decimal_info[];
    static const Q_UINT16 symmetricPairs[];
    static const int symmetricPairsSize;
#endif
    static const Q_UINT8 line_break_info[];
    static const unsigned char otherScripts[];
    static const unsigned char indicScripts[];
    static const unsigned char scriptTable[];
    enum { SCRIPTS_INDIC = 0x7e };

    // see http://www.unicode.org/reports/tr14/tr14-13.html
    // we don't use the XX and AI properties and map them to AL instead.
    enum LineBreakClass {
	LineBreak_OP, LineBreak_CL, LineBreak_QU, LineBreak_GL, LineBreak_NS,
	LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR, LineBreak_PO,
	LineBreak_NU, LineBreak_AL, LineBreak_ID, LineBreak_IN, LineBreak_HY,
	LineBreak_BA, LineBreak_BB, LineBreak_B2, LineBreak_ZW, LineBreak_CM,
	LineBreak_SA, LineBreak_BK, LineBreak_CR, LineBreak_LF, LineBreak_SG,
	LineBreak_CB, LineBreak_SP
    };
};


inline QChar::Category category( const QChar &c )
{
#ifdef QT_NO_UNICODETABLES
    if ( c.unicode() > 0xff ) return QChar::Letter_Uppercase; //########
    return (QChar::Category)QUnicodeTables::unicode_info[c.unicode()];
#else
    register int uc = ((int)QUnicodeTables::unicode_info[c.row()]) << 8;
    uc += c.cell();
    return (QChar::Category)QUnicodeTables::unicode_info[uc];
#endif // QT_NO_UNICODETABLES
}

inline QChar lower( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    int row = c.row();
    int cell = c.cell();
    register int ci = QUnicodeTables::case_info[row];
    register int uc = ((int)QUnicodeTables::unicode_info[c.row()]) << 8;
    uc += c.cell();
    if (QUnicodeTables::unicode_info[uc] != QChar::Letter_Uppercase || !ci)
	return c;
    Q_UINT16 lower = QUnicodeTables::case_info[(ci<<8)+cell];
    return lower ? QChar(lower) : c;
#else
    if ( c.row() )
	return c;
    return QChar( tolower((uchar) c.latin1()) );
#endif
}

inline QChar upper( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    int row = c.row();
    int cell = c.cell();
    register int ci = QUnicodeTables::case_info[row];
    register int uc = ((int)QUnicodeTables::unicode_info[c.row()]) << 8;
    uc += c.cell();
    if (QUnicodeTables::unicode_info[uc] != QChar::Letter_Lowercase || !ci)
	return c;
    Q_UINT16 upper = QUnicodeTables::case_info[(ci<<8)+cell];
    return upper ? QChar(upper) : c;
#else
    if ( c.row() )
	return c;
    return QChar( toupper((uchar) c.latin1()) );
#endif
}

inline QChar::Direction direction( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::direction_info[c.row()];
    return (QChar::Direction) (QUnicodeTables::direction_info[(pos<<8)+c.cell()] & 0x1f);
#else
    Q_UNUSED(c);
    return QChar::DirL;
#endif
}

inline bool mirrored( const QChar &c )
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::direction_info[c.row()];
    return QUnicodeTables::direction_info[(pos<<8)+c.cell()] > 128;
#else
    Q_UNUSED(c);
    return FALSE;
#endif
}


inline QChar mirroredChar( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    if(!::mirrored( ch ))
	return ch;

    int i;
    int c = ch.unicode();
    for (i = 0; i < QUnicodeTables::symmetricPairsSize; i ++) {
	if (QUnicodeTables::symmetricPairs[i] == c)
	    return QUnicodeTables::symmetricPairs[(i%2) ? (i-1) : (i+1)];
    }
#endif
    return ch;
}

inline QChar::Joining joining( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    register int pos = QUnicodeTables::direction_info[ch.row()];
    return (QChar::Joining) ((QUnicodeTables::direction_info[(pos<<8)+ch.cell()] >> 5) &0x3);
#else
    Q_UNUSED(ch);
    return QChar::OtherJoining;
#endif
}

inline bool isMark( const QChar &ch )
{
    QChar::Category c = ::category( ch );
    return c >= QChar::Mark_NonSpacing && c <= QChar::Mark_Enclosing;
}

inline unsigned char combiningClass( const QChar &ch )
{
#ifndef QT_NO_UNICODETABLES
    const int pos = QUnicodeTables::combining_info[ch.row()];
    return QUnicodeTables::combining_info[(pos<<8) + ch.cell()];
#else
    Q_UNUSED(ch);
    return 0;
#endif
}

inline bool isSpace( const QChar &ch )
{
    if( ch.unicode() >= 9 && ch.unicode() <=13 ) return TRUE;
    QChar::Category c = ::category( ch );
    return c >= QChar::Separator_Space && c <= QChar::Separator_Paragraph;
}

inline int lineBreakClass( const QChar &ch )
{
    register int pos = ((int)QUnicodeTables::line_break_info[ch.row()] << 8) + ch.cell();
    return QUnicodeTables::line_break_info[pos];
}

inline int scriptForChar( ushort uc )
{
    unsigned char script = QUnicodeTables::scriptTable[(uc>>8)];
    if ( script >= QUnicodeTables::SCRIPTS_INDIC ) {
	if ( script == QUnicodeTables::SCRIPTS_INDIC ) {
	    script = QUnicodeTables::indicScripts[ (uc-0x0900)>>7 ];
	} else {
	    // 0x80 + SCRIPTS_xx
	    unsigned char index = script-0x80;
	    unsigned char cell = uc &0xff;
	    while( QUnicodeTables::otherScripts[index++] < cell )
		index++;
	    script = QUnicodeTables::otherScripts[index];
	}
    }
    return script;
}

#ifdef Q_WS_X11
#define SCRIPT_FOR_CHAR( script, c ) 	\
do { 						\
    unsigned short _uc = (c).unicode(); 		\
    if ( _uc < 0x100 ) {				\
	script = QFont::Latin;		\
    } else { 					\
        script = (QFont::Script)scriptForChar( _uc ); 	\
    } 						\
} while( FALSE )
#else
#define SCRIPT_FOR_CHAR( script, c ) \
    script = (QFont::Script)scriptForChar( (c).unicode() )
#endif

#endif
