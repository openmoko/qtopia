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
#include <stdlib.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include "audiodevice.h"

#if ( defined Q_WS_QWS || defined(_WS_QWS_) ) && !defined(QT_NO_COP)
#include "qpe/qcopenvelope_qws.h"
#endif

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <qsoundqss_qws.h>
#include <qtimer.h>

static void _sleep( long sleep )
{
    struct timespec time, elapsed;
    time.tv_sec = 0;
    time.tv_nsec = 1000000*sleep;
    int ret = nanosleep( &time, &elapsed );
    while( ret == -1 && errno == EINTR ) {
        struct timespec remaining = elapsed;
        nanosleep( &remaining, &elapsed );
    }
}

#define BUFFER_SIZE 1024

class AudioDevicePrivate : public QObject 
{
    Q_OBJECT
public:
    struct DeviceSettings {
        int channels;
        int speed;
    };
    
    AudioDevicePrivate( QObject* parent );
    
    ~AudioDevicePrivate();

    // Open audio device with given settings
    void open( unsigned int frequency, unsigned int channels );
    
    // Send completedIO signal after writing to device
    void setSignalCompleted( bool signal );
    
    unsigned int channels() const;
    
    unsigned int frequency() const;
    
    // Close audio device
    void close();
    
    // Write buffer to audio device
    // ### Currently only supports AFMT_S16_LE format data
    bool write( char* buffer, unsigned int length );
    
    static bool muted();
    
    static void setMuted( bool mute );
    
    // Assign current volume to left and right
    static void volume( int& left, int& right );
    
    // Set current volume
    // Only the left value is used
    static void setVolume( int left, int right );
    
signals:
    void completedIO();

    void deviceReady();
    
    void deviceOpenError();
    
private slots:
    // Process error from client
    void clientError( int );

    // Process message from server
    void receive( QSSMessage message );
        
private:
    enum State { Closed, Pending, Opened, Interrupted };
    
    enum Event { Open, Close, Release, Ack, Nak, Deferred };
    
    // Perform transition for given event
    void execute( Event event );
    
    // Perform transition behavior for state begin to state end
    void transition( State begin, State end );

    // Open audio device and set device parameters
    void openAudioDevice(); 
    
    // Close audio device
    void closeAudioDevice();
    
    // Stereo to stereo write delegate
    void stereo_stereo( char* in, unsigned int length );
    
    // Stereo to mono write delegate
    void stereo_mono( char* in, unsigned int length );
    
    // Mono to mono write delegate
    void mono_mono( char* in, unsigned int length );
    
    // Mono to stereo write delegate
    void mono_stereo( char* in, unsigned int length );
    
    int dsp;
    DeviceSettings requested, actual;
    int current_speed;
    bool signal_completed;
    
    static int volume_;
    static bool muted_;
    static pthread_mutex_t mutex;
    
    short write_buffer[BUFFER_SIZE/2];
    void (AudioDevicePrivate::*write_delegate)( char* in, unsigned int length );
    
    State current_state;
    bool client_error;
    QWSSoundClient client;
};

int AudioDevicePrivate::volume_ = 0xFFFF;

bool AudioDevicePrivate::muted_ = false;

pthread_mutex_t AudioDevicePrivate::mutex = PTHREAD_MUTEX_INITIALIZER;

AudioDevicePrivate::AudioDevicePrivate( QObject* parent )
    : QObject( parent )
{
    connect( &client, SIGNAL( received( QSSMessage ) ), this, SLOT( receive( QSSMessage ) ) );
    
    connect( &client, SIGNAL( error( int ) ), this, SLOT( clientError( int ) ) );
    
    current_state = Closed;
    
    client_error = false;
}

AudioDevicePrivate::~AudioDevicePrivate()
{
    if( current_state != Closed ) {
        execute( Close );
    }
}

void AudioDevicePrivate::open( unsigned int frequency, unsigned int channels )
{
    requested.channels = channels;
    requested.speed = frequency;
    execute( Open );
}

void AudioDevicePrivate::setSignalCompleted( bool signal )
{
    signal_completed = signal;
}

unsigned int AudioDevicePrivate::channels() const
{
    return requested.channels;
}

unsigned int AudioDevicePrivate::frequency() const
{
    return requested.speed;
}

void AudioDevicePrivate::close()
{
    execute( Close );
}

