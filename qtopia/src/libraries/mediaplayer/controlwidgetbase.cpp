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
#include "imageutil.h"
#include "mediaplayerstate.h"
#include "controlwidgetbase.h"
#include "audiodevice.h"

#include <qtopia/config.h>
#include <qtopia/mediaplayerplugininterface.h>
#include <qtopia/resource.h>
#include <qtopia/contextbar.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qbutton.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qwidget.h>

#include <stdlib.h>
#include <stdio.h>


/*!
  \class ControlWidgetBase controlwidgetbase.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    It is the base class used to make the skinned audio and video
    views.
*/


ControlWidgetBase::ControlWidgetBase( QWidget* parent, const QString &skinPath,
						const QString &type, const char* name ) :
    QWidget(parent, name), buttonCount(0), slider(Qt::Horizontal, this),
    time(this), sliderBeingMoved(false), volumeTimeout(this), drawVolume(false)
{
    setCaption( tr("Media Player") );

//    setMouseTracking( TRUE );

    slider.setFixedHeight( 20 );
    slider.setMinValue( 0 );
    slider.setMaxValue( 1 );
    slider.setFocusPolicy( QWidget::NoFocus );

    time.setFocusPolicy( QWidget::NoFocus );
    changeTextColor( &time );

    skinW = 0;
    skinH = 0;
    skinScaleW = 0;
    skinScaleH = 0;
    skin = "";
    skinFile = skinPath;
    skinType = type;
    imagesLoaded = FALSE;
    imagesScaled = FALSE;
    hadFirstResize = FALSE;
    resized = FALSE;
    canPaint = FALSE;
    actualScaleW = 0;
    actualScaleH = 0;

    connect( &slider,	       SIGNAL( sliderPressed() ),      this, SLOT( sliderPressed() ) );
    connect( &slider,	       SIGNAL( sliderReleased() ),     this, SLOT( sliderReleased() ) );

    connect( mediaPlayerState, SIGNAL( skinChanged(const QString&) ),  this, SLOT( setSkin(const QString&) ) );
    connect( mediaPlayerState, SIGNAL( lengthChanged(long) ),  this, SLOT( setLength(long) ) );
    connect( mediaPlayerState, SIGNAL( positionChanged(long) ),this, SLOT( setPosition(long) ) );
    connect( mediaPlayerState, SIGNAL( positionUpdated(long) ),this, SLOT( setPosition(long) ) );
    connect( mediaPlayerState, SIGNAL( seekableToggled(bool) ),this, SLOT( setSeekable(bool) ) );
    //connect( mediaPlayerState, SIGNAL( viewChanged(View) ),    this, SLOT( setView(View) ) );
    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ), this, SLOT( setLooping(bool) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled(bool) ),  this, SLOT( setPaused(bool) ) );
    connect( mediaPlayerState, SIGNAL( playingToggled(bool) ), this, SLOT( setPlaying(bool) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled(bool) ), this, SLOT( setFullscreen(bool) ) );

    connect( this, SIGNAL( moreClicked() ),      mediaPlayerState, SIGNAL( increaseVolume() ) );
    connect( this, SIGNAL( lessClicked() ),      mediaPlayerState, SIGNAL( decreaseVolume() ) );
    connect( this, SIGNAL( moreReleased() ),     mediaPlayerState, SIGNAL( endIncreaseVolume() ) );
    connect( this, SIGNAL( lessReleased() ),     mediaPlayerState, SIGNAL( endDecreaseVolume() ) );

    connect( this, SIGNAL( forwardClicked() ),   mediaPlayerState, SIGNAL( scanForward() ) );
    connect( this, SIGNAL( backwardClicked() ),  mediaPlayerState, SIGNAL( scanBackward() ) );
    connect( this, SIGNAL( forwardReleased() ),  mediaPlayerState, SIGNAL( endScanForward() ) );
    connect( this, SIGNAL( backwardReleased() ), mediaPlayerState, SIGNAL( endScanBackward() ) );

    connect( &volumeTimeout, SIGNAL(timeout()), this, SLOT(hideVolumeOverlay()) );

    setFocusPolicy( QWidget::StrongFocus );
}


