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

#include "brightness.h"

#include <qtopia/power.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qpainter.h>
#include <qslider.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpixmap.h>

#include <qtopia/applnk.h>

extern int qpe_sysBrightnessSteps();

//
//
//
BrightnessControl::BrightnessControl(QWidget *parent, const char *name,
    WFlags f) : QFrame(parent, name, WStyle_StaysOnTop | WType_Popup | f)
{
    setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
    setBackgroundMode(PaletteButton);

    QGridLayout *gl = new QGridLayout( this, 3, 2, 6, 3 );
    gl->setRowStretch( 1, 100 );

    int	maxbright = qpe_sysBrightnessSteps();
    slider = new QSlider(this);
    slider->setBackgroundMode(PaletteButton);
    slider->setMaxValue(maxbright);
    slider->setOrientation(QSlider::Vertical);
    slider->setTickmarks(QSlider::Right);
    slider->setTickInterval(QMAX(1, maxbright / 16));
    slider->setLineStep(QMAX(1, maxbright / 16));
    slider->setPageStep(QMAX(1, maxbright / 16));
    gl->addMultiCellWidget( slider, 0, 2, 0, 0 );

    QPixmap onPm( Resource::loadPixmap("light-and-power/light-on") );
    QLabel *l = new QLabel( this );
    l->setBackgroundMode(PaletteButton);
    l->setPixmap( onPm );
    gl->addWidget( l, 0, 1 );

    QPixmap offPm( Resource::loadPixmap("light-and-power/light-off") );
    l = new QLabel( this );
    l->setBackgroundMode(PaletteButton);
    l->setPixmap( offPm );
    gl->addWidget( l, 2, 1 );

    setFixedHeight(100);
    setFixedWidth(sizeHint().width());
    setFocusPolicy(QWidget::NoFocus);

}

//
//
//
BrightnessApplet::BrightnessApplet(QWidget *parent, const char *name)
    : QWidget(parent, name), bc(0) 
{
    QImage  img = Resource::loadImage("light-and-power/Light");
    img = img.smoothScale(AppLnk::smallIconSize(), AppLnk::smallIconSize());
    brightnessPixmap = new QPixmap();
    brightnessPixmap->convertFromImage(img);

    setFixedWidth(AppLnk::smallIconSize());
    setFixedHeight(AppLnk::smallIconSize());

    brightness = -1;
#ifndef QT_NO_COP
    QCopChannel * sysChannel = new QCopChannel("QPE/System", this);
    connect( sysChannel, SIGNAL(received(const QCString&, const QByteArray&)),
                 this, SLOT(sysMessage(const QCString&, const QByteArray&)));
#endif
}

//
//
//
BrightnessApplet::~BrightnessApplet()
{
    delete brightnessPixmap;
}

//
// This magic stolen from
// settings/light-and-power/light.cpp: LightSettings::applyBrightness().
//
int BrightnessApplet::calcBrightnessValue(void)
{
    int v = bc->slider->maxValue() - bc->slider->value();

    return (v * 255 + bc->slider->maxValue() / 2) / bc->slider->maxValue();
}

//
// This magic stolen from
// settings/light-and-power/light.cpp: LightSettings::applyMode().
//
int
BrightnessApplet::calcSliderValue(void)
{
    int v;
    if (brightness == -1)
        v = 255 - readSystemBrightness();
    else {
        v = 255 - brightness;
        brightness = -1;
    }

    return (bc->slider->maxValue() * v + 128) / 255;
}

//
//
//
void
BrightnessApplet::mousePressEvent(QMouseEvent *)
{
    bc = new BrightnessControl(0, 0, WDestructiveClose);

    bc->slider->setValue(calcSliderValue());
    connect(bc->slider, SIGNAL(valueChanged(int)),
	this, SLOT(sliderMoved(int)));

    QPoint curPos = mapToGlobal(rect().topLeft());
    if ( curPos.x() + bc->sizeHint().width() > qApp->desktop()->width() )
	bc->move( qApp->desktop()->width() - bc->sizeHint().width(), curPos.y() - 101 );
    else 
	bc->move(curPos.x() - (bc->sizeHint().width()-width())/2, curPos.y() - 101);
    bc->show();
}

//
//
//
void
BrightnessApplet::sliderMoved(int /* value */)
{
#ifndef QT_NO_COP
    QCopEnvelope e("QPE/System", "setBacklight(int)");
    e << calcBrightnessValue();
#endif // QT_NO_COP

    // XXXX Should only write once when the control vanishes.  Fix this.
    writeSystemBrightness(calcBrightnessValue());
}

//
// Store the brightness in the currently active power configuration.
//
void
BrightnessApplet::writeSystemBrightness(int brightness)
{
    PowerStatus ps = PowerStatusManager::readStatus();

    Config  cfg("qpe");

    if (ps.acStatus() == PowerStatus::Online) {
	cfg.setGroup("ExternalPower");
    } else {
	cfg.setGroup("BatteryPower");
    }
    cfg.writeEntry("Brightness", brightness);

    // Compatibility.
    cfg.setGroup("Screensaver");
    cfg.writeEntry("Brightness", brightness);
}

//
// Read the brightness from the currently active power configuration.
//
int
BrightnessApplet::readSystemBrightness(void)
{
    PowerStatus ps = PowerStatusManager::readStatus();

    Config  cfg("qpe");

    if (PowerStatusManager::APMEnabled() && ps.acStatus() == PowerStatus::Online) {
	cfg.setGroup("ExternalPower");
    } else {
	cfg.setGroup("BatteryPower");
    }

    return cfg.readNumEntry("Brightness", 255);
}

//
//
//
void
BrightnessApplet::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.drawPixmap(0, 0, *brightnessPixmap);
}

//
//
//
void BrightnessApplet::sysMessage(const QCString &msg, const QByteArray& data)
{
#ifndef QT_NO_COP
    QDataStream s(data, IO_ReadOnly);
    if (msg == "setBacklight(int)")
    {
        if (brightness != -1) //ignore msg before first update
            s >> brightness;
    }
#endif
}
