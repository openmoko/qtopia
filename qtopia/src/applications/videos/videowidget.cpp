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
#include <qtopia/resource.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
#endif
#include <qtopia/mediaplayerplugininterface.h>
#include <qtopia/resource.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qdrawutil.h>
#include "action.h"
#include "imageutil.h"
#include "mediaplayerstate.h"
#include "videowidget.h"


#ifdef Q_WS_QWS
# define HAVE_PIXMAP_BITS
# define USE_DIRECT_PAINTER
# include <qdirectpainter_qws.h>
# include <qgfxraster_qws.h>
#endif


// Layout information for the videoButtons (and if it is a toggle button or not)
MediaButton videoButtons[] = {
    { FALSE, FALSE, FALSE, FALSE, "previous",   PreviousButton   }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "backward",   BackwardButton   }, // No tr
    {  TRUE, FALSE, FALSE, FALSE, "play",       PlayButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "forward",    ForwardButton    }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "next",       NextButton       }, // No tr
    {  TRUE, FALSE, FALSE, FALSE, "loop",       LoopButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "stop",	StopButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "up",		VolumeUpButton   }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "down",	VolumeDownButton }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "playlist",	PlayListButton   }, // No tr
    // {  TRUE, FALSE, FALSE, FALSE, "fullscreen", FullscreenButton },
};



VideoOutput::VideoOutput(VideoWidget* parent)
    : VideoOutputWidget(parent), parentWidget(parent)
{
}


VideoOutput::~VideoOutput()
{
}


void VideoOutput::paintEvent( QPaintEvent *pe )
{
    // Previously the code would try to draw the a video frame, but this
    // has potential race conditions. Events causing repaints would also
    // cause it to move to the next frame early, and this would
    // be a problem if paused and then repaint events are generated, it
    // would slowly move a frame forward at a time. There is no easy
    // solution when direct painting is involved because we don't have
    // stored the last generated frame, and with some decoders it is
    // rather problematic to try and backup the video one frame, consider
    // the case of streamed video. Direct painting where possible is
    // worth it so this compromise is needed. The effect will be that if
    // the video is paused and a repaint occurs, the screen will be blanked,
    // the current frame is not held on the screen.
    // playNextVideoFrame();
    QWidget::paintEvent(pe);
}


void VideoOutput::mouseReleaseEvent( QMouseEvent * )
{
    // Effectively blank the view next time we show it
//    targetRect = QRect( 0, 0, 0, 0 );
    parentWidget->setNextMode();
}


void VideoOutput::keyPressEvent( QKeyEvent * )
{
    parentWidget->setNextMode();
}


VideoWidget::VideoWidget(QWidget* parent, const QString& skin, const char* name ) :
    ControlWidgetBase( parent, skin, "video", name ),  // No tr
    cornerButton(this), cornerMenu(0), screenMode(InvalidMode), videoOutput(this)
{
    setButtonData( videoButtons, sizeof(videoButtons)/sizeof(MediaButton) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ),    this, SLOT( setView(View) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled(bool) ), this, SLOT( setFullscreen(bool) ) );

    setMode( mediaPlayerState->fullscreen() ? Fullscreen : Normal );

//    slider.setEnabled( mediaPlayerState->decoderIsSeekable() );

    cornerButton.setPixmap( Resource::loadPixmap( "videos/turnup" ) );
    cornerButton.setFlat( TRUE );
    cornerButton.setFocusPolicy( QWidget::NoFocus );
    connect( &cornerButton, SIGNAL( clicked() ), this, SLOT( doModeMenu() ) );

    cornerMenu.insertItem( tr("Fullscreen"), Fullscreen, Fullscreen );
    cornerMenu.insertItem( tr("Large"), Large, Large );
    cornerMenu.insertItem( tr("Normal"), Normal, Normal );
    connect( &cornerMenu, SIGNAL( activated(int) ), this, SLOT( setMode(int) ) );

#ifdef QTOPIA_PHONE
    // Create context menu
    ContextMenu *contextMenu = new ContextMenu( this, 0, ContextMenu::Modal );

    ContextBar::setLabel( this, Key_Select, "pause", tr("Pause") );

//    Action *tbInfo    = new Action( this, tr( "Information" ),    "info",		    this,		SLOT(showInfo()) );
//    Action *tbShuffle = new Action( this, tr( "Randomize" ),  "mediaplayer/shuffle", mediaPlayerState, SLOT(setShuffled(bool)), TRUE );
    Action *tbLoop    = new Action( this, tr( "Loop" ),       "mediaplayer/loop",   mediaPlayerState, SLOT(setLooping(bool)), TRUE );
    Action *tbFull    = new Action( this, tr( "Fullscreen" ), "fullscreen",	    mediaPlayerState, SLOT(setFullscreen(bool)), TRUE );
    Action *tbScale   = new Action( this, tr( "Scale" ),      "mediaplayer/scale",  mediaPlayerState, SLOT(setScaled(bool)), TRUE );

    // Add actions to the context menu.
    //tbInfo->addTo( contextMenu );
    //tbShuffle->addTo( contextMenu );
    tbFull->addTo( contextMenu );
    tbScale->addTo( contextMenu );
    tbLoop->addTo( contextMenu );

    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ),    tbLoop,    SLOT( setOn2(bool) ) );
    //connect( mediaPlayerState, SIGNAL( shuffledToggled(bool) ),   tbShuffle, SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled(bool) ), tbFull,    SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( scaledToggled(bool) ),     tbScale,   SLOT( setOn2(bool) ) );

    tbLoop->setOn( mediaPlayerState->looping() );
    //tbShuffle->setOn( mediaPlayerState->shuffled() );
    tbFull->setOn( mediaPlayerState->fullscreen() );
    tbScale->setOn( mediaPlayerState->scaled() );

    setFocusPolicy( StrongFocus );
