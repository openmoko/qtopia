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

#ifndef SNAKE_H
#define SNAKE_H

#include "snakeiteminterface_p.h"
#include "snakemanager_p.h"

class SnakeConnector;
class QRectF;
class QGraphicsScene;
class SnakeItem;
class QRegion;
class Mouse;


class Snake : public QObject, public SnakeItemInterface
{
    Q_OBJECT

public:

    static const int TYPE;

    Snake(QGraphicsScene *scene,SnakeManager *snakeManager);

    virtual ~Snake();

    int type() const { return TYPE; }

    SnakeManager::Direction getDirection() const;

    QPointF pos() const;

    void setZValue(int zValue);

    void reset(bool resetSpeed = true);

    void pause();

    void turn(SnakeManager::Direction direction);

    bool resize(const QRectF &oldSceneGeometry,const QRectF &newSceneGeometry);

    int getSpeed() const { return speed; }

    void increaseSpeed();

    void goGirl();

    void move();

    void grow();

    void shrink();

    int getPendingSize() const;

    QRegion getRegion() const;

signals:

    void snakeDead();

    void mouseEaten(Mouse *);

private:

    // Starting speed for the snake, i.e. it moves a fraction every 250 milliseconds.
    // The speed can get adjusted via changeSpeed(...).
    static const int START_SPEED = 250;
    // The amount by which the milliseconds in speed are reduced during increaseSpeed().
    static const int SPEED_INCREMENT = 25;
    // The lowest in milliseconds that speed can be reduced to (we can't keep getting
    // faster indefinitely).
    static const int FASTEST_SPEED = 150;

    void collisionsCheck();
    void dieSnakeDie();

    SnakeConnector *connector;
    SnakeManager *snakeManager;

    QGraphicsScene *scene;

    SnakeItem *headItem;
    SnakeItem *tailItem;

    SnakeItem *newBodyItem;

    int bodyCount;

    SnakeManager::Direction newDirection;

    int score;

    QTimer *timer;
    int speed;
    int amountToMove;
};

class SnakeConnector : public QObject
{
    Q_OBJECT

public:

    SnakeConnector(Snake *_snake);

public slots:

    void moveSnake();

private:

    Snake *snake;
};

#endif
