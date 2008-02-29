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
#include <qpe/resource.h>
#include <qpe/mediaplayerplugininterface.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qslider.h>
#include <qdrawutil.h>
#include "imageutil.h"
#include "videowidget.h"
#include "mediaplayerstate.h"


#ifdef Q_WS_QWS
# define USE_DIRECT_PAINTER
# include <qdirectpainter_qws.h>
# include <qgfxraster_qws.h>
#endif


// Layout information for the videoButtons (and if it is a toggle button or not)
MediaButton videoButtons[] = {
    { FALSE, FALSE, FALSE, FALSE, "previous",   PreviousButton   },
    { FALSE, FALSE, FALSE, FALSE, "backward",   BackwardButton   },
    {  TRUE, FALSE, FALSE, FALSE, "play",       PlayButton       },
    { FALSE, FALSE, FALSE, FALSE, "forward",    ForwardButton    },
    { FALSE, FALSE, FALSE, FALSE, "next",       NextButton       },
    {  TRUE, FALSE, FALSE, FALSE, "loop",       LoopButton       },
    // {  TRUE, FALSE, FALSE, FALSE, "fullscreen", FullscreenButton },
};


VideoWidget::VideoWidget(QWidget* parent, const QString& skin, const char* name ) :
    ControlWidgetBase( parent, skin, "video", name ), scaledWidth( 0 ), scaledHeight( 0 ), currentFrame( 0 )
{
    setButtonData( videoButtons, sizeof(videoButtons)/sizeof(MediaButton) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ),    this, SLOT( setView(View) ) );
}


VideoWidget::~VideoWidget()
{
    if ( currentFrame )
        delete currentFrame;
}


void VideoWidget::internalResize()
{
    int h = height();
    int w = width();

    int border = w / 25;
    const int timeHeight = 20;

    int scaleW = w > 400 ? 400 : w;
    int scaleH = h > 400 ? 400 : h;

    int buttonHeight = resizeObjects( w, h, scaleW, scaleH );

    movieBorder = w / 50;
    outerMovieArea = QRect( border, border, w - 2 * border, h - timeHeight - 4 * border - buttonHeight );
    innerMovieArea = QRect( border + movieBorder, border + movieBorder, w - 2 * (movieBorder + border), h - timeHeight - 4 * border - buttonHeight - 2 * movieBorder );

    if ( currentFrame )
	delete currentFrame;
    currentFrame = new QImage( innerMovieArea.width() + 2, innerMovieArea.height(), (QPixmap::defaultDepth() == 16) ? 16 : 32 );
}


void VideoWidget::setView( View view )
{
    if ( view == VideoView ) {
	makeVisible();
    } else {
	// Effectively blank the view next time we show it so it looks nicer
	scaledWidth = 0;
	scaledHeight = 0;
	hide();
    }
}


QString VideoWidget::timeAsString( long length )
{
    double frameRate = 1.0;
    if ( mediaPlayerState->decoder() )
	frameRate = mediaPlayerState->decoder()->videoFrameRate( 0 );
    if ( frameRate == 0.0 )
	frameRate = 1.0;
    length = long((double)length / frameRate);
    int minutes = length / 60;
    int seconds = length % 60;
    return QString("%1:%2%3").arg( minutes ).arg( seconds / 10 ).arg( seconds % 10 );
}


void VideoWidget::updateSlider( long i, long max )
{
    if ( mediaPlayerState->fullscreen() )
	return;
    updateSliderBase( i, max );
}


void VideoWidget::paintButton( QPainter& p, int i )
{
    if ( !mediaPlayerState->fullscreen() )
        ControlWidgetBase::paintButton( p, i );
}


void VideoWidget::mouseReleaseEvent( QMouseEvent *event )
{
    if ( mediaPlayerState->fullscreen() ) {
	mediaPlayerState->setFullscreen( FALSE );
	makeVisible();
    } else { 
	if ( innerMovieArea.contains( event->pos() ) ) {
	    mediaPlayerState->setFullscreen( TRUE );
	    makeVisible();
	} else
	    mouseMoveEvent( event );
    }
}


void VideoWidget::makeVisible()
{
    if ( mediaPlayerState->fullscreen() ) {
	setBackgroundMode( QWidget::NoBackground );
	showFullScreen();
#ifdef QWS
	resize( qApp->desktop()->size() );
#endif
	slider.hide();
	time.hide();
    } else {
	setBackgroundPixmap( bgPix );
	showNormal();
	showMaximized();
	slider.show();
	time.show();
    }
}