bool AudioDevicePrivate::write( char* in, unsigned int length )
{
    int result = false;
    
    pthread_mutex_lock( &mutex );
    
    // If audio device open, process audio and write to device
    // Otherwise, simulate write to device
    if( current_state == Opened && !muted_ ) {
        (this->*write_delegate)( in, length );
        if( signal_completed ) {
            QTimer::singleShot( 0, this, SIGNAL( completedIO() ) );
        }
        result = true;
    } else {
        // Simulate write to device
        _sleep( 1000*length/(requested.speed*requested.channels*2) );
        if( signal_completed ) {
            QTimer::singleShot( 0, this, SIGNAL( completedIO() ) );
        }
        result = true;
    }
    
    pthread_mutex_unlock( &mutex );
    
    return result;
}

void AudioDevicePrivate::stereo_stereo( char* in, unsigned int length )
{
    short *i = (short*)in;
    while( length ) {
        int j = 0;
        short *o = write_buffer;
        while( j < BUFFER_SIZE && length ) {
            // Get left and right samples
            // Apply gain to both channels
            int l = (*i)*volume_/0xFFFF;
            int r = (*(i+1))*volume_/0xFFFF;
    
            // Resample
            current_speed += actual.speed;
            while( j < BUFFER_SIZE && current_speed >= requested.speed ) {
                current_speed -= requested.speed;
                *o++ = l;
                *o++ = r;
                j += 4;
            }
            if( current_speed < requested.speed ) {
                i += 2;
                length -= 4;
            }
         }
         ::write( dsp, write_buffer, j );
    }
}
    
void AudioDevicePrivate::stereo_mono( char* in, unsigned int length )
{
    short *i = (short*)in;
    while( length ) {
        int j = 0;
        short *o = write_buffer;
        while( j < BUFFER_SIZE && length ) {
            // Add both channels together and limit result
            int l = *i + *(i+1);
            l = QMAX( QMIN( l, 32767 ), -32768 );
            // Apply gain
            l = l*volume_/0xFFFF;
        
            // Resample
            current_speed += actual.speed;
            while( j < BUFFER_SIZE && current_speed >= requested.speed ) {
                current_speed -= requested.speed;
                *o++ = l;
                j += 2;
            }
            if( current_speed < requested.speed ) {
                i += 2;
                length -= 4;
            }
         }
        ::write( dsp, write_buffer, j );
    }
}
    
void AudioDevicePrivate::mono_mono( char* in, unsigned int length )
{
    short *i = (short*)in;
    while( length ) {
        int j = 0;
        short *o = write_buffer;
        while( j < BUFFER_SIZE && length ) {
            // Get sample and apply gain
            int l = (*i)*volume_/0xFFFF;
        
            // Resample
            current_speed += actual.speed;
            while( j < BUFFER_SIZE && current_speed >= requested.speed ) {
                current_speed -= requested.speed;
                *o++ = l;
                j += 2;
            }
            if( current_speed < requested.speed ) {
                ++i;
                length -= 2;
            }
         }
        ::write( dsp, write_buffer, j );
    }
}
    
void AudioDevicePrivate::mono_stereo( char* in, unsigned int length )
{
    short *i = (short*)in;
    while( length ) {
        int j = 0;
        short *o = write_buffer;
        while( j < BUFFER_SIZE && length ) {
            // Get sample and apply gain
            int l = (*i)*volume_/0xFFFF;
        
            // Resample
            current_speed += actual.speed;
            while( j < BUFFER_SIZE && current_speed >= requested.speed ) {
                current_speed -= requested.speed;
                *o++ = l;
                *o++ = l;
                j += 4;
            }
            if( current_speed < requested.speed ) {
                ++i;
                length -= 2;
            }
         }
        ::write( dsp, write_buffer, j );
    }
}

bool AudioDevicePrivate::muted()
{
    bool mute;
    
    pthread_mutex_lock( &mutex );
    
    mute = muted_;
    
    pthread_mutex_unlock( &mutex );
    
    return mute;
}

void AudioDevicePrivate::setMuted( bool mute )
{
    pthread_mutex_lock( &mutex );
    
    muted_ = mute;
    
    pthread_mutex_unlock( &mutex );
}

void AudioDevicePrivate::volume( int& left, int& right )
{
    pthread_mutex_lock( &mutex );
    
    left = right = AudioDevicePrivate::volume_;
    
    pthread_mutex_unlock( &mutex );
}

#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )

void AudioDevicePrivate::setVolume( int left, int right )
{
    pthread_mutex_lock( &mutex );
    
    AudioDevicePrivate::volume_ = LIMIT( left, 0, 0xFFFF );
    
    pthread_mutex_unlock( &mutex );
}

