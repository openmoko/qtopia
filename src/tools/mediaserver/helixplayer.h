/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef HELIXPLAYER_H
#define HELIXPLAYER_H

#include <interfaces.h>
#include <helixutil.h>
#include <helixsite.h>
#include <mediaserver.h>

#include <QtCore>

#include <config.h>
#include <hxcom.h>
#include <hxcore.h>
#include <hxausvc.h>
#include <hxerror.h>


class PlayerProgressAdvise;
class PlayerStateAdvise;
class PlayerVolumeAdvise;
class PlayerErrorSink;
class MediaServer;


class HelixPlayer : public QObject,
    public BasicControl,
    public PlayerState,
    public SeekControl,
    public VolumeControl,
    public PlaybackStatus,
    public VideoRender,
    public ErrorReport,
    public PlayerSettings,
    public Observer
{
    Q_OBJECT
    Q_INTERFACES(BasicControl)
    Q_INTERFACES(PlayerState)
    Q_INTERFACES(SeekControl)
    Q_INTERFACES(VolumeControl)
    Q_INTERFACES(PlaybackStatus)
    Q_INTERFACES(VideoRender)
    Q_INTERFACES(ErrorReport)
    Q_INTERFACES(PlayerSettings)
public:
    HelixPlayer(IHXClientEngine* engine);
    ~HelixPlayer();

    // BasicControl
    void open( const QString& url );

    void play();
    void pause();
    void stop();

    // PlayerState
    QtopiaMedia::State playerState() const;

    // SeekControl
    void seek( quint32 ms );

    // VolumeControl
    void setMuted( bool mute );
    bool isMuted() const;

    void setVolume( int volume );
    int volume() const;

    // PlaybackStatus
    quint32 length() const;
    quint32 position() const;

    // VideoRender
    bool hasVideo() const;
    QWidget* createVideoWidget();

    // ErrorReport
    QString errorString() const;

    // PlayerSettings
    QVariant value(QString const&) const;
    void setValue(QString const&, QVariant const& value);

    // Observer
    void update( Subject* subject );

private:
    IHXClientEngine*    m_engine;
    IHXPlayer *m_player;
    IHXVolume *m_volume;
    IHXErrorSinkControl *m_errorcontrol;

    PlayerProgressAdvise *m_progressadvise;
    PlayerStateAdvise *m_stateadvise;
    PlayerVolumeAdvise *m_volumeadvise;

    HelixSiteSupplier *m_sitesupplier;
    PlayerErrorSink *m_errorsink;
    GenericContext *m_context;
};

#endif // HELIXPLAYER_H
