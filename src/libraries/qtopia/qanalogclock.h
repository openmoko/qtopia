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
#ifndef QANALOGCLOCK_H
#define QANALOGCLOCK_H

#include <QTime>
#include <QFrame>
#include <QPixmap>

#include <qtopiaglobal.h>

class QAnalogClockPrivate;

class QTOPIA_EXPORT QAnalogClock : public QFrame
{
    Q_OBJECT
public:
    explicit QAnalogClock( QWidget *parent = 0 );
    ~QAnalogClock();

    void display( const QTime& time );
    void setFace( const QPixmap& face );

protected:
    void paintEvent( QPaintEvent *event );
    void drawContents( QPainter *p );
    void drawHand( QPainter *p, QPoint, QPoint );

private:
    QAnalogClockPrivate* d;
};

#endif //QANALOGCLOCK_H
