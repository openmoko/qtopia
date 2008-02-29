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

#ifndef __WAIT_H__
#define __WAIT_H__

#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>


class Wait : public QWidget
{
public:
    Wait( QWidget *parent );
    void setWaiting( bool w );
    void paintEvent( QPaintEvent * );
    static Wait *getWaitObject();
private:
    QPixmap pm;
    bool waiting;
};


#endif // __WAIT_H__

