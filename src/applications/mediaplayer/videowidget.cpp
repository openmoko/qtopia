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
#include <qtopia/resource.h>
#include <qtopia/mediaplayerplugininterface.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qdrawutil.h>
#include "imageutil.h"
#include "videowidget.h"
#include "mediaplayerstate.h"
#include "maindocumentwidgetstack.h"


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
    // {  TRUE, FALSE, FALSE, FALSE, "fullscreen", FullscreenButton },
};


/* XPM */
static const char * turnup_xpm[] = {
"24 24 3 1",
" 	c None",
".	c #000000",
"+	c #DBD081",
"            ",
" .......................",
" .++++++++++++++++++++. ",
" .+++++++++++++++++++.  ",
" .++++++++++++++++++.   ",
" .+++++++++++++++++.    ",
" .++++++++++++++++.     ",
" .+++++++++++++++.      ",
" .++++++++++++++.       ",
" .+++++++++++++.        ",
" .++++++++++++.         ",
" .+++++++++++.          ",
" .++++++++++.           ",
" .+++++++++.            ",
" .++++++++.             ",
" .+++++++.              ",
" .++++++.               ",
" .+++++.                ",
" .++++.                 ",
" .+++.                  ",
" .++.                   ",
" .+.                    ",
" ..                     ",
" .                      "};


VideoOutput::VideoOutput( VideoWidget* parent ) : QWidget( parent), parentWidget( parent ), targetRect( 0, 0, 0, 0 )
{
    // Create a QImage which is large enough to hold a video frame no matter what the
    // orientation of the screen or the movie is or regardless of any rotations in effect
    int size = QMAX( qApp->desktop()->width(), qApp->desktop()->height() );
    currentFrame = new QImage( size, size, (QPixmap::defaultDepth() == 16) ? 16 : 32 );
    rotatedFrame = new QImage( size, size, (QPixmap::defaultDepth() == 16) ? 16 : 32 );
    setBackgroundColor( Qt::black );
}


VideoOutput::~VideoOutput()
{
    if ( currentFrame )
        delete currentFrame;
    delete rotatedFrame;
}


void VideoOutput::paintEvent( QPaintEvent * )
{
    // Draw the current frame
    playVideo();
}


void VideoOutput::mouseReleaseEvent( QMouseEvent * )
{
    // Effectively blank the view next time we show it
//    targetRect = QRect( 0, 0, 0, 0 );
    parentWidget->setNextMode();
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


VideoWidget::VideoWidget(QWidget* parent, const QString& skin, const char* name ) :
    ControlWidgetBase( parent, skin, "video", name ),  // No tr
    cornerButton( this ), cornerMenu( 0 ), screenMode( InvalidMode ), videoOutput( this )
{
    setButtonData( videoButtons, sizeof(videoButtons)/sizeof(MediaButton) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ),    this, SLOT( setView(View) ) );

    setMode( mediaPlayerState->fullscreen() ? Fullscreen : Normal );

    qDebug("vw 1"); 
//    slider.setEnabled( mediaPlayerState->decoderIsSeekable() );
    qDebug("vw 2"); 

    cornerButton.setPixmap( QPixmap( turnup_xpm ) );
    cornerButton.setFlat( TRUE );
    cornerButton.setFocusPolicy( QWidget::NoFocus );
    connect( &cornerButton, SIGNAL( clicked() ), this, SLOT( doModeMenu() ) );

    cornerMenu.insertItem( tr("Fullscreen"), Fullscreen, Fullscreen );
    cornerMenu.insertItem( tr("Large"), Large, Large );
    cornerMenu.insertItem( tr("Normal"), Normal, Normal );
    connect( &cornerMenu, SIGNAL( activated( int ) ), this, SLOT( setMode( int ) ) );
}


VideoWidget::~VideoWidget()
{
}


void VideoWidget::virtualResize()
{
    // Resize or dynamic rotation from size which supports
    // Large mode to size which does not
    if ( screenMode == Large && !showCornerButton() ) 
	screenMode = Normal;

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
    if ( scaleW > 400 ) {
	scaleW = 400;
	scaleH = h * 400 / w;
    }
    if ( scaleH > 400 ) {
	scaleW = w * 400 / h;
	scaleH = 400;
    }

    if ( screenMode != Large )
        buttonHeight = resizeObjects( w, h, scaleW, scaleH, ( showCornerButton() ) ? 24 : 0 );
}


void VideoWidget::updateVideoOutputGeometry()
{
    if ( screenMode == Fullscreen )
	return;

    int bH = ( screenMode == Large ) ? 0 : buttonHeight;
    int h = height();
    int w = width();
    int border = w / 40;
    const int timeHeight = 20;
    movieBorder = w / 50;

    w -= 2 * (border + movieBorder);
    h -= 4 * border + timeHeight + bH + 2 * movieBorder;

    if ( screenMode == Normal && w > 352 )
	w = 352;
    if ( screenMode == Large && w > 576 ) 
	w = 576;

    innerMovieArea = QRect( (width() - w) / 2, border + movieBorder, w, h );
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
	makeVisible();
    } else {
	videoOutput.hide();
	if ( screenMode == Fullscreen ) {
	    videoOutput.reparent( this, innerMovieArea.topLeft() );
	    videoOutput.resize( innerMovieArea.size() );
	    videoOutput.unsetCursor();
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
	videoOutput.setCursor( QCursor( blankCursor ) );
	videoOutput.showFullScreen();
    } else {
	mainDocumentWindow->raiseWidget( this );
	updateVideoOutputGeometry();
	videoOutput.reparent( this, innerMovieArea.topLeft() );
	videoOutput.resize( innerMovieArea.size() );
	videoOutput.unsetCursor();
	videoOutput.show();
    }
    qApp->processEvents();
}


