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

#include "wall_p.h"
#include "snakemanager_p.h"
#include "brick_p.h"
#include <QGraphicsScene>


const int Wall::TYPE = QGraphicsItem::UserType + SnakeManager::WALL_TYPE;
const double Wall::innerWallPercent = 0.4;
const double Wall::innerWallPosition = 0.27;


Wall::Wall(QGraphicsScene *_scene,const QPixmap &_pixmap,uint _position)
    : scene(_scene)
    , position(_position)
    , pixmap(_pixmap)
{
}

QPoint Wall::pos() const
{
    if ( bricks.count() == 0 ) {
        return QPoint(-1,-1);
    }

    return bricks.first()->pos().toPoint();
}

QRect Wall::sceneBoundingRect() const
{
    return bounds;
}

bool Wall::isBorder()
{
    return position == SnakeManager::Up || position == SnakeManager::Down;
}

void Wall::setZValue(int zValue)
{
    for ( QList<Brick *>::iterator control = bricks.begin(); control != bricks.end(); control++ ) {
        (*control)->setZValue(zValue);
    }
}

void Wall::rebuild(int newLengthInBricks)
{
    if ( !scene ) {
        qWarning("Wall::rebuild - cannot rebuild wall since there is no scene");
        return;
    }

    int oldLengthInBricks = bricks.size();
    if ( newLengthInBricks > oldLengthInBricks ) {
        // Need to add more bricks.
        for ( int i = oldLengthInBricks; i < newLengthInBricks; i++ ) {
            Brick *newItem = new Brick(pixmap,TYPE);
            bricks.append(newItem);
            scene->addItem(newItem);
        }
    } else if ( newLengthInBricks < oldLengthInBricks ) {
        // Need to trash some of the bricks.
        for ( int i = newLengthInBricks; i < oldLengthInBricks; i++ ) {
            Brick *oldItem = bricks.takeLast();
            if ( oldItem ) {
                scene->removeItem(oldItem);
                delete oldItem; oldItem = 0;
            } else {
                qWarning("Wall::rebuild - trying to remove an item that does not exist!!");
            }
        }
    }
}

// Implemented for SnakeItemInterface
bool Wall::resize(const QRectF &,const QRectF &newSceneGeometry)
{
    if ( pixmap.isNull() ) {
        qWarning("Cannot recalculate for wall at position %d, since pixmap is null.",position);
        return false;
    }

    QPoint oldPos = pos();
    int x = -1, y = -1;

    int oldLengthInBricks = bricks.size();
    int newLengthInBricks = 1; // we'll have at least 1 brick
    // This tells us if any bricks we have to add are going from left-to-right (xIncreasing)
    // or top-to-bottom (nQPainterPath Wall::nshape() constot xIncreasing).
    bool xIncreasing = false;

    if ( position & SnakeManager::Right && position & SnakeManager::Left ) {
        // Wall going from left to right.
        x = SnakeManager::BORDER + pixmap.width();
        // Length of wall should be length of scene, left to right, less amount for two
        // borders, less amount for two corners.
        double wallLengthInPixels = newSceneGeometry.width() - (SnakeManager::BORDER*2) - (pixmap.width()*2);
        newLengthInBricks = qRound(wallLengthInPixels/pixmap.width());
        // Adjust for remaining gap at the end.
        if ( wallLengthInPixels - (newLengthInBricks * pixmap.width()) > 1 ) {
            newLengthInBricks++;
        }

        xIncreasing = true;
    } else if ( position & SnakeManager::Right ) {
        x = static_cast<int>(newSceneGeometry.width()) - 1 - pixmap.width() - SnakeManager::BORDER;
    } else if ( position & SnakeManager::Left ) {
        x = SnakeManager::BORDER;
    }

    if ( position & SnakeManager::Down && position & SnakeManager::Up ) {
        // Wall going up-down.
        y = SnakeManager::BORDER + pixmap.width();
        // Length of wall should be length of scene, up to down, less amount for two
        // borders, less amount for two corners.
        qreal wallLengthInPixels = newSceneGeometry.height() - (SnakeManager::BORDER*2) - (pixmap.height()*2);
        newLengthInBricks = qRound(wallLengthInPixels/pixmap.height());
        // Adjust for remaining gap at the end.
        if ( wallLengthInPixels - (newLengthInBricks * pixmap.height()) > 1 ) {
            newLengthInBricks++;
        }
    } else if ( position & SnakeManager::Down ) {
        y = static_cast<int>(newSceneGeometry.height())-1 - pixmap.height() - SnakeManager::BORDER;
    } else if ( position & SnakeManager::Up ) {
        y = SnakeManager::BORDER;
    }

    // Handle special cases, i.e. of walls that don't exist on the border.
    if ( x == -1 ) {
        xIncreasing = true;
        qreal length = newSceneGeometry.width() * innerWallPercent;
        newLengthInBricks = qRound(length/pixmap.height());
        x = static_cast<int>((newSceneGeometry.width() - length)/2);
        qreal halfHeight = newSceneGeometry.height()/2;
        if ( position & SnakeManager::Up ) {
            y = static_cast<int>(halfHeight * (1-innerWallPosition));
        } else if ( position & SnakeManager::Down ) {
            y = static_cast<int>(halfHeight + (halfHeight * innerWallPosition));
        }
    }

    if ( (x != oldPos.x()) || (y != oldPos.y())
         || (newLengthInBricks != oldLengthInBricks) ) {
        // The wall position or dimensions needs to change.
        if ( xIncreasing ) {
            // Horizontal wall.
            bounds = QRect(x,y,newLengthInBricks * pixmap.width(),pixmap.height());
        } else {
            // Vertical wall.
            bounds = QRect(x,y,pixmap.width(),newLengthInBricks * pixmap.height());
        }

        // Add or remove bricks.
        rebuild(newLengthInBricks);

        // And modify position for all bricks.
        for ( QList<Brick *>::iterator control = bricks.begin(); control != bricks.end(); control++ ) {
            QGraphicsItem *item = *control;
            item->setPos(x,y);
            if ( xIncreasing ) {
                // Going across.
                x += pixmap.width();
            } else {
                // Going down.
                y += pixmap.height();
            }
        }
    }

    return true;
}












