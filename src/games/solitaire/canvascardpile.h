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
#ifndef CANVAS_CARD_PILE_H
#define CANVAS_CARD_PILE_H


#include <qcanvas.h>
#include <qimage.h>
#include "cardpile.h"
#include "canvascard.h"
#include "canvasshapes.h"


class CanvasCardPile : public CardPile, public CanvasRoundRect
{
public:
    CanvasCardPile(QPoint p, QString name, QCanvas *canvas)
        : CardPile(p, name), CanvasRoundRect(p, canvas) { }
    ~CanvasCardPile() { }

    QRect boundingRect() {
	int x1 = (int)x();
	int y1 = (int)y();
	int x2 = hypertheticalNextCardPos().x() - x1 + CardMetrics::width();
	int y2 = hypertheticalNextCardPos().y() - y1 + CardMetrics::height();
	return QRect( x1, y1, x2, y2 );
    }
};


class AnimatedCanvasCardPile : public AnimatedItem, public QCanvasRectangle
{
public:
    AnimatedCanvasCardPile(QCanvas *canvas)
	: AnimatedItem(), QCanvasRectangle( canvas ) { 
	pile = new QPixmap( 0, 0 );
	firstCard = NULL;
    }
    void reset() {
	firstCard = NULL;
    }

    QPoint pos() const { return QPoint((int)x(), (int)y()); }
    void setPos(QPoint p) { setX(p.x()); setY(p.y()); }

    void addCards(CanvasCard *card);
    void animatedMove() { animatedMove(saved, 400); }
    void savePos() { saved = pos(); }
    void animatedMove(QPoint p, int msecs);
    void updatePosition(double percent);

protected:
    virtual void draw(QPainter& p);

private:
    QPixmap *pile;
    QImage doubleBuffer;
    CanvasCard *firstCard;
    QPoint dest, delta, saved;
};


#endif // CANVAS_CARD_PILE_H

