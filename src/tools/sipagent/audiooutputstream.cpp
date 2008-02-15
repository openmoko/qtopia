/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <qtimer.h>
#include "audiooutputstream.h"
#include <qtopia/audio/qaudiooutput.h>
#include <qtopiaipcenvelope.h>
#include <qvaluespace.h>
#include <qtopialog.h>

static const int interval_multiplier = 10;  // XXX
static const int max_retry_count = 4;

AudioOutputStream::AudioOutputStream( QObject *parent )
    : MediaStream( parent )
{
    audio = 0;
    timer = 0;
    isOpen = false;
    retryCount = 0;
    interval = 2;   // XXX

    dumpAudioData = qLogEnabled(SipAudioData);
    open( WriteOnly );
}

AudioOutputStream::~AudioOutputStream()
{
}

void AudioOutputStream::start( int frequency, int channels, int bitsPerSample )
{
    audio = new QAudioOutput( this );
    audio->setFrequency( frequency );
    audio->setChannels( channels );
    audio->setBitsPerSample( bitsPerSample );

#if defined(MEDIA_SERVER)
    // Media server must relinquish the audio device
    {
        QtopiaIpcEnvelope e("QPE/MediaServer", "setPriority(int)" );

        e << 1; // RingTone
    }

    retryOpen();
#else
    isOpen = audio->open( WriteOnly );
#endif
}

void AudioOutputStream::stop()
{
    if ( audio ) {
        delete audio;
        audio = 0;
#if defined(MEDIA_SERVER) 
        delete timer;
        timer = 0;
        retryCount = 0;
        interval = 2;
        isOpen = false;
        // Media server can use the audio device
        {
            QtopiaIpcEnvelope e("QPE/MediaServer", "setPriority(int)" );

            e << 0; // Default
        }
#endif
    }
}

QByteArray toHexDump( char *data, int size, uint offset=0 );

qint64 AudioOutputStream::writeData( const char *data, qint64 len )
{
    if ( dumpAudioData ) {
        qLog(SipAudioData) << "output bytes:" << len;
    }
    if ( audio && isOpen )
        return audio->write( data, len );
    else
        return len;
}

void AudioOutputStream::retryOpen()
{
#if defined(MEDIA_SERVER) 
    if (audio != 0 && !audio->open(WriteOnly))
    {
        if (timer == 0)
        {
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(retryOpen()));
            timer->setSingleShot(true);
        }

        if (retryCount++ < max_retry_count)
            timer->start(interval *= interval_multiplier);
    }
    else
        isOpen = true;
#endif
}

