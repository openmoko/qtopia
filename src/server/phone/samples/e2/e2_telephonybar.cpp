/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "e2_telephonybar.h"
#include "e2_colors.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFontMetrics>

E2TelephonyBar::E2TelephonyBar(QWidget *parent, Qt::WFlags flags)
: QWidget(parent, flags),
  fillBrush(":image/samples/e2_telephonybar"),
  batPix(":image/samples/e1_bat"),
  signalPix(":image/samples/e1_signal"),
  time("/UI/DisplayTime/Time"),
  battery("/Hardware/Accessories/QPowerSource/DefaultBattery/Charge"),
  signal("/Hardware/Accessories/QSignalSource/DefaultSignal/SignalStrength")
{
    setFixedHeight(fillBrush.height());

    QObject::connect(&time, SIGNAL(contentsChanged()),
                     this, SLOT(timeChanged()));
    QObject::connect(&battery, SIGNAL(contentsChanged()),
                     this, SLOT(batteryChanged()));
    QObject::connect(&signal, SIGNAL(contentsChanged()),
                     this, SLOT(signalChanged()));

    batteryChanged();
    signalChanged();
    timeChanged();
}

void E2TelephonyBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.drawTiledPixmap(0, 0, width(), height(), fillBrush);

    p.drawPixmap(4, height() - 4 - cSignalPix.height(), cSignalPix);

    QFontMetrics fm(font());
    int clockWidth = fm.width("000000");
    p.drawPixmap(width() - 1 - clockWidth - 1 - cBatPix.width() - 8,
                 height() - 4 - cBatPix.height(), cBatPix);

    int aClockWidth = fm.width(cTime);
    p.drawText(width() - 1 - aClockWidth - 8, height() - 4, cTime);
}

void E2TelephonyBar::batteryChanged()
{
    int percent = battery.value().toInt();
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;

    // Clamp to image
    int image = 0;
    if(percent < 25)
        image = 0;
    else if(percent < 50)
        image = 1;
    else if(percent < 75)
        image = 2;
    else
        image = 3;

    // Display image
    QPixmap pix = batPix.copy(QRect(image * batPix.width() / 4, 0,
                              batPix.width() / 4,
                              batPix.height()));

    cBatPix = pix;
    update();
}

void E2TelephonyBar::signalChanged()
{
    int percent = signal.value().toInt();
    if(percent < 0) percent = 0;
    if(percent > 100) percent = 100;

    int image = 0;
    if(percent < 16)
        image = 0;
    else if(percent < 32)
        image = 1;
    else if(percent < 48)
        image = 2;
    else if(percent < 64)
        image = 3;
    else if(percent < 80)
        image = 4;
    else
        image = 5;

    // Display image
    QRect cp(image * signalPix.width() / 6, 0,
             signalPix.width() / 6, signalPix.height());
    QPixmap pix = signalPix.copy(cp);

    cSignalPix = pix;
    update();
}

void E2TelephonyBar::timeChanged()
{
    cTime = time.value().toString();
    update();
}

