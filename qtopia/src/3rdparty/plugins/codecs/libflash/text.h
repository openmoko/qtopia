/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998,1999 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
#ifndef _TEXT_H_
#define _TEXT_H_

struct Glyph {
	long	index;
	long	xAdvance;
	long	code;	// Ascii code
};

struct TextRecord {

	// Normal text record
	Glyph		*glyphs;
	long		 nbGlyphs;

	// Control text record
	TextFlags	 flags;
	SwfFont		*font;
	long		 fontHeight;
	Color		 color;
	long		 xOffset;
	long		 yOffset;

	TextRecord *next;

	TextRecord();
	~TextRecord();

	char		*getText();
};

class Text : public Character {

	Rect		 boundary;
	Matrix		 textMatrix;
	TextRecord	*textRecords;	// List

public:
	Text(long id);
	~Text();

	void		 setTextBoundary(Rect rect);
	void		 setTextMatrix(Matrix m);
	void		 addTextRecord(TextRecord *tr);
	int		 execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform);
	void		 getRegion(GraphicDevice *gd, Matrix *matrix, 
                                   void *id, ScanLineFunc scan_line_func);
	int		 doText(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, ShapeAction action,
                                void *id, ScanLineFunc scan_line_func);
	void		 getBoundingBox(Rect *bb, DisplayListEntry *e);
	TextRecord 	*getTextRecords();

#ifdef DUMP
	void	 dump(BitStream *bs);
#endif
};

#endif /* _TEXT_H_ */
