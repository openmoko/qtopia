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
#include <qtimer.h>
#include <qframe.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpe/resource.h>
#include <qpe/mediaplayerplugininterface.h>
#include "imageutil.h"
#include "audiowidget.h"
#include "mediaplayerstate.h"


ControlWidgetBase::ControlWidgetBase( QWidget* parent, const QString &skinPath, const QString &type, const char* name ) :
    QWidget( parent, name ), buttonCount( 0 ), slider( Qt::Horizontal, this ), time( this ), sliderBeingMoved( FALSE )
{
    setCaption( tr("Media Player") );

    slider.setFixedHeight( 20 );
    slider.setMinValue( 0 );
    slider.setMaxValue( 1 );
    slider.setFocusPolicy( QWidget::NoFocus );

    time.setFocusPolicy( QWidget::NoFocus );
    time.setAlignment( Qt::AlignCenter );
    changeTextColor( &time );

    skinW = 0;
    skinH = 0;
    skinScaleW = 0;
    skinScaleH = 0;
    skin = "";
    skinName = skinPath;
    skinType = type;
    imagesLoaded = FALSE;
    imagesScaled = FALSE;
    hadFirstResize = FALSE;
    resized = FALSE;

    connect( &slider,	       SIGNAL( sliderPressed() ),      this, SLOT( sliderPressed() ) );
    connect( &slider,	       SIGNAL( sliderReleased() ),     this, SLOT( sliderReleased() ) );

    connect( mediaPlayerState, SIGNAL( skinChanged(const QString&) ),  this, SLOT( setSkin(const QString&) ) );
    connect( mediaPlayerState, SIGNAL( lengthChanged(long) ),  this, SLOT( setLength(long) ) );
    connect( mediaPlayerState, SIGNAL( positionChanged(long) ),this, SLOT( setPosition(long) ) );
    connect( mediaPlayerState, SIGNAL( positionUpdated(long) ),this, SLOT( setPosition(long) ) );
    //connect( mediaPlayerState, SIGNAL( viewChanged(View) ),    this, SLOT( setView(View) ) );
    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ), this, SLOT( setLooping(bool) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled(bool) ),  this, SLOT( setPaused(bool) ) );
    connect( mediaPlayerState, SIGNAL( playingToggled(bool) ), this, SLOT( setPlaying(bool) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled(bool) ), this, SLOT( setFullscreen(bool) ) );
}


ControlWidgetBase::~ControlWidgetBase()
{
}


void ControlWidgetBase::setSkin( const QString& skinName )
{
    if ( skin == skinName )
	return;

    qDebug("set %s skin", skinType.latin1() );

    // Don't load and scale images yet.
    // Do this a bit at a time and as needed on demand
    tmpButtonImgLoaded[0] = FALSE;
    tmpButtonImgLoaded[1] = FALSE;
    buttonPixCreated[0] = FALSE;
    buttonPixCreated[1] = FALSE;
    for ( int i = 0; i < MaxButtonTypes; i++ ) {
	tmpButtonMaskLoaded[i] = FALSE;
	buttonMaskCreated[i] = FALSE;
    }
    imagesLoaded = FALSE;
    bgImgLoaded = FALSE;

    skin = skinName;

    // setSkin can be called from selecting a skin in the menus. It looks bad
    // to load the images etc while the menu is up so we delay the loading in a timer
    if ( resized || imagesScaled )
	// If we have had a resize event we know what size to scale to and can
	// load and also scale silently in the background until we get a paintEvent
        QTimer::singleShot( 100, this, SLOT( scaleImages() ) );
    else
        QTimer::singleShot( 0, this, SLOT( loadImages() ) );
}


void ControlWidgetBase::scaleImages()
{
    resizeObjects( skinW, skinH, skinScaleW, skinScaleH );
    // Also set up the masks in the background if we can
    for ( int i = 0; i < buttonCount; i++ ) 
	getButtonMask( i );
}


