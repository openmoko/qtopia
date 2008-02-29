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
#ifndef CANVAS_ITEM_TIMER_H
#define CANVAS_ITEM_TIMER_H


#include <qobject.h>
#include <qdatetime.h>
#include <qcanvas.h>


class AnimatedCanvasItem //: public QCanvasRectangle
{
public:
    AnimatedCanvasItem();
    virtual ~AnimatedCanvasItem();
    virtual void updatePosition( double percent ) = 0;
};


class CanvasItemTimer : public QObject {
    Q_OBJECT
public:
    CanvasItemTimer( AnimatedCanvasItem *c, int msecs );
    ~CanvasItemTimer();
public slots:
    void processNext();
private:
    AnimatedCanvasItem *card;
    QTime counter;
    int msecs;
};


#endif // CANVAS_ITEM_TIMER_H

