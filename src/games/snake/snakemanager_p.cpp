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

#include "snakemanager_p.h"
#include "snake_p.h"
#include "mouse_p.h"
#include "wall_p.h"

#include <QGraphicsScene>


// left, right, up, down

// This maps the Direciton enum values to array key indices, 0, 1, 2, 3
// You get a Dirction value, and ask directionKey for a new index. You use
// the new index to map into headKey, bodyKey, tailKey. You use the result
// to index into map, which gives you the Pixmap you want.
static int directionKey[9] = {
    0, // unused
    0, // 1 = Left, maps to 0
    1, // 2 = Right, maps to 1
    0, // 3 unused
    2, // 4 = Up, maps to 2
    0, // 5 unused
    0, // 6 unused
    0, // 7 unused
    3 // 8 = Down, maps to 3
};

//static int headKey[4] = { 0, 1, 3, 2 };
// Image portion for snake's head - headKey accessed by headKey[idx], where
// idx = directionKey[direction]
static int headKey[4] = {
    0,  // Left
    1,  // Right
    3,  // Up
    2   // Down
};

// Image portion for snake's body - bodyKey accessed by bodyKey[oldIdx][newIdx],
// where oldIdx = directionKey[oldDirection], and newIdx = directionKey[newDirection]
static int bodyKey[4][4] = {
    {8, 8, 12, 10}, // oldDirection Left

    {8, 8, 13, 11}, // oldDirection Right

    { 11, 10, 9, 9 }, // oldDirection Up

    {13, 12, 9, 9 } // oldDirection Down
}; // oldDirection Down

// Image portion for snake's tail - tailKey accessed by headKey[idx], where
// idx = directionKey[direction]
static int tailKey[9] = {
    5, // Left
    4, // Right
    6, // up
    7  // Down
};

// Image portion for wall. Index into this directly using combinations of Direction.
// For example, Left | Up gives you 5, and wallKey[5] gives you the image index for
// the top left hand cornerstone.
// left = 1, right = 2, up = 4, down = 8, mask
static int wallKey[16] = {
    27, // 0x00, null
    14, //27, // 0x02, left
    15, //27, // 0x01, right
    27, // 0x03, left, right
    29, // 0x08, up
    20, // 0x0A, up left
    19, // 0x09, up right
    29, // 0x0B, up left right
    29, // 0x04, down
    18, // 0x06, down left
    21, // 0x05, down right
    29, // 0x07, down left right
    27, // 0x0C, down up
    28, // 0x0E, down up left
    28, // 0x0D, down up right
    27  // 0x0F, down up right left
};

static const int wallLeftEndIdx = 14; // Still used??
static const int wallRightEndIdx = 15; // Still used ??

static const int mouseIdx = 30;
static const int groundIdx = 31;


/*!
  \internal
  \class SnakeManager

  \brief SnakeManager stores all pixmaps for the snake application, and provides general
  constants and definitions.
*/


/*!
  \internal
  \fn SnakeManager::SnakeManager(const QImage &image)
  \a image: The collective image containing snake segments, wall segments, the mouse pixmap
  and the grass (background).
*/
SnakeManager::SnakeManager(const QImage &image)
{
    int x = 0;
    int y = 0;

    if ( image.isNull() ) {
        qWarning("SnakeManager ctor: bad master image.");
        return;
    }

    // The image has all the wall stuff over on its right side, and all the other
    // stuff on its left side. The images on the left are divided into 2 columns,
    // so you'll get a column of images at x = 0, and a column of images at x = TILE_SIZE.
    // Likewise, we progress down the columns by TILE_SIZE.

    // The snake is the first set of images on the left.
    int i = 0;
    for (; i < 14; ++i) {
        map[i] = QPixmap::fromImage(image.copy(x,y,TILE_SIZE,TILE_SIZE));
        if (i % 2) {
            // Odd number
            x = 0;
            y += TILE_SIZE;
        } else {
            // even number
            x = TILE_SIZE;
        }
    }

    // walls
    x = 2*TILE_SIZE;
    y = 0;
    for (; i < 30; ++i) {
        map[i] = QPixmap::fromImage(image.copy(x,y,TILE_SIZE,TILE_SIZE));
        if (i % 2) {
            x = 2*TILE_SIZE;
            y += TILE_SIZE;
        } else {
            x = 3*TILE_SIZE;
        }
    }

    // mouse
    map[i++] = QPixmap::fromImage(image.copy(0,TILE_SIZE*7,TILE_SIZE,TILE_SIZE));

    // ground
    map[i++] = QPixmap::fromImage(image.copy(TILE_SIZE,TILE_SIZE*7,TILE_SIZE,TILE_SIZE));
}

/*!
  \internal
  \fn QPixmap SnakeManager::getSnakeImage(SnakePart snakePart,Direction oldDirection,Direction newDirection) const
  Returns a pixmap for a snake segment, or a null pixmap if the requested pixmap could not
  be found.
  \a snakePart: The part of the snake's body that is required, i.e. head, tail, body.
  \a oldDirection: Used when the snake is turning. If the snake is not turning, should be Any.
  \a newDirection: The direction for this snake segment.
*/
QPixmap SnakeManager::getSnakeImage(SnakePart snakePart,
                                     Direction oldDirection,Direction newDirection) const
{
    switch ( snakePart ) {
    case Head:
        return map[headKey[directionKey[newDirection]]];
        break;
    case Body:
        if ( oldDirection == Any ) {
            return map[bodyKey[directionKey[newDirection]][directionKey[newDirection]]];
        }
        return map[bodyKey[directionKey[oldDirection]][directionKey[newDirection]]];
        break;
    case Tail:
        return map[tailKey[directionKey[newDirection]]];
    default:
        qWarning("SnakeManager::getSnakeImage(...), invalid snake part %d", snakePart);
    }

    // Return an empty (null) pixmap.
    return QPixmap();
}

/*!
  \internal
  \fn QPixmap SnakeManager::getGroundImage() const
  Returns a pixmap for the grass (ground). This can be repeated to form the entire background.
*/
QPixmap SnakeManager::getGroundImage() const
{
    return map[groundIdx];
}

/*!
  \internal
  \fn QPixmap SnakeManager::getMouseImage() const
  Returns the pixmap for a mouse.
*/
QPixmap SnakeManager::getMouseImage() const
{
    return map[mouseIdx];
}

/*!
  \internal
  \fn QPixmap SnakeManager::getWallImage(int position) const
  Returns a pixmap for a brick, i.e. part of a wall.
  \a position: This is a set of bits that indicate the type of wall. If the bit positions
  Left and Right are set, the wall is horizontal. If the bit positions Up and Down are set,
  the wall is vertical. An example of a corner brickstone is Left and Up, which is the top
  left-hand corner.
*/
QPixmap SnakeManager::getWallImage(int position) const
{
    return map[wallKey[position]];
}

/*
QPixmap SnakeManager::getWallEnd(bool left) const
{
    return (left ? map[wallLeftEndIdx] : map[wallRightEndIdx]);
}
*/
