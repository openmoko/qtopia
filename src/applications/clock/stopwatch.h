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
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include "ui_stopwatchbase.h"
#include <qdatetime.h>

class QTimer;
class QLabel;
class QAnalogClock;
class QBoxLayout;
class QToolButton;

class StopWatch : public QWidget, Ui::StopWatchBase
{
    Q_OBJECT
public:
    StopWatch( QWidget *parent=0, Qt::WFlags fl=0 );
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
    QVector<int> swatch_splitms;
    bool swatch_running;
    int swatch_currLap;
    int swatch_dispLap;
    QToolButton *prevLapBtn;
    QToolButton *nextLapBtn;
    QTimer *lapTimer;
    QAnalogClock* analogStopwatch;
    QLCDNumber* stopwatchLcd;
    QBoxLayout *swLayout;
    bool init;
};

#endif

