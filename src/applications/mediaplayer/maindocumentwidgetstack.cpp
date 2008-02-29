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
#include <qpainter.h>
#include <qtimer.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include "mediaplayerstate.h"
#include "playlistwidget.h"
#include "loopcontrol.h"
#include "mediaplayer.h"
#include "maindocumentwidgetstack.h"


MediaPlayerState *mediaPlayerState = 0;
MediaPlayer *mediaPlayer = 0;
PlayListWidget *playList = 0;
LoopControl *loopControl = 0;
MainDocumentWidgetStack *mainDocumentWindow = 0;


MainDocumentWidgetStack::MainDocumentWidgetStack( QWidget *parent, const char *name, WFlags /* fl */)
    : QWidgetStack( parent, name )
{
    loading = TRUE;
    setCaption( tr("Media Player") );
    setMinimumSize( 128, 128 );
    QObject::connect( qApp, SIGNAL( appMessage(const QCString &, const QByteArray &) ), this, SLOT( appMessage(const QCString &, const QByteArray &) ) );
    mainDocumentWindow = this;

    QTimer::singleShot( 0, this, SLOT( createMediaPlayer() ) );
}


void MainDocumentWidgetStack::setLoading( bool b )
{
    loading = b;
    if ( !loading ) {
	if ( !docString.isEmpty() ) 
	    emit handleSetDocument( docString );
	else if ( !url.isEmpty() )
	    emit openURL( url, mimetype );
    }
}


bool MainDocumentWidgetStack::havePendingSignals()
{
    return !docString.isEmpty() || !url.isEmpty();
}


void MainDocumentWidgetStack::setDocument( const QString& fileref )
{
    if ( loading )
	docString = fileref;
    else
	emit handleSetDocument( fileref );
}


void MainDocumentWidgetStack::closeEvent( QCloseEvent *ce )
{
    if ( !visibleWidget() || visibleWidget()->close() )
	QWidgetStack::closeEvent( ce );
}


void MainDocumentWidgetStack::appMessage( const QCString &msg, const QByteArray &data )
{
    if ( msg == "openURL(QString,QString)" ) {
	QDataStream stream( data, IO_ReadOnly );
	stream >> url >> mimetype;
	QPEApplication::setKeepRunning();
    }
}


MainDocumentWidgetStack::~MainDocumentWidgetStack()
{
    delete mediaPlayer;
    delete loopControl;
    delete playList;
    delete mediaPlayerState;
}


void MainDocumentWidgetStack::setWaiting( bool w )
{
    if ( waiting != w ) {
	waiting = w;
	if ( waiting ) {	
	    QPainter p( this );
	    QPixmap waitPix = Resource::loadPixmap( "wait" );
	    p.drawPixmap( (width() - waitPix.width()) / 2, (height() - waitPix.height()) / 2, waitPix );
//	    a.processEvents(); // (show hourglass icon while loading)
	} else {

	}
    }
}


void MainDocumentWidgetStack::createMediaPlayer()
{
    setWaiting( true );

    mediaPlayerState = new MediaPlayerState( 0, "mediaPlayerState" );

    playList = new PlayListWidget( this, "playList" );
    addWidget( playList, 0 );
    playList->setCaption( tr("Media Player") );

    if ( !havePendingSignals() ) {
	raiseWidget( playList );
	// If started normally, show playlist first
	playList->show(); // show 
	qApp->processEvents(); // (shows up now)
    }
    // Set for recieving setDocument
    QObject::connect( this, SIGNAL( handleSetDocument( const QString& ) ), playList, SLOT( setDocument( const QString& ) ) );
    
    mediaPlayerState->loadPlugins(); // Load plugins and create loopcontrol and mediaplayer classes
    loopControl = new LoopControl( 0, "loopControl" );
    mediaPlayer = new MediaPlayer( 0, "mediaPlayer" );

    setLoading( FALSE );

    playList->loadFiles(); // Load files after getting the main window up and handling setDocuments
}

