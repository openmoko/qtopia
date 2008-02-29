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

#ifndef INTERFACES_H
#define INTERFACES_H

#include <qtopiamedia/observer.h>

#include <QtPlugin>

#include "media.h"

class BasicControl
{
public:
    virtual ~BasicControl() { }

    // Open url
    virtual void open( const QString& url ) = 0;

    // Commence/resume playback
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
};

Q_DECLARE_INTERFACE(BasicControl,
    "com.trolltech.Qtopia.MediaPlayer.BasicControl/1.0")

class PlayerState : public Subject
{
public:
    virtual ~PlayerState() { }

    virtual QtopiaMedia::State playerState() const = 0;
};

Q_DECLARE_INTERFACE(PlayerState,
    "com.trolltech.Qtopia.MediaPlayer.PlayerState/1.0")

class ErrorReport : public Subject
{
public:
    virtual ~ErrorReport() { }

    virtual QString errorString() const = 0;
};

Q_DECLARE_INTERFACE(ErrorReport,
    "com.trolltech.Qtopia.MediaPlayer.ErrorReport/1.0")

class SeekControl
{
public:
    virtual ~SeekControl() { }

    // Seek ms milliseconds into presentation
    virtual void seek( quint32 ms ) = 0;
};

Q_DECLARE_INTERFACE(SeekControl,
    "com.trolltech.Qtopia.MediaPlayer.SeekControl/1.0")

class VolumeControl : public Subject
{
public:
    virtual ~VolumeControl() { }

    virtual void setMuted( bool mute ) = 0;
    virtual bool isMuted() const = 0;

    virtual void setVolume( int volume ) = 0;
    virtual int volume() const = 0;
};

Q_DECLARE_INTERFACE(VolumeControl,
    "com.trolltech.Qtopia.MediaPlayer.VolumeControl/1.0")

class PlaybackStatus : public Subject
{
public:
    virtual ~PlaybackStatus() { }

    // Return total length in milliseconds
    virtual quint32 length() const = 0;
    // Return current position in milliseconds
    virtual quint32 position() const = 0;
};

Q_DECLARE_INTERFACE(PlaybackStatus,
    "com.trolltech.Qtopia.MediaPlayer.PlaybackStatus/1.0")

class VideoRender : public Subject
{
public:
    virtual ~VideoRender() { }

    // Return true when video available for display
    virtual bool hasVideo() const = 0;
    // Return new video widget
    virtual QWidget* createVideoWidget() = 0;
};

Q_DECLARE_INTERFACE(VideoRender,
    "com.trolltech.Qtopia.MediaPlayer.VideoRender/1.0")

class PlayerSettings
{
public:
    virtual ~PlayerSettings() { }

    virtual QVariant value(QString const&) const = 0;
    virtual void setValue(QString const& name, QVariant const& value ) = 0;
};

Q_DECLARE_INTERFACE(PlayerSettings,
    "com.trolltech.Qtopia.MediaPlayer.PlayerSettings/1.0")

#endif // INTERFACES_H
