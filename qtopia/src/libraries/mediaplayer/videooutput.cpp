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
#include <qtopia/mediaplayerplugininterface.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include "videooutput.h"
#include "mediaplayerstate.h"


#ifdef Q_WS_QWS
# define HAVE_PIXMAP_BITS
# define USE_DIRECT_PAINTER
# include <qdirectpainter_qws.h>
# include <qgfxraster_qws.h>
#endif


VideoOutputWidget *videoOutput = 0;


VideoOutputWidget::VideoOutputWidget(QWidget *parent)
    : QWidget(parent), fullscreen(false), targetRect( 0, 0, 0, 0 )
{
    // Create a QImage which is large enough to hold a video frame no matter what the
    // orientation of the screen or the movie is or regardless of any rotations in effect
    int size = QMAX( qApp->desktop()->width(), qApp->desktop()->height() );
    currentFrame = new QImage( size, size, (QPixmap::defaultDepth() == 16) ? 16 : 32 );
    rotatedFrame = new QImage( size, size, (QPixmap::defaultDepth() == 16) ? 16 : 32 );
    setBackgroundColor( Qt::black );
    videoOutput = this;
}


VideoOutputWidget::~VideoOutputWidget()
{
    delete currentFrame;
    delete rotatedFrame;
    videoOutput = 0;
}


