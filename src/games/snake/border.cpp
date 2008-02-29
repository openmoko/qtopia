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

#include "border.h"
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
	px = canvas()->width() - 8 - (canvas()->width()-8) % 16;
	m.scale(1.0, double(canvas()->height()) / double(wall.width()));
	m.rotate(90.0);
	borderPix = wall.xForm(m);
	break;

    case South:
	py = canvas()->height() - 8 - (canvas()->height()-8) % 16;
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
