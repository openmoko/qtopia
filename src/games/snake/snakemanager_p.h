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

#ifndef SNAKEMANAGER_H
#define SNAKEMANAGER_H

#include <QMap>
#include <QPixmap>


class SnakeManager
{
public:

    enum Direction {Any, Left=1, Right=2, Up=4, Down=8};
    enum SnakePart {Head, Body, Tail};

    static const int TILE_SIZE = 8;

    static const uint BORDER = 0;

    // The following offsets are added to QGraphicsItem::UserType to form
    // the RTTI types for the various objects.
    static const int WALL_TYPE = 1;
    static const int MOUSE_TYPE = 2;
    static const int SNAKE_TYPE = 3;

    SnakeManager(const QImage &);

    QPixmap getGroundImage() const;

    QPixmap getMouseImage() const;

    QPixmap getSnakeImage(SnakePart snakePart,Direction oldDirection,Direction newDirection) const;

    QPixmap getWallImage(int position) const;

//    QPixmap getWallEnd(bool left) const;

private:

    QMap<int,QPixmap> map;
};

#endif