void ControlWidgetBase::loadImages()
{
    if ( imagesLoaded )
	return;

    qDebug("load %s images", skinType.latin1() ); 

    QString skinPath = "mediaplayer/skins/" + skin;

    if ( !bgImgLoaded ) {
	bgPix = Resource::loadPixmap( QString("%1/background").arg(skinPath) );
	bgNeedsScaling = FALSE;
	if ( bgPix.isNull() ) 
	    bgPix = Resource::loadPixmap( QString("%1/background_tiled").arg(skinPath) );
	if ( bgPix.isNull() ) {
	    bgImg = Resource::loadImage( QString("%1/background_scaled").arg(skinPath) );
	    bgNeedsScaling = TRUE;
	}
	bgImgLoaded = TRUE;
    }
    if ( !tmpButtonImgLoaded[0] ) {
        tmpButtonImg[0] = Resource::loadImage( QString("%1/%2_up").arg(skinPath).arg(skinType) );
	tmpButtonImgLoaded[0] = TRUE;
    }
    if ( !tmpButtonImgLoaded[1] ) {
	tmpButtonImg[1] = Resource::loadImage( QString("%1/%2_down").arg(skinPath).arg(skinType) );
	tmpButtonImgLoaded[1] = TRUE;
    }
    for ( int button = 0; button < buttonCount; button++ ) {
	if ( !tmpButtonMaskLoaded[button] ) {
	    QString filename = skinPath + "/" + skinType + "_mask_" + buttons[button].skinImage;
	    QString file = Resource::findPixmap( filename );
	    if ( ( buttons[button].hasImage = !file.isNull() ) ) {
		tmpButtonMask[button] = QImage( file ).convertDepth( 1 );
		tmpButtonMaskLoaded[button] = TRUE;
	    }
	}
    }

    imagesLoaded = TRUE;

    qDebug("loaded %s images", skinType.latin1() ); 
}


void ControlWidgetBase::resizeEvent( QResizeEvent * )
{
/*
    // Don't handle the first resize event we get because
    // it is usually bogus and a waste of time scaling a stack of images
    // only to get another resize event.
    // This is made safe by the fact that if we get a paint event and
    // we haven't handled a resize yet, we then handle a resize first
    // in the paint event.
    bool oldHadFirstResize = hadFirstResize;
    hadFirstResize = TRUE;
    if ( !oldHadFirstResize )
	return;
*/
    resized = TRUE;
    internalResize();
}


void ControlWidgetBase::paintEvent( QPaintEvent *pe )
{
qDebug("paint event");
    if ( /*hadFirstResize &&*/ !resized ) {
        internalResize();
	resized = TRUE;
    }

    internalPaint( pe );

    // Set up the masks
    for ( int i = 0; i < buttonCount; i++ ) 
	getButtonMask( i );
}


int ControlWidgetBase::resizeObjects( int w, int h, int scaleW, int scaleH )
{
    qDebug("scale %s images -> %i %i %i %i", skinType.latin1(), w, h, scaleW, scaleH );
 
    loadImages();

    if ( !w || !h || !scaleW || !scaleH || !tmpButtonImg[0].width() || !tmpButtonImg[0].height() )
	return 0;

    if ( bgNeedsScaling ) 
	bgPix = bgImg.smoothScale( w, h );
    setBackgroundPixmap( bgPix );
    slider.setBackgroundOrigin( QWidget::ParentOrigin );
    slider.setBackgroundPixmap( bgPix );

    // Save parameters used
    skinW = w;
    skinH = h;
    skinScaleW = scaleW;
    skinScaleH = scaleH;
    scaleSize( scaleW, scaleH, tmpButtonImg[0].width(), tmpButtonImg[0].height(), ScaleMin );
    actualScaleW = scaleW;
    actualScaleH = scaleH;

    int border = w / 25;
    const int timeWidth = 70;
    const int timeHeight = 20;

    imgButtonMask = QImage( scaleW, scaleH, 8, 255 );
    imgButtonMask.fill( 0 );

    slider.setFixedWidth( w - timeWidth - 3 * border );
    slider.setGeometry( QRect( border, h - timeHeight - border, w - timeWidth - 2 * border, timeHeight ) );
    time.setGeometry( QRect( w - timeWidth - border, h - timeHeight - border, timeWidth, timeHeight ) );

    xoff = ( w - scaleW ) / 2; // Center buttons horizontally
    yoff = h - scaleH - 2 * border - timeHeight; // Position buttons above slider and the time lineedit

    // Force these to update
    buttonPixCreated[0] = FALSE;
    buttonPixCreated[1] = FALSE;
    for ( int i = 0; i < buttonCount; i++ ) 
	buttonMaskCreated[i] = FALSE;

    getButtonPix( FALSE );
    //getButtonPix( TRUE ); // delayed

    imagesScaled = TRUE;

    qDebug("scaled %s images", skinType.latin1() ); 
    return actualScaleH;
}


void ControlWidgetBase::setButtonData( MediaButton *mediaButtons, int count )
{
    buttons = mediaButtons;
    buttonCount = count;
    setSkin( skinName );

    // Intialise state
    setLength( mediaPlayerState->length() );
    setPosition( mediaPlayerState->position() );
    setLooping( mediaPlayerState->looping() );
    setPaused( mediaPlayerState->paused() );
    setPlaying( mediaPlayerState->playing() );
    setFullscreen( mediaPlayerState->fullscreen() );
}