ControlWidgetBase::~ControlWidgetBase()
{
}


void ControlWidgetBase::setSkin( const QString& newskin )
{
    if ( skin == newskin )
	return;

    qDebug("set %s skin", skinType.latin1() );

    // Don't load and scale images yet.
    // Do this a bit at a time and as needed on demand
    mediaButtonImg[0].loaded = FALSE;
    mediaButtonImg[0].scaled = FALSE;
    mediaButtonImg[1].loaded = FALSE;
    mediaButtonImg[1].scaled = FALSE;
    for (int button = 0; button < MaxButtonTypes; button++) {
	mediaButtonMask[button].loaded = FALSE;
	mediaButtonMask[button].scaled = FALSE;
    }
    background.loaded = FALSE;
    imagesLoaded = FALSE;

    skin = newskin;

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

    //qDebug("load %s images", skinType.latin1() ); 

    QString skinPath = "mediaplayer/skins/" + skin + "/";
    // Using findPixmap is a bit of an abuse
    Config cfg(Resource::findPixmap(skinPath+"config"), Config::File);
    cfg.setGroup("Skin");

    if ( !background.loaded ) {
	background.scaledPix = Resource::loadPixmap( skinPath+cfg.readEntry("background", "background") );
	bgNeedsScaling = FALSE;
	if ( background.scaledPix.isNull() )
	    background.scaledPix = Resource::loadPixmap( skinPath+cfg.readEntry("background_tiled", "background_tiled") );
	if ( background.scaledPix.isNull() ) {
	    background.unscaledImg = Resource::loadImage( skinPath+cfg.readEntry("background_scaled", "background_scaled") );
	    bgNeedsScaling = TRUE;
	}
	background.loaded = TRUE;
    }
    if ( !mediaButtonImg[0].loaded ) {
        QString filename = skinType+"_up";
        mediaButtonImg[0].unscaledImg = Resource::loadImage( skinPath+cfg.readEntry(filename, filename) );
	int xOff = cfg.readNumEntry(filename+"_x",0);
	int yOff = cfg.readNumEntry(filename+"_y",0);
	mediaButtonImg[0].offset = QPoint(xOff, yOff);
	mediaButtonImg[0].loaded = TRUE;
    }
    if ( !mediaButtonImg[1].loaded ) {
        QString filename = skinType+"_down";
        mediaButtonImg[1].unscaledImg = Resource::loadImage( skinPath+cfg.readEntry(filename, filename) );
	int xOff = cfg.readNumEntry(filename+"_x",0);
	int yOff = cfg.readNumEntry(filename+"_y",0);
	mediaButtonImg[1].offset = QPoint(xOff, yOff);
	mediaButtonImg[1].loaded = TRUE;
    }
    for ( int button = 0; button < buttonCount; button++ ) {
	if ( !mediaButtonMask[button].loaded ) {
	    QString filename = skinType + "_mask_" + buttons[button].skinImage; // No tr
	    int maskXOff = cfg.readNumEntry(filename+"_x",0);
	    int maskYOff = cfg.readNumEntry(filename+"_y",0);
	    filename = skinPath+cfg.readEntry(filename,filename);
	    QString file = Resource::findPixmap( filename );
	    if ( ( buttons[button].hasImage = !file.isNull() ) ) {
		mediaButtonMask[button].unscaledImg = QImage( file ).convertDepth( 1 );
		mediaButtonMask[button].offset = QPoint(maskXOff, maskYOff);
		mediaButtonMask[button].loaded = TRUE;
	    }
	}
    }

    imagesLoaded = TRUE;

    //qDebug("loaded %s images", skinType.latin1() ); 
}


void ControlWidgetBase::resetButtons()
{
    emit moreReleased(); 
    emit lessReleased();
    emit forwardReleased();
    emit backwardReleased();

    // Reset these buttons to unclicked 
    for ( int i = 0; i < buttonCount; i++ ) {
	switch ( buttons[i].buttonType ) {
	    case VolumeUpButton:
	    case VolumeDownButton:
	    case ForwardButton:
	    case BackwardButton:
		buttons[i].isDown = FALSE;
		buttons[i].isHeld = FALSE;
	    default:
		break;
	}
    }
}


