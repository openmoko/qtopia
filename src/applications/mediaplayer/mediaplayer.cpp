/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mediaplayer.h"

#include "playercontrol.h"
#include "playerwidget.h"
#include "mediabrowser.h"

#include <qmediatools.h>
#include <requesthandler.h>

#include <qtopiaapplication.h>

class MediaServiceRequestHandler : public RequestHandler
{
public:
    struct Context
    {
        MediaPlayer *mediaplayer;
    };

    MediaServiceRequestHandler( const Context& context, RequestHandler* handler = 0 )
        : RequestHandler( handler ), m_context( context )
    { }

    // RequestHandler
    void execute( ServiceRequest* request );

private:
    Context m_context;
};

void MediaServiceRequestHandler::execute( ServiceRequest* request )
{
    switch( request->type() )
    {
    case ServiceRequest::OpenUrl:
        {
        OpenUrlRequest *req = (OpenUrlRequest*)request;
        m_context.mediaplayer->openUrl( req->url() );

        delete request;
        }
        break;
    case ServiceRequest::OpenPlaylist:
        {
        OpenPlaylistRequest *req = (OpenPlaylistRequest*)request;
        m_context.mediaplayer->setPlaylist( req->playlist() );

        delete request;
        }
        break;
    case ServiceRequest::CuePlaylist:
        {
        CuePlaylistRequest *req = (CuePlaylistRequest*)request;
        PlaylistCue *playlistcue = qobject_cast<PlaylistCue*>(m_context.mediaplayer->playlist());
        if( playlistcue ) {
            playlistcue->cue( req->playlist() );
        }

        delete request;
        }
        break;
    case ServiceRequest::ShowPlayer:
        {
        m_context.mediaplayer->setPlayerVisible( true );

        delete request;
        }
        break;
    default:
        RequestHandler::execute( request );
        break;
    }
}

MediaPlayer::MediaPlayer( QWidget* parent, Qt::WFlags f ):
    QWidget( parent, f ),
    m_playerwidget( 0 ),
    m_closeonback( false ),
    m_playlist( 0 )
{
    setWindowTitle( tr( "Media Player" ) );

    m_layout = new QVBoxLayout;

    m_playercontrol = new PlayerControl( this );

    m_playerwidget = new PlayerWidget( m_playercontrol );

    m_layout->addWidget( m_playerwidget );

    MediaServiceRequestHandler::Context requestcontext = { this };
    m_requesthandler = new MediaServiceRequestHandler( requestcontext );

    m_playlist = new BasicPlaylist( QStringList() );

    m_mediabrowser = new MediaBrowser( m_playercontrol, m_requesthandler );

    m_mediabrowser->setCurrentPlaylist( m_playlist );

    m_layout->addWidget( m_mediabrowser );
    setLayout( m_layout );

    setPlayerVisible( false );

    QMediaContentContext *context = new QMediaContentContext( this );
    connect( m_playercontrol, SIGNAL(contentChanged(QMediaContent*)),
        context, SLOT(setMediaContent(QMediaContent*)) );
    context->addObject( m_playerwidget );
    context->addObject( m_mediabrowser );
}

void MediaPlayer::setPlaylist( Playlist* playlist )
{
    if( playlist != m_playlist ) {
        // Open playlist in player
        m_mediabrowser->setCurrentPlaylist( playlist );
        m_playerwidget->setPlaylist( playlist );

        delete m_playlist;
        m_playlist = playlist;

        // Connect to new playlist
        connect( m_playlist, SIGNAL(playingChanged(const QModelIndex&)),
            this, SLOT(playingChanged(const QModelIndex&)) );
    }

    setPlayerVisible( true );
}

void MediaPlayer::setPlayerVisible( bool visible )
{
    if( visible ) {
        m_playerwidget->show();
        m_playerwidget->setFocus();

        m_mediabrowser->hide();
    } else {
        m_playerwidget->hide();

        m_mediabrowser->show();
        m_mediabrowser->setFocus();
    }
}

static Playlist* construct_playlist( const QString& filename )
{
#define SUFFIX_LENGTH 4

    QString suffix = filename.right( SUFFIX_LENGTH );

    if( suffix == ".m3u" ) {
        return new M3UPlaylist( filename );
    }

    if( suffix == ".pls" ) {
        return new PLSPlaylist( filename );
    }

    return new BasicPlaylist( QStringList( filename ) );
}

void MediaPlayer::openUrl( const QString& url )
{
    Playlist *playlist = construct_playlist( url );
    playlist->setPlaying( playlist->index( 0 ) );

    setPlaylist( playlist );
}

void MediaPlayer::setDocument( const QString& doc )
{
    m_closeonback = true;

    Playlist *playlist = construct_playlist( doc );
    playlist->setPlaying( playlist->index( 0 ) );

    setPlaylist( playlist );
}

void MediaPlayer::playingChanged( const QModelIndex& index )
{
    if( !index.isValid() ) {
        setPlayerVisible( false );
    }
}

void MediaPlayer::closeEvent( QCloseEvent* e )
{
    if( !m_closeonback ) {
        if( m_playerwidget->isVisible() ) {
            setPlayerVisible( false );
            e->ignore();
        } else if( m_mediabrowser->hasBack() ) {
            m_mediabrowser->goBack();
            e->ignore();
        } else if( m_playercontrol->state() != PlayerControl::Stopped ) {
            // Hide if player active
            hide();
            e->ignore();
        } else {
            m_closeonback = true;
        }
    }

    if( m_closeonback ) {
        QtopiaApplication::instance()->unregisterRunningTask( this );
    } else {
        QtopiaApplication::instance()->registerRunningTask( "Media Player", this );
    }
}