bool VideoWidget::playVideo()
{
    bool result = FALSE;

    int stream = 0; 

    int sw = mediaPlayerState->decoder()->videoWidth( stream );
    int sh = mediaPlayerState->decoder()->videoHeight( stream );
    int dd = QPixmap::defaultDepth();
    int w = height();
    int h = width();

    // prevent divide by zero
    sh = (sh == 0) ? 1 : sh;
    sw = (sw == 0) ? 1 : sw;

    ColorFormat format = (dd == 16) ? RGB565 : BGRA8888;

    if ( mediaPlayerState->fullscreen() ) {
#ifdef USE_DIRECT_PAINTER
	QDirectPainter p(this);

	if ( ( qt_screen->transformOrientation() == 3 ) &&
	     ( ( dd == 16 ) || ( dd == 32 ) ) && ( p.numRects() == 1 ) ) {

	    w = 320;
	    h = 240;

	    if ( mediaPlayerState->scaled() ) {
		// maintain aspect ratio
		if ( w * sh > sw * h ) 
		    w = sw * h / sh;
		else
		    h = sh * w / sw;
	    } else  {
		w = sw;
		h = sh;	    
	    }

	    w--; // we can't allow libmpeg to overwrite.
	    QPoint roff = qt_screen->mapToDevice( p.offset(), QSize( qt_screen->width(), qt_screen->height() ) );
	    
	    int ox = roff.x() - height() + 2 + (height() - w) / 2;
	    int oy = roff.y() + (width() - h) / 2;
	    int sx = 0, sy = 0;

	    uchar* fp = p.frameBuffer() + p.lineStep() * oy;
	    fp += dd * ox / 8;
	    uchar **jt = new uchar*[h];
	    for ( int i = h; i; i-- ) {
		jt[h - i] = fp;
		fp += p.lineStep();
	    }

	    result = mediaPlayerState->decoder()->videoReadScaledFrame( jt, sx, sy, sw, sh, w, h, format, 0) == 0;

	    delete [] jt;
	} else {
#endif
	    QPainter p(this);

	    w = 320;
	    h = 240;

	    if ( mediaPlayerState->scaled() ) {
		// maintain aspect ratio
		if ( w * sh > sw * h ) 
		    w = sw * h / sh;
		else
		    h = sh * w / sw;
	    } else  {
		w = sw;
		h = sh;	    
	    }

	    int bytes = ( dd == 16 ) ? 2 : 4;
	    QImage tempFrame( w, h, bytes << 3 );
	    result = mediaPlayerState->decoder()->videoReadScaledFrame( tempFrame.jumpTable(),
							0, 0, sw, sh, w, h, format, 0) == 0;
	    if ( result ) {

		int rw = h, rh = w;
		QImage rotatedFrame( rw, rh, bytes << 3 );

		ushort* in  = (ushort*)tempFrame.bits();
		ushort* out = (ushort*)rotatedFrame.bits();
		int spl = rotatedFrame.bytesPerLine() / bytes;
		for (int x=0; x<h; x++) {
		    if ( bytes == 2 ) {
			ushort* lout = out++ + (w - 1)*spl;
			for (int y=0; y<w; y++) {
			    *lout=*in++;
			    lout-=spl;
			}
		    } else {
			ulong* lout = ((ulong *)out)++ + (w - 1)*spl;
			for (int y=0; y<w; y++) {
			    *lout=*((ulong*)in)++;
			    lout-=spl;
			}
		    }
		}

		p.drawImage( (240 - rw) / 2, (320 - rh) / 2, rotatedFrame, 0, 0, rw, rh );
	    }
#ifdef USE_DIRECT_PAINTER
	}
#endif
    } else {

	w = innerMovieArea.width();
	h = innerMovieArea.height();

	// maintain aspect ratio
	if ( w * sh > sw * h ) 
	    w = sw * h / sh;
	else
	    h = sh * w / sw;

	result = mediaPlayerState->decoder()->videoReadScaledFrame( currentFrame->jumpTable(), 0, 0, sw, sh, w, h, format, 0) == 0;

	QPainter p( this );

	// Image changed size, therefore need to blank the possibly unpainted regions first
	if ( scaledWidth != w || scaledHeight != h ) {
	    p.setBrush( QBrush( Qt::black ) );
	    p.drawRect( innerMovieArea );
	}

	scaledWidth = w;
	scaledHeight = h;

	if ( result ) {
	    p.drawImage( innerMovieArea.left() + (innerMovieArea.width() - scaledWidth) / 2, innerMovieArea.top() + (innerMovieArea.height() - scaledHeight) / 2, *currentFrame, 0, 0, scaledWidth, scaledHeight );
	}

    }

    return result;
}


void VideoWidget::internalPaint( QPaintEvent * )
{
    QPainter p( this );

    if ( mediaPlayerState->fullscreen() ) {
	// Clear the background 
	p.setBrush( QBrush( Qt::black ) );
	p.drawRect( rect() );

	// Draw the current frame
	playVideo();
    } else {
	// draw border
	qDrawShadePanel( &p, outerMovieArea, colorGroup(), TRUE, movieBorder, NULL );

	// Clear the movie screen first
	p.setBrush( QBrush( Qt::black ) );
	p.drawRect( innerMovieArea );

	// draw current frame (centrally positioned from scaling to maintain aspect ratio)
	playVideo();

	// draw the buttons
	paintAllButtons( p );

	// draw the slider
	slider.repaint( TRUE );
    }
}

