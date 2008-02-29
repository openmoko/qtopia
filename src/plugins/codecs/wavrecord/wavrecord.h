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
#ifndef WAV_RECORD_H 
#define WAV_RECORD_H


#include <qstring.h>
#include <qapplication.h>
#include <qtopia/mediarecorderplugininterface.h>

class WavRecorderPlugin : public MediaRecorderEncoder {

public:
    WavRecorderPlugin();
    virtual ~WavRecorderPlugin() { end(); }

    // About Plugin 
    int pluginNumFormats() const;
    QString pluginFormatName( int format ) const;
    QString pluginFormatTag( int format ) const;
    QString pluginComment() const { return qApp->translate("WavRecorder", "This plugin is used to record wav files"); }
    double pluginVersion() const { return 1.0; }
    QString pluginMimeType() const { return "audio/x-wav"; }

    // I/O device management
    bool begin( QIODevice *device, const QString& formatTag );
    bool end();
    bool isActive() const { return ( device != 0 ); }

    // Audio record functionality
    bool setAudioChannels( int channels );
    bool setAudioFrequency( int frequency );
    bool writeAudioSamples( const short *samples, long numSamples );

    // Add comments and other meta information
    bool addComment( const QString&, const QString& ) { return TRUE; }

    // Space estimation.
    long estimateAudioBps( int frequency, int channels, const QString& formatTag );

    // Capabilities
    bool supportsAudio() const { return TRUE; }
    bool supportsVideo() const { return FALSE; }
    bool supportsComments() const { return FALSE; }
    bool requiresDirectAccess() const { return TRUE; }

private:

    QIODevice *device;
    bool encodeAsGsm;
    int start;
    int channels;
    int frequency;
    bool writtenHeader;
    long totalSamples;
    long totalBytes;
    bool byteSwap;
    int headerLen;
    short gsmBuffer[320];
    int gsmPosn;
    void *gsmHandle;

    // Write byte-swapped data to the I/O device.
    int writeByteSwapped( const char *data, uint len );

    // Write the wav file header to the I/O device.  Called once
    // before outputting the first sample block, and then again
    // at the end of recording to update the length values.
    bool writeHeader();

    // Flush the current GSM block.
    void gsmFlush( bool last );

};


#endif