void ControlWidgetBase::keyPressEvent( QKeyEvent *e )
{
    // Ignore all auto repeats
    if ( e->isAutoRepeat() )
	return;

    switch( e->key() ) {
	case Key_Left:
	    setToggleButton(BackwardButton,true);
	    emit backwardClicked();
	    return;
	case Key_Right:
	    setToggleButton(ForwardButton,true);
	    emit forwardClicked();
	    return;
	case Key_Up:
	    setToggleButton(VolumeUpButton,true);
	    emit moreClicked();
	    return;
	case Key_Down:
	    setToggleButton(VolumeDownButton,true);
	    emit lessClicked();
	    return;
#ifdef QTOPIA_PHONE
	case Key_Select:
#endif
	case Key_Enter:
	case Key_Space:
	    mediaPlayerState->setPaused( !mediaPlayerState->paused() );
	    return;
#ifdef QTOPIA_PHONE
	case Key_No:
	case Key_Back:
#endif
	case Key_Escape:
	    mediaPlayerState->closeView();
	    return;
	default:
	    break;
    }
}


void ControlWidgetBase::keyReleaseEvent( QKeyEvent *e )
{
    // Ignore all auto repeats
    if ( e->isAutoRepeat() )
	return;

    switch( e->key() ) {
	case Key_Left:
	    setToggleButton(BackwardButton,false);
            emit backwardReleased();
	    break;
	case Key_Right:
	    setToggleButton(ForwardButton,false);
            emit forwardReleased();
	    break;
	case Key_Up:
	    setToggleButton(VolumeUpButton,false);
            emit moreReleased();
	    return;
	case Key_Down:
	    setToggleButton(VolumeDownButton,false);
            emit lessReleased();
	    return;
	default:
	    break;
    }
}


void ControlWidgetBase::resizeEvent( QResizeEvent * )
{
    resized = TRUE;
    virtualResize();
}


static void paintIcon( QPainter& p, const char *icon, int x, int y, int size )
{
    QIconSet playIcon = Resource::loadIconSet(icon);
    QPixmap playPix = playIcon.pixmap(QIconSet::Large, QIconSet::Normal);
    QImage playImg = playPix.convertToImage();
    playImg = playImg.smoothScale(size, size);
    p.drawImage(x, y, playImg);
}


void ControlWidgetBase::paintEvent( QPaintEvent *pe )
{
    if ( !resized ) {
        virtualResize();
	resized = TRUE;
    }

    virtualPaint( pe );

    if ( mediaButtonImg[0].scaledPix.isNull() ) {
	QPainter p(this);
	p.setPen( QPen(Qt::black, 1) );
	int radius = QMIN(skinScaleW, skinScaleH) & ~2;
	p.drawEllipse( xoff, yoff, radius, radius );
	p.drawEllipse( xoff + radius/4, yoff + radius/4, radius/2, radius/2 );
	int iconSize = radius * 10 / 48; // divide by approximately sqrt(2)
	int centX = xoff + radius/2 - iconSize/2;
	int centY = yoff + radius/2 - iconSize/2;
	int space = radius*3/8;
	paintIcon(p, "fastback", centX - space, centY, iconSize);
	paintIcon(p, "play", centX, centY, iconSize);
	paintIcon(p, "fastforward", centX + space, centY, iconSize);
	paintIcon(p, "volume", centX, centY - space, iconSize);
	paintIcon(p, "volume", centX, centY + space, iconSize);
    }

    drawOverlays();

    // Set up the masks
    for ( int i = 0; i < buttonCount; i++ ) 
	getButtonMask( i );
}


static bool getPoint(Config &cfg, QString name, QPoint &pnt, int w, int h)
{
    int x = cfg.readNumEntry(name+"_x",0x666666);
    int y = cfg.readNumEntry(name+"_y",0x666666);
    if (x < 0)
	x = QMAX(w + x, 0);
    if (y < 0)
	y = QMAX(h + y, 0);
    if (x != 0x666666 && y != 0x666666) {
	pnt = QPoint(x, y);
	return true;
    }
    return false;
}


