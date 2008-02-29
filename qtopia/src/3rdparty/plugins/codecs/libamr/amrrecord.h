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
#ifndef AMR_RECORD_H 
#define AMR_RECORD_H


#include <qstring.h>
#include <qapplication.h>
#include <qtopia/mediarecorderplugininterface.h>

class AmrAudioFormatHandler;

class AmrRecorderPlugin : public MediaRecorderEncoder {

public:
    AmrRecorderPlugin();
    virtual ~AmrRecorderPlugin() { end(); }

    // About Plugin 
    int pluginNumFormats() const { return 1; }
    QString pluginFormatName( int ) const { return "MMS AMR"; }
    QString pluginFormatTag( int ) const { return "amr"; }
    QString pluginComment() const { return qApp->translate("AmrRecorder", "This plugin is used to record AMR files"); }
    double pluginVersion() const { return 1.0; }
    QString pluginMimeType() const { return "audio/amr"; }

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
    bool requiresDirectAccess() const { return FALSE; }

private:

    QIODevice *device;
    int channels;
    int frequency;
    bool writtenHeader;
    short block[160];
    int blockLen;
    void *amrState;
    void *sidSync;
    AmrAudioFormatHandler *handler;

    // Write the AMR file header to the I/O device.  Called once
    // before outputting the first sample block, and then again
    // at the end of recording to update the length values.
    bool writeHeader();

    // Flush the current AMR block.
    bool amrFlush();

};


#endif

