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
#ifndef MEDIA_RECORDER_PLUGIN_INTERFACE_H
#define MEDIA_RECORDER_PLUGIN_INTERFACE_H

#include <qfactoryinterface.h>
#include <qtopiaglobal.h>

class QTOPIA_EXPORT MediaRecorderEncoder {

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


struct QTOPIA_EXPORT MediaRecorderCodecFactoryInterface : public QFactoryInterface
{
    virtual MediaRecorderEncoder *encoder() = 0;
};

#define MediaRecorderCodecFactoryInterface_iid "com.trolltech.Qtopia.MediaRecorderCodecFactoryInterface"
Q_DECLARE_INTERFACE(MediaRecorderCodecFactoryInterface, MediaRecorderCodecFactoryInterface_iid)

class QTOPIA_EXPORT MediaRecorderCodecPlugin : public QObject, public MediaRecorderCodecFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(MediaRecorderCodecFactoryInterface:QFactoryInterface)
public:
    explicit MediaRecorderCodecPlugin( QObject* parent = 0 );
    ~MediaRecorderCodecPlugin();

    virtual QStringList keys() const = 0;

    virtual MediaRecorderEncoder *encoder() = 0;
};

#endif

