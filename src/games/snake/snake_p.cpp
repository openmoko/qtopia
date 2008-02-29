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

#include "snake_p.h"
#include "wall_p.h"
#include "mouse_p.h"
#include "snakeitem_p.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QList>
#include <QTimer>
#include <QRegion>


const int Snake::TYPE = QGraphicsItem::UserType + SnakeManager::SNAKE_TYPE;


/*!
  \internal
  \class Snake

  \brief Snake manages a series of SnakeItem segments: A head, a tail, and one or more
  body segments.

  \mainclass
  The snake always knows its own direction (i.e. the head's direction). It can be
  started via goGirl(), turned or paused. The snake itself is aware of when it collides
  with foreign objects, such as a wall (causing the snake's death and the signal snakeDead()
  to be emitted), or a mouse (causing the snake to grow and to emit mouseEating(Mouse *)).

  \sa SnakeItem
*/


/*!
  \internal
  \fn Snake::Snake(QGraphicsScene *_scene,SnakeManager *_snakeManager)
*/
Snake::Snake(QGraphicsScene *_scene,SnakeManager *_snakeManager)
    : connector(0)
    , snakeManager(_snakeManager)
    , scene(_scene)
    , headItem(0)
    , tailItem(0)
    , newBodyItem(0)
    , bodyCount(0)
    , newDirection(SnakeManager::Any)
    , timer(0)
    , speed(START_SPEED)
    , amountToMove(SnakeManager::TILE_SIZE)
{
    connector = new SnakeConnector(this);

    headItem = new SnakeItem(SnakeManager::Head,snakeManager,0);
    // The snake always starts out with a single body portion.
    SnakeItem *bodyItem = new SnakeItem(SnakeManager::Body,snakeManager,headItem);
    //bodyItems.append(bodyItem);
    tailItem = new SnakeItem(SnakeManager::Tail,snakeManager,bodyItem);

    if ( scene ) {
        scene->addItem(headItem);
        scene->addItem(bodyItem);
        bodyCount++;
        scene->addItem(tailItem);
    }

    // Timer for moving the snake.
    timer = new QTimer(connector);
    QObject::connect(timer, SIGNAL(timeout()), connector, SLOT(moveSnake()));

    // Start off with the snake at 'no' position - resize(...) sorts this out.
    headItem->setPos(QPointF(-1,-1));

    // Before the snake is displayed, resize(...) will be called. This will cause
    // tailItem->reset(...) to be called, which will adjust the directions and pixmaps
    // for all the snake segments.
}

/*!
  \internal
  \fn Snake::~Snake()
*/
Snake::~Snake()
{
    if ( timer ) {
        delete timer;
    }

    delete connector;

    if ( newBodyItem ) {
        delete newBodyItem;
    }
}

/*!
  \internal
  \fn SnakeManager::Direction Snake::getDirection() const
  Returns the current direction of the head.
*/
SnakeManager::Direction Snake::getDirection() const
{
    return headItem->getDirection();
}

/*!
  \internal
  \fn QPointF Snake::pos() const
  Returns the current scene position of the head.
*/
QPointF Snake::pos() const
{
    return headItem->pos();
}

/*!
  \internal
  \fn void Snake::setZValue(int zValue)
  Sets the stacking value for all the snake segments.
*/
void Snake::setZValue(int zValue)
{
    tailItem->setZValue(zValue);

    SnakeItem *prev = tailItem->getPrevious();
    while ( prev && (prev->getBodyPart() != SnakeManager::Head) ) {
        prev->setZValue(zValue);
        prev = prev->getPrevious();
    }

    headItem->setZValue(zValue + 1);
}

/*!
  \internal
  \fn QRegion Snake::getRegion() const
  Returns the area occupied by the snake.
*/
QRegion Snake::getRegion() const
{
    // Start with the tail region, then iterate upwards through all the snake segments.
    QRect rectangle = tailItem->sceneBoundingRect().toRect();
    rectangle.setWidth(rectangle.width()+1);

    QRegion region(rectangle);

    SnakeItem *prev = tailItem->getPrevious();
    while ( prev ) {
        rectangle = prev->sceneBoundingRect().toRect();
        rectangle.setWidth(rectangle.width()+1);
        region |= QRegion(rectangle);
        prev = prev->getPrevious();
    }

    return region;
}

