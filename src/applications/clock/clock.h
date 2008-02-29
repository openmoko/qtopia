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
#ifndef CLOCK_H
#define CLOCK_H

#include "ui_clockbase.h"
#include <qdatetime.h>
#include <qwidget.h>

class QTimer;
class QLabel;
class QDialog;
class AnalogClock;
class QBoxLayout;
class QToolButton;

class Clock : public QWidget, Ui::ClockBase
{
    Q_OBJECT

public:
    Clock( QWidget *parent=0, Qt::WFlags fl=0 );
    ~Clock();

protected:
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

private slots:
    void updateClock();
    void changeClock( bool );

private:
    QTimer *t;
    bool init;
    bool ampm;
};

#endif

