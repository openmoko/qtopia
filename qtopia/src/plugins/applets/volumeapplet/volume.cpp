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

#include "volume.h"

#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qpainter.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpixmap.h>

#include <qtopia/applnk.h>

VolumeControl::VolumeControl( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, WStyle_StaysOnTop | WType_Popup | f )
{
    setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
    setBackgroundMode(PaletteButton);

    QVBoxLayout *vbox = new QVBoxLayout( this );
    slider = new QSlider( this );
    slider->setBackgroundMode(PaletteButton);
    muteBox = new QCheckBox( tr("Mute"), this );
    muteBox->setBackgroundMode(PaletteButton);
    slider->setRange( 0, 100 );
    slider->setTickmarks( QSlider::Both );
    slider->setTickInterval( 20 );
    slider->setFocusPolicy( QWidget::NoFocus );
    muteBox->setFocusPolicy( QWidget::NoFocus );
    vbox->setMargin( 6 );
    vbox->setSpacing( 3 );
    vbox->addWidget( slider, 0, Qt::AlignVCenter | Qt::AlignHCenter );
    vbox->addWidget( muteBox );
    setFixedHeight( 100 );
    setFixedWidth( sizeHint().width() );
    setFocusPolicy(QWidget::NoFocus);
}

void VolumeControl::keyPressEvent( QKeyEvent *e) 
{
    switch(e->key()) {
	case Key_Up:
	    slider->subtractStep();
	    break;
	case Key_Down:
	    slider->addStep();
	    break;
	case Key_Space:
	    muteBox->toggle();
	    break;
	case Key_Escape:
	    close();
	    break;
	default:
	    e->ignore();
	    break;
    }
}

//===========================================================================

VolumeApplet::VolumeApplet( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    setFixedHeight(AppLnk::smallIconSize());
    setFixedWidth(AppLnk::smallIconSize());

    QImage  img = Resource::loadImage("icons/14x14/volume");
    img = img.smoothScale(AppLnk::smallIconSize(), AppLnk::smallIconSize() - 4);
    volumePixmap.convertFromImage(img);

    readSystemVolume();

    connect( qApp, SIGNAL( volumeChanged(bool) ), this, SLOT( volumeChanged(bool) ) );
}

VolumeApplet::~VolumeApplet()
{
}

void VolumeApplet::mousePressEvent( QMouseEvent *)
{
    // Create a small volume control window to adjust the volume with
    VolumeControl *vc = new VolumeControl(0,0,WDestructiveClose);
    vc->slider->setValue( 100 - volumePercent );
    vc->muteBox->setChecked( muted );
    connect( vc->slider, SIGNAL( valueChanged(int) ), this, SLOT( sliderMoved(int) ) );
    connect( vc->muteBox, SIGNAL( toggled(bool) ), this, SLOT( mute(bool) ) );
    QPoint curPos = mapToGlobal( rect().topLeft() );
    if ( curPos.x() + vc->sizeHint().width() > qApp->desktop()->width() )
	vc->move( qApp->desktop()->width() - vc->sizeHint().width(), curPos.y() - 101 );
    else 
	vc->move( curPos.x()-(vc->sizeHint().width()-width())/2, curPos.y() - 101 );
    vc->show();
}

void VolumeApplet::volumeChanged( bool nowMuted )
{
    int previousVolume = volumePercent;
    int previousMute = muted;

    readSystemVolume();

    // Handle case where muting it toggled
    if ( previousMute != nowMuted ) {
	muted = nowMuted;
	repaint( TRUE );
	return;
    }

    // Avoid over repainting
    if ( previousVolume != volumePercent )
	repaint( 2, height() - 4, width() - 4, 2, FALSE );
}


void VolumeApplet::mute( bool toggled )
{
    muted = toggled;
    // clear if removing mute
    repaint( !toggled );
    writeSystemVolume();
}


void VolumeApplet::sliderMoved( int percent )
{
    setVolume( 100 - percent );
}


void VolumeApplet::readSystemVolume()
{
    Config cfg("Sound");
    cfg.setGroup("System");
    volumePercent = cfg.readNumEntry("Volume");
    muted = cfg.readBoolEntry("Muted",false);
}


void VolumeApplet::setVolume( int percent )
{
    // clamp volume percent to be between 0 and 100
    volumePercent = (percent < 0) ? 0 : ((percent > 100) ? 100 : percent);
    // repaint just the little volume rectangle
    repaint( 2, height() - 4, width() - 4, 2, FALSE );
    writeSystemVolume();
}


void VolumeApplet::writeSystemVolume()
{
    {
	Config cfg("Sound");
	cfg.setGroup("System");
	cfg.writeEntry("Volume",volumePercent);
	cfg.writeEntry("Muted",muted);
    }
#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
    // Send notification that the volume has changed
    QCopEnvelope( "QPE/System", "volumeChange(bool)" ) << muted;
#endif
}


void VolumeApplet::paintEvent( QPaintEvent* )
{
    QPainter p(this);

    int yoff=0;
    p.drawPixmap( 0, yoff, volumePixmap );
    p.setPen( darkGray );
    p.drawRect( 1, height() - 5, width() - 2, 4 );

    int pixelsWide = volumePercent * (width() - 4) / 100;
    p.fillRect( 2, height() - 4, pixelsWide, 2, red );
    p.fillRect( pixelsWide + 2, height() - 4, width() - 4 - pixelsWide, 2, lightGray );

    if ( muted ) {
	p.setPen( red );
	p.drawLine( 1, yoff+1, width() - 2, height() - 4 );
	p.drawLine( 1, yoff+2, width() - 2, height() - 5 );
	p.drawLine( width() - 2, yoff+1, 1, height() - 4 );
	p.drawLine( width() - 2, yoff+2, 1, height() - 5 );
    }
}