/*!
  \internal
  \fn void Snake::increaseSpeed()
  Increases the snake's speed by SPEED_INCREMENT, until it reaches FASTEST_SPEED.
*/
void Snake::increaseSpeed()
{
    if ( speed > FASTEST_SPEED ) {
        speed -= SPEED_INCREMENT;
    }

    if ( timer->isActive() ) {
        timer->start(speed);
    }
}

/*!
  \internal
  \fn void Snake::goGirl()
  Causes the snake to start moving.
*/
void Snake::goGirl()
{
    timer->start(speed);
}

/*!
  \internal
  \fn void Snake::reset(bool resetSpeed)
  Removes all excess body portions so that the snake is shrunk down to its smallest size,
  resets the speed if necessary, and causes the snake to be positioned in the top left-hand
  corner, facing right.
  \a resetSpeed: If true, sets the snake's speed back to START_SPEED.
*/
void Snake::reset(bool resetSpeed)
{
    // Remove all but one of the body items.
    shrink();

    // Get rid of any pending body item.
    if ( newBodyItem ) {
        delete newBodyItem;
        newBodyItem = 0;
    }

    if ( resetSpeed ) {
        speed = START_SPEED;
    }

    // position snake just inside the top-left wall, pointing right
    tailItem->reset(QPointF(SnakeManager::BORDER + SnakeManager::TILE_SIZE,
                            SnakeManager::BORDER + SnakeManager::TILE_SIZE),
                    SnakeManager::Right);
}

/*!
  \internal
  \fn void Snake::pause()
  Causes the snake to stop moving.
*/
void Snake::pause()
{
    timer->stop();
}

// Causes the snake to stop moving, and emits the signal snakeDead().
void Snake::dieSnakeDie()
{
    timer->stop();
    emit snakeDead();
}

/*!
  \internal
  \fn void Snake::move()
  Moves the snake forwards by one unit. If there has been a change of direction scheduled, or if
  there has been growth scheduled (i.e. a mouse has been eaten), this is handled in this function.
  In addition to moving, the snake checks for collisions with foreign objects. This may result in
  the snake's death (the snake has collided with itself or with a wall) or in a mouse being eaten.
  May emit signals snakeDead() or mouseEaten().
*/
void Snake::move()
{
    // To do any kind of move, we just move each item into its previous item's position.
    // Then we move the head, depending on the direction or new direction.
    // The tail might stay where it is, if we have a new body item to slot in (i.e. the
    // snake grew - see grow()).
    if ( newBodyItem ) {
        // Insert the new body item at the end and just before the tail.
        newBodyItem->setPrevious(tailItem->getPrevious());
        tailItem->setPrevious(newBodyItem);
        // Set its position to the tail position, so that when we move them all forward it
        // will move into the position of the previous body item, while the tail item
        // will not move.
        newBodyItem->setPos(tailItem->pos());

        if ( scene ) {
            scene->addItem(newBodyItem);
            bodyCount++;
        }
        newBodyItem = 0;
    }

    if ( newDirection != SnakeManager::Any ) {
        // A change of direction has been scheduled.
        headItem->setDirection(newDirection);
        newDirection = SnakeManager::Any;
    }

    // Ask the tail item to iteratively move forwards. This has a ripple effect: the
    // tail item moves into the position of its previous item. Its previous item also
    // moves into the next position, and so on. All directions and pixmaps are also adjusted
    // except for the final segment, the head. This also causes a redraw of each segment (except
    // the head).
    tailItem->moveForwards();

    // Finally, move the head forwards (causes a redraw).
    headItem->move(amountToMove);

    // Check for anything colliding with the snake's head. If there is a collision, either
    // the snake will die or the snake will get to eat a mouse.
    collisionsCheck();
}

