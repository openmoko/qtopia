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

#include "helicopter.h"
#include "man.h"
#include "codes.h"

#include <qtopia/resource.h>

#include <qregexp.h>

static QList<Helicopter> all;

Helicopter::Helicopter(QCanvas* canvas) :
    QCanvasSprite(0, canvas),
    chikachika("aland01")
{
    all.append(this);
    hits = 0;
    QCanvasPixmapArray* helicopterarray = new QCanvasPixmapArray();
    QString h0 = Resource::findPixmap("parashoot/helicopter0001");
    h0.replace(QRegExp("0001"),"%1");
    helicopterarray->readPixmaps(h0,4 );

    setSequence(helicopterarray);
    setAnimated(true);
    move(canvas->width(), 5);
    setVelocity(-2, 0);
    chikachika.playLoop();
    show();
}

Helicopter::~Helicopter()
{
    all.remove(this);
}

int fr = 0;

void Helicopter::advance(int phase)
{
   QCanvasSprite::advance(phase);
   if (phase == 0) {
	if (frame() == 3) {
	    delete this;
	    return;
	}

	if (hits >= 2) {
	    setFrame(3);
	} else {
	    setFrame(fr%3);
	    fr++;
	    checkCollision();
	}
   }
}

void Helicopter::checkCollision()
{
    if (x() == 6) {
        setAnimated(false); //setVelocity(0, 0);
        dropman();
    }
    if (x() < 0)
	done();
}

void Helicopter::dropman()
{
    (void)new Man(canvas(), 15, 25); 
    (void)new Man(canvas(), 35, 25);
    takeOff();
}

void Helicopter::done()
{
    hits++;
}

void Helicopter::takeOff()
{
    setVelocity(-1, 0);
}

int Helicopter::rtti() const
{
    return helicopter_rtti;
}

void Helicopter::silenceAll()
{
    for (Helicopter* h = all.first(); h; h = all.next())
	h->chikachika.stop();
}

void Helicopter::deleteAll()
{
    Helicopter* h;
    while ((h = all.first()))
	delete h;
}

