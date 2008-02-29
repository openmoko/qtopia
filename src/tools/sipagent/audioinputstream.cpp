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

#include "audioinputstream.h"
#include <qtopia/audio/qaudioinput.h>
#include <qvaluespace.h>
#include <qtopialog.h>

#include <QtopiaIpcEnvelope>
#include <QtopiaIpcAdaptor>

AudioInputStream::AudioInputStream( QObject *parent )
    : MediaStream( parent )
{
    audio = 0;
    dumpAudioData = qLogEnabled(SipAudioData);
    open( WriteOnly );
}

AudioInputStream::~AudioInputStream()
{
    stop();
}

void AudioInputStream::start( int frequency, int channels )
{
    //Request mic input from audio system,

    QtopiaIpcAdaptor         *mgr;
    QByteArray               domain("Phone");

    mgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);
    mgr->send("setDomain(QByteArray,int)",domain,1);
    QtopiaIpcEnvelope e("QPE/AudioVolumeManager", "setActiveDomain(QString)");
    e << "Phone";


    stop();

    audio = new QAudioInput( this );
    connect( audio, SIGNAL(readyRead()), this, SLOT(audioReady()) );
    audio->setFrequency( frequency );
    audio->setChannels( channels );
    audio->setSamplesPerBlock( frequency * channels / 50 ); // 20 ms frame size
    if ( !audio->open( ReadOnly ) )
        qLog(Sip) << "AudioInputStream::start - open failed";

    // Perform an initial dummy read to start audio input.
    audio->read( buffer, sizeof( buffer ) );
}

void AudioInputStream::stop()
{
    qLog(Sip) << "AudioInputStream::stop";
    if ( audio )
        delete audio;
    audio = 0;
}

qint64 AudioInputStream::writeData( const char *, qint64 len )
{
    // Not used for audio input streams.
    return len;
}

QByteArray toHexDump( char *data, int size, uint offset=0 );

QByteArray toHexDump( char *data, int size, uint offset )
{
    const char hexdigits[] = "0123456789ABCDEF";

    QByteArray tmp;
    int lines = (size + 15) & ~15;
    tmp.resize(lines * ( 9 + 16 * 3 + 1 + 2 + 16 + 1 ));
            // "offset:", 16 space-separated bytes, space for column 8,
            // two spaces before ASCII, ASCII, \n

    char *in = data;
    char *out = tmp.data();
    int posn;
    int index = -(((int)offset) & 15);
    offset &= ~((uint)15);
    while (index < size) {
        *out++ = hexdigits[((int)(offset >> 28)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 24)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 20)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 16)) & 0x0F];
        *out++ = hexdigits[((int)(offset >> 12)) & 0x0F];
        *out++ = hexdigits[((int)(offset >>  8)) & 0x0F];
        *out++ = hexdigits[((int)(offset >>  4)) & 0x0F];
        *out++ = hexdigits[((int)(offset      )) & 0x0F];
        *out++ = ':';
        for (posn = 0; posn < 16 && (index + posn) < size; ++posn) {
            if ( posn == 8 )
                *out++ = ' ';
            *out++ = ' ';
            if ((index + posn) >= 0) {
                *out++ = hexdigits[(in[index + posn] >> 4) & 0x0F];
                *out++ = hexdigits[in[index + posn] & 0x0F];
            } else {
                *out++ = ' ';
                *out++ = ' ';
            }
        }
        while ( posn < 16 ) {
            if ( posn == 8 )
                *out++ = ' ';
            *out++ = ' ';
            *out++ = ' ';
            *out++ = ' ';
            ++posn;
        }
        *out++ = ' ';
        *out++ = ' ';
        for (posn = 0; posn < 16 && (index + posn) < size; ++posn) {
            if ((index + posn) >= 0) {
                char ch = in[index + posn];
                if ( ch >= 0x20 && ch <= 0x7E )
                    *out++ = ch;
                else
                    *out++ = '.';
            } else {
                *out++ = ' ';
            }
        }
        index += 16;
        if ( index < size )
            *out++ = '\n';
        offset += 16;
    }

    tmp.truncate(out - tmp.data());
    return tmp;
}

void AudioInputStream::audioReady()
{
    if ( dumpAudioData ) {
        qLog(SipAudioData) << "audioReady() - bytesAvailable ="
                           << audio->bytesAvailable();
    }
    int count=0;
    for(;;) {
        if(count>5) break;
        count++;
        int len = audio->read( buffer, sizeof( buffer ) );
        if ( len > 0 ) {
            if ( dumpAudioData ) {
                qLog(SipAudioData) << "input bytes:";
                qLog(SipAudioData) << "writing" << len << "to encoder";
            }
            writeNext( buffer, len );
            break;
        } else {
            qLog(SipAudioData) << "audio read failed with" << len;
            break;
        }
    }
    if ( dumpAudioData )
        qLog(SipAudioData) << "end audioReady()";
}
