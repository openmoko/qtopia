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
#include <qtopia/qpeapplication.h>
#include <qtopia/qlibrary.h>
#include <qtopia/resource.h>
#include <qtopia/config.h>

#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qwidgetstack.h>
#include <qfile.h>

#include "mediaplayer.h"
#include "playlistwidget.h"
#include "audiowidget.h"
#include "loopcontrol.h"
#include "audiodevice.h"

#include "mediaplayerstate.h"
#include "maindocumentwidgetstack.h"


extern PlayListWidget *playList;
extern LoopControl *loopControl;


MediaPlayer::MediaPlayer( QObject *parent, const char *name )
    : QObject( parent, name ), currentFile( NULL ), upTimerId( 0 ), dnTimerId( 0 ), rtTimerId( 0 ), ltTimerId( 0 )
{
    connect( mainDocumentWindow, SIGNAL( openURL( const QString&, const QString& ) ), this, SLOT( openURL( const QString&, const QString& ) ) );
    connect( mediaPlayerState, SIGNAL( playingToggled( bool ) ), this, SLOT( setPlaying( bool ) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled( bool ) ),  this, SLOT( pauseCheck( bool ) ) );
    connect( mediaPlayerState, SIGNAL( next() ),		 this, SLOT( next() ) );
    connect( mediaPlayerState, SIGNAL( prev() ),		 this, SLOT( prev() ) );

    connect( mediaPlayerState, SIGNAL( increaseVolume() ),	 this, SLOT( startIncreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( endIncreaseVolume() ),	 this, SLOT( stopIncreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( decreaseVolume() ),	 this, SLOT( startDecreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( endDecreaseVolume() ),	 this, SLOT( stopDecreasingVolume() ) );

    connect( mediaPlayerState, SIGNAL( scanForward() ),		 this, SLOT( startScanningForward() ) );
    connect( mediaPlayerState, SIGNAL( endScanForward() ),	 this, SLOT( stopScanningForward() ) );
    connect( mediaPlayerState, SIGNAL( scanBackward() ),	 this, SLOT( startScanningBackward() ) );
    connect( mediaPlayerState, SIGNAL( endScanBackward() ),	 this, SLOT( stopScanningBackward() ) );

    // Handle all key presses
    qApp->installEventFilter( this );
}


MediaPlayer::~MediaPlayer()
{
}


void MediaPlayer::pauseCheck( bool b )
{
    // Only pause if playing
    if ( b && !mediaPlayerState->playing() )
	mediaPlayerState->setPaused( FALSE );
}


void MediaPlayer::play()
{
    mediaPlayerState->setPlaying( FALSE );
    mediaPlayerState->setPlaying( TRUE );
}


void MediaPlayer::error( const QString& error, const QString& errorMsg )
{
    QMessageBox::critical( 0, error, errorMsg );
    mediaPlayerState->setPlaying( FALSE );
}


void MediaPlayer::openURL( const QString &URL, const QString &mimetype )
{
    setPlaying( false );

    if ( !mediaPlayerState->streamingDecoder( URL, mimetype ) ) {
	error( tr( "No decoder found"), tr( "Sorry, no appropriate decoders found for the url: %1" ).arg( "<i>" + URL + "</i>" ) );
	return;
    }

    if ( !loopControl->init( URL, mimetype, true ) ) {
	error( tr( "Error opening file"), tr( "Sorry, an error occurred trying to play the url: %1" ).arg( "<i>" + URL + "</i>" ) );
	return;
    }

    loopControl->play();

    if ( !loopControl->hasVideo() ) {
	QString tickerText = " " + tr( "URL: %1" ).arg( URL );
	long seconds = loopControl->totalPlaytime() / 1000;
	if ( seconds > 0 ) 
	    tickerText += "   " + tr( "Length: %1" ).arg( ControlWidgetBase::toTimeString( seconds ) );
	mediaPlayerState->audioUI()->setTickerText( tickerText + "  " );
    }
}


void MediaPlayer::setPlaying( bool play )
{
    if ( !play ) {
	loopControl->stop( FALSE );
	mediaPlayerState->setPaused( FALSE );
	return;
    }

    if ( mediaPlayerState->paused() ) {
	mediaPlayerState->setPaused( FALSE );
	return;
    }

    const DocLnk *playListCurrent = playList->current();

    if ( playListCurrent != NULL ) {
	loopControl->stop( TRUE );
	currentFile = playListCurrent;
    }

    if ( currentFile == NULL ) {
	error( tr( "No file"), tr( "Error: There is no file selected" ) );
	return;
    }

    QString fileStr = currentFile->file();
    QString nameStr = currentFile->name();

    if ( !QFile::exists( fileStr ) ) {
	error( tr( "File not found" ), tr( "The following file was not found: <br>%1<br><i>%2</i>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    if ( !mediaPlayerState->decoder( fileStr ) ) {
	error( tr( "No decoder found" ), tr( "Sorry, no appropriate decoders found for this file: <br>%1<br><i>%2</i>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    if ( !loopControl->init( fileStr ) ) {
	error( tr( "Error opening file" ), tr( "Sorry, an error occurred trying to play the file: <br>%1<br><i>%2</i>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    loopControl->play();

    if ( !loopControl->hasVideo() ) {
	QString tickerText = " " + tr( "File: %1" ).arg( nameStr );
	long seconds = loopControl->totalPlaytime() / 1000;
	if ( seconds > 0 ) 
	    tickerText += "   " + tr( "Length: %1" ).arg( ControlWidgetBase::toTimeString( seconds ) );
	mediaPlayerState->audioUI()->setTickerText( tickerText + "  " );
    }
}


void MediaPlayer::prev()
{
    if ( playList->prev() )
	play();
    else 
	mediaPlayerState->setList();
}


void MediaPlayer::next()
{
    if ( playList->next() )
	play();
    else    
	mediaPlayerState->setList();
}


void MediaPlayer::startDecreasingVolume()
{
    if ( dnTimerId )
	return;
    dnTimerId = startTimer( 100 );
    AudioDevice::decreaseVolume();
}


void MediaPlayer::startIncreasingVolume()
{
    if ( upTimerId )
	return; 
    upTimerId = startTimer( 100 );
    AudioDevice::increaseVolume();
}


void MediaPlayer::startScanningBackward()
{
    if ( !mediaPlayerState->seekable() )
	return;
    if ( ltTimerId )
	return;
    mediaPlayerState->setPaused( !mediaPlayerState->paused() );
    ltTimerId = startTimer( 200 );
    mediaPlayerState->startTemporaryMute();
}


void MediaPlayer::startScanningForward()
{
    if ( !mediaPlayerState->seekable() )
	return;
    if ( rtTimerId )
	return;
    mediaPlayerState->setPaused( !mediaPlayerState->paused() );
    rtTimerId = startTimer( 200 );
    mediaPlayerState->startTemporaryMute();
}


bool drawnOnScreenDisplay = FALSE;
unsigned int onScreenDisplayVolume = 0;
const int yoff = 110;


void MediaPlayer::updateOnscreenDisplay()
{
    // Display an on-screen display volume
    unsigned int l, r, v;
    bool m;

    AudioDevice::getVolume( l, r, m );
    v = ((l + r) * 11) / (2*0xFFFF);

    if ( drawnOnScreenDisplay && onScreenDisplayVolume == v )
	return;

    // Must make this draw according to the screen size
    int w = mediaPlayerState->audioUI()->width();
    int h = mediaPlayerState->audioUI()->height();

    int vW = w * 90 / 100;
    int mW = vW / 10;
    int mH = mW * 3 / 2;
    int tickWidth = mW / 2;

    if ( drawnOnScreenDisplay ) {
	if ( onScreenDisplayVolume > v )
	    mediaPlayerState->audioUI()->repaint( (w - vW) / 2 + v * mW + 10, h - yoff + 40 - mH, (onScreenDisplayVolume - v) * mW + 9, mH, FALSE );
    }

    drawnOnScreenDisplay = TRUE;
    onScreenDisplayVolume = v;

    QPainter p( mediaPlayerState->audioUI() );
    p.setPen( QColor( 0x10, 0xD0, 0x10 ) );
    p.setBrush( QColor( 0x10, 0xD0, 0x10 ) );

    QFont f;
    f.setPixelSize( mW );
    f.setBold( TRUE );
    p.setFont( f );
    p.drawText( (w - vW) / 2, h - yoff + 20 - mH, tr("Volume") );

    for ( unsigned int i = 0; i < 10; i++ ) {
	if ( v > i ) 
	    p.drawRect( (w - vW) / 2 + i * mW + 10, h - yoff + 40 - mH, tickWidth, mH );
	 else 
	    p.drawRect( (w - vW) / 2 + i * mW + 13, h - yoff + 40 - mH + mW/2, tickWidth / 2, mH - mW );
    }
}


void MediaPlayer::hideOnscreenDisplay()
{
    if ( drawnOnScreenDisplay == FALSE )
	return;

    // Get rid of the on-screen display stuff
    drawnOnScreenDisplay = FALSE;
    onScreenDisplayVolume = 0;

    // Must fix this to repaint the area scaled by the screen size    
    int w = mediaPlayerState->audioUI()->width();
    int h = mediaPlayerState->audioUI()->height();
    int vW = w * 90 / 100;
    int mW = vW / 10;
    int mH = mW * 3 / 2;
    mediaPlayerState->audioUI()->repaint( (w - vW) / 2, h - yoff - mH, vW + 9, 50 + mH, FALSE );
}


void MediaPlayer::stopDecreasingVolume()
{
    if ( dnTimerId ) {
	killTimer( dnTimerId );
	dnTimerId = 0;
	hideOnscreenDisplay();
    }
}


void MediaPlayer::stopIncreasingVolume()
{
    if ( upTimerId ) {
	killTimer( upTimerId );
	upTimerId = 0;
	hideOnscreenDisplay();
    }
}


void MediaPlayer::stopScanningBackward()
{
    if ( ltTimerId ) {
	killTimer( ltTimerId );
	ltTimerId = 0;
	mediaPlayerState->setPaused( !mediaPlayerState->paused() );
	mediaPlayerState->stopTemporaryMute( 500 );
    }
}


void MediaPlayer::stopScanningForward()
{
    if ( rtTimerId ) {
	killTimer( rtTimerId );
	rtTimerId = 0;
	mediaPlayerState->setPaused( !mediaPlayerState->paused() );
	mediaPlayerState->stopTemporaryMute( 500 );
    }
}


void MediaPlayer::timerEvent( QTimerEvent *te )
{
    if ( te->timerId() == dnTimerId ) {
        AudioDevice::decreaseVolume();
	updateOnscreenDisplay();
    } else if ( te->timerId() == upTimerId ) {
	AudioDevice::increaseVolume();
	updateOnscreenDisplay();
    } else if ( te->timerId() == ltTimerId ) {
	scanning = TRUE;
	int newPos;
	if ( mediaPlayerState->view() == AudioView )
	    newPos = mediaPlayerState->position() - 200000;
	else {
	    int increment = mediaPlayerState->length() / 50;
	    newPos = mediaPlayerState->position() - increment;
	}
	if ( newPos > 0 )
	    mediaPlayerState->setPosition( newPos );
	else
	    mediaPlayerState->setPosition( 0 );
    } else if ( te->timerId() == rtTimerId ) {
	scanning = TRUE;
	int newPos;
	if ( mediaPlayerState->view() == AudioView )
	    newPos = mediaPlayerState->position() + 200000;
	else {
	    int increment = mediaPlayerState->length() / 50;
	    newPos = mediaPlayerState->position() + increment;
	}
	if ( newPos <= mediaPlayerState->length() )
	    mediaPlayerState->setPosition( newPos );
	else
	    mediaPlayerState->setPosition( mediaPlayerState->length() );
    }
}


bool MediaPlayer::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::KeyPress )
	if ( keyPressEvent( (QKeyEvent *)e ) )
	    return TRUE;
    if ( e->type() == QEvent::KeyRelease )
	if ( keyReleaseEvent( (QKeyEvent *)e ) )
	    return TRUE;
    return QObject::eventFilter( o, e );
}
  

bool MediaPlayer::keyPressEvent( QKeyEvent *e )
{
    if ( !mediaPlayerState->playing() ) {

	// Ignore key presses that will be handled by the menubar
	if ( !playList->listHasFocus() )
	    return FALSE;

	switch ( e->key() ) {
	    case Key_Left:
		mediaPlayerState->setPrevTab();
		return TRUE;
	    case Key_Right:
		mediaPlayerState->setNextTab();
		return TRUE;
	    case Key_Space:
		play();
		return TRUE;
	    default:
		break;
	}
    } else {

	if ( e->isAutoRepeat() )
	    return FALSE;

	switch( e->key() ) {
	    case Key_Left:
		scanning = FALSE;
		startScanningBackward();
		return TRUE;
	    case Key_Right:
		scanning = FALSE;
		startScanningForward();
		return TRUE;
	    case Key_Up:
		startIncreasingVolume();
		return TRUE;
	    case Key_Down:
		startDecreasingVolume();
		return TRUE;
	    case Key_Space:
		mediaPlayerState->setPaused( !mediaPlayerState->paused() );
		return TRUE;
	    case Key_Escape:
		mediaPlayerState->setList();
		return TRUE;
	    default:
		break;
	}
    }

    return FALSE;
}


bool MediaPlayer::keyReleaseEvent( QKeyEvent *e )
{
    // Ignore all auto repeats
    if ( e->isAutoRepeat() )
	return FALSE;

    switch( e->key() ) {
	case Key_Left:
	    if ( ltTimerId ) {
		if ( !scanning ) {
		    prev();
		    mediaPlayerState->setPaused( !mediaPlayerState->paused() );
		}
		stopScanningBackward();
		return TRUE;
	    }
	    break;
	case Key_Right:
	    if ( rtTimerId ) {
		if ( !scanning ) {
		    next();
		    mediaPlayerState->setPaused( !mediaPlayerState->paused() );
		}
		stopScanningForward();
		return TRUE;
	    }
	    break;
    }

    if ( !mediaPlayerState->playing() )
	return FALSE;

    switch( e->key() ) {
	case Key_Up:
	    stopIncreasingVolume();
	    return TRUE;
	case Key_Down:
	    stopDecreasingVolume();
	    return TRUE;
	default:
	    break;
    }

    return FALSE;
}

