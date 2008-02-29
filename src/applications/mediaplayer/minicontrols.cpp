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

#include <stdlib.h>
#include <stdio.h>
#include <qwidget.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qbutton.h>
#include <qpainter.h>
#include <qframe.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpe/resource.h>
#include <qpe/mediaplayerplugininterface.h>
#include "imageutil.h"
#include "audiowidget.h"
#include "mediaplayerstate.h"


MiniControls::MiniControls( QWidget* parent, const QString &skinPath, const char* name ) :
    QWidget( parent, name ), sliderBeingMoved( FALSE )
{
    setCaption( tr("Media Player") );

    QHBox *hbox = new QHBox( this );

    slider = new QSlider( Qt::Horizontal, hbox );
    slider->setFixedHeight( 20 );
    slider->setMinValue( 0 );
    slider->setMaxValue( 1 );
    slider->setFocusPolicy( QWidget::NoFocus );

    time = new QLineEdit( hbox );
    time->setFocusPolicy( QWidget::NoFocus );
    time->setAlignment( Qt::AlignCenter );
    changeTextColor( time );

    QButton *prev = new QButton( hbox );
    QButton *rew  = new QButton( hbox );
    QButton *play = new QButton( hbox );
    QButton *fwd  = new QButton( hbox );
    QButton *next = new QButton( hbox );
    QButton *loop = new QButton( hbox );
    QButton *up   = new QButton( hbox );
    QButton *dn   = new QButton( hbox );
    QButton *done = new QButton( hbox );

    connect( slider,	       SIGNAL( sliderPressed() ),      this, SLOT( sliderPressed() ) );
    connect( slider,	       SIGNAL( sliderReleased() ),     this, SLOT( sliderReleased() ) );
    connect( mediaPlayerState, SIGNAL( lengthChanged(long) ),  this, SLOT( setLength(long) ) );
    connect( mediaPlayerState, SIGNAL( positionChanged(long) ),this, SLOT( setPosition(long) ) );
    connect( mediaPlayerState, SIGNAL( positionUpdated(long) ),this, SLOT( setPosition(long) ) );
    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ), this, SLOT( setLooping(bool) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled(bool) ),  this, SLOT( setPaused(bool) ) );
    connect( mediaPlayerState, SIGNAL( playingToggled(bool) ), this, SLOT( setPlaying(bool) ) );

    connect( play, SIGNAL( clicked() ), this, SLOT( (startPlaying()) ) );
//    connect( stop, SIGNAL( clicked() ), this, SLOT( (stopPlaying()) ) );
    connect( prev, SIGNAL( clicked() ), mediaPlayerState, SLOT( (setPrev()) ) );
    connect( next, SIGNAL( clicked() ), mediaPlayerState, SLOT( (setNext()) ) );
//    connect( pause, SIGNAL( toggled(bool) ), mediaPlayerState, SLOT( (setPaused(bool)) ) );
    connect( loop, SIGNAL( toggled(bool) ), mediaPlayerState, SLOT( (setLooping(bool)) ) );
    connect( up,  SIGNAL( pressed() ),  this, SLOT( (moreClicked()) ) );
    connect( dn,  SIGNAL( pressed() ),  this, SLOT( (lessClicked()) ) );
    connect( fwd, SIGNAL( pressed() ),  this, SLOT( (forwardClicked()) ) );
    connect( rew, SIGNAL( pressed() ),  this, SLOT( (backwardClicked()) ) );
    connect( up,  SIGNAL( released() ), this, SLOT( (moreReleased()) ) );
    connect( dn,  SIGNAL( released() ), this, SLOT( (lessReleased()) ) );
    connect( fwd, SIGNAL( released() ), this, SLOT( (forwardReleased()) ) );
    connect( rew, SIGNAL( released() ), this, SLOT( (backwardReleased()) ) );

    // Intialise state
    setLength( mediaPlayerState->length() );
    setPosition( mediaPlayerState->position() );
    setLooping( mediaPlayerState->looping() );
    setPaused( mediaPlayerState->paused() );
    setPlaying( mediaPlayerState->playing() );
}


MiniControls::~MiniControls()
{
}


void MiniControls::setPaused( bool b )
{
    setToggleButton( PlayButton, !b );
}


void MiniControls::setLooping( bool b )
{
    setToggleButton( LoopButton, b );
}


void MiniControls::setPlaying( bool b )
{
    setToggleButton( PlayButton, b );
}


void MiniControls::sliderPressed()
{
    sliderBeingMoved = TRUE;
}


void MiniControls::sliderReleased()
{
    if ( sliderBeingMoved ) {
	sliderBeingMoved = FALSE;
	if ( slider->width() == 0 )
	    return;
	mediaPlayerState->startTemporaryMute();
	mediaPlayerState->setPosition( ( slider->value() * mediaPlayerState->length() ) / slider->width() );
	mediaPlayerState->stopTemporaryMute( 1000 );
    }
}


void MiniControls::setPosition( long i )
{
    long length = mediaPlayerState->length();
    updateSlider( i, length );
}


void MiniControls::setLength( long max )
{
    long pos = mediaPlayerState->position();
    updateSlider( pos, max );
}


void MiniControls::updateSlider( long, long )
{
    time->setText( timeAsString( i ) + " / " + timeAsString( max ) );
    if ( max == 0 )
	return;
    // Will flicker too much if we don't do this
    // Scale to something reasonable 
    int width = slider->width();
    int val = int((double)i * width / max);
    if ( !sliderBeingMoved ) {
	if ( slider->value() != val )
	    slider->setValue( val );
	if ( slider->maxValue() != width )
	    slider->setMaxValue( width );
    }
}


QString MiniControls::timeAsString( long length )
{
    if ( mediaPlayerState->decoder() ) {
	int freq = mediaPlayerState->decoder()->audioFrequency( 0 );
	if ( freq )
	    length /= freq;
    }
    int minutes = length / 60;
    int seconds = length % 60;
    return QString("%1:%2%3").arg( minutes ).arg( seconds / 10 ).arg( seconds % 10 );
}


void MiniControls::stopPlaying()
{
    QString length = timeAsString( mediaPlayerState->length() );
    mediaPlayerState->setPlaying( FALSE );
    time->setText( "0:00 / " + length );
}


void MiniControls::startPlaying()
{
    if ( mediaPlayerState->playing() ) 
	mediaPlayerState->setPaused( !buttons[i].isDown );
    else 
	mediaPlayerState->setPlaying( TRUE );
}



void MiniControls::closeEvent( QCloseEvent* )
{
    mediaPlayerState->setList();
}


