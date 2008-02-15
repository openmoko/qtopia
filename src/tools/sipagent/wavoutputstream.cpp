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

#include "wavoutputstream.h"
#include <QFile>

/*

The WavOutputStream class is used for debugging.  Audio input and output
can be dumped to a .wav file while a session is in progress so that it can
be independently checked afterwards.

*/

const int WAV_PCM_HEADER_LEN = 44;
const int WAV_MAX_HEADER_LEN = WAV_PCM_HEADER_LEN;

WavOutputStream::WavOutputStream( const QString& filename, QObject *parent )
    : MediaStream( parent )
{
    this->filename = filename;
    this->file = 0;
    open( WriteOnly );
}

WavOutputStream::~WavOutputStream()
{
    stop();
}

void WavOutputStream::start( int frequency, int channels )
{
    stop();

    // Reset the recording state.
    this->channels = channels;
    this->frequency = frequency;
    writtenHeader = false;
    totalBytes = 0;
    totalSamples = 0;
    headerLen = 0;

    // Determine if we need to byte-swap samples before writing
    // them to the output device (wav is little-endian).
    union
    {
        short v1;
        char  v2[2];
    } un;
    un.v1 = 0x0102;
    byteSwap = (un.v2[0] == 0x01);

    // Open the file for writing.
    file = new QFile( filename );
    if ( !file->open( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
        delete file;
        file = 0;
        return;
    }
}

void WavOutputStream::stop()
{
    // Bail out if we were not recording.
    if ( !file )
        return;

    // Back-patch the header with the total file length.
    if ( writtenHeader ) {
        if ( file->seek( 0 ) )
            writeHeader();
    } else {
        writeHeader();
    }

    file->close();
    delete file;
    file = 0;
}

qint64 WavOutputStream::writeData( const char *data, qint64 len )
{
    // Bail out if we are not currently recording.
    if ( !file )
        return writeNext( data, len );

    // Write the header if necessary.
    if ( !writtenHeader ) {
        if(!writeHeader())
            return false;
        writtenHeader = true;
    }

    // Write the audio data to the output device.
    if ( !byteSwap ) {

        // Write little-endian data directly to the device.
        if ( file->write( data, len ) != len )
            return writeNext( data, len );

    } else {

        // Byte-swap big-endian data before writing to the device.
        if ( writeByteSwapped( data, (uint)len ) != len )
            return writeNext( data, len );

    }
    totalBytes += (long)len;
    return writeNext( data, len );
}

// Write a little-endian short value to a buffer.
static inline void write_short(char *buf, int value)
{
    buf[0] = (char)(value);
    buf[1] = (char)(value >> 8);
}

// Write a little-endian long value to a buffer.
static inline void write_long(char *buf, long value)
{
    buf[0] = (char)(value);
    buf[1] = (char)(value >> 8);
    buf[2] = (char)(value >> 16);
    buf[3] = (char)(value >> 24);
}

bool WavOutputStream::writeHeader()
{
    if ( !file )
        return false;

    // Construct the complete wav header, up to the start of the data.
    char header[WAV_MAX_HEADER_LEN];
    strncpy( header, "RIFF", 4 );
    write_long( header + 4, totalBytes + (WAV_PCM_HEADER_LEN - 8) );
    strncpy( header + 8, "WAVE", 4 );
    strncpy( header + 12, "fmt ", 4 );
    write_long( header + 16, 16 );                       // size of "fmt"
    write_short( header + 20, 1 );                       // WAVE_FORMAT_PCM
    write_short( header + 22, channels );                // nChannels
    write_long( header + 24, frequency );                // nSamplesPerSec
    write_long( header + 28, frequency * channels * 2 ); // nAvgBytesPerSec
    write_short( header + 32, channels * 2 );            // nBlockAlign
    write_short( header + 34, 16 );                      // nBitsPerSample
    strncpy( header + 36, "data", 4 ); // No tr
    write_long( header + 40, totalBytes );
    headerLen = WAV_PCM_HEADER_LEN;

    // Write the header to the output device.
    return ( file->write( header, (uint)headerLen ) == headerLen );
}

qint64 WavOutputStream::writeByteSwapped( const char *data, uint len )
{
    char buf[256];
    uint writelen, templen, posn;

    writelen = 0;
    while ( len > 0) {

        // Determine how many bytes that we can process this time.
        if (len > sizeof(buf) )
            templen = sizeof(buf);
        else
        templen = len;

        // Byte-swap the input data segument.
        for ( posn = 0; posn < templen; posn += 2 ) {
            buf[posn] = data[posn + 1];
            buf[posn + 1] = data[posn];
        }

        // Write the byte-swapped data segment to the output device.
        if ( file->write( buf, templen ) != (int)templen )
            break;

        // Advance to the next buffer segment.
        data += templen;
        len -= templen;
        writelen += templen;
    }

    return writelen;
}

