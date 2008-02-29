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
#define QTOPIA_INTERNAL_LOADTRANSLATIONS
#include <qtopia/qpeapplication.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/applnk.h>
#include <qtopia/resource.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qfile.h>

#ifndef QT_NO_TRANSLATION
#include <stdlib.h>
#endif

#include "audiodevice.h"
#include "controlwidgetbase.h"
#include "loopcontrolbase.h"
#include "mediaselectorwidget.h"
#include "mediaplayer.h"


/*!
  \class MediaPlayer mediaplayer.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class creates all the elements of playing media files
    and ties them together. It abstracts what GUI elements are
    used with a few virtual functions: createMediaSelector(),
    createAudioUI() and createVideoUI(). This should make it
    possible to make some different frontends with the backend
    mostly the same abstracted behind an API like this. This
    API is subject to change.
*/


MediaPlayerState *mediaPlayerState = 0;

MediaPlayer::MediaPlayer( QWidget *parent, const char *name, WFlags )
    : QWidgetStack( parent, name ),
    scanSpeed( 0 ),
    upTimerId( 0 ), dnTimerId( 0 ),
    rtTimerId( 0 ), ltTimerId( 0 ),
    aw( 0 ), vw( 0 ), loopControlBase( 0 ), ad( 0 ), list( 0 ),
    docLnk( 0 ),
    disabledSuspendScreenSaver(false), previousSuspendMode(false)
{
#ifndef QT_NO_TRANSLATION
    QPEApplication::loadTranslations(QString("libmediaplayer"));
#endif

    setMinimumSize( 128, 128 );
    QObject::connect( qApp, SIGNAL( appMessage(const QCString&,const QByteArray&) ), this, SLOT( appMessage(const QCString&,const QByteArray&) ) );
    QTimer::singleShot( 0, this, SLOT( createMediaPlayer() ) );
    qApp->installEventFilter( this );
}


MediaPlayer::~MediaPlayer()
{
    delete docLnk;
    delete list;
    delete loopControlBase;
    delete mediaPlayerState;
    delete aw;
    delete vw;
}


