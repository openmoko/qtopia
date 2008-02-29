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

#include "zoomer_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>

const QString Zoomer::description("Zoom");


void Zoomer::animate(QPainter *painter,SelectedItem *item,qreal percent)
{
    GridItem *currentItem = item->getCurrent();
    if ( !currentItem ) {
        return;
    }
    int imageSize = currentItem->getSelectedImageSize();

    if ( percent < stop1 ) {
        // decreasing opacity
        qreal decreaseOpacityBy = (percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * decreaseOpacityBy);

        // increasing up to growthFactor
        qreal increaseBy = percent * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * increaseBy);
        draw(painter,item,sz,sz);
    } else if ( percent < stop2 ) {
        // inverse
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);

        qreal decreaseBy = (1.0-percent) * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * decreaseBy);
        draw(painter,item,sz,sz);
    } else {
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);
        draw(painter,item,imageSize,imageSize);
    }
}

/*
void Zoomer::animate(QPainter *painter,QSvgRenderer *renderer,
                     int imageSize,QGraphicsRectItem *item,qreal percent)
{
    if ( percent < stop1 ) {
        // decreasing opacity
        qreal decreaseOpacityBy = (percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * decreaseOpacityBy);

        // increasing up to growthFactor
        qreal increaseBy = percent * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * increaseBy);
        draw(painter,renderer,item,sz,sz);
    } else if ( percent < stop2 ) {
        // inverse
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);

        qreal decreaseBy = (1.0-percent) * growthFactor * 2;
        int sz = static_cast<int>(imageSize + imageSize * decreaseBy);
        draw(painter,renderer,item,sz,sz);
    } else {
        qreal increaseOpacityBy = (1.0-percent) * stop1;
        painter->setOpacity(painter->opacity() - painter->opacity() * increaseOpacityBy);
        draw(painter,renderer,item,imageSize,imageSize);
    }
}
*/