void ControlWidgetBase::setPaused( bool b )
{
    // setToggleButton( PauseButton, b );
    setToggleButton( PlayButton, !b );
}


void ControlWidgetBase::setLooping( bool b )
{
    setToggleButton( LoopButton, b );
}


void ControlWidgetBase::setPlaying( bool b )
{
    setToggleButton( PlayButton, b );
}


void ControlWidgetBase::setFullscreen( bool b )
{
    setToggleButton( FullscreenButton, b );
}


void ControlWidgetBase::setToggleButton( MediaButtonType butType, bool down )
{
    for ( int i = 0; i < buttonCount; i++ )
	if ( buttons[i].buttonType == butType )
	    if ( down != buttons[i].isDown )
		toggleButton( i );
}


void ControlWidgetBase::toggleButton( int i )
{
    buttons[i].isDown = !buttons[i].isDown;
    QPainter p( this );
    paintButton( p, i );
}


void ControlWidgetBase::getButtonPix( bool down )
{
    if ( !buttonPixCreated[down] ) {
qDebug("scaling %s buttons", skinType.latin1() );
	// QImage tmpImg = fastScale( tmpButtonImg[down], actualScaleW, actualScaleH );
	QImage tmpImg = tmpButtonImg[down].smoothScale( actualScaleW, actualScaleH );
	QPoint pt( xoff, yoff );
	buttonPix[down].resize( actualScaleW, actualScaleH );
	QPainter p;
	p.begin( &buttonPix[down] );
	p.drawTiledPixmap( buttonPix[down].rect(), bgPix, pt );
	p.drawImage( 0, 0, tmpImg );
	p.end();
	buttonPixCreated[down] = TRUE;
qDebug("scaled %s buttons", skinType.latin1() );
    }
}


void ControlWidgetBase::getButtonMask( int button )
{
    if ( imgButtonMask.isNull() )
	return;

    if ( !buttonMaskCreated[button] && tmpButtonMaskLoaded[button] ) {
	buttonMaskCreated[button] = TRUE;

	if ( buttons[button].hasImage ) {
	    getButtonPix( TRUE );

	    qDebug( "creating %s %s file", skinType.latin1(), buttons[button].skinImage );

	    int w = actualScaleW;
	    int h = actualScaleH;
	    int iw = tmpButtonMask[button].width();
	    int ih = tmpButtonMask[button].height();
	    int dx = iw * 65535 / w;
	    int dy = ih * 65535 / h;

	    uchar **src = tmpButtonMask[button].jumpTable();
	    uchar **dest = imgButtonMask.jumpTable();
	    uchar bits[(((w+8)*(h+1))/8)+32]; // over allocate just to be safe
	    uchar *dst = bits - 1;
	    for ( int j = 0, y = 0; j < h; j++, y+=dy ) {
		uchar *srcLine = src[y>>16];
		uchar *line = dest[j];
		for ( int i = 0, x = 0, bit = 8; i < w; i++, x+=dx, bit++ ) {
		    int srcBit = 7 - ((x>>16) % 8);
		    int srcOfs = (x>>16) / 8;
		    uint pixel = srcLine[ srcOfs ] & (1 << srcBit);
		    if ( bit == 8 ) {
			bit = 0;
			*(++dst) = 0;
		    }
		    if ( pixel ) {
			line[i] = button + 1;
			*dst |= 1 << bit;
		    }
		}
	    }

	    buttonMask[button] = QBitmap( w, h, bits, TRUE );
	}
    }
}


void ControlWidgetBase::paintButton( QPainter& p, int i )
{
    if ( !imagesLoaded || !imagesScaled || !buttons[i].hasImage )
	return;
    getButtonMask( i );
    bool down = buttons[i].isDown; 
    buttonPix[down].setMask( buttonMask[i] );
    p.drawPixmap( xoff, yoff, buttonPix[down] );
}


void ControlWidgetBase::paintAllButtons( QPainter& p )
{
    // Get something there quick, effectively paints all buttons up
    buttonPix[0].setMask( QBitmap() );
    p.drawPixmap( xoff, yoff, buttonPix[0] );

    // Paint any down buttons
    for ( int i = 0; i < buttonCount; i++ ) {
	getButtonMask( i );
	if ( buttons[i].isDown )
	    paintButton( p, i );
    }
}


void ControlWidgetBase::sliderPressed()
{
    sliderBeingMoved = TRUE;
}