void MediaPlayer::createMediaPlayer()
{
    QPainter p( this );
    QPixmap waitPix = Resource::loadPixmap( "bigwait" );
    p.drawPixmap( (width() - waitPix.width()) / 2, (height() - waitPix.height()) / 2, waitPix );

    mediaPlayerState = new MediaPlayerState( 0, "mediaPlayerState" );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ), this, SLOT( setView(View) ) );

    list = createMediaSelector();
    if ( list ) {
        addWidget( list->widget(), 0 );
        if ( docString.isEmpty() && url.isEmpty() ) {
            raiseWidget( list->widget() );
            // If started normally, show playlist first
            list->widget()->show(); // show 
            qApp->processEvents(); // (shows up now)
            // Start loading up the UIs up in the background if they haven't been already
            QTimer::singleShot( 1000, this, SLOT( audioUI() ) );
            QTimer::singleShot( 2000, this, SLOT( videoUI() ) );
        }
    }

    mediaPlayerState->loadPlugins(); // Load plugins and create loopControlBasecontrol and mediaplayer classes

    connect( mediaPlayerState, SIGNAL( playingToggled(bool) ),   this, SLOT( setPlaying(bool) ) );
    connect( mediaPlayerState, SIGNAL( pausedToggled(bool) ),    this, SLOT( pauseCheck(bool) ) );

    connect( mediaPlayerState, SIGNAL( increaseVolume() ),	 this, SLOT( startIncreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( endIncreaseVolume() ),	 this, SLOT( stopIncreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( decreaseVolume() ),	 this, SLOT( startDecreasingVolume() ) );
    connect( mediaPlayerState, SIGNAL( endDecreaseVolume() ),	 this, SLOT( stopDecreasingVolume() ) );

    connect( mediaPlayerState, SIGNAL( scanForward() ),		 this, SLOT( startScanningForward() ) );
    connect( mediaPlayerState, SIGNAL( endScanForward() ),	 this, SLOT( stopScanningForward() ) );
    connect( mediaPlayerState, SIGNAL( scanBackward() ),	 this, SLOT( startScanningBackward() ) );
    connect( mediaPlayerState, SIGNAL( endScanBackward() ),	 this, SLOT( stopScanningBackward() ) );

    setView( ListView );

    loopControlBase = createLoopControl();

    if ( !docString.isEmpty() )
        setDocument( docString );
    else if ( !url.isEmpty() )
        playURL( url, mimetype );
    else if ( docLnk )
	playDocLnk( docLnk );

    if ( list )
        list->loadFiles(); // Load files after getting the main window up and handling setDocuments
}


ControlWidgetBase *MediaPlayer::audioUI()
{
    if ( !aw ) {
        aw = createAudioUI();
        if ( aw )
            addWidget( aw, 1 );
    }
    return aw;
}


ControlWidgetBase *MediaPlayer::videoUI()
{
    if ( !vw ) {
        vw = createVideoUI();
        if ( vw )
            addWidget( vw, 2 );
    }
    return vw;
}


AudioDevice *MediaPlayer::audioDevice()
{
    if ( !ad )
        ad = createAudioDevice();
    return ad;
}


void MediaPlayer::setDocument( const QString& fileref )
{
    if ( loopControlBase ) { 
        if ( fileref.isNull() ) 
            error(tr("Invalid File"), tr("<qt>There was a problem in getting the file.</qt>"));
        else {
	    if ( list ) {
                // Apply heuristic on file to determine if media or playlist
                if( list->isPlaylist( fileref ) ) {
                    list->setPlaylist( fileref );
                } else {
                    list->setDocument( fileref );
                }
            }
            // Start playing
            mediaPlayerState->setPlaying(false);
            QTimer::singleShot( 1, mediaPlayerState, SLOT( setPlaying() ) );
        }
    } else {
	docString = fileref;
    }
}


void MediaPlayer::closeEvent( QCloseEvent *ce )
{
    if ( !visibleWidget() || visibleWidget()->close() )
	QWidgetStack::closeEvent( ce );
}


void MediaPlayer::appMessage( const QCString &msg, const QByteArray &data )
{
    /*@ \service PlayMedia */
    if ( msg == "openURL(QString,QString)" ) {
        /*@
            \message openURL(QString,QString)

            \list
            \i url = a fully-specified URL
            \i mimetype = the mimetype of the media file.
            \endlist
            
            */
	QDataStream stream( data, IO_ReadOnly );
	stream >> url >> mimetype;
	QPEApplication::setKeepRunning();
        playURL( url, mimetype );
    }
}


void MediaPlayer::setView( View view )
{
    switch ( view ) {
        case AudioView:
            if ( audioUI() )
	        raiseWidget( audioUI() );
            break;
        case VideoView:
            if ( videoUI() )
                raiseWidget( videoUI() );
            break;
        case ListView:
            if ( list )
                raiseWidget( list->widget() );
            break;
        default:
            break;
    }
}


bool MediaPlayer::eventFilter( QObject *o, QEvent *e )
{
    if ( e->type() == QEvent::WindowActivate && o == this && mediaPlayerState ) {
        if ( mediaPlayerState->view() == VideoView && videoUI() )
            videoUI()->makeVisible();
        if ( mediaPlayerState->view() == AudioView && audioUI() )
            audioUI()->makeVisible();
    }
    return QObject::eventFilter( o, e );
}



void MediaPlayer::pauseCheck( bool b )
{
    // Only pause if playing
    if ( b && !mediaPlayerState->playing() )
	mediaPlayerState->setPaused( false );
}


void MediaPlayer::error(const QString& error, const QString& errorMsg)
{
    QMessageBox::critical( 0, error, errorMsg );
    mediaPlayerState->setPlaying(false);
    mediaPlayerState->closeView();
}


void MediaPlayer::setTickerText(QString file)
{
    QString tickerText = " " + file + " ";
    long seconds = loopControlBase->totalPlaytime() / 1000;
    if ( seconds > 0 ) 
        tickerText += tr( "Length: %1" ).arg( ControlWidgetBase::toTimeString( seconds ) ) + "  ";

    if ( audioUI() ) 
        audioUI()->setTickerText( tickerText );

    if ( videoUI() ) 
        videoUI()->setTickerText( tickerText );
}


void MediaPlayer::setPlaying(bool play)
{
    if ( play ) {
        if ( mediaPlayerState->paused() ) {
            // resume from pause
            mediaPlayerState->setPaused(false);
        } else {
	    if ( list )
		playDocLnk(list->current());
	    else
		playDocLnk(docLnk);
        }
    } else {
	stop(false);
	mediaPlayerState->setPaused(false);
    }
}


void MediaPlayer::startDecreasingVolume()
{
    if ( dnTimerId )
	return;
    dnTimerId = startTimer( 100 );
    audioDevice()->decreaseVolume();
}


void MediaPlayer::startIncreasingVolume()
{
    if ( upTimerId )
	return; 
    upTimerId = startTimer( 100 );
    audioDevice()->increaseVolume();
}


void MediaPlayer::startScanningBackward()
{
    scanning = FALSE;
    if ( ltTimerId )
	return;
    scanSpeed = 64;
    mediaPlayerState->setSeeking( true );
    ltTimerId = startTimer( 100 );
}


void MediaPlayer::startScanningForward()
{
    scanning = FALSE;
    if ( rtTimerId )
	return;
    scanSpeed = 64;
    mediaPlayerState->setSeeking( true );
    rtTimerId = startTimer( 100 );
}


void MediaPlayer::stopDecreasingVolume()
{
    if ( dnTimerId ) {
	killTimer( dnTimerId );
	dnTimerId = 0;
    }
}


void MediaPlayer::stopIncreasingVolume()
{
    if ( upTimerId ) {
	killTimer( upTimerId );
	upTimerId = 0;
    }
}


void MediaPlayer::stopScanningBackward()
{
    if ( ltTimerId ) {
	killTimer( ltTimerId );
	ltTimerId = 0;
	if ( rtTimerId )
	    return;
	mediaPlayerState->setSeeking( false );
        if ( !scanning )
            mediaPlayerState->setPrev();
    }
}


void MediaPlayer::stopScanningForward()
{
    if ( rtTimerId ) {
	killTimer( rtTimerId );
	rtTimerId = 0;
	if ( ltTimerId )
	    return;
	mediaPlayerState->setSeeking( false );
        if ( !scanning ) 
            mediaPlayerState->setNext();
    }
}


void MediaPlayer::timerEvent( QTimerEvent *te )
{
    scanSpeed = (scanSpeed < 8) ? 8 : scanSpeed;
    if ( te->timerId() == dnTimerId ) {
        audioDevice()->decreaseVolume();
        if ( audioUI() )
	    audioUI()->updateVolume();
        if ( videoUI() )
	    videoUI()->updateVolume();
    } else if ( te->timerId() == upTimerId ) {
	audioDevice()->increaseVolume();
        if ( audioUI() )
	    audioUI()->updateVolume();
        if ( videoUI() )
	    videoUI()->updateVolume();
    } else if ( te->timerId() == ltTimerId ) {
	internalScan(Left);
    } else if ( te->timerId() == rtTimerId ) {
	internalScan(Right);
    }
}


void MediaPlayer::internalScan(ScanDirection scanDirection)
{
    scanning = TRUE;
    if ( mediaPlayerState->seekable() ) {
	int length = mediaPlayerState->length();
	int increment = length / scanSpeed;
	scanSpeed = scanSpeed * 7 / 8;
	int newPos = mediaPlayerState->position() + ((scanDirection == Right) ? increment : -increment);
	if ( newPos > 0 && newPos <= length )
	    mediaPlayerState->setPosition( newPos );
	else if ( newPos > length )
	    mediaPlayerState->setPosition( length );
	else
	    mediaPlayerState->setPosition( 0 );
    }
}



void MediaPlayer::play()
{
    bool hasVideo = loopControlBase->hasVideo();
    if ( !disabledSuspendScreenSaver || previousSuspendMode != hasVideo ) {
	disabledSuspendScreenSaver = TRUE; 
	previousSuspendMode = hasVideo;
        // Stop the screen from blanking and power saving state
	QPEApplication::setTempScreenSaverMode(hasVideo ? QPEApplication::Disable : QPEApplication::DisableSuspend);
    }
    mediaPlayerState->setView( hasVideo ? VideoView : AudioView );
    loopControlBase->start();
    mediaPlayerState->setPlaying( true );
}


void MediaPlayer::stop(bool willPlayAgainShortly)
{
    if ( !willPlayAgainShortly && disabledSuspendScreenSaver ) {
	disabledSuspendScreenSaver = FALSE; 
	// Re-enable the suspend mode
	QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
    }
    loopControlBase->stop();
}


static inline bool is_url( const QString& string )
{
    return string.contains( "://" );
}


void MediaPlayer::playDocLnk(const DocLnk *lnk)
{
    if ( !loopControlBase )
	return;
    if ( !lnk ) {
	return;
    }
    delete docLnk;
    docLnk = new DocLnk(*lnk);

    QString file = docLnk->file();
    QString name = docLnk->name();

    qDebug("playDocLnk %s %s", file.latin1(), name.latin1());

    // Apply heuristic on doclnk to determine whether to play url or file
    if( is_url( file ) ) {
        playURL( file, QString::null );
    } else {
        playFile( file, name );
    } 
}


void MediaPlayer::playFile(const QString &fileStr, const QString &nameStr)
{
    stop(true);
    mediaPlayerState->setPaused(false);

    if ( !QFile::exists( fileStr ) ) {
	error( tr( "File not found" ), tr( "<qt>The following file was not found: <br>%1<br><i>%2</i></qt>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    if ( !mediaPlayerState->decoder( fileStr ) ) {
	error( tr( "No decoder found" ), tr( "<qt>Sorry, no appropriate decoders found for this file: <br>%1<br><i>%2</i></qt>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    if ( !loopControlBase->init(fileStr) ) {
	error( tr( "Error opening file" ), tr( "<qt>Sorry, an error occurred trying to play the file: <br>%1<br><i>%2</i></qt>" ).arg( nameStr ).arg( fileStr ) );
	return;
    }

    play();

    setTickerText(tr("File: %1").arg(nameStr));
}


void MediaPlayer::playURL(const QString &URL, const QString &mime)
{
    url = URL;
    mimetype = mime;
    if ( !loopControlBase )
	return;

    stop(true);
    mediaPlayerState->setPaused(false);

    if ( !mediaPlayerState->streamingDecoder(URL, mimetype) ) {
	error( tr("No decoder found"), tr("<qt>Sorry, no appropriate decoders found for the url: <i>%1</i></qt>").arg(URL) );
	return;
    }

    if ( !loopControlBase->init(URL, mimetype, true) ) {
	error( tr("Error opening file"), tr("<qt>Sorry, an error occurred trying to play the url: <i>%1</i></qt>").arg(URL) );
	return;
    }

    play();

    setTickerText(tr("URL: %1").arg(URL));
}


