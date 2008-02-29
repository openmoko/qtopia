/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef CANVAS_CARD_H
#define CANVAS_CARD_H


#include <qpainter.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qcanvas.h>
#include <qdatetime.h>
#include "cardgame.h"
#include "canvasitemtimer.h"


static const int canvasCardId = 2434321;


class CanvasCard : public Card, public AnimatedItem, public QCanvasRectangle
{
public:
    CanvasCard(Value v, Suit s, bool f, QCanvas *canvas);
    virtual ~CanvasCard() { canvas()->removeItem(this); }

    void cardBackChanged();
    void draw(QPainter &p);

    virtual int rtti() const { return canvasCardId; }

    virtual void updatePosition(double percent);

    virtual void setPos(QPoint p) { move(p.x(), p.y()); }
    virtual QPoint pos() const { return QPoint((int)x(), (int)y()); }

    virtual void setHeight(int z) { setZ(z); }
    virtual int height() const { return (int)z(); }

    virtual void setVisible(bool b) { QCanvasRectangle::setVisible(b); } 
    virtual bool isVisible() const { return visible(); }

    virtual void moveTo(QPoint p, int duration = 0);
    virtual void flipTo(QPoint p, int duration = 0);

    bool isFlipping() { return flipping; }

private:
    void animatedMove(QPoint p, int msecs);

    QPoint offset;
    QPoint dest;
    QPoint delta;
    int origHeight;
    bool origFace;
    bool flipping;
    double scaleX, scaleY;
    QPixmap cachedPixmap;
    bool haveCache;
};


#endif

