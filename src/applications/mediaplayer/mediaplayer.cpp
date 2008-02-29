/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <private/keyhold_p.h>
#include <private/requesthandler_p.h>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#ifndef NO_NICE
#include <unistd.h>
#endif

class MediaServiceRequestHandler : public RequestHandler
{
public:
    struct Context
    {
        MediaPlayer *mediaplayer;
        PlayerControl *control;
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
    case ServiceRequest::PlayNow:
        {
        PlayNowRequest *req = (PlayNowRequest*)request;
        PlaylistCue *playlistcue = qobject_cast<PlaylistCue*>(m_context.mediaplayer->playlist());
        if( playlistcue ) {
            playlistcue->playNow( req->playlist() );
            m_context.control->setState( PlayerControl::Playing );
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

static const int KEY_BACK_HOLD = Qt::Key_unknown + Qt::Key_Back;

MediaPlayer::MediaPlayer( QWidget* parent, Qt::WFlags f ):
    QWidget( parent, f ),
    m_playerwidget( 0 ),
    m_closeonback( false ),
    m_acceptclose( false ),
    m_playlist( 0 )
{
    setWindowTitle( tr( "Media Player" ) );

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_playercontrol = new PlayerControl( this );

    m_playerwidget = new PlayerWidget( m_playercontrol );

    m_layout->addWidget( m_playerwidget );

    MediaServiceRequestHandler::Context requestcontext = { this, m_playercontrol };
    m_requesthandler = new MediaServiceRequestHandler( requestcontext );

    m_playlist = new BasicPlaylist( QStringList() );

    m_mediabrowser = new MediaBrowser( m_playercontrol, m_requesthandler );

#ifdef QTOPIA_KEYPAD_NAVIGATION
#ifndef NO_HELIX
    QSoftMenuBar::menuFor( m_mediabrowser )->addAction( m_playerwidget->settingsAction() );
#endif
#endif

    m_mediabrowser->setCurrentPlaylist( m_playlist );
    m_playerwidget->setPlaylist( m_playlist );

    m_layout->addWidget( m_mediabrowser );
    setLayout( m_layout );

    setPlayerVisible( false );

    QMediaContentContext *context = new QMediaContentContext( this );
    connect( m_playercontrol, SIGNAL(contentChanged(QMediaContent*)),
        context, SLOT(setMediaContent(QMediaContent*)) );
    context->addObject( m_playerwidget );
    context->addObject( m_mediabrowser );

    new KeyHold( Qt::Key_Back, KEY_BACK_HOLD, 500, this, this );

    QtopiaApplication::instance()->registerRunningTask( "Media Player", this );

    // Initialize volume
    QSettings config( "Trolltech", "MediaPlayer" );
    int volume = config.value( "Volume", 50 ).toInt();

    m_playercontrol->setVolume( volume );

#ifndef NO_NICE
    static const int NICE_DELTA = -15;

    // Increase process priority to improve gui responsiveness
    nice( NICE_DELTA );
#endif
}

MediaPlayer::~MediaPlayer()
{
    delete m_playlist;
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
        connect( m_playlist, SIGNAL(playingChanged(QModelIndex)),
            this, SLOT(playingChanged(QModelIndex)) );
    }

    // Launch playlist if playing index is valid
    if( m_playlist->playing().isValid() ) {
        setPlayerVisible( true );
        m_playercontrol->setState( PlayerControl::Playing );
    }
}

bool MediaPlayer::isPlayerVisible() const
{
    return m_playerwidget->isVisible();
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

void MediaPlayer::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Back:
        if( !m_closeonback ) {
            if( isPlayerVisible() ) {
                setPlayerVisible( false );
            } else if( m_mediabrowser->hasBack() ) {
                m_mediabrowser->goBack();
            } else if( m_playercontrol->state() != PlayerControl::Stopped ) {
                // Hide if player active
                hide();
            } else {
                m_closeonback = true;
            }
        }

        if( m_closeonback ) {
            m_acceptclose = true;
            m_playercontrol->close();
        }
        break;
    case KEY_BACK_HOLD:
        if( isPlayerVisible() ) {
            setPlayerVisible( false );
        }

        // Return to main menu
        while( m_mediabrowser->hasBack() ) {
            m_mediabrowser->goBack();
        }
        break;

    case Qt::Key_Hangup:
        if (m_playercontrol->state() != PlayerControl::Stopped)
            hide();
        else
        {
            m_acceptclose = true;
            m_playercontrol->close();
        }
        break;

    default:
        // Ignore
        break;
    }

    e->ignore();
}

void MediaPlayer::closeEvent( QCloseEvent* e )
{
    if( m_acceptclose ) {
        QtopiaApplication::instance()->unregisterRunningTask( this );
        e->accept();

        // Save volume settings
        QSettings config( "Trolltech", "MediaPlayer" );
        config.setValue( "Volume", m_playercontrol->volume() );
    } else {
        // ### FIXME Ensure focused widget always has edit focus
        focusWidget()->setEditFocus( true );

        e->ignore();
    }
}


