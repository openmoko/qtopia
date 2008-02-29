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

#include "playercontrol.h"

static const int VOLUME_MAX = 100;

PlayerControl::PlayerControl( QObject* parent )
    : QObject( parent ), m_content( 0 ), m_control( 0 ), m_state( Stopped ), m_volume( VOLUME_MAX ), m_ismute( false )
{
    m_notifier = new QMediaControlNotifier( QMediaControl::name(), this );
    connect( m_notifier, SIGNAL(valid()), this, SLOT(activate()) );
}

PlayerControl::~PlayerControl()
{
    delete m_control;
    delete m_content;
}

void PlayerControl::open( const QString& url )
{
    setMediaContent( new QMediaContent( QUrl(url) ) );
}

void PlayerControl::open( const QContent& content )
{
    setMediaContent( new QMediaContent( content ) );
}

void PlayerControl::close()
{
    setMediaContent( 0 );
}

void PlayerControl::setState( State state )
{
    if( state != m_state ) {
        m_state = state;

        if( m_control ) {
            switch( m_state )
            {
            case Playing:
                m_control->start();
                break;
            case Paused:
                m_control->pause();
                break;
            case Stopped:
                m_control->stop();
                break;
            }
        }

        emit stateChanged( m_state );
    }
}

void PlayerControl::setVolume( int volume )
{
    m_volume = volume;
}

void PlayerControl::setMute( bool mute )
{
    m_ismute = mute;
}

void PlayerControl::activate()
{
    m_control = new QMediaControl( m_content );
    connect( m_control, SIGNAL(volumeChanged(int)),
        this, SLOT(setVolume(int)) );
    connect( m_control, SIGNAL(volumeMuted(bool)),
        this, SLOT(setMute(bool)) );

    m_control->setVolume( m_volume );
    m_control->setMuted( m_ismute );

    if( state() == Playing ) {
        m_control->start();
    }
}

void PlayerControl::setMediaContent( QMediaContent* content )
{
    delete m_control;
    m_control = 0;

    m_notifier->setMediaContent( content );
    emit contentChanged( content );

    delete m_content;
    m_content = content;
}