void ControlWidgetBase::sliderReleased()
{
    if ( sliderBeingMoved ) {
	sliderBeingMoved = FALSE;
	if ( slider.width() == 0 )
	    return;
	mediaPlayerState->startTemporaryMute();
	// Use double precision to avoid integer overflows which can happen easily otherwise
	long val = long((double)slider.value() * mediaPlayerState->length() / slider.width());
	mediaPlayerState->setPosition( val );
	mediaPlayerState->stopTemporaryMute( 1000 );
    }
}


void ControlWidgetBase::setPosition( long i )
{
    long length = mediaPlayerState->length();
    updateSlider( i, length );
}


void ControlWidgetBase::setLength( long max )
{
    long pos = mediaPlayerState->position();
    updateSlider( pos, max );
}


void ControlWidgetBase::updateSlider( long, long )
{
}


QString ControlWidgetBase::timeAsString( long length )
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


void ControlWidgetBase::updateSliderBase( long i, long max )
{
    time.setText( timeAsString( i ) + " / " + timeAsString( max ) );
    if ( max == 0 )
	return;
    // Will flicker too much if we don't do this
    // Scale to something reasonable 
    int width = slider.width();
    int val = int((double)i * width / max);
    if ( !sliderBeingMoved ) {
	if ( slider.value() != val )
	    slider.setValue( val );
	if ( slider.maxValue() != width )
	    slider.setMaxValue( width );
    }
}


void ControlWidgetBase::stopPlaying()
{
    QString length = timeAsString( mediaPlayerState->length() );
    mediaPlayerState->setPlaying( FALSE );
    time.setText( "0:00 / " + length );
}


void ControlWidgetBase::mouseMoveEvent( QMouseEvent *event )
{
    for ( int i = 0; i < buttonCount; i++ ) {
	if ( buttons[i].hasImage ) {
	    if ( event->state() == QMouseEvent::LeftButton ) {

		// The test to see if the mouse click is inside the button or not
		int x = event->pos().x() - xoff;
		int y = event->pos().y() - yoff;

		bool isOnButton = ( x > 0 && y > 0 && x < imgButtonMask.width()
		    && y < imgButtonMask.height() && imgButtonMask.pixelIndex( x, y ) == i + 1 );

		if ( isOnButton && !buttons[i].isHeld ) {
		    buttons[i].isHeld = TRUE;
		    toggleButton(i);
		    switch ( buttons[i].buttonType ) {
			case VolumeUpButton:	    emit moreClicked(); return;
			case VolumeDownButton:	    emit lessClicked(); return;
			case ForwardButton:	    emit forwardClicked(); return;
			case BackwardButton:	    emit backwardClicked(); return;
			default: break;
		    }
		} else if ( !isOnButton && buttons[i].isHeld ) {
		    buttons[i].isHeld = FALSE;
		    toggleButton(i);
		    switch ( buttons[i].buttonType ) {
			case VolumeUpButton:	    emit moreReleased(); return;
			case VolumeDownButton:	    emit lessReleased(); return;
			case ForwardButton:	    emit forwardReleased(); return;
			case BackwardButton:	    emit backwardReleased(); return;
			default: break;
		    }
		}
	    } else {
		if ( buttons[i].isHeld ) {
		    buttons[i].isHeld = FALSE;
		    if ( !buttons[i].isToggle && buttons[i].isDown )
			toggleButton( i );
		    switch ( buttons[i].buttonType ) {
			case PlayButton:	    if ( mediaPlayerState->playing() ) 
							mediaPlayerState->setPaused( !buttons[i].isDown );
						    else 
						        mediaPlayerState->setPlaying( TRUE );
						    return;
			case StopButton:	    stopPlaying(); return;
			case PauseButton:	    mediaPlayerState->setPaused( buttons[i].isDown ); return;
			case NextButton:	    mediaPlayerState->setNext(); return;
			case PreviousButton:	    mediaPlayerState->setPrev(); return;
			case LoopButton:	    mediaPlayerState->setLooping( buttons[i].isDown ); return;
			case VolumeUpButton:	    emit moreReleased(); return;
			case VolumeDownButton:	    emit lessReleased(); return;
			case ForwardButton:	    emit forwardReleased(); return;
			case BackwardButton:	    emit backwardReleased(); return;
			case PlayListButton:	    mediaPlayerState->setList();  return;
			//case FullscreenButton:	mediaPlayerState->setFullscreen( TRUE ); makeVisible(); return;
			default: break;
		    }
		}
	    }
	}
    }
}


void ControlWidgetBase::mousePressEvent( QMouseEvent *event )
{
    mouseMoveEvent( event );
}


void ControlWidgetBase::mouseReleaseEvent( QMouseEvent *event )
{
    mouseMoveEvent( event );
}


void ControlWidgetBase::closeEvent( QCloseEvent* )
{
    mediaPlayerState->setList();
}


