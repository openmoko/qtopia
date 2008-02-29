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
#include "videoviewer.h"
#include "mediaplayerstate.h"
#include "controlwidgetbase.h"
#include "loopcontrol.h"
#include "audiodevice.h"
#include "videooutput.h"
#include <qtopia/applnk.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qapplication.h>
#include <qfontmetrics.h>
#include <qtoolbutton.h>
#include <qtopia/resource.h>


class SimpleVideoWidget : public ControlWidgetBase {
public:
    SimpleVideoWidget::SimpleVideoWidget(QWidget *parent);
    ~SimpleVideoWidget() { }
    void makeVisible() { }
    void updateVolume() { }
protected:
    void virtualPaint(QPaintEvent *) { }
    void virtualResize() { }
    void virtualUpdateSlider() { updateSlider(); }
};


SimpleVideoWidget::SimpleVideoWidget(QWidget *parent) :
    ControlWidgetBase(parent, "", "", "EmbeddedVideoControl") // No tr
{
    setButtonData(0,0);
    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBox *hbox1 = new QHBox(this);
    hbox1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    hbox1->setLineWidth(3);
    VideoOutputWidget *videoOutput = new VideoOutputWidget(hbox1);
    videoOutput->setFullscreenMode(false);
    QHBox *hbox2 = new QHBox(this);
    QToolButton *pb2 = new QToolButton(hbox2);
    pb2->setIconSet(Resource::loadIconSet("play"));
    connect(pb2,SIGNAL(clicked()),this,SLOT(play()));
    QToolButton *pb3 = new QToolButton(hbox2);
    pb3->setIconSet(Resource::loadIconSet("stop"));
    connect(pb3,SIGNAL(clicked()),this,SLOT(stop()));
    QFontMetrics fm(qApp->font());
    time.setMinimumWidth(fm.width(" 00:00 / 00:00 "));
    time.reparent(hbox2,QPoint(0,0));
    QToolButton *pb1 = new QToolButton(hbox2);
    pb1->setIconSet(Resource::loadIconSet("back"));
    connect(pb1,SIGNAL(pressed()),this,SIGNAL(backwardClicked()));
    connect(pb1,SIGNAL(released()),this,SIGNAL(backwardReleased()));
    slider.reparent(hbox2,QPoint(0,0));
    QToolButton *pb4 = new QToolButton(hbox2);
    pb4->setIconSet(Resource::loadIconSet("forward"));
    connect(pb4,SIGNAL(pressed()),this,SIGNAL(forwardClicked()));
    connect(pb4,SIGNAL(released()),this,SIGNAL(forwardReleased()));
    layout->addWidget(hbox1,1);
    layout->addWidget(hbox2);
}


ControlWidgetBase *VideoViewer::createVideoUI()
{
    return new SimpleVideoWidget(this);
}


LoopControlBase *VideoViewer::createLoopControl()
{
    return new LoopControl(0, "loopControl", audioDevice());
}


AudioDevice *VideoViewer::createAudioDevice()
{
    return new AudioDevice(this, "embeddedAudioDevice");
}


