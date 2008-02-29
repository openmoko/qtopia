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

QString escapeMultiLineString( const QString& plain )
{
    bool leadingWhitespace = TRUE;
    int col = 0;
    QString rich;
    rich += "<p>";
    for ( int i = 0; i < int(plain.length()); ++i ) {
	if ( !plain[i].isSpace() )
	    leadingWhitespace = FALSE;

        if ( plain[i] == '\n' ) {
	    rich += "<br>";
	    leadingWhitespace = TRUE;
	    col = 0;
	} else if ( plain[i] == '\t' ) {
            rich += 0x00a0U;
            while ( col % 4 ) {
                rich += 0x00a0U;
                ++col;
            }
        } else if ( plain[i].isSpace() ) {
	    if ( leadingWhitespace )
		rich += 0x00a0U;
	    else
		rich += " ";
	} else if ( plain[i] == '<' )
            rich += "&lt;";
        else if ( plain[i] == '>' )
            rich += "&gt;";
        else if ( plain[i] == '&' )
            rich += "&amp;";
	else if ( plain[i] == '"' )
	    rich += "&quot;";
        else
            rich += plain[i];
        ++col;
    }
    rich += "</p>";
    return rich;
}

QString dehyphenate(const QString& s)
{
    QChar shy(0x00ad);
    int i=0;
    while (i<(int)s.length() && s[i]!=shy)
	i++;
    if ( i==(int)s.length() )
	return s;
    QString r = s.left(i);
    for (; i<(int)s.length(); ++i) {
	if ( s[i]!=shy )
	    r += s[i];
    }
    return r;
}

} // namespace QPC
