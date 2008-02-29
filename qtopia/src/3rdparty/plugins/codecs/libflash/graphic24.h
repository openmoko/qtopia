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
////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
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
//  Author : Olivier Debon  <odebon@club-internet.fr>
//  

#include "swf.h"

class GraphicDevice24: public GraphicDevice {
private:
	long GraphicDevice24::allocColor(Color color);

public:
	GraphicDevice24(FlashDisplay *fd);

	void clearCanvas();
	void fillLineAA(FillStyleDef *f, long y, long start, long end);
	void fillLine(FillStyleDef *f, long y, long start, long end);
	void fillLineBitmap(FillStyleDef *f, long y, long start, long end);
	void fillLineLG(Gradient *grad, long y, long start, long end);
	void fillLineRG(Gradient *grad, long y, long start, long end);
	void drawLine(long x1, long y1, long x2, long y2, long width);
};