static bool getRect(Config &cfg, QString name, QRect &rect, int w, int h)
{
    QPoint tl = rect.topLeft();
    QPoint br = rect.bottomRight();
    if ( getPoint(cfg, name+"_tl", tl, w, h) && getPoint(cfg, name+"_br", br, w, h) ) {
	rect = QRect(tl, br);
	return true;
    }
    return false;
}


bool ControlWidgetBase::videoGeometry(QRect &geometry)
{
    QString skinPath = "mediaplayer/skins/" + skin + "/";
    Config cfg(Resource::findPixmap(skinPath+"config"), Config::File);
    cfg.setGroup("Skin");
    return getRect(cfg, skinType+"_output", geometry, width(), height());
}


void ControlWidgetBase::resizeObjects( int w, int h, int scaleW, int scaleH, int cornerWidgetWidth )
{
    //qDebug("scale %s images -> %i %i %i %i", skinType.latin1(), w, h, scaleW, scaleH );
 
    loadImages();

    if ( !w || !h || !scaleW || !scaleH )
	return;

    if ( bgNeedsScaling && !background.unscaledImg.isNull() ) 
	background.scaledPix = background.unscaledImg.smoothScale( w, h );
    if ( !background.scaledPix.isNull() ) {
	setBackgroundPixmap( background.scaledPix );
	slider.setBackgroundOrigin( QWidget::ParentOrigin );
	slider.setBackgroundPixmap( background.scaledPix );
    }

    QString skinPath = "mediaplayer/skins/" + skin + "/";
    Config cfg(Resource::findPixmap(skinPath+"config"), Config::File);
    cfg.setGroup("Skin");

    int buttonsW, buttonsH;
    if ( !mediaButtonImg[0].unscaledImg.isNull() ) {
	buttonsW = mediaButtonImg[0].unscaledImg.width();
	buttonsH = mediaButtonImg[0].unscaledImg.height();
    } else {
	if ( skinType == "video" )
	    scaleW = scaleH = 1;
	buttonsW = buttonsH = 1;
    }
    scaleSize( scaleW, scaleH, buttonsW, buttonsH, ScaleMin );

    // Save parameters used
    skinW = w;
    skinH = h;
    skinScaleW = scaleW;
    skinScaleH = scaleH;

    int border = w / 50;
    QFontMetrics fm( qApp->font() );
    int timeWidth = fm.width( " 00:00 / 00:00 " );
    int timeHeight = 20; // fm.height();

    xoff = ( w - scaleW ) / 2; // Center buttons horizontally
    yoff = h - scaleH - timeHeight - 4*border; // Position buttons above slider and the time lineedit

    QRect skinButtonGeometry(xoff, yoff, scaleW, scaleH);
    getRect(cfg, skinType+"_button", skinButtonGeometry, w, h);
    xoff = skinButtonGeometry.x();
    yoff = skinButtonGeometry.y();
    scaleW = skinButtonGeometry.width();
    scaleH = skinButtonGeometry.height();
    scaleSize( scaleW, scaleH, buttonsW, buttonsH, ScaleMin );
    actualScaleW = scaleW;
    actualScaleH = scaleH;

    imgButtonMask = QImage( scaleW, scaleH, 8, 255 );
    imgButtonMask.fill( 0 );

    int y1 = h - timeHeight - 2 * border;
    int x2 = w - timeWidth - border - cornerWidgetWidth;
    QRect sliderGeometry( border, y1, x2 - 2*border, timeHeight );
    QRect timeGeometry( x2, y1, timeWidth, timeHeight );
    getRect(cfg, skinType+"_slider", sliderGeometry, w, h);
    getRect(cfg, skinType+"_time", timeGeometry, w, h);

    slider.setGeometry(sliderGeometry);
    time.setGeometry(timeGeometry);

    // Force these to update
    mediaButtonImg[0].scaled = FALSE;
    mediaButtonImg[1].scaled = FALSE;
    for (int button = 0; button < buttonCount; button++) 
	mediaButtonMask[button].scaled = FALSE;

    getButtonPix( FALSE );
    //getButtonPix( TRUE ); // delayed

    imagesScaled = TRUE;
    //qDebug("scaled %s images", skinType.latin1() ); 
}