bool VideoOutputWidget::playNextVideoFrame()
{
    if ( !isVisible() )
	return true;

    int stream = 0;
    int dd = QPixmap::defaultDepth();
    ColorFormat format = (dd == 16) ? RGB565 : RGBA8888;
    bool useDirectFrameBuffer = FALSE;
    bool result = FALSE;
    bool rotateMovie90 = FALSE;
    bool rotateFullscreen90 = FALSE;

    int movieWidth = mediaPlayerState->decoder()->videoWidth( stream );
    int movieHeight = mediaPlayerState->decoder()->videoHeight( stream );

    int scaleWidth;
    int scaleHeight;

    int targetX;
    int targetY;
    int targetWidth;
    int targetHeight;

    int decodeX;
    int decodeY;
    int decodeWidth;
    int decodeHeight;

    QPainter p( this );

    // Work out the target area
    if ( fullscreen ) {
	int fullscreenWidth = qApp->desktop()->width();
	int fullscreenHeight = qApp->desktop()->height();
	/*
	    If the device has a portrait screen, rotate in fullscreen mode
	    so that the maximum size of the movie can be displayed (which is
	    usually what fullscreen mode is for :)
	*/
	targetX = 0;
	targetY = 0;
	if ( fullscreenHeight > fullscreenWidth ) {
	    rotateFullscreen90 = TRUE;
	    targetWidth = fullscreenHeight;
	    targetHeight = fullscreenWidth;
	} else {
	    targetWidth = fullscreenWidth;
	    targetHeight = fullscreenHeight;
	}
    } else {
	// Non-fullscreen case
	targetX = 0; // innerMovieArea.left();
	targetY = 0; // innerMovieArea.top();
	targetWidth = width(); // innerMovieArea.width();
	targetHeight = height(); // innerMovieArea.height();
    }

    // prevent divide by zero
    movieHeight = (movieHeight == 0) ? 1 : movieHeight;
    movieWidth = (movieWidth == 0) ? 1 : movieWidth;

    if ( movieHeight > movieWidth ) {
	/*
	    Movie has a portrait orientation, this is not normal for movie clips.
	    From experience downloading movie files from the internet designed
	    for PocketPC devices with such orientations are actually rotated and meant to
	    be rotated back by the software to a landscape orientation.
	    Examples encoded this way can be found at http://www.pocketmatrix.com/multimedia/
	    Presumably they are designed to be played with the 'Pocket DivX' movie player.
	*/
	rotateMovie90 = TRUE;
	decodeWidth = movieHeight;
	decodeHeight = movieWidth;
    } else {
	decodeWidth = movieWidth;
	decodeHeight = movieHeight;
    }

    if ( mediaPlayerState->scaled() ) {
	decodeX = 0;
	decodeY = 0;
	// maintain aspect ratio
	if ( targetHeight * decodeWidth > decodeHeight * targetWidth ) {
	    scaleWidth = targetWidth;
	    scaleHeight = (decodeWidth) ? decodeHeight * targetWidth / decodeWidth : 0;
	} else {
	    scaleWidth = (decodeHeight) ? decodeWidth * targetHeight / decodeHeight : 0;
	    scaleHeight = targetHeight;
	}
    } else {
	// Must ensure that the decoded pixels stays within the target frame
	int tw = decodeWidth;
	int th = decodeHeight;
	decodeWidth  = QMIN( tw, targetWidth );
	decodeHeight = QMIN( th, targetHeight );
	scaleWidth = decodeWidth;
	scaleHeight = decodeHeight;
	decodeX = (tw - decodeWidth) / 2;
	decodeY = (th - decodeHeight) / 2;
    }

    /*
	At this stage we now have the following variables setup with the following meanings:

	    movieWidth		- The source width of the movie (after any portrait to landscape correction)
	    movieHeight		- The source height of the movie (after any portrait to landscape correction)
	    decodeWidth		- The source width of the movie
	    decodeHeight	- The source height of the movie
	    rotateMovie90	- Flag which sets if there needs to be portrait to landscape correction


	    scaleWidth		- This is the required target scaled size (with correct aspect ratio) to output
	    scaleHeight		- This is the required target scaled size (with correct aspect ratio) to output
	    rotateFullscreen90	- Flag which specifies if the movie fits best rotated in fullscreen mode

	    targetWidth		- The area to display in to
	    targetHeight	- The area to display in to
    */

    int x1 = targetX + (targetWidth - scaleWidth) / 2;
    int y1 = targetY + (targetHeight - scaleHeight) / 2;

    // rotate device coordinates
    if ( rotateFullscreen90 ) {
	int t = x1;
	x1 = y1;
	y1 = t;
	t = scaleHeight;
	scaleHeight = scaleWidth;
	scaleWidth = t;
	t = targetHeight;
	targetHeight = targetWidth;
	targetWidth = t;
    }

    QRect targetArea = QRect( x1, y1, scaleWidth, scaleHeight );
    // Image changed size, therefore need to blank the target region first
    if ( targetRect != targetArea ) {
	p.setPen( Qt::black );
	p.setBrush( Qt::black );
	p.drawRect( QRect( targetX, targetY, targetWidth, targetHeight ) );
	targetRect = targetArea;
    }

    int rotation;
    if ( !rotateMovie90 && !rotateFullscreen90 )
	rotation = 0;
    else if ( !rotateMovie90 && rotateFullscreen90 )
	rotation = 1;
    else if ( rotateMovie90 && !rotateFullscreen90 )
	rotation = 3;
    else 
	rotation = 0;
#ifdef Q_WS_QWS
    int rotationWithScreen = (qt_screen->transformOrientation()+4-rotation)%4;
#else
    int rotationWithScreen = rotation;
#endif

    FrameBuffer *fb = 0;

#ifdef USE_DIRECT_PAINTER
    QDirectPainter dp( this );

    // Don't use direct painter if there are no regions returned
    if ( dp.numRects() != 0 ) {
	// area we will decode in to in global untransformed coordinates
	QRect drawRect = QRect( mapToGlobal( QPoint( x1, y1 ) ), QSize( scaleWidth, scaleHeight ) );
	// area transformed in to device coordinates
	QRect deviceRect = qt_screen->mapToDevice( drawRect, QSize( qt_screen->width(), qt_screen->height() ) );
	// The top corner point of this area
	QPoint offset = deviceRect.topLeft();

	//qDebug("rect = %i %i %i %i", drawRect.x(), drawRect.y(), drawRect.width(), drawRect.height() );
	//qDebug("x: %i  y: %i", offset.x(),  offset.y() );
	directFb.create( dp.frameBuffer(), dp.lineStep(), offset.x(), offset.y(), dp.width(), dp.height() );

	QSize deviceSize = qt_screen->mapToDevice( QSize( qt_screen->width(), qt_screen->height() ) );
	if (( dd == 16 || dd == 32 )) {
	    QRegion imageRegion;

	    // global untransformed coordinates
	    imageRegion = drawRect;

	    for ( int i = 0; i < dp.numRects(); i++ ) {
		// map from device coordinates and subtract from our output region
		imageRegion -= qt_screen->mapFromDevice( dp.rect(i), deviceSize );
		//qDebug("dp.rect(%i) = %i %i %i %i", i, dp.rect(i).x(), dp.rect(i).y(), dp.rect(i).width(), dp.rect(i).height() );
	    }

	    // if we have completely subtracted everything from our output region,
	    // there is no clipping needed against the draw region so we use direct painting
	    useDirectFrameBuffer = imageRegion.isEmpty();
	    //useDirectFrameBuffer = FALSE; // Uncomment to test the non useDirectFrameBuffer case

	    // XXX It is possible that when clipping is needed, partially the
	    // frame could be decoded directly to the framebuffer where whole
	    // lines are not obsurced, and the remainder decoded via a pixmap.
	}

	if ( useDirectFrameBuffer && !rotationWithScreen )
	    fb = &directFb;
    }
#endif

/*
    qDebug("rotateMovie90: %s  rotateFullscreen90: %s  rotation: %d  rotationWithScreen: %d  useDirectFrameBuffer: %s  transformOrientation: %i",
	rotateMovie90 ? "true" : "false",
	rotateFullscreen90 ? "true" : "false",
	rotation,
	rotationWithScreen,
	useDirectFrameBuffer ? "true" : "false",
	qt_screen->transformOrientation() );
*/

    // rotate decode coordinates
    if ( rotateMovie90 ) {
	// Swap arguments
	int t = decodeHeight;
	decodeHeight = decodeWidth;
	decodeWidth = t;
	t = decodeX;
	decodeX = decodeY;
	decodeY = t;
    }

    // rotate output coordinates
    if ( rotation&1 ) {
	int t = scaleHeight;
	scaleHeight = scaleWidth;
	scaleWidth = t;
    }

#ifdef HAVE_PIXMAP_BITS
    QPixmap *pix = 0;
    FrameBuffer pixFb;
    if ( !useDirectFrameBuffer ) {
	if ( rotation && !rotationWithScreen ) {
	    pix = new QPixmap( qt_screen->mapToDevice( QSize( scaleWidth, scaleHeight ) ) );
	    pixFb.create( pix->scanLine(0), pix->bytesPerLine(), 0, 0, scaleWidth, scaleHeight );
	    fb = &pixFb;
	}
    }
#endif

    if ( !fb ) {
	imageFb.create( currentFrame->bits(), currentFrame->bytesPerLine(), 0, 0, scaleWidth, scaleHeight );
	fb = &imageFb;
    }

    result = mediaPlayerState->decoder()->videoReadScaledFrame( fb->jumpTable(),
		    decodeX, decodeY, decodeWidth, decodeHeight, scaleWidth, scaleHeight, format, 0) == 0;

    // ### I think I found a case which is broken, screenRotated=FALSE and rotateMovie90=TRUE and rotateFullscreen90=FALSE and useDirectFrameBuffer=FALSE, image is rotated 180
    // and when screenRotated=TRUE for this case, it can crash

    if ( result ) {
	if ( !useDirectFrameBuffer ) {
	    if ( rotation ) {
		//qDebug("case 1");
		// eg PDA 240x320, you really want to view it rotated long-wise
#ifdef HAVE_PIXMAP_BITS
		if ( !rotationWithScreen ) {
		    p.drawPixmap( x1, y1, *pix );
		} else
#endif
		{
		    rotatedBuffer.create( rotatedFrame->bits(), rotatedFrame->bytesPerLine(), 0, 0, scaleHeight, scaleWidth );
		    fb->rotateToBuffer( &rotatedBuffer, 0, 0, 0, 0, scaleWidth, scaleHeight, rotationWithScreen );
		    p.drawImage( x1, y1, *rotatedFrame, 0, 0, scaleHeight, scaleWidth );
		}
	    } else {
		//qDebug("case 2");
		// eg 640x480 device, it is best not to rotate image around
		p.drawImage( x1, y1, *currentFrame, 0, 0, scaleWidth, scaleHeight );
	    }
	} else {
	    if ( rotationWithScreen ) {
		//qDebug("case 3");
		fb->rotateToBuffer( &directFb, 0, 0, 0, 0, scaleWidth, scaleHeight, rotationWithScreen );
	    } else {
		//qDebug("case 4");
	    }
	}
    }

#ifdef HAVE_PIXMAP_BITS
    delete pix;
#endif

    return result;
}


