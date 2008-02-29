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

#include "target.h"
#include "codes.h"
#include "sprites.h"

#include <qtopia/resource.h>

#include <stdlib.h>

Target::Target(QCanvas* canvas) 
       : QCanvasSprite(0, canvas)
{
   setSequence(SpriteDB::spriteCache());
   setFrame(SpriteDB::mouse());
   newTarget();
}

void Target::newTarget()
{
   static bool first_time = TRUE;
   if (first_time) {
         first_time = FALSE;
         QTime midnight(0, 0, 0);
         srand(midnight.secsTo(QTime::currentTime()) );
    }
}

bool Target::onTop()
{
   QCanvasItem* item;
   QCanvasItemList l= canvas()->allItems(); //collisions(FALSE);
   for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
          item = *it; 
          if (item != this && item->collidesWith(this)) return true;
      }
   return false;
}

void Target::done()
{
   delete this;
}

int Target::rtti() const
{
   return target_rtti;
}

Target::~Target()
{
}

//
// Try and position the target.
//
bool
Target::position(void)
{
    int max_position_tries = 100;
    int tilesize = SpriteDB::tileSize();

    // should be somewhere else.
    int screenwidth = canvas()->width() / tilesize;
    if ((canvas()->width() % tilesize) > (tilesize >>1))
	screenwidth++;
    int screenheight = canvas()->height() / tilesize;
    if ((canvas()->height() % tilesize) > (tilesize >>1))
	screenheight++;


    // we don't get to consider borders.
    screenwidth -= 2;
    screenheight -= 2;
    do {
	int x = rand() % screenwidth;
	x = tilesize + x*tilesize;
	int y = rand() % screenheight;
	y = tilesize + y*tilesize;
	move(x, y);
    } while (onTop() && --max_position_tries);

    return max_position_tries > 0;
}
