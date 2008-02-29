/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "sprites.h"
#include <QObject>

class QGraphicsScene;
class QAnimatedPixmapItem;
class QTimer;

class Snake : public QObject
{
    Q_OBJECT

public:
    Snake(QGraphicsScene*);
    ~Snake();

    void reset();
    void start();
    void stop();
    void increaseSpeed();
    bool go(int newkey);
    void setScore(int amount);
    int getScore() const;

signals:
    void dead();
    void ateMouse();
    void scoreChanged();

private slots:
    void moveSnake();

private:
    void move(SpriteDB::Direction newdir);
    void detectCrash();
    void clear();

    QList<QAnimatedPixmapItem*> snakelist;
    QTimer* autoMoveTimer;
    QGraphicsScene* scene;
    int grow;
    int speed;
    int score;
    SpriteDB::Direction currentdir;
    QList< SpriteDB::Direction > moves;
};

#endif