void AudioDevicePrivate::execute( Event event )
{
    pthread_mutex_lock( &mutex );
        
    switch( current_state )
    {
    case Closed:
        switch( event )
        {
        case Open:
            if( client_error ) {
                emit deviceOpenError();
            } else {
                transition( Closed, Pending );
                current_state = Pending;
            }
            break;
        default:
            qDebug( "AudioDevicePrivate::execute state %d unexpected event %d", current_state, event );
        }
        break;
    case Pending:
        switch( event )
        {
        case Ack:
            transition( Pending, Opened );
            current_state = Opened;
            break;
        case Nak:
            transition( Pending, Closed );
            current_state = Closed;
            break;
        case Deferred:
            transition( Pending, Interrupted );
            current_state = Interrupted;
            break;
        default:
            qDebug( "AudioDevicePrivate::execute state %d unexpected event %d", current_state, event );
        }
        break;
    case Opened:
        switch( event )
        {
        case Close:
            transition( Opened, Closed );
            current_state = Closed;
            break;
        case Release:
            transition( Opened, Interrupted );
            current_state = Interrupted;
            break;
        default:
            qDebug( "AudioDevicePrivate::execute state %d unexpected event %d", current_state, event );
        }
        break;
    case Interrupted:
        switch( event )
        {
        case Close:
            transition( Interrupted, Closed );
            current_state = Closed;
            break;
        case Ack:
            transition( Interrupted, Opened );
            current_state = Opened;
            break;
        default:
            qDebug( "AudioDevicePrivate::execute state %d unexpected event %d", current_state, event );
        }
        break;
    }
    
    pthread_mutex_unlock( &mutex );
}

void AudioDevicePrivate::transition( State begin, State end )
{
    switch( begin )
    {
    case Closed:
        switch( end )
        {
        case Pending:
            qDebug( "AudioDevicePrivate::transition( Closed, Pending )" );
            // Request audio device from server
            client.send( QSS_Request );
            break;
        }
        break;
    case Pending:
        switch( end )
        {
        case Closed:
            qDebug( "AudioDevicePrivate::transition( Pending, Closed )" );
            // Request denied
            // Fake device open
            emit deviceReady();
            break;
        case Opened:
            qDebug( "AudioDevicePrivate::transition( Pending, Opened )" );
            // Request granted, open audio device
            openAudioDevice();
            break;
        case Interrupted:
            qDebug( "AudioDevicePrivate::transition( Pending, Interrupted )" );
            // Request deferred
            // Fake device open
            emit deviceReady();
            break;
        }
        break;
    case Opened:
        switch( end )
        {
        case Closed:
            qDebug( "AudioDevicePrivate::transition( Opened, Closed )" );
            // Close audio device
            closeAudioDevice();
            // Send notification of release to server
            client.send( QSS_Release );
            break;
        case Interrupted:
            qDebug( "AudioDevicePrivate::transition( Opened, Interrupted )" );
            // Close audio device
            closeAudioDevice();
            // Send acknowledgment of interruption to server
            client.send( QSS_Ack );
            break;
        }
        break;
    case Interrupted:
        switch( end )
        {
        case Opened:
            qDebug( "AudioDevicePrivate::transition( Interrupted, Opened )" );
            // Open audio device
            openAudioDevice();
            break;
        case Closed:
            qDebug( "AudioDevicePrivate::transition( Interrupted, Closed )" );
            // Send notification of release to server
            client.send( QSS_Release );
            break;
        }
        break;
    }
}

void AudioDevicePrivate::clientError( int error )
{
    switch( error )
    {
    case QSocket::ErrConnectionRefused:
        client_error = true;
        break;
    default:
        qDebug( "AudioDevicePrviate::clientError unexpected error %d", error );
    }
}

void AudioDevicePrivate::receive( QSSMessage message )
{
    switch( message )
    {
    case QSS_Release:
        qDebug( "AudioDevicePrivate::receive( Release )" );
        execute( Release );
        break;
    case QSS_Ack:
        qDebug( "AudioDevicePrivate::receive( Ack )" );
        execute( Ack );
        break;
    case QSS_Nak:
        qDebug( "AudioDevicePrivate::receive( Nak )" );
        execute( Nak );
        break;
    case QSS_Deferred:
        qDebug( "AudioDevicePrivate::receive( Deffered )" );
        execute( Deferred );
        break;
    default:
        qDebug( "AudioDevicePrivate::receive unexpected message %d", message );
    }
}

#define AUDIO_DEVICE "/dev/dsp"

