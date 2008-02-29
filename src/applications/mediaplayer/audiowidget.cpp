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
#include <qlineedit.h>
#include <qtopia/resource.h>
#include "imageutil.h"
#include "audiowidget.h"
#include "mediaplayerstate.h"
#include "maindocumentwidgetstack.h"


// Layout information for the audioButtons (and if it is a toggle button or not)
MediaButton audioButtons[] = {
    { TRUE,  FALSE, FALSE, FALSE, "play",     PlayButton       },
    { FALSE, FALSE, FALSE, FALSE, "stop",     StopButton       },
    { FALSE, FALSE, FALSE, FALSE, "next",     NextButton       },
    { FALSE, FALSE, FALSE, FALSE, "previous", PreviousButton   },
    { FALSE, FALSE, FALSE, FALSE, "up",       VolumeUpButton   },
    { FALSE, FALSE, FALSE, FALSE, "down",     VolumeDownButton },
    { TRUE,  FALSE, FALSE, FALSE, "loop",     LoopButton       },
    { FALSE, FALSE, FALSE, FALSE, "playlist", PlayListButton   },
    { FALSE, FALSE, FALSE, FALSE, "forward",  ForwardButton    },
    { FALSE, FALSE, FALSE, FALSE, "backward", BackwardButton   },
};


AudioWidget::AudioWidget( QWidget* parent, const QString &skin, const char* name ) :
    ControlWidgetBase( parent, skin, "audio", name ), songInfo( this )
{
    setButtonData( audioButtons, sizeof(audioButtons)/sizeof(MediaButton) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ), this, SLOT( setView(View) ) );

    songInfo.setFocusPolicy( QWidget::NoFocus );
    changeTextColor( &songInfo );
}


AudioWidget::~AudioWidget()
{
}


void AudioWidget::virtualResize()
{
    int h = height();
    int w = width();

    int border = w / 25;
    const int timeHeight = 20;

    int scaleW = w;
    int scaleH = h - 2 * border - 2 * timeHeight;

    songInfo.setGeometry( QRect( border, border, w - 2 * border, timeHeight ) );

    resizeObjects( w, h, scaleW, scaleH );
}


void AudioWidget::setView( View view )
{
    if ( view == AudioView ) {
	mainDocumentWindow->raiseWidget( this );
    } else {
	canPaint = FALSE;
    }
    resetButtons();
}


void AudioWidget::virtualUpdateSlider()
{
    updateSlider();
}


void AudioWidget::virtualPaint( QPaintEvent *pe )
{
    if ( !pe->erased() ) {
	// Combine with background and double buffer
	QPixmap pix( pe->rect().size() );
	QPainter p( &pix );
	p.translate( -pe->rect().topLeft().x(), -pe->rect().topLeft().y() );
	p.drawTiledPixmap( pe->rect(), bgPix, pe->rect().topLeft() );
	paintAllButtons( p );
	QPainter p2( this );
	p2.drawPixmap( pe->rect().topLeft(), pix );
    } else {
	QPainter p( this );
	paintAllButtons( p );
    }
}


Ticker::Ticker( QWidget* parent ) : QFrame( parent )
{
    setFrameStyle( WinPanel | Sunken );
    scrollText = tr( "No Song" );
}


Ticker::~Ticker()
{
}


void Ticker::setText( const QString& text )
{
    pos = 0; // reset it everytime the text is changed
    scrollText = text;

    int pixelLen = fontMetrics().width( text );
    QPixmap pm( pixelLen, contentsRect().height() );
    pm.fill( QColor( 167, 212, 167 ) );
    QPainter pmp( &pm );
    pmp.setPen( Qt::black );
    pmp.drawText( 0, 0, pixelLen, contentsRect().height(), AlignVCenter, scrollText );
    pmp.end();
    scrollTextPixmap = pm;

    killTimers();
    if ( pixelLen > contentsRect().width() )
	startTimer( 50 );
    update();
}


void Ticker::resizeEvent( QResizeEvent * )
{
    setText( scrollText );
}


void Ticker::timerEvent( QTimerEvent * )
{
    pos = ( pos <= 0 ) ? scrollTextPixmap.width() : pos - 1;
    repaint( FALSE );
}


void Ticker::drawContents( QPainter *p )
{
    int pixelLen = scrollTextPixmap.width(); 
    p->drawPixmap( pos, contentsRect().y(), scrollTextPixmap );
    if ( pixelLen > contentsRect().width() ) // Scrolling
	p->drawPixmap( pos - pixelLen, contentsRect().y(), scrollTextPixmap );
}

