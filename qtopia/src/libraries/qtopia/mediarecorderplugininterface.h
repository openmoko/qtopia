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
#ifndef MEDIA_RECORDER_PLUGIN_INTERFACE_H
#define MEDIA_RECORDER_PLUGIN_INTERFACE_H

#include <qtopia/qcom.h>

#ifndef QT_NO_COMPONENT
// {c0093632-b44c-4cf7-a279-d82fe8a8890d}
# ifndef IID_MediaRecorderPlugin
#  define IID_MediaRecorderPlugin QUuid( 0xc0093632, 0xb44c, 0x4cf7, 0xa2, 0x79, 0xd8, 0x2f, 0xe8, 0xa8, 0x89, 0x0d )
# endif
#endif

class MediaRecorderEncoder {

public:
    virtual ~MediaRecorderEncoder() { }

    // About Plugin 
    virtual int pluginNumFormats() const = 0;
    virtual QString pluginFormatName( int format ) const = 0;
    virtual QString pluginFormatTag( int format ) const = 0;
    virtual QString pluginComment() const = 0;
    virtual double pluginVersion() const = 0;
    virtual QString pluginMimeType() const = 0;

    // I/O device management
    virtual bool begin( QIODevice *device, const QString& formatTag ) = 0;
    virtual bool end() = 0;
    virtual bool isActive() const = 0;

    // Audio record functionality
    virtual bool setAudioChannels( int channels ) = 0;
    virtual bool setAudioFrequency( int frequency ) = 0;
    virtual bool writeAudioSamples( const short *samples, long numSamples ) = 0;

    // TODO - video recording API's

    // Add comments and other meta information
    virtual bool addComment( const QString& tag, const QString& contents ) = 0;

    // Space estimation.
    virtual long estimateAudioBps( int frequency, int channels, const QString& formatTag ) = 0;

    // Capabilities
    virtual bool supportsAudio() const = 0;
    virtual bool supportsVideo() const = 0;
    virtual bool supportsComments() const = 0;
    virtual bool requiresDirectAccess() const = 0;
};


struct MediaRecorderPluginInterface : public QUnknownInterface
{
    virtual MediaRecorderEncoder *encoder() = 0;
};


#endif