void AudioDevicePrivate::openAudioDevice()
{
    if( ( dsp = ::open( AUDIO_DEVICE, O_WRONLY ) ) < 0 ) {
        qDebug( "AudioDevicePrivate::openAudioDevice unable to open audio device." );
        emit deviceOpenError(); 
        // ### more error handling needed
        return;
    }
    
    // Attempt to set audio buffer size
    // 0x10000 * fragments + fragment shift
    int fragment = 0x10000 * 4 + 12;
    if( ioctl( dsp, SNDCTL_DSP_SETFRAGMENT, &fragment ) == -1 ) {
        qDebug( "AudioDevicePrivate::openAudioDevice unable to set audio buffer size." );
    }
    
    int format = AFMT_S16_LE;
    ioctl( dsp, SNDCTL_DSP_SETFMT, &format );
    if( format != AFMT_S16_LE ) {
        qDebug( "AudioDevicePrivate::openAudioDevice unable to set format.");
        emit deviceOpenError();
        // ### more error handling needed
        return;
    }
    
    // Attempt to set requested device settings
    actual.channels = requested.channels;
    ioctl( dsp, SNDCTL_DSP_CHANNELS, &actual.channels );
    if( actual.channels != requested.channels )
        qDebug( "AudioDevicePrivate::openAudioDevice unable to set channels %d, using %d.", requested.channels, actual.channels );
        
    if( requested.channels == 2 && actual.channels == 2 ) {
        write_delegate = &AudioDevicePrivate::stereo_stereo;
    } else if( requested.channels == 2 && actual.channels == 1 ) {
        write_delegate = &AudioDevicePrivate::stereo_mono;
    } else if( requested.channels == 1 && actual.channels == 1 ) {
        write_delegate = &AudioDevicePrivate::mono_mono;
    } else if( requested.channels == 1 && actual.channels == 2 ) {
        write_delegate = &AudioDevicePrivate::mono_stereo;
    } else {
        qDebug( "AudioDevicePrivate::openAudioDevice unable to set channels." );
        emit deviceOpenError();
        // ### more error handling needed
        return;
    }
    
    actual.speed = requested.speed;
    ioctl( dsp, SNDCTL_DSP_SPEED, &actual.speed );
    if( actual.speed != requested.speed )
        qDebug( "AudioDevicePrivate::openAudioDevice unable to set speed %d, using %d.", requested.speed, actual.speed );
        
    current_speed = 0;

    emit deviceReady();
}

void AudioDevicePrivate::closeAudioDevice()
{
    ::close( dsp );
}

void AudioDevice::volume( int &left, int &right )
{
    AudioDevicePrivate::volume( left, right );
}

void AudioDevice::setVolume( int left, int right )
{
    AudioDevicePrivate::setVolume( left, right );
}


bool AudioDevice::muted()
{
    return AudioDevicePrivate::muted();
}


void AudioDevice::setMuted( bool mute )
{
    AudioDevicePrivate::setMuted( mute );
}

AudioDevice::AudioDevice( QObject *parent, const char *name ) 
    : QObject( parent, name )
{ 
    d = new AudioDevicePrivate( this );
    
    connect( d, SIGNAL( completedIO() ), this, SIGNAL( completedIO() ) );
    
    connect( d, SIGNAL( deviceReady() ), this, SIGNAL( deviceReady() ) );
    
    connect( d, SIGNAL( deviceOpenError() ), this, SIGNAL( deviceOpenError() ) );
}

AudioDevice::~AudioDevice()
{ 
    delete d;
}

void AudioDevice::open( unsigned int frequency, unsigned int channels, unsigned int bps, bool signal )
{
    d->open( frequency, channels );
    
    d->setSignalCompleted( signal );
}

void AudioDevice::close()
{
    d->close();
}

void AudioDevice::volumeChanged( bool muted )
{
    setMuted( muted );
}

bool AudioDevice::write( char *buffer, unsigned int length )
{
    return d->write( buffer, length );
}

unsigned int AudioDevice::channels() const
{
    return d->AudioDevicePrivate::channels();
}


unsigned int AudioDevice::frequency() const
{
    return d->AudioDevicePrivate::frequency();
}

unsigned int AudioDevice::bytesPerSample() const
{
    return 2;
}

unsigned int AudioDevice::bufferSize() const
{
#define AUDIO_BUFFER_SIZE (1<<14)

    return AUDIO_BUFFER_SIZE;
}

unsigned int AudioDevice::canWrite() const
{
    return 0;
}

int AudioDevice::bytesWritten()
{
    return 0;
}

#include "audiodevice_oss.moc"
