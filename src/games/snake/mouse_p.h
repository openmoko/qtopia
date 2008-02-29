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

#ifndef MOUSE_H
#define MOUSE_H

#include "snakeiteminterface_p.h"
#include <QGraphicsPixmapItem>

class QPixmap;
class QRectF;
class MouseConnector;
class QTimeLine;


class Mouse : public QGraphicsPixmapItem, public SnakeItemInterface
{
public:

    static const int TYPE;

    Mouse(QGraphicsScene *,const QPixmap &);

    ~Mouse();

    int type() const;

    bool resize(const QRectF &oldSceneGeometry,const QRectF &newSceneGeometry);

    void changeFrightStatus(bool frightened = true);

    void tremble(int n);

private:

    static const int TREMBLE_DURATION = 1000;

    QTimeLine *trembler;
    int distanceToTremble;

    MouseConnector *connector;

};


class MouseConnector : public QObject
{
    Q_OBJECT
public:

    MouseConnector(Mouse *);

public slots:

    void trembling(int);

    //void tremblerStateChanged(QTimeLine::State);

private:

    Mouse *mouse;
};

#endif
