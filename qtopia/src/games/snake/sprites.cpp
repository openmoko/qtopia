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

#include "sprites.h"
#include <qcanvas.h>
#include <qimage.h>
#include <qtopia/resource.h>



// Might want to make this a menu option,
// e.g, pick your snake size, (label with images of snakes head + tail)
// can smoothscale 3/4 very nicely with medium snake, giving sizes of
// 8, 12, 16
// might be possible to do the smallsnake down to 4,  (increments of 4 make more sense)
// Warning though, the code is not currently set up for this to be a dynamic change.
#ifdef QTOPIA_PHONE
int SpriteDB::tilesize = 8;
const QCString tilename = "snake/smallsnake";
#else
int SpriteDB::tilesize = 16;
const QCString tilename = "snake/mediumsnake";
#endif

QCanvasPixmapArray *SpriteDB::internalcache = 0;
// left, right, up, down
static int headKey[4] = { 0, 1, 3, 2 };
static int bodyKey[4][4] = { {8, 8, 12, 10}, {8, 8, 13, 11}, { 11, 10, 9, 9 }, {13, 12, 9, 9 } };
static int tailKey[4] = { 4, 5, 6, 7 };

// left = 1, right = 2, up = 4, down = 8, mask
static int wallKey[16] = {
    27, // 0x00, null
    15, // 0x01, left
    14, // 0x02, right
    29, // 0x03, left, right
    17, // 0x04, up
    21, // 0x05, up left
    18, // 0x06, up right
    25, // 0x07, up left right
    16, // 0x08, down
    19, // 0x09, down left
    20, // 0x0A, down right
    24, // 0x0B, down left right
    28, // 0x0C, down up
    23, // 0x0D, down up left
    22, // 0x0E, down up right
    26  // 0x0F, down up right left
};


QCanvasPixmapArray *SpriteDB::spriteCache()
{
    if (!internalcache) {

    //QList<QPixmap> pixl;
    //pixl.append(&borderPix);
//
    //QPoint nullp;
    //QList<QPoint> pl;
    //pl.append(&nullp);
   //
    //QCanvasPixmapArray* borderarray = new QCanvasPixmapArray(pixl, pl);
	QCanvasPixmapArray *parts = new QCanvasPixmapArray();
	QImage image = Resource::findPixmap(tilename);
	if (!image.valid(tilesize*2-1,tilesize*7-1)) {
	    qWarning("couldn't find image");
	    return 0;
	}
	int x = 0;
	int y = 0;
	
	//snake..
	int i = 0;
	for (; i < 14; ++i) {
	    parts->setImage( i, new QCanvasPixmap(image.copy(x, y, tilesize, tilesize)) );
	    if (i % 2) {
		x = 0;
		y += tilesize;
	    } else {
		x = tilesize;
	    }
	}
	x = 2*tilesize;
	y = 0;
	for (; i < 30; ++i) {
	    parts->setImage( i, new QCanvasPixmap(image.copy(x, y, tilesize, tilesize)) );
	    if (i % 2) {
		x = 2*tilesize;
		y += tilesize;
	    } else {
		x = 3*tilesize;
	    }
	}
	parts->setImage(i++, new QCanvasPixmap(image.copy(0, tilesize*7, tilesize, tilesize)));
	parts->setImage(i++, new QCanvasPixmap(image.copy(tilesize, tilesize*7, tilesize, tilesize)));
	internalcache = parts;
    }
    return internalcache;
}

int SpriteDB::snakeHead(Direction dir)
{
    return headKey[dir];
}

int SpriteDB::snakeBody(Direction olddir, Direction newdir)
{
    return bodyKey[olddir][newdir];
}

int SpriteDB::snakeTail(Direction dir)
{
    return tailKey[dir];
}


int SpriteDB::mouse()
{
    return 30;
}

int SpriteDB::ground()
{
    return 31;
}


int SpriteDB::wall(bool l, bool r, bool u, bool d)
{
    int index = 0;
    if (l)
	index |= 0x01;
    if (r)
	index |= 0x02;
    if (u)
	index |= 0x04;
    if (d)
	index |= 0x08;

    int res = wallKey[index];
    if (res > 30 || res < 14) qWarning("index was %d", index);
    return (res > 30 || res < 14) ? 29 : res;
}

