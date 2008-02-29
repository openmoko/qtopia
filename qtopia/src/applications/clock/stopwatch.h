/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <stopwatchbase.h>
#include <qdatetime.h>

class QTimer;
class QLabel;
class AnalogClock;
class QBoxLayout;
class QToolButton;

class StopWatch : public StopWatchBase
{
    Q_OBJECT
public:
    StopWatch( QWidget *parent=0, const char *name=0, WFlags fl=0 );
    ~StopWatch();
    
private slots:
    void stopStartStopWatch();
    void resetStopWatch();
    void prevLap();
    void nextLap();
    void lapTimeout();
    void updateClock();
    void changeClock( bool );

protected:
    void updateLap();
    void setSwatchLcd( QLCDNumber *lcd, int ms, bool showMs );
    bool eventFilter( QObject *o, QEvent *e );
    void showEvent(QShowEvent *e);

private:
    QTimer *t;
    QTime swatch_start;
    int swatch_totalms;
    QArray<int> swatch_splitms;
    bool swatch_running;
    int swatch_currLap;
    int swatch_dispLap;
    QToolButton *prevLapBtn;
    QToolButton *nextLapBtn;
    QTimer *lapTimer;
    AnalogClock* analogStopwatch;
    QLCDNumber* stopwatchLcd;
    QBoxLayout *swLayout;
    bool init;
};

#endif

