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

#include "border.h"
#include "sprites.h"
#include "codes.h"

#include <qtopia/resource.h>
#include <qwmatrix.h>

Border::Border(QCanvas* canvas, borderSide side)
         : QCanvasSprite(0, canvas)
{
    newBorder(side);
} 

void
Border::newBorder(borderSide side)
{
    int		px,py;
    QPixmap	wall(Resource::loadPixmap("snake/wall"));
    QPixmap	borderPix;
    QWMatrix	m;
    px = py = 0;

    //
    // Setup a pixmap that uses the same wall texture as the
    // Obstacle, and use this for the borders of the wall.
    // Requires scaling (and for East / West, rotation) of the
    // wall texture.
    //
    switch (side) {
    case North:
	m.scale(double(canvas()->width()) / double(wall.width()), 1.0);
	borderPix = wall.xForm(m);
	break;

    case West:
	m.scale(1.0, double(canvas()->height()) / double(wall.width()));
	m.rotate(90.0);
	borderPix = wall.xForm(m);
	break;

    case East:
	px = canvas()->width() - 8 - (canvas()->width()-8) % SpriteDB::tileSize();
	m.scale(1.0, double(canvas()->height()) / double(wall.width()));
	m.rotate(90.0);
	borderPix = wall.xForm(m);
	break;

    case South:
	py = canvas()->height() - 8 - (canvas()->height()-8) % SpriteDB::tileSize();
	m.scale(double(canvas()->width()) / double(wall.width()), 1.0);
	borderPix = wall.xForm(m);
	break;

    default:
	return;
    }

    QList<QPixmap> pixl;
    pixl.append(&borderPix);

    QPoint nullp;
    QList<QPoint> pl;
    pl.append(&nullp);
   
    QCanvasPixmapArray* borderarray = new QCanvasPixmapArray(pixl, pl);
    setSequence(borderarray);

    move(px,py);
    setZ( -100 );
    show();
    canvas()->update();
}

int Border::rtti() const
{
   return border_rtti;
}

Border::~Border()
{
}
