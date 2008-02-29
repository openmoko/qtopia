/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "wait.h"


#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qvariant.h>
#include <qstyle.h>


Wait *lastWaitObject = NULL;


Wait::Wait( QWidget *parent ) : QWidget( parent ),
        pm(QIcon(":icon/wait").pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize))),
        waiting( false )
{
    setFixedSize( pm.size() );
    lastWaitObject = this;
    hide();
}


Wait *Wait::getWaitObject()
{
    return lastWaitObject;
}


void Wait::setWaiting( bool w )
{
    waiting = w;
    if ( w )
        show();
    else
        hide();
}


void Wait::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.drawPixmap( 0, 0, pm );
}


