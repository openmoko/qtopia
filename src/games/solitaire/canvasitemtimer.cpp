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
#include "canvasitemtimer.h"
#include <qtimer.h>


CanvasItemTimer::CanvasItemTimer( AnimatedCanvasItem *c, int ms ) : QObject(), card( c ), msecs( ms )
{
    counter.start();
    QTimer::singleShot( 1, this, SLOT( processNext() ) );
}


CanvasItemTimer::~CanvasItemTimer()
{
}


void CanvasItemTimer::processNext()
{
    double percent = (double)counter.elapsed() / msecs;

    if ( percent >= 1.0 ) {
	card->updatePosition( 1.0 );
	delete this;
    } else {
	card->updatePosition( percent );
	QTimer::singleShot( 1, this, SLOT( processNext() ) );
    }
}


AnimatedCanvasItem::AnimatedCanvasItem()
{
}


AnimatedCanvasItem::~AnimatedCanvasItem()
{
}


