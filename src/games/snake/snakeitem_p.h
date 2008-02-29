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

#ifndef SNAKEITEM_H
#define SNAKEITEM_H


#include "snakemanager_p.h"

#include <QGraphicsPixmapItem>


class QPixmap;


class SnakeItem : public QGraphicsPixmapItem
{
public:

    SnakeItem(SnakeManager::SnakePart,SnakeManager *,SnakeItem *);

    int type() const;

    SnakeManager::SnakePart getBodyPart() const { return bodyPart; }

    SnakeManager::Direction getDirection() const { return direction; }

    void setDirection(SnakeManager::Direction direction);

    SnakeItem *getPrevious() const { return previous; }

    void setPrevious(SnakeItem *);

    void reset(QPointF position,SnakeManager::Direction direction);

    void move(int pixels);

    void moveForwards();

    QRectF boundingRect() const;

private:

    void setPixmap(const QPixmap &pixmap);

    SnakeManager::SnakePart bodyPart;
    SnakeManager *snakeManager;
    SnakeItem *previous;
    SnakeManager::Direction direction;
    bool turning;

    QRectF bounds;

};

#endif
