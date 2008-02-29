/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef WALL_H
#define WALL_H

#include "snakeiteminterface_p.h"
#include <QList>
#include <QPixmap>
class QGraphicsScene;
class Brick;


// There are several types of walls: 1) the corners, which are LU, RU, LD, RD, 2) the border
// walls, which are LRU (top), LUD (left side), RUD (right side), LRD (bottom), 3) the
// non-border walls which are a) U (the highest, going from left to right) and b) D (the
// lowest, going from left to right).

// DON't forget to write dtor!!!!!!!!!!!
// Except that the scene destroys the bricks
class Wall : public SnakeItemInterface
{
public:

    static const int TYPE;

    Wall(QGraphicsScene *,const QPixmap &,uint position);

    int type() const { return TYPE; }

    int getPosition() const { return position; }

    QPoint pos() const;

    QRect sceneBoundingRect() const;

    bool isBorder();

    void setZValue(int zValue);

    // Implemented for SnakeItemInterface
    bool resize(const QRectF &oldSceneGeometry,const QRectF &newSceneGeometry);

private:

    // Percentage of the view width that is used to calculate length of inner walls.
    static const double innerWallPercent;
    // Percentage of the view height that is used to calculate position from centre.
    static const double innerWallPosition;

    void rebuild(int newLengthInBricks);

    QGraphicsScene *scene;

    QList<Brick *> bricks;

    // An or'd representation of SnakeManager::Left, SnakeManager::Right, SnakeManager::Up, SnakeManager::Down.
    uint position;

    QPixmap pixmap;

    // The scene bounding rectangle of the wall.
    QRect bounds;
};

#endif