#endif
}


VideoWidget::~VideoWidget()
{
}


void VideoWidget::setFullscreen(bool b)
{
    if ( mediaPlayerState->view() == VideoView ) {
	screenMode = b ? Fullscreen : Normal;
        videoOutput.setFullscreenMode(screenMode == Fullscreen);
	makeVisible();
    }
}


void VideoWidget::setNextMode()
{
    if ( screenMode == Fullscreen ) {
	setMode( Normal );
    } else if ( screenMode == Large || !showCornerButton() )
	setMode( Fullscreen );
    else
	setMode( Large );
}


void VideoWidget::virtualResize()
{
    // Resize or dynamic rotation from size which supports
    // Large mode to size which does not
    if ( screenMode == Large && !showCornerButton() ) {
	screenMode = Normal;
        videoOutput.setFullscreenMode(screenMode == Fullscreen);
    }

    updateVideoOutputGeometry();

    if ( screenMode == Fullscreen )
	return;

    int w = width();
    int h = height();

    int scaleW = w;
    int scaleH = h;

    if ( showCornerButton() ) {
        cornerButton.setGeometry( w - 24, h - 24, 24, 24 );
	cornerButton.show();
    } else {
	cornerButton.hide();
    }

    // scale width and height to be both less than 400
    if ( scaleW > 400 && w ) {
	scaleW = 400;
	scaleH = h * 400 / w;
    }
    if ( scaleH > 400 && h ) {
	scaleW = w * 400 / h;
	scaleH = 400;
    }

    resizeObjects( w, h, scaleW, scaleH, ( showCornerButton() ) ? 24 : 0 );
}


void VideoWidget::updateVideoOutputGeometry()
{
    if ( screenMode == Fullscreen )
	return;

    int bH = ( screenMode == Large ) ? 0 : actualScaleH;
    int h = height();
    int w = width();
    int border = w / 50;
    const int timeHeight = 20;
    movieBorder = w / 50;

    w -= 2 * border;
    h -= 8 * border + timeHeight + bH;

    if ( screenMode == Normal && w > 352 )
	w = 352;
    if ( screenMode == Large && w > 576 ) 
	w = 576;

    innerMovieArea = QRect( (width() - w) / 2, border + movieBorder, w, h );
    videoGeometry(innerMovieArea);
    outerMovieArea = QRect( innerMovieArea.left() - movieBorder, innerMovieArea.top() - movieBorder, 
				innerMovieArea.width() + 2 * movieBorder, innerMovieArea.height() + 2 * movieBorder );

    videoOutput.setGeometry( innerMovieArea );
}


void VideoWidget::setView( View view )
{
    if ( view == VideoView ) {
	if ( mediaPlayerState->fullscreen() )
	    screenMode = Fullscreen;
	else if ( screenMode == Fullscreen )
	    screenMode = Normal;
        videoOutput.setFullscreenMode(screenMode == Fullscreen);
	makeVisible();
    } else {
	videoOutput.hide();
	if ( screenMode == Fullscreen ) {
	    videoOutput.reparent( this, innerMovieArea.topLeft() );
	    videoOutput.resize( innerMovieArea.size() );
#ifndef QT_NO_CURSOR
	    videoOutput.unsetCursor();
#endif
	}
	canPaint = FALSE;
    }
    resetButtons();
}


void VideoWidget::makeVisible()
{
    videoOutput.hide();
    if ( screenMode == Fullscreen ) {
	videoOutput.reparent( 0, QPoint( 0, 0 ) );
	videoOutput.resize( qApp->desktop()->size() );
#ifndef QT_NO_CURSOR
	videoOutput.setCursor( QCursor( blankCursor ) );
#endif
	videoOutput.showFullScreen();
    } else {
	updateVideoOutputGeometry();
	videoOutput.reparent( this, innerMovieArea.topLeft() );
	videoOutput.resize( innerMovieArea.size() );
#ifndef QT_NO_CURSOR
	videoOutput.unsetCursor();
#endif
	videoOutput.show();
	setFocus();
    }
    //qApp->processEvents();
}


void VideoWidget::setMode( int mode )
{
    if ( screenMode == (Mode)mode )
	return;

    screenMode = (Mode)mode;
    videoOutput.setFullscreenMode(screenMode == Fullscreen);

    if ( mediaPlayerState->fullscreen() != ( mode == Fullscreen ) ) {
	// toggle from/to Fullscreen from/to Normal or Large mode
	mediaPlayerState->setFullscreen( ( mode == Fullscreen ) );
    } else {
	// toggle from/to Large to/from Nomal mode
	updateVideoOutputGeometry();
	repaint(); 
    }
}


void VideoWidget::doModeMenu()
{
    cornerMenu.popup( QCursor::pos() );
}


void VideoWidget::virtualUpdateSlider()
{
    if ( screenMode == Fullscreen )
	return;
    updateSlider();
}


void VideoWidget::paintButton( QPainter& p, int i )
{
    if ( screenMode == Normal )
        ControlWidgetBase::paintButton( p, i );
}


void VideoWidget::virtualPaint( QPaintEvent * )
{
    if ( screenMode == Fullscreen )
	return;

    QPainter p( this );

    // Clear the background 
    p.setBrush( QBrush( Qt::black ) );
    p.drawRect( innerMovieArea );

    // draw border
    qDrawShadePanel( &p, outerMovieArea, colorGroup(), TRUE, movieBorder, NULL );

    // draw the buttons
    if ( screenMode == Normal )
	paintAllButtons( p );
}

