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
#include <qcanvas.h>
#include <qtimer.h>

#ifndef SNAKE
#define SNAKE
#include "sprites.h"

class Snake : public QObject
{
	Q_OBJECT

public:

   Snake(QCanvas*);
   ~Snake();
   void pause();
   void go(int newkey);
   void move(Direction dir);
   void changeHead(int last);
   void changeTail();
   void detectCrash();
   void createSnake();
   void extendSnake(); 
   void setScore(int amount);
   int getScore();

signals:
   void dead();
   void targethit();
   void scorechanged();

private slots:
   void moveSnake();
   void increaseSpeed();

private:
   QList<QCanvasSprite>snakelist;
   QTimer* autoMoveTimer; 
   QCanvas* canvas;
   int grow;
   int last;
   int speed;
   int score;
   Direction currentdir;
   Direction newdir;

   bool paused;
   bool deadSnake;
};

#endif
