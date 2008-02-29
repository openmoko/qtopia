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

/*!
    \class QSoundControl
    \mainclass

    \brief The QSoundControl class extends the functionality of the QSound
    class.

    The QSoundControl class can be used to control the volume, priority and
    provide the status for a QSound object.

    \ingroup multimedia

    Construct a QSoundControl object with an exising QSound object.

    \code
        QSoundControl *control = new QSoundControl( new QSound( "test.wav" ) );
    \endcode

    The control may now be used to alter the behavior of the sound object.

    \code
        control->setVolume( 50 );
        control->setPriority( QSoundControl::RingTone );
    \endcode

    In additon the control may be used to listen for when the sound object
    finishes playback.

    \code
        connect( control, SIGNAL(done()), this, SLOT(doSomething()) );
    \endcode

    The control object does not take owenership of the sound object. Care must
    be taken to ensure the sound object remains valid for the life of the
    control object.

    \code
        delete control->sound();
        delete control;
    \endcode

    \sa QSound
*/

/*!
    \enum QSoundControl::Priority

    This enum type specifies the different priorities available for QSound
    objects.

    \value Default
        This is the lowest priority.
    \value RingTone
        This is the highest priority. Sounds with this priority will be played
        in preference to all other sounds.
*/

/*!
    \fn QSoundControl::QSoundControl( QSound* sound, QObject* parent )

    Constructs a sound control for the given \a sound object and a \a parent.
    Ownership of the sound object is not taken.
*/
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

/*!
    \fn void QSoundControl::setVolume( int volume )

    Adjusts the volume of the sound object to \a volume. The given value should
    be between 0 and 100.
*/
void QSoundControl::setVolume( int volume )
{
    m_volume = volume;

    if( !m_id.isNull() ) {
        QCopMessage message( SERVER_CHANNEL, "setVolume(QUuid,int)" );
        message << m_id << m_volume;
    }
}

/*!
    \fn int QSoundControl::volume() const

    Returns the current volume of the sound object.
*/

/*!
    \fn void QSoundControl::setPriority( Priority priority )

    Sets the priority of the sound object to \a priority.
*/
void QSoundControl::setPriority( Priority priority )
{
    m_priority = priority;

    if( !m_id.isNull() ) {
        QCopMessage message( SERVER_CHANNEL, "setPriority(QUuid,int)" );
        message << m_id << m_priority;
    }
}

/*!
    \fn Priority QSoundControl::priority() const

    Returns the current priority of the sound object.
*/

/*!
    \fn QSound* QSoundControl::sound() const

    Returns the sound object the control is associated with.
*/

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

/*!
    \fn void QSoundControl::done()

    This signal is emitted when the sound object finishes playing. For looped
    sound object this signal will only be emitted once all loops have played
    through.
*/