// Checks to see if the snake has crashed into anything, and deals with the consequences.
// If the snake has collided with a wall or with itself, it must die. If a snake collides with
// a mouse, it gets to eat the mouse and grow.
//  May emit signals snakeDead() or mouseEaten().
void Snake::collisionsCheck()
{
    // Find every graphical item that's in the vicinity of the snake's head.
    QRectF bounds = headItem->sceneBoundingRect();

    QList<QGraphicsItem *> collisionItems = headItem->collidingItems();
    for ( QList<QGraphicsItem *>::const_iterator control = collisionItems.begin(); control != collisionItems.end(); control++ ) {
        QGraphicsItem *item = *control;
        if ( item->type() == TYPE ) {
            // ***HACK HACK HACK***
            // ...to get collision detection to work. For a start, we had to override
            // boundingRect() in SnakeItem. But even so, we're getting bogus items returned.
            if ( bounds.intersects(item->sceneBoundingRect()) ) {
                // This is a true collision.
                dieSnakeDie();
                break;
            }
        }

        if ( item->type() == Wall::TYPE ) {
            // The snake has collided with a wall - this is fatal.
            dieSnakeDie();
            break;
        }

        if ( item->type() == Mouse::TYPE ) {
            //Mouse *mouse = qgraphicsitem_cast<Mouse *>(item);
            Mouse *mouse = static_cast<Mouse *>(item);
            if ( mouse ) {
                grow();
                emit mouseEaten(mouse);
                break;
            } else {
                qWarning("Snake::collisionsCheck() - coudl not cast Mouse!!!");
            }
        }
    }
}

/*!
  \internal
  \fn void Snake::grow()
  Typically called in response to a mouse being eaten. This function schedules the growth of
  the snake by a new segment - the new segment will be attached and drawn during the next call
  to \l{move()}.
*/
void Snake::grow()
{
    // We SCHEDULE a new body portion for growth, then slot it in on the next move.
    // Therefore, don't add it to the scene or link it in to tailItem, as yet.
    newBodyItem = new SnakeItem(SnakeManager::Body,snakeManager,0);
    newBodyItem->setZValue(tailItem->zValue());
}

/*!
  \internal
  \fn void Snake::shrink()
  Gets rid of all the body items except the original one, so that the snake is down to a head,
  a tail and a single body item.
*/
void Snake::shrink()
{
    // Iterate thru all the body segments, starting with the one just above the tail.
    SnakeItem *prev = tailItem->getPrevious();

    // Get rid of all the body segments except the one just before the head.
    while ( prev
            && prev->getPrevious()
            && (prev->getPrevious()->getBodyPart() == SnakeManager::Body) ) {
        SnakeItem *next = prev->getPrevious();
        scene->removeItem(prev);
        bodyCount--;
        delete prev;
        prev = next;
    }

    if ( !prev ) {
        qWarning("Snake::shrink(): No body part for snake tail to point at!!");
    } else {
        tailItem->setPrevious(prev);
    }
}

/*!
  \internal
  \fn int Snake::getPendingSize()
  Returns the length of the snake, in pixels, which includes any pending growth.
*/
int Snake::getPendingSize() const
{
    int numItems = (newBodyItem ? 1 : 0);
    // Allow for body, tail, and all body items.
    numItems += (2 + bodyCount);
    return numItems * SnakeManager::TILE_SIZE;
}

/*!
  \internal
  \fn void Snake::turn(SnakeManager::Direction _newDirection)
  Schedules a turn, which will be honoured in the next call to \l{move()}. Disallows
  the snake jumping back on itself, i.e. turning from Left to Right or v.v., or turning
  from Up to Down or v.v.
*/
void Snake::turn(SnakeManager::Direction _newDirection)
{
    SnakeManager::Direction direction = headItem->getDirection();

    if ( ((direction == SnakeManager::Left) && (_newDirection == SnakeManager::Right))
         || ((direction == SnakeManager::Right) && (_newDirection == SnakeManager::Left))
         || ((direction == SnakeManager::Up) && (_newDirection == SnakeManager::Down))
         || ((direction == SnakeManager::Down) && (_newDirection == SnakeManager::Up)) ) {
        // Ignore this one - the snake cannot turn back on itself.
        return;
    }

    // Schedule a change of direction. We cannot update the head's pixmap etc since a
    // change of pixmap will be visible immediately.
    newDirection = _newDirection;
}

/*!
  \internal
  \fn bool Snake::resize(const QRectF &,const QRectF &)
  Implemented for SnakeItemInterface. May call reset().
*/
bool Snake::resize(const QRectF &,const QRectF &)
{
    if ( headItem->pos() == QPointF(-1,-1) ) {
        reset();
        return true;
    }
    return false;
}

SnakeConnector::SnakeConnector(Snake *_snake)
    : snake(_snake)
{
}

void SnakeConnector::moveSnake()
{
    snake->move();
}
