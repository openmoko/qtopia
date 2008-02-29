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


#ifndef QFONTDATA_P_H
#define QFONTDATA_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qmenudata.cpp, qmenubar.cpp, qmenubar.cpp, qpopupmenu.cpp,
// qmotifstyle.cpp and qwindowssstyle.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#endif // QT_H

struct QFontDef {				// font definition
    QString	family;
    QString	addStyle;
    int		pointSize;
    uint	styleHint	: 8;
    uint	styleStrategy	: 8;
    uint	charSet		: 8;
    uint	weight		: 8;
    uint	italic		: 1;
    uint	underline	: 1;
    uint	strikeOut	: 1;
    uint	fixedPitch	: 1;
    uint	hintSetByUser	: 1;
    uint	rawMode		: 1;
    uint	dirty		: 1;
    short	lbearing;
    short	rbearing;
};


class QFontInternal;
class QTextCodec;

struct QFontData : public QShared {
    QFontData()
	: exactMatch(FALSE), fin(0), printerHackFont( 0 )
	{}
    QFontData( const QFontData &d )
	: QShared(d), req(d.req), exactMatch(d.exactMatch), fin(d.fin),
	  printerHackFont(0)
	// Copy the QShared count as well. The count may need to be
	// reset when using the QFontData class, see QFont::QFont(QFontData*)
	{}
   ~QFontData()
	{}
    QFontData &operator=( const QFontData &d )
	{
	    req = d.req;
	    exactMatch = d.exactMatch;
	    fin = d.fin;
	    printerHackFont=d.printerHackFont;
	    return *this;
	}
    QFontDef	      req;			// requested font
    bool	      exactMatch;
    QFontInternal    *fin;
    QFont            *printerHackFont;
    const QTextCodec *mapper()  const;
    void	     *fontSet() const;
};


#endif // QFONTDATA_P_H
