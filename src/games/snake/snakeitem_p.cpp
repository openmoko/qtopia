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

#include "snakeitem_p.h"
#include "snake_p.h"


/*!
  \internal
  \class SnakeItem

  \brief A SnakeItem is a single segment of a Snake, either its head, its tail, or one of
  its body segments.

  \mainclass SnakeItem is capable of having an effect on previous segments in the snake (going
  from the tail up to the head), since the SnakeItem's act as a linked list. For example, the
  tail points at the final body segment, and the first body segment (the neck) points at the
  head.

  SnakeItem does not know how to grow or shrink, but does know about inserted items.
*/


/*!
  \internal
  \fn SnakeItem::SnakeItem(SnakeManager::SnakePart bodyPart,SnakeManager *snakeManager,SnakeItem *previousItem)
  \a bodyPart: Indicates whether the segment is a head, tail or body.
  \a snakeManager: Used to retrieve pixmaps.
  \a previousItem: Used to form the linked list. Can be 0.
*/
SnakeItem::SnakeItem(SnakeManager::SnakePart _bodyPart,SnakeManager *_snakeManager,SnakeItem *_previousItem)
    : bodyPart(_bodyPart)
    , snakeManager(_snakeManager)
    , previous(_previousItem)
    , direction(SnakeManager::Any)
    , turning(false)
{
//    setShapeMode(BoundingRectShape);
}

/*!
  \internal
  \fn int SnakeItem::type() const
  Returns Snake::TYPE, for RTTI information.
*/
int SnakeItem::type() const
{
    return Snake::TYPE;
}

/*!
  \internal
  \fn SnakeManager::SnakePart getBodyPart() const
  Returns head, tail or body.
*/

/*!
  \internal
  \fn SnakeManager::Direction getDirection() const
  Returns the current direction of this body part.
*/

/*!
  \internal
  \fn void SnakeItem::setDirection(SnakeManager::Direction newDirection)
  Modifies this segment's direction and its pixmap. Is non-iterative, i.e. does not affect
  other segments.
*/
void SnakeItem::setDirection(SnakeManager::Direction newDirection)
{
    if ( direction != newDirection ) {
        direction = newDirection;
        setPixmap(snakeManager->getSnakeImage(bodyPart,SnakeManager::Any,direction));
    }
}

/*!
  \internal
  \fn SnakeItem *getPrevious() const
  Returns the previous body segment, or 0 if this segment is for the head. The direction of
  the linked list is tail to head.
*/

/*!
  \internal
  \fn void SnakeItem::setPrevious(SnakeItem *previousItem)
  Resets this item in the linked list.
*/
void SnakeItem::setPrevious(SnakeItem *_previousItem)
{
    previous = _previousItem;
}

/*!
  \internal
  \fn void SnakeItem::reset(QPointF position,SnakeManager::Direction direction)
  An iterative function, i.e. has an effect on other segments.
  Straightens the snake from this link forwards, positioning it at \a position,
  and pointing in \a direction.
*/
void SnakeItem::reset(QPointF position,SnakeManager::Direction direction)
{
    // Update this item - its position, direction and pixmap.
    setPos(position);
    setDirection(direction);

    SnakeItem *prev = previous;
    SnakeItem *current = this;

    qreal x = pos().x();
    qreal y = pos().y();

    while ( prev ) {
        // Set position of the previous item according to the direction and the position of
        // the current item.
        switch ( direction ) {
        case SnakeManager::Left:
            x -= pixmap().width();
            break;
        case SnakeManager::Right:
            x += pixmap().width();
            break;
        case SnakeManager::Up:
            y -= pixmap().height();
            break;
        case SnakeManager::Down:
            y += pixmap().height();
            break;
        default:
            break;
        }
        prev->setPos(x,y);

        // Set direction and pixmap of the previous item.
        prev->setDirection(direction);

        current = prev;
        prev = current->previous;
    }
}

/*!
  \internal
  \fn void SnakeItem::move(int amountToMove)
  Non-iterative function, i.e. affects this item only. Moves this segment along,
  according to its current direction.
  \a amountToMove: Number of pixels to move.
*/
void SnakeItem::move(int amountToMove)
{
    switch ( direction ) {
    case SnakeManager::Left:
        moveBy(-amountToMove,0);
        break;
    case SnakeManager::Right:
        moveBy(amountToMove,0);
        break;
    case SnakeManager::Up:
        moveBy(0,-amountToMove);
        break;
    case SnakeManager::Down:
        moveBy(0,amountToMove);
        break;
    default:
        break;
    }
}

/*!
  \internal
  \fn void SnakeItem::moveForwards()
  An iterative function, i.e. affects other segments.
  This would typially be called on the snake's tail, which then iterates from its 'previous' (body)
  item all the way up to the head, moving each item into the position of the previous item, and
  also adjusting direction and pixmap accordingly.
*/
void SnakeItem::moveForwards()
{
    SnakeItem *prev = previous;
    SnakeItem *current = this;

    // Iterate thru the snake segments, pushing each forwards.
    while ( prev ) {
        // If the current item's position is the same as the previous
        // one, we are inserting the item 'prev'. This means that
        // the current item will stay put, and will change neither its
        // direction nor its pixmap.
        if ( prev->pos() != current->pos() ) {
            // Move the current item forwards.
            current->setPos(prev->pos());

            if ( current->direction != prev->direction ) {
                if ( current->bodyPart == SnakeManager::Tail ) {
                    current->setPixmap(snakeManager->getSnakeImage(current->bodyPart,SnakeManager::Any,prev->direction));
                } else if ( prev->bodyPart == SnakeManager::Head ) { // neck
                    // Current is the neck of the snake, and so the snake is turning.
                    current->setPixmap(snakeManager->getSnakeImage(current->bodyPart,current->direction,prev->direction));
                } else { // body
                    current->setPixmap(prev->pixmap());
                }

                current->direction = prev->direction;
                current->turning = true;
            } else {
                if ( current->turning && (current->bodyPart == SnakeManager::Body) ) {
                    current->setPixmap(snakeManager->getSnakeImage(current->bodyPart,SnakeManager::Any,current->direction));
                }
                current->turning = false;
            }
        }

        current = prev;
        prev = current->previous;
    }
}

//Part of the HACK HACK HACK, and shouldn't be like this anyway because setPixmap is not virtual.
void SnakeItem::setPixmap(const QPixmap &pixmap)
{
    QGraphicsPixmapItem::setPixmap(pixmap);
    bounds = QRectF(-0.5,-0.5,pixmap.width()-1,pixmap.height()-1);
}

// ***HACK HACK HACK***
// Had to override this, otherwise collision detection doesn't work, since snake segments touch.
// Attempts to make them 1 pixel apart were visible.
//!!!!!! This is a hack to get collision detection to work. For a start, QGraphicsPixmapItem::boundingRect
// returns a rectangle half a pixel wider to each side than the actual pixmap, which wasn't helpful
// for this application.
QRectF SnakeItem::boundingRect() const
{
    return bounds;
}
