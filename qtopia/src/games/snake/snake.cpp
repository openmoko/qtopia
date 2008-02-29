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

#include "snake.h"
#include "sprites.h"
#include "target.h"
#include "codes.h"

#include <qtopia/resource.h>
#include <qimage.h>

#include <qregexp.h>

Snake::Snake(QCanvas* c)
{
   paused = FALSE;
   deadSnake = FALSE;
   canvas = c; 
   score = 0;
   snakelist.setAutoDelete(true);
   autoMoveTimer = new QTimer(this);
   connect( autoMoveTimer, SIGNAL(timeout()), this, SLOT(moveSnake()) );
   createSnake();
}

void Snake::createSnake()
{
   QCanvasPixmapArray* snakeparts = SpriteDB::spriteCache();
   if (snakeparts == 0) {
       qDebug("Couldn't not find sprite cache");
       return;
   }
 
   grow = 0;
   last = Key_Right;
 
   QCanvasSprite* head = new QCanvasSprite(snakeparts, canvas );
   head->setFrame(SpriteDB::snakeHead(right));
   snakelist.insert(0, head);
   head->show();
   head->move(2*SpriteDB::tileSize(), SpriteDB::tileSize());
 
   QCanvasSprite* body = new QCanvasSprite(snakeparts, canvas );
   body->setFrame(SpriteDB::snakeBody(right, right));
   snakelist.append( body );
   body->show();
   body->move(SpriteDB::tileSize(), SpriteDB::tileSize());
 
   QCanvasSprite* end = new QCanvasSprite(snakeparts, canvas );
   end->setFrame(SpriteDB::snakeTail(right));
   snakelist.append( end );
   end->show();
   end->move(0, SpriteDB::tileSize());
 
   currentdir = right; 
   speed = 250;
   autoMoveTimer->start(speed);
   moveSnake();
}

void Snake::increaseSpeed()
{
   if (speed > 150) 
       speed = speed - 5;
   autoMoveTimer->start(speed);
}

void Snake::go(int newkey)
{
    int direction = newkey;
    switch(newkey) {
	default:
	    return;
#ifdef QTOPIA_PHONE
	case Key_6:
	    direction = Key_Right;
	    // FALL THROUGH
#endif
	case Key_Right:
	    if (currentdir == left) return;
	    break;
#ifdef QTOPIA_PHONE
	case Key_4:
	    direction = Key_Left;
	    // FALL THROUGH
#endif
	case Key_Left:
	    if (currentdir == right) return;
	    break;
#ifdef QTOPIA_PHONE
	case Key_2:
	    direction = Key_Up;
	    // FALL THROUGH
#endif
	case Key_Up:
	    if (currentdir == down) return;
	    break;
#ifdef QTOPIA_PHONE
	case Key_8:
	    direction = Key_Down;
	    // FALL THROUGH
#endif
	case Key_Down:
	    if (currentdir == up) return;
	    break;
#ifdef QTOPIA_PHONE
	case Key_1:
	    // Up or down, depending on direction.
	    if (currentdir == left || currentdir == right)
		direction = Key_Up;
	    else
		direction = Key_Left;
	    break;
	case Key_3:
	    // Up or down, depending on direction.
	    if (currentdir == left || currentdir == right)
		direction = Key_Up;
	    else
		direction = Key_Right;
	    break;
	case Key_7:
	    // Up or down, depending on direction.
	    if (currentdir == left || currentdir == right)
		direction = Key_Down;
	    else
		direction = Key_Left;
	    break;
	case Key_9:
	    // Up or down, depending on direction.
	    if (currentdir == left || currentdir == right)
		direction = Key_Down;
	    else
		direction = Key_Right;
	    break;
#endif
    }
    Snake::changeHead(direction);
    Snake::moveSnake();
}   

void Snake::move(Direction dir)
{
   autoMoveTimer->start(speed);
   int x = 0;
   int y = 0;
   newdir = dir;
   switch (dir) {
       case right: x = SpriteDB::tileSize(); break;
       case left: x = -SpriteDB::tileSize(); break;
       case down: y = SpriteDB::tileSize(); break;
       case up: y = -SpriteDB::tileSize(); break;
   }
   QCanvasSprite* sprite = new QCanvasSprite(SpriteDB::spriteCache(), canvas );
   sprite->setFrame(SpriteDB::snakeBody(currentdir, newdir));
   snakelist.insert(1, sprite);
   sprite->move(snakelist.first()->x(), snakelist.first()->y() );

   snakelist.first()->moveBy(x, y);
   if (grow <= 0)
      changeTail();
   else
      grow--;
   sprite->show();
 
   currentdir = dir;
   canvas->update();
}                

void Snake::changeTail()
{
   snakelist.removeLast();
 
   double lastx = snakelist.last()->x();
   double prevx = snakelist.prev()->x();

   Direction dir;
   if ( prevx == lastx ) {  //vertical
       if ( snakelist.prev()->y() > snakelist.last()->y() )
	   dir = down;
       else
	   dir = up;
   } else {  //horizontal
       if (snakelist.prev()->x() > snakelist.last()->x() )
	   dir = left;
       else
	   dir = right;
   }

     snakelist.last()->setFrame(SpriteDB::snakeTail(dir));
}
 
void Snake::changeHead(int lastkey)
{
   last = lastkey;
 
   switch (last)
   {
       case Key_Up: 
	   snakelist.first()->setFrame(SpriteDB::snakeHead(up));
	   break;
       case Key_Left:
	   snakelist.first()->setFrame(SpriteDB::snakeHead(left));
	   break;
       case Key_Right:
	   snakelist.first()->setFrame(SpriteDB::snakeHead(right));
	   break;
       case Key_Down: 
	   snakelist.first()->setFrame(SpriteDB::snakeHead(down));
	   break;
   }
 
}

void Snake::extendSnake()
{
   grow++;
}         

void Snake::moveSnake()
{
   switch (last)
   {
      case Key_Up: move(up); break;
      case Key_Left: move(left);  break;
      case Key_Right: move(right); break;
      case Key_Down: move(down);  break;
    }
    detectCrash();
}

void Snake::detectCrash()
{
    QCanvasSprite* head = snakelist.first();
    QCanvasItem* item;
    QCanvasItemList l=head->collisions(FALSE);
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
       item = *it;
       // check if snake hit target
       if ( (item->rtti()== 1500 ) && (item->collidesWith(head)) ) {
              Target* target = (Target*) item;
              target->done();
              emit targethit();
              extendSnake();
              setScore(5); 
              return;
       }
       // check if snake hit obstacles
       if ( (item->rtti()==1600) && (item->collidesWith(head)) ) {
	     deadSnake = TRUE;
             emit dead();
             autoMoveTimer->stop();
             return;
       }
       // check if snake hit border
       if ( (item->rtti()==1700) && (item->collidesWith(head)) ) {
	     deadSnake = TRUE;
             emit dead();
             autoMoveTimer->stop();
             return;
       }
    }
    //check if snake hit itself
    for (uint i = 3; i < snakelist.count(); i++) {
       if (head->collidesWith(snakelist.at(i)) ) {
	   deadSnake = TRUE;
            emit dead(); 
            autoMoveTimer->stop();
            return;
       }
   }
} 

void Snake::setScore(int amount)
{ 
   score = score + amount;
   emit scorechanged();
}

int Snake::getScore()
{
   return score;
}

void Snake::pause() {
    if (!deadSnake) {
	if (paused) 
	    autoMoveTimer->start(speed);
	else
	    autoMoveTimer->stop();
	paused = !paused;
    }
}

Snake::~Snake()
{
    autoMoveTimer->stop();
}
