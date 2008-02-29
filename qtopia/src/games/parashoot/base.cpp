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
#include "codes.h"  
#include "base.h"
#include "man.h"

#include <qtopia/resource.h>

#include <qregexp.h>

int damage;

Base::Base(QCanvas* canvas) :
    QCanvasSprite(0, canvas),
    kaboom("landmine"),
    ohdear("crmble01")
{
   basearray = new QCanvasPixmapArray();
   QString b0 = Resource::findPixmap("parashoot/b0001");
   b0.replace(QRegExp("0001"),"%1");
   basearray->readPixmaps(b0, 4);
   setSequence(basearray);
   setFrame(0);
   move(2, canvas->height()-50);
   setZ(10);
   show();
   damage = 0;
}

void Base::damageBase()
{
   damage++;

   switch(damage) {
      case 1: setFrame(1); ohdear.play(); break;
      case 2: setFrame(2); ohdear.play(); break;
      case 3: setFrame(3); kaboom.play(); break;
   }
   show(); 
}

bool Base::baseDestroyed()
{
   return (damage >= 3);
}

Base::~Base()
{
}

int Base::rtti() const
{
   return base_rtti;
}

void
Base::reposition(void)
{
    move(2, canvas()->height()-50);
}
