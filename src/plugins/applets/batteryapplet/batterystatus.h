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
#ifndef BATTERY_STATUS_H
#define BATTERY_STATUS_H

#include <qframe.h>

class PowerStatus;

class BatteryStatus : public QFrame
{
public:
    BatteryStatus( const PowerStatus *s, bool pop, QWidget *parent=0, WFlags f=0 );
    ~BatteryStatus();

    void updatePercent( int );
    QSize sizeHint() const;

protected:
    void drawSegment( QPainter *p, const QRect &r, const QColor &topgrad, const QColor &botgrad, const QColor &highlight, int hightlight_height );
    void drawContents( QPainter *p );

private:
    const PowerStatus *ps;
    int percent;
    bool popup;
};

#endif

