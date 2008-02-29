/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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


class CanvasCard : public Card, public AnimatedCanvasItem, public QCanvasRectangle
{
public:
    CanvasCard( eValue v, eSuit s, bool f, QCanvas *canvas );
    virtual ~CanvasCard() { canvas()->removeItem(this); }

    int rtti () const { return canvasCardId; }
    void move(QPoint p) { QCanvasItem::move( p.x(), p.y() ); }
    void move(int x, int y) { QCanvasItem::move( x, y ); }
    void animatedMove(int x, int y, int msecs );
    void animatedMove() { animatedMove( savedX, savedY, 1500 ); }
    void savePos(void) { savedX = (int)x(); savedY = (int)y(); }
    void moveToPile(int p) { Q_UNUSED(p); }
    void cardBackChanged();
    
    void flipTo(int x, int y, int msecs );
    void setPos( int x, int y, int z ) { setX( x ); setY( y ); setZ( z ); }
    void showCard(void) { show(); }
    void redraw(void) { hide(); show(); }
    void draw(QPainter &p);
    void updatePosition( double percent );

protected:
    void flip(void) { redraw(); }

private:
    int destX, destY;
    double deltaX, deltaY;
    bool origFace;

    bool flipping;
    int savedX, savedY;
    double scaleX, scaleY;
    int xOff, yOff;
    QPixmap cachedPixmap;
    bool haveCache;
};


#endif