void VideoWidget::setMode( int mode )
{
    if ( screenMode == (Mode)mode )
	return;

    screenMode = (Mode)mode;

    if ( mediaPlayerState->fullscreen() != ( mode == Fullscreen ) ) {
	// toggle from/to Fullscreen from/to Normal or Large mode
	mediaPlayerState->setFullscreen( ( mode == Fullscreen ) );
	makeVisible();
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


bool VideoWidget::playVideo()
{
    return videoOutput.playVideo();
}


bool VideoOutput::playVideo()
{
    if ( !isVisible() )
	return TRUE;
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
    if ( parentWidget->mode() == VideoWidget::Fullscreen ) {
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
	    scaleHeight = decodeHeight * targetWidth / decodeWidth;
	} else {
	    scaleWidth = decodeWidth * targetHeight / decodeHeight;
	    scaleHeight = targetHeight;
	}
    } else {
	// Must ensure that the decoded pixels stays within the target frame
	movieWidth = decodeWidth;
	movieHeight = decodeHeight;
	decodeWidth  = QMIN( movieWidth, targetWidth );
	decodeHeight = QMIN( movieHeight, targetHeight );
	scaleWidth = decodeWidth;
	scaleHeight = decodeHeight;
	decodeX = (movieWidth - decodeWidth) / 2; 
	decodeY = (movieHeight - decodeHeight) / 2;
    }

    /*
	At this stage we now have the following variables setup with the following meanings:

	    movieWidth		- The source width of the movie (after any portrait to landscape correction)
	    movieHeight		- The source height of the movie (after any portrait to landscape correction)
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
	p.setBrush( QBrush( Qt::black ) );
	p.drawRect( QRect( targetX, targetY, targetWidth, targetHeight ) );
	targetRect = targetArea;
    }

    bool screenRotated = (qt_screen->transformOrientation() & 1);
    bool needRotation = rotateMovie90 != rotateFullscreen90;
    bool needRotationWithScreen = screenRotated != needRotation;

    imageFb.create( currentFrame->bits(), currentFrame->bytesPerLine(), 0, 0, scaleWidth, scaleHeight );
    rotatedBuffer.create( rotatedFrame->bits(), rotatedFrame->bytesPerLine(), 0, 0, scaleHeight, scaleWidth );
    FrameBuffer *fb = &imageFb;

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
	}

	if ( useDirectFrameBuffer && !needRotationWithScreen )
	    fb = &directFb;
    }
#endif

/*
    qDebug("rotateMovie90: %s  rotateFullscreen90: %s  needRotation: %s  needRotationWithScreen: %s  useDirectFrameBuffer: %s  transformOrientation: %i",
	rotateMovie90 ? "true" : "false",
	rotateFullscreen90 ? "true" : "false",
	needRotation ? "true" : "false",
	needRotationWithScreen ? "true" : "false",
	useDirectFrameBuffer ? "true" : "false",
	qt_screen->transformOrientation() );

    qDebug("useDirectFrameBuffer: %s", useDirectFrameBuffer ? "true" : "false" );
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
    if ( needRotation ) {
	int t = scaleHeight;
	scaleHeight = scaleWidth;
	scaleWidth = t;
    }

#ifdef HAVE_PIXMAP_BITS
    QPixmap *pix = 0;
    FrameBuffer pixFb;
    if ( !useDirectFrameBuffer ) {
	if ( needRotation ) {
	    if ( !needRotationWithScreen ) {
		pix = new QPixmap( qt_screen->mapToDevice( QSize( scaleWidth, scaleHeight ) ) );
		pixFb.create( pix->scanLine(0), pix->bytesPerLine(), 0, 0, scaleWidth, scaleHeight );
		fb = &pixFb;
	    }
	}
    }
#endif

    result = mediaPlayerState->decoder()->videoReadScaledFrame( fb->jumpTable(),
		    decodeX, decodeY, decodeWidth, decodeHeight, scaleWidth, scaleHeight, format, 0) == 0;

    // ### I think I found a case which is broken, screenRotated=FALSE and rotateMovie90=TRUE and rotateFullscreen90=FALSE and useDirectFrameBuffer=FALSE, image is rotated 180
    // and when screenRotated=TRUE for this case, it can crash

    if ( result ) {
	if ( !useDirectFrameBuffer ) {
	    if ( needRotation ) {
		//qDebug("case 1");
		// eg PDA 240x320, you really want to view it rotated long-wise
#ifdef HAVE_PIXMAP_BITS
		if ( !needRotationWithScreen ) {
		    p.drawPixmap( x1, y1, *pix );
		} else
#endif
		{
		    fb->rotateToBuffer( &rotatedBuffer, 0, 0, 0, 0, scaleWidth, scaleHeight, !rotateMovie90 );
		    p.drawImage( x1, y1, *rotatedFrame, 0, 0, scaleHeight, scaleWidth );
		}
	    } else {
		//qDebug("case 2");
		// eg 640x480 device, it is best not to rotate image around
		p.drawImage( x1, y1, *currentFrame, 0, 0, scaleWidth, scaleHeight );
	    }
	} else {
	    if ( needRotationWithScreen ) {
		//qDebug("case 3");
		// Work out if will need to rotate clockwise or anticlockwise by 90 degrees
		bool screenRotatedMoreThan180 = (qt_screen->transformOrientation() & 2);
		fb->rotateToBuffer( &directFb, 0, 0, 0, 0, scaleWidth, scaleHeight, !screenRotatedMoreThan180 );
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

