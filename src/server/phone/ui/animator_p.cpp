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

#include "animator_p.h"
#include "selecteditem.h"
#include "griditem.h"

#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QGraphicsRectItem>
#include <QSvgRenderer>
#include <QDebug>


static const qreal gradientWidth = 0.1;


/*!
  \internal
  \class Animator

  \brief Animator is the superclass of all the objects that can manually animate icons
  within the PhoneLauncherView.

  \mainclass An animation is generally controlled by a timeline, or similar. At each step
  during the timeline's life cycle, the animate(...) method should be called. This method
  will know how to transform a SelectedItem's representation, and has the option of
  calling Animator's draw(QPainter *,SelectedItem *,int width,int height).
*/


/*!
 \internal
  \fn virtual void Animator::animate(QPainter *,SelectedItem *,qreal percent) = 0

   Derived classes should implement this function to transform the appearance of the
   SelectedItem at each step during the animation process.
   \a painter: Used to draw the SelectedItem's image.
   \a item: The item that is to be drawn, by asking its current GridItem for the renderer
   or for the pixmap.
   \a percent: Value between 0 and 1 which will indicate how far into the animation
   we are, and therefore how the object should be drawn.
*/

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,SelectedItem *item,int width,int height)

  This method can be called by derived classes to draw the SelectedItem for the
  given width and height parameters. It uses the item's SVG renderer, if it can -
  otherwise it attempts to retrieve the pixmap for the selected item.
  The method is intended to be called at each step of the animation, from the
  method animate(...).
*/
void Animator::draw(QPainter *painter,SelectedItem *item,int w,int h)
{
    // Get the GridItem object that is currently associated with the SelectedItem, and
    // find out whether we have a valid renderer we can use to draw it, or whether we
    // must use the pixmap.
    GridItem *currentItem = item->getCurrent();
    if ( currentItem ) {
        QSvgRenderer *renderer = currentItem->getSvgRenderer();
        if ( renderer ) {
            draw(painter,renderer,item,w,h);
            return;
        }
        const QPixmap &pixmap = currentItem->selectedPic(); // use the pixmap
        if ( !(pixmap.isNull()) ) {
            draw(painter,pixmap,item,w,h);
        }
    }
}

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,const QPixmap &pixmap,QGraphicsRectItem *item,int width,int height)
   Draws the SelectedItem's pixmap, for the given width and height.
*/
void Animator::draw(QPainter *painter,const QPixmap &pixmap,QGraphicsRectItem *item,int w,int h)
{
    // Find out if we have to scale the image to fit the given width/height parameters.
    if ( pixmap.width() != w || pixmap.height() != h ) {
        QImage image = pixmap.toImage();
        image = image.scaled(w,h);

        // The graphical item contains the pixmap. If the pixmap is too large for it, we will
        // need to temporarily enlarge the graphical item, so that the pixmap will display
        // completely, without being clipped.
        qreal oldWidth = item->rect().width();
        qreal oldHeight = item->rect().height();
        if ( w > oldWidth || h > oldHeight ) {
            qreal oldX = item->rect().x();
            qreal oldY = item->rect().y();
            qreal x = oldX - (w-oldWidth)/2;
            qreal y = oldY - (h-oldHeight)/2;
            item->setRect(QRectF(x,y,w+2,h+2));
            painter->drawPixmap(static_cast<int>(item->rect().x() + (item->rect().width() - image.width())/2),

                                static_cast<int>(item->rect().y() + (item->rect().height() - image.height())/2),
                                QPixmap::fromImage(image));
            // After drawing the pixmap, put the grahical item's dimensions back the way they were.
            item->setRect(QRectF(oldX,oldY,oldWidth,oldHeight));
        } else {
            painter->drawPixmap(static_cast<int>(item->rect().x() + (item->rect().width() - image.width())/2),
                                static_cast<int>(item->rect().y() + (item->rect().height() - image.height())/2),
                                QPixmap::fromImage(image));
        }
    } else {
        // No scaling involved.
        painter->drawPixmap(static_cast<int>(item->rect().x() + (item->rect().width() - pixmap.width())/2),
                            static_cast<int>(item->rect().y() + (item->rect().height() - pixmap.height())/2),
                            pixmap);
    }
}

/*!
  \internal
  \fn void Animator::draw(QPainter *painter,QSvgRenderer *renderer,QGraphicsRectItem *item,int width,int height)
   Draws the SelectedItem for the given width and height, using the renderer.
*/
void Animator::draw(QPainter *painter,QSvgRenderer *renderer,QGraphicsRectItem *item,int w,int h)
{
    // If what we want to draw is going to be too large for the graphical item, we will
    // need to temporarily enlarge the graphical item, so that the drawing will display
    // completely, without being clipped.
    qreal oldWidth = item->rect().width();
    qreal oldHeight = item->rect().height();
    if ( w > oldWidth || h > oldHeight ) {
        QRectF bounds = getRenderingBounds(item,w,h);
        item->setRect(QRectF(item->rect().x()-1,item->rect().y()-1,w+2,h+2));
        renderer->render(painter,bounds);
        // After drawing, put the grahical item's dimensions back the way they were.
        item->setRect(QRectF(item->rect().x()+1,item->rect().y()+1,oldWidth,oldHeight));
    } else {
        QRectF bounds = getRenderingBounds(item,w,h);
        renderer->render(painter,bounds);
    }
}

/*!
  \internal
  \fn QRectF Animator::getRenderingBounds(QGraphicsRectItem *item,int width,int height)
  Returns the geometry for the renderer, i.e. position, width and height.
*/
QRectF Animator::getRenderingBounds(QGraphicsRectItem *item,int w,int h)
{
    // Position the image in the middle of the drawing area.
    qreal x = item->rect().x();
    qreal y = item->rect().y();
    x += (item->rect().width() - w)/2;
    y += (item->rect().height() - h)/2;

    return QRectF(x,y,w,h);
}
