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

//
// Pinched from settings/light-and-power/light-on.xpm
//
/* XPM */
static const char * const light_on_xpm[] = {
"9 16 5 1",
"       c None",
".      c #FFFFFFFF0000",
"X      c #000000000000",
"o      c #FFFFFFFFFFFF",
"O      c #FFFF6C6C0000",
"         ",
"   XXX   ",
"  XoooX  ",
" Xoooo.X ",
"Xoooooo.X",
"Xoooo...X",
"Xooo.o..X",
" Xooo..X ",
" Xoo...X ",
"  Xoo.X  ",
"  Xoo.XX ",
"  XOOOXX ",
"  XOOOXX ",
"   XOXX  ",
"    XX   ",
"         "};


/* XPM */
static const char * const light_off_xpm[] = {
"9 16 4 1",
" 	c None",
".	c #000000000000",
"X	c #6B6B6C6C6C6C",
"o	c #FFFF6C6C0000",
"         ",
"         ",
"   ...   ",
"  .   .  ",
" .    X. ",
".      X.",
".    XXX.",
".   X XX.",
" .   XX. ",
" .  XXX. ",
"  .  X.  ",
"  .  X.. ",
"  .ooo.. ",
"  .ooo.. ",
"   .o..  ",
"    ..   "};

extern int qpe_sysBrightnessSteps();

//
//
//
BrightnessControl::BrightnessControl(QWidget *parent, const char *name,
    WFlags f) : QFrame(parent, name, WStyle_StaysOnTop | WType_Popup | f)
{
    setFrameStyle(QFrame::PopupPanel | QFrame::Raised);

    QGridLayout *gl = new QGridLayout( this, 3, 2, 6, 3 );
    gl->setRowStretch( 1, 100 );

    int	maxbright = qpe_sysBrightnessSteps();
    slider = new QSlider(this);
    slider->setMaxValue(maxbright);
    slider->setOrientation(QSlider::Vertical);
    slider->setTickmarks(QSlider::Right);
    slider->setTickInterval(QMAX(1, maxbright / 16));
    slider->setLineStep(QMAX(1, maxbright / 16));
    slider->setPageStep(QMAX(1, maxbright / 16));
    gl->addMultiCellWidget( slider, 0, 2, 0, 0 );

    QPixmap onPm( (const char **)light_on_xpm );
    QLabel *l = new QLabel( this );
    l->setPixmap( onPm );
    gl->addWidget( l, 0, 1 );

    QPixmap offPm( (const char **)light_off_xpm );
    l = new QLabel( this );
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
    QImage  img = Resource::loadImage("Light");
    img = img.smoothScale(AppLnk::smallIconSize(), AppLnk::smallIconSize());
    brightnessPixmap = new QPixmap();
    brightnessPixmap->convertFromImage(img);

    setFixedWidth(AppLnk::smallIconSize());
    setFixedHeight(AppLnk::smallIconSize());
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
    int v = 255 - readSystemBrightness();

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

    if (ps.acStatus() == PowerStatus::Online) {
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
