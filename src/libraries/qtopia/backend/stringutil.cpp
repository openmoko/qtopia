/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
' ', // 
'!', //!
'"', //"  ;"
'#', //#
'$', //$
'%', //%
'&', //&
'\'', //'
'(', //(
')', //)
'*', //*
'+', //+
',', //,
'-', //-
'.', //.
'/', ///
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
':', //:
';', //;
'<', //<
'=', //=
'>', //>
'?', //?
'@', //@
'A', //A
'B', //B
'C', //C
'D', //D
'E', //E
'F', //F
'G', //G
'H', //H
'I', //I
'J', //J
'K', //K
'L', //L
'M', //M
'N', //N
'O', //O
'P', //P
'Q', //Q
'R', //R
'S', //S
'T', //T
'U', //U
'V', //V
'W', //W
'X', //X
'Y', //Y
'Z', //Z
'[', //[
'\\', //backslash
']', //]
'^', //^
'_', //_
'`', //`
'A', //a
'B', //b
'C', //c
'D', //d
'E', //e
'F', //f
'G', //g
'H', //h
'I', //i
'J', //j
'K', //k
'L', //l
'M', //m
'N', //n
'O', //o
'P', //p
'Q', //q
'R', //r
'S', //s
'T', //t
'U', //u
'V', //v
'W', //w
'X', //x
'Y', //y
'Z', //z
'{', //{
'|', //|
'}', //}
'~', //~
'', //
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
' ', // 
'¡', //¡
'¢', //¢
'£', //£
'¤', //¤
'¥', //¥
'¦', //¦
'§', //§
'¨', //¨
'©', //©
'A', //ª
'«', //«
'¬', //¬
'­', //­
'®', //®
'¯', //¯
'O', //°
'±', //±
'²', //²
'³', //³
'´', //´
'µ', //µ
'P', //¶
'·', //·
'¸', //¸
'¹', //¹
'O', //º
'»', //»
'¼', //¼
'½', //½
'¾', //¾
'¿', //¿
'A', //À
'A', //Á
'A', //Â
'A', //Ã
'A', //Ä
'A', //Å
'A', //Æ
'C', //Ç
'E', //È
'E', //É
'E', //Ê
'E', //Ë
'I', //Ì
'I', //Í
'I', //Î
'I', //Ï
'D', //Ð
'N', //Ñ
'O', //Ò
'O', //Ó
'O', //Ô
'O', //Õ
'O', //Ö
'×', //×
'O', //Ø
'U', //Ù
'U', //Ú
'U', //Û
'U', //Ü
'Y', //Ý
'T', //Þ
'S', //ß
'A', //à
'A', //á
'A', //â
'A', //ã
'A', //ä
'A', //å
'A', //æ
'C', //ç
'E', //è
'E', //é
'E', //ê
'E', //ë
'I', //ì
'I', //í
'I', //î
'I', //ï
'D', //ð
'N', //ñ
'O', //ò
'O', //ó
'O', //ô
'O', //õ
'O', //ö
'÷', //÷
'O', //ø
'U', //ù
'U', //ú
'U', //û
'U', //ü
'Y', //ý
'T', //þ
'Y', //ÿ
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
