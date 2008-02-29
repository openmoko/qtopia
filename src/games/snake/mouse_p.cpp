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

#include "mouse_p.h"
#include "snakemanager_p.h"

#include <QPixmap>
#include <QGraphicsItem>
#include <QTimeLine>
#include <QDebug>


const int Mouse::TYPE = QGraphicsItem::UserType + SnakeManager::MOUSE_TYPE;


/*!
  \internal
  \class Mouse

  \brief A graphical item to represent a mouse in the snake game.
  \mainclass The mouse is capable of showing fear when instructed, which will typically
  be when the snake is close.
*/


/*!
  \internal
  \fn Mouse::Mouse(QGraphicsScene *scene,const QPixmap &pixmap)
*/
Mouse::Mouse(QGraphicsScene *scene,const QPixmap &pixmap)
    : QGraphicsPixmapItem(pixmap,0,scene)
    , trembler(0)
    , distanceToTremble(0)
    , connector(0)
{
    connector = new MouseConnector(this);  // signals/slots

    if ( pixmap.width() != SnakeManager::TILE_SIZE ) {
        qWarning() << "Mouse::Mouse(): pixmap width " << pixmap.width() << ", but tile size " << SnakeManager::TILE_SIZE;
    }
    // The distance by which a mouse might tremble, to and fro.
    distanceToTremble = qRound(pixmap.width() / 2.0 / 5);

    trembler = new QTimeLine(TREMBLE_DURATION,connector);
    trembler->setFrameRange(0,100);
    trembler->setUpdateInterval(100);  // only call 10 times per second
    QObject::connect(trembler,SIGNAL(frameChanged(int)),connector,SLOT(trembling(int)));
//    QObject::connect(trembler,SIGNAL(stateChanged(QTimeLine::State)),connector,SLOT(tremblerStateChanged(QTimeLine::State)));
}

/*!
  \internal
  \fn Mouse::~Mouse()
*/
Mouse::~Mouse()
{
    delete trembler;
    delete connector;
}

/*!
  \internal
  \fn int Mouse::type() const
  Used to distinguish a Mouse from other QGraphicItem objects at run-time. Returns
  Mouse::TYPE.
*/
int Mouse::type() const
{
    return TYPE;
}

/*!
  \internal
  \fn bool Mouse::resize(const QRectF &oldSceneGeometry,const QRectF &newSceneGeometry)
  Required by SnakeItemInterface. Used during resizing to reposition the Mouse at its relative x-y
  position.
*/
bool Mouse::resize(const QRectF &oldSceneGeometry,const QRectF &newSceneGeometry)
{
    // Keep my relative position in the new geometry.
    qreal ratioX = pos().x()/oldSceneGeometry.width();
    qreal ratioY = pos().y()/oldSceneGeometry.height();

    setPos(newSceneGeometry.width() * ratioX, newSceneGeometry.height() * ratioY);

    return true;
}

/*!
  \internal
  \fn void Mouse::changeFrightStatus(bool frightened)
  Starts/stops the mouse being frightened, according to \a frightened.
*/
void Mouse::changeFrightStatus(bool frightened)
{
    if ( frightened ) {
        if ( trembler->state() != QTimeLine::Running ) {
            trembler->start();
        }
    } else {
        if ( trembler->state() != QTimeLine::NotRunning ) {
            trembler->stop();
        }
    }
}

/*!
  \internal
  \fn void Mouse::tremble(int n)
  Designed to be called repetitively while the mouse is trembling. Moves the mouse by a small
  distance, either to its left or to its right.
  \a n: A value between 0 and 100, which gives the amount of the trembling duration that has passed.
*/
void Mouse::tremble(int n)
{
    // From 0 to 4 mouse moves to the left (phase 0), from 5 to 14 mouse moves to the right (phase 1),
    // from 15 to 25 swings back to the left (phase 2), etc. Work out the phase.
    int phase = (n - 5)/10 + 1;
    // Work out whether the phase is swinging to the left or the right.
    if ( phase % 2 == 0 ) {
        // Even - swinging to the left.
        moveBy(-distanceToTremble,0);
    } else {
        // Odd - swinging to the right.
        moveBy(distanceToTremble,0);
    }
}

MouseConnector::MouseConnector(Mouse *_mouse)
    : mouse(_mouse)
{
}

void MouseConnector::trembling(int n)
{
    mouse->tremble(n);
}
