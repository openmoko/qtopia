/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef E3_CLOCK
#define E3_CLOCK

#include <QWidget>
#include <QTime>
#include <QBasicTimer>

class E3Clock : public QWidget
{
    Q_OBJECT
public:
    E3Clock(QWidget *parent=0, Qt::WFlags f=0);

    void showTime( const QTime& time );
    void showCurrentTime();

protected:
    void paintEvent( QPaintEvent *event );
    void drawContents( QPainter *p );
    void drawHand( QPainter *p, QPoint, QPoint, int width );
    void timerEvent(QTimerEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private:
    QPoint rotate( QPoint center, QPoint p, int angle );

    QTime currTime;
    QTime prevTime;
    bool isEvent;
    QRegion changed;
    bool drawSeconds;
    QBasicTimer timer;
    bool showCurrent;
};

#endif
