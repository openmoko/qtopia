/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "brick_p.h"
#include "snakemanager_p.h"
#include "wall_p.h"
#include <QPixmap>


/*!
  \internal
  \class Brick

  \brief A Brick is used to construct a wall. It is added to the scene as an individual
  object.
*/


/*!
  \internal
  \fn Brick::Brick(const QPixmap &pixmap,int _itemType)
  \a itemType: Used for RTTI. This will typically be Wall::TYPE.
*/
Brick::Brick(const QPixmap &pixmap,int _itemType)
    : QGraphicsPixmapItem(pixmap)
    , itemType(_itemType)
    , bounds(-0.5,-0.5,pixmap.width()-1,pixmap.height()-1)
{
}

/*!
  \internal
  \fn int Brick::type() const
  Used to distinguish a Brick from other QGraphicItem objects at run-time. Typically, this
  will be Wall::TYPE, and is supplied by the ctor.
*/
int Brick::type() const
{
    return itemType;
}

/*!
  \internal
  \fn QRectF Brick::boundingRect() const
  ***HACK HACK HACK***
 Had to override this, otherwise collision detection doesn't work. Sets the bounding rect
 as 1 pixel less wide and less high than the pixmap width/height.
*/
QRectF Brick::boundingRect() const
{
    return bounds;
}
