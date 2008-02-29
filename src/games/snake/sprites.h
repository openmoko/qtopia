/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef SPRITES_H
#define SPRITES_H

#include <QGraphicsItem>

const int mouse_rtti = 1500;
const int wall_rtti = 1600;

class QAnimatedPixmapItem : public QGraphicsItem
{
public:
    QAnimatedPixmapItem(const QList<QPixmap> &animation, QGraphicsScene *scene = 0);
    QAnimatedPixmapItem(QGraphicsScene *scene = 0);

    void setFrame(int frame);
    inline int frame() const
    { return currentFrame; }
    inline int frameCount() const
    { return frames.size(); }
    inline QPixmap image(int frame) const
    { return frames.isEmpty() ? QPixmap() : frames.at(frame % frames.size()).pixmap; }
    inline void setVelocity(qreal xvel, qreal yvel)
    { vx = xvel; vy = yvel; }
    inline qreal xVelocity() const
    { return vx; }
    inline qreal yVelocity() const
    { return vy; }

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void advance(int phase);

    void setSequence(const QList<QPixmap> &animation);

private:
    struct Frame {
        QPixmap pixmap;
        QRectF boundingRect;
    };

    int currentFrame;
    QList<Frame> frames;
    qreal vx, vy;
};

class SpriteDB
{
public:
    enum Direction {
        left = 0,
        right = 1,
        up = 2,
        down = 3,
    };

    static QList<QPixmap> &spriteCache();

    static int snakeHead(Direction);
    static int snakeBody(Direction olddir, Direction newdir);
    static int snakeTail(Direction);

    static int mouse();

    static int wall(bool east, bool west, bool north, bool south);

    static int ground();

    static int tileSize() { return tilesize; }
private:

    static int tilesize;
    static QList<QPixmap> parts;
};

#endif