void ControlWidgetBase::setButtonData( MediaButton *mediaButtons, int count )
{
    for (int butType = 0; butType < MaxButtonTypes; butType++) {
	buttonMap[butType] = -1;
	for (int i = 0; i < count; i++)
	    if ( mediaButtons[i].buttonType == butType )
		buttonMap[butType] = i;
    }
    buttons = mediaButtons;
    buttonCount = count;
    setSkin( skinFile );

    // Intialise state
    setLength( mediaPlayerState->length() );
    setPosition( mediaPlayerState->position() );
    setLooping( mediaPlayerState->looping() );
    setPaused( mediaPlayerState->paused() );
    setPlaying( mediaPlayerState->playing() );
    setSeekable( mediaPlayerState->seekable() );
    setFullscreen( mediaPlayerState->fullscreen() );
}


void ControlWidgetBase::setPaused( bool b )
{
    // setToggleButton( PauseButton, b );
    setToggleButton( PlayButton, !b );

#ifdef QTOPIA_PHONE
    QString text = "";
    QString pixmap;
    if (!mediaPlayerState->seeking()) {
	if (b) {
	    text = mediaPlayerState->playing() ? tr("Resume") : tr("Play", "play music or sound");
	    pixmap = "play";
	} else {
	    text = tr("Pause");
	    pixmap = "pause";
	}
	ContextBar::setLabel( this, Key_Select, pixmap, text );
    } else {
	ContextBar::setLabel( this, Key_Select, ContextBar::NoLabel);
    }
#endif
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


void ControlWidgetBase::setSeekable( bool b )
{
    slider.setEnabled( b );
    slider.setDisabled( !b );
    if ( b ) {
	slider.unsetPalette();
    } else {
	QColorGroup cg;
	for (int role = 0; role < QColorGroup::NColorRoles; role++) {
	    QColor color = slider.palette().color( QPalette::Normal, (QColorGroup::ColorRole)role );
	    cg.setColor( (QColorGroup::ColorRole)role, color.light(300) );
	}
	slider.setPalette( QPalette(cg,cg,cg) );
    }
}


void ControlWidgetBase::setToggleButton( MediaButtonType butType, bool down )
{
    int i = buttonMap[butType];
    if ( i != -1 && down != buttons[i].isDown )
	toggleButton( i );
}


void ControlWidgetBase::toggleButton( int i )
{
    buttons[i].isDown = !buttons[i].isDown;
    QPainter p( this );
    paintButton( p, i );
    drawOverlays();
}


void ControlWidgetBase::getButtonPix( bool down )
{
    if ( !mediaButtonImg[down].scaled && !mediaButtonImg[down].unscaledImg.isNull() ) {
	// QImage tmpImg = fastScale( mediaButtonImg[down], actualScaleW, actualScaleH );
	QImage tmpImg = mediaButtonImg[down].unscaledImg.smoothScale( actualScaleW, actualScaleH );
	if (!tmpImg.isNull()) {
	    QPoint pt( xoff, yoff );
	    mediaButtonImg[down].scaledPix.resize( actualScaleW, actualScaleH );
	    QPainter p;
	    p.begin( &mediaButtonImg[down].scaledPix );
	    p.drawTiledPixmap( mediaButtonImg[down].scaledPix.rect(), background.scaledPix, pt );
	    p.drawImage( mediaButtonImg[down].offset, tmpImg );
	    p.end();
	    mediaButtonImg[down].scaled = TRUE;
	    //qDebug("scaled %s buttons", skinType.latin1() );
	}
    }
}


void ControlWidgetBase::getButtonMask( int button )
{
    if ( imgButtonMask.isNull() || mediaButtonImg[0].unscaledImg.isNull() )
	return;

    if ( !mediaButtonMask[button].scaled && mediaButtonMask[button].loaded ) {
	mediaButtonMask[button].scaled = TRUE;

	if ( buttons[button].hasImage ) {
	    getButtonPix( TRUE );

	    //qDebug( "creating %s %s file", skinType.latin1(), buttons[button].skinImage );

            int srcW = mediaButtonImg[0].unscaledImg.width();
            int srcH = mediaButtonImg[0].unscaledImg.height();
            QImage tmpImg(srcW, srcH, 1, 2, QImage::BigEndian);
            tmpImg.fill(0);
	    int x1 = mediaButtonMask[button].offset.x();
	    int y1 = mediaButtonMask[button].offset.y();
            int w1 = mediaButtonMask[button].unscaledImg.width();
            int h1 = mediaButtonMask[button].unscaledImg.height();
            //### This loop ideally needs optimizing away by rolling this in to the loop below
            for (int j1 = 0; j1<h1 && j1+y1<srcH; j1++)
                for (int i1 = 0; i1<w1 && i1+x1<srcW; i1++)
                      tmpImg.setPixel(i1+x1,j1+y1,mediaButtonMask[button].unscaledImg.pixelIndex(i1,j1));

	    int w = actualScaleW;
	    int h = actualScaleH;
	    int iw = srcW;
	    int ih = srcH;
	    int dx = iw * 65535 / w;
	    int dy = ih * 65535 / h;
	    uchar **src = tmpImg.jumpTable();
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

	    mediaButtonMask[button].scaledPix = QBitmap( w, h, bits, TRUE );
	}
    }
}


void ControlWidgetBase::paintButton( QPainter& p, int button )
{
    if ( !canPaint || !imagesLoaded || !imagesScaled || !buttons[button].hasImage )
	return;
    getButtonMask(button);
    bool down = buttons[button].isDown; 
    if ( !mediaButtonMask[button].scaledPix.isNull() && mediaButtonMask[button].scaledPix.isQBitmap() )
        mediaButtonImg[down].scaledPix.setMask( *(QBitmap*)&mediaButtonMask[button].scaledPix );
    if ( !mediaButtonImg[down].scaledPix.isNull() )
        p.drawPixmap( xoff, yoff, mediaButtonImg[down].scaledPix );
}


void ControlWidgetBase::drawOverlays()
{
    if ( drawVolume )
	drawVolumeOverlay(true);
}


void ControlWidgetBase::updateVolume()
{
    // Display an on-screen display volume
    if ( drawVolume == true )
	hideVolumeOverlay();
    drawVolume = true;
    drawVolumeOverlay(false);
    volumeTimeout.start(1000, true);
}


void ControlWidgetBase::drawVolumeOverlay(bool inPaintEvent)
{
    const int volYOff = 90;

    int w = width();
    int h = height();

    int vW = w * 90 / 100;
    int mW = vW / 10;
    int mH = mW * 3 / 2;
    int tickWidth = mW / 2;

    QPainter p(this);
    p.setPen( QColor( 0x10, 0xD0, 0x10 ) );
    p.setBrush( QColor( 0x10, 0xD0, 0x10 ) );

    QFont f;
    f.setPixelSize( mW );
    f.setBold( TRUE );
    p.setFont( f );
    p.drawText( (w - vW) / 2, h - volYOff + 20 - mH, tr("Volume") );

    int l, r;
    AudioDevice::volume(l, r);
    int newVolume = ((l + r) * 11) / (2*0x10000);
    for ( int i = 0; i < 10; i++ ) {
	if ( newVolume > i ) {
	    p.drawRect( (w - vW) / 2 + i * mW + 10, h - volYOff + 40 - mH, tickWidth, mH );
	} else {
	    if ( i + 1 == drawnVolume && !inPaintEvent )
		repaint( (w - vW) / 2 + i * mW + 10, h - volYOff + 40 - mH, tickWidth, mH );
	    p.drawRect( (w - vW) / 2 + i * mW + 13, h - volYOff + 40 - mH + mW/2, tickWidth / 2, mH - mW );
	}
    }
    drawnVolume = newVolume;

    if ( AudioDevice::muted() ) {
	f.setPixelSize( mH );
	p.setFont( f );
	p.setPen( QColor( 0xD0, 0x10, 0x10 ) );
	p.drawText( (w - vW) / 2 + 10, h - volYOff + 40 - mH, 
	    10*mW + tickWidth, mH, AlignHCenter | AlignVCenter, tr("Muted") );
    }
}


void ControlWidgetBase::hideVolumeOverlay()
{
    if ( drawVolume == false )
	return;
    drawVolume = false;
    drawnVolume = -1;
    // Get rid of the on-screen display stuff
    repaint(background.scaledPix.isNull());
}


void ControlWidgetBase::paintAllButtons( QPainter& p )
{
    // Get something there quick, effectively paints all buttons up
    if ( !mediaButtonImg[0].scaledPix.isNull() ) {
	mediaButtonImg[0].scaledPix.setMask( QBitmap() );
        p.drawPixmap( xoff, yoff, mediaButtonImg[0].scaledPix );
    }

    canPaint = TRUE;

    // Paint any down buttons
    for ( int i = 0; i < buttonCount; i++ ) {
	getButtonMask( i );
	if ( buttons[i].isDown ) 
	    paintButton( p, i );
    }
}


void ControlWidgetBase::sliderPressed()
{
    mediaPlayerState->setSeeking( true );
    sliderBeingMoved = TRUE;
}


void ControlWidgetBase::sliderReleased()
{
    if ( sliderBeingMoved ) {
	sliderBeingMoved = FALSE;
	if ( slider.width() == 0 )
	    return;
	// Use double precision to avoid integer overflows which can happen easily otherwise
	long val = long((double)slider.value() * mediaPlayerState->length() / slider.width());
	mediaPlayerState->setPosition( val );
	mediaPlayerState->setSeeking( false );
    }
}


void ControlWidgetBase::setPosition( long )
{
    virtualUpdateSlider();
}


void ControlWidgetBase::setLength( long )
{
    virtualUpdateSlider();
}


QString ControlWidgetBase::toTimeString( long length )
{
    int minutes = length / 60;
    int seconds = length % 60;
    return QString("%1:%2%3").arg( minutes ).arg( seconds / 10 ).arg( seconds % 10 );
}


QString ControlWidgetBase::timeAsString( long length )
{
    if ( mediaPlayerState->decoder() ) {
	if ( mediaPlayerState->decoder()->videoStreams() > 0 ) {
	    double frameRate = mediaPlayerState->decoder()->videoFrameRate( 0 );
	    if ( frameRate == 0.0 )
		frameRate = 1.0;
	    length = long((double)length / frameRate);
	} else {
	    int freq = mediaPlayerState->decoder()->audioFrequency( 0 );
	    if ( freq )
		length /= freq;
	}
    }
    return toTimeString( length );
}

 
void ControlWidgetBase::updateSlider()
{
    int i = mediaPlayerState->position();
    int max = mediaPlayerState->length();

    QString currentTime;
    QString totalTime;

    // Work out currentTime and totalTime strings
    if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	if ( decoder->currentTimeAvailable() ) 
	    currentTime = toTimeString( decoder->currentTime() / 1000 );
	else
	    currentTime = "?:??";
	if ( decoder->totalTimeAvailable() )
	    totalTime = " / " + toTimeString( decoder->totalTime() / 1000 );
    } else {
	currentTime = timeAsString( i );
	if ( max > 1 )
	    totalTime = " / " + timeAsString( max );
    }

    QString text = currentTime + totalTime;
    if ( text.length() > 12 )
        time.setText( currentTime );
    else
        time.setText( text );

    if ( max == 0 )
	return;

    // Will flicker too much if we don't do this.
    // Scale position to the pixel along the slider.
    int width = slider.width();
    int val = int((double)i * width / max);
    if ( !sliderBeingMoved ) {
	if ( slider.value() != val )
	    slider.setValue( val );
	if ( slider.maxValue() != width )
	    slider.setMaxValue( width );
    }
}


void ControlWidgetBase::stop()
{
    mediaPlayerState->setPlaying( false );
    setPosition( 0 );
    slider.setValue( 0 );
}


void ControlWidgetBase::play()
{
    if ( mediaPlayerState->playing() )
	mediaPlayerState->togglePaused();
    else
	mediaPlayerState->setPlaying( true );
}


void ControlWidgetBase::mouseMoveEvent( QMouseEvent *event )
{
    if ( imgButtonMask.isNull() )
	return;

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
			case PlayButton:	    play(); return;
			case StopButton:	    stop(); return;
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
    mediaPlayerState->closeView();
}

