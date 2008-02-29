/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "qsoundcontrol.h"

#define SERVER_CHANNEL "QPE/MediaServer"

class QCopMessage : public QDataStream
{
public:
    QCopMessage( const QString& channel, const QString& message )
        : QDataStream( new QBuffer ), m_channel( channel ), m_message( message )
    {
        device()->open( QIODevice::WriteOnly );
    }

    ~QCopMessage()
    {
#ifndef QT_NO_COP
        QCopChannel::send( m_channel, m_message, ((QBuffer*)device())->buffer() );
#endif
        delete device();
    }

private:
    QString m_channel;
    QString m_message;
};

QSoundControl::QSoundControl( QSound* sound, QObject* parent )
    : QObject( parent ), m_sound( sound ), m_volume( 100 ), m_priority( Default ), m_channel( 0 )
{
#ifndef QT_NO_COP
    m_channel = new QCopChannel( QString( "QPE/QSound/" ).append( m_sound->objectName() ), this );
    connect( m_channel, SIGNAL(received(const QString&,const QByteArray&)),
        this, SLOT(processMessage(const QString&,const QByteArray&)) );
#endif

    m_id = m_sound->objectName();
}

void QSoundControl::setVolume( int volume )
{
    m_volume = volume;

    if( !m_id.isNull() ) {
        QCopMessage message( SERVER_CHANNEL, "setVolume(QUuid,int)" );
        message << m_id << m_volume;
    }
}

void QSoundControl::setPriority( Priority priority )
{
    m_priority = priority;

    if( !m_id.isNull() ) {
        QCopMessage message( SERVER_CHANNEL, "setPriority(QUuid,int)" );
        message << m_id << m_priority;
    }
}

void QSoundControl::processMessage( const QString& msg, const QByteArray& data )
{
    Q_UNUSED(data);
    if( msg == "done()" ) {
        // If sound done, emit done signal
        if( !m_sound->loopsRemaining() ) {
            emit done();
        }
    }
}
