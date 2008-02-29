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

#include "helixplayer.h"

#include <media.h>
#include <helixvideosurface.h>
#include <reporterror.h>

#include <hxclsnk.h>
#include <hxprefs.h>
#include <hxccf.h>
#include <ihxpckts.h>
#include <HXErrorCodeStrings.h>

class PlayerProgressAdvise : public IHXClientAdviseSink,
    public Subject
{
public:
    PlayerProgressAdvise()
        : m_refCount( 0 ), m_length( 0 ), m_position( 0 )
    { }

    quint32 length() const { return m_length; }

    void setPosition( quint32 position ) { m_position = position; }
    quint32 position() const { return m_position; }

    // IHXClientAdviseSink
    STDMETHOD(OnBegin) (THIS_ ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnBuffering) (THIS_ ULONG32, UINT16)
        { return HXR_OK; }
    STDMETHOD(OnContacting) (THIS_ const char*)
        { return HXR_OK; }
    STDMETHOD(OnPause) (THIS_ ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnPosLength) (THIS_
        UINT32 ulPosition,
        UINT32 ulLength);
    STDMETHOD(OnPostSeek) (THIS_ ULONG32, ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnPreSeek) (THIS_ ULONG32, ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnPresentationClosed) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnPresentationOpened) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnStatisticsChanged) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnStop) (THIS)
        { return HXR_OK; }

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;

    quint32 m_length;
    quint32 m_position;
};

STDMETHODIMP PlayerProgressAdvise::OnPosLength( UINT32 ulPosition, UINT32 ulLength )
{
    m_length = ulLength;
    m_position = ulPosition;

    notify();

    return HXR_OK;
}

STDMETHODIMP_(ULONG32) PlayerProgressAdvise::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) PlayerProgressAdvise::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP PlayerProgressAdvise::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXClientAdviseSink*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXClientAdviseSink ) ) {
        AddRef();
        *object = (IHXClientAdviseSink*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

class PlayerStateAdvise : public IHXClientAdviseSink,
    public Subject
{
public:
    PlayerStateAdvise()
        : m_refCount( 0 ), m_state( QtopiaMedia::Stopped )
    { }

    QtopiaMedia::State state() const { return m_state; }

    // IHXClientAdviseSink
    STDMETHOD(OnBegin) (THIS_ ULONG32);
    STDMETHOD(OnBuffering) (THIS_ ULONG32, UINT16);
    STDMETHOD(OnContacting) (THIS_ const char*)
        { return HXR_OK; }
    STDMETHOD(OnPause) (THIS_ ULONG32);
    STDMETHOD(OnPosLength) (THIS_ UINT32, UINT32)
        { return HXR_OK; }
    STDMETHOD(OnPostSeek) (THIS_ ULONG32, ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnPreSeek) (THIS_ ULONG32, ULONG32)
        { return HXR_OK; }
    STDMETHOD(OnPresentationClosed) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnPresentationOpened) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnStatisticsChanged) (THIS)
        { return HXR_OK; }
    STDMETHOD(OnStop) (THIS);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;

    QtopiaMedia::State m_state;
};

STDMETHODIMP PlayerStateAdvise::OnBegin( ULONG32 )
{
    m_state = QtopiaMedia::Playing;

    notify();

    return HXR_OK;
}

STDMETHODIMP PlayerStateAdvise::OnBuffering(ULONG32, UINT16)
{
    m_state = QtopiaMedia::Buffering;

    notify();

    return HXR_OK;
}

STDMETHODIMP PlayerStateAdvise::OnPause( ULONG32 )
{
    m_state = QtopiaMedia::Paused;

    notify();

    return HXR_OK;
}

STDMETHODIMP PlayerStateAdvise::OnStop()
{
    m_state = QtopiaMedia::Stopped;

    notify();

    return HXR_OK;
}

STDMETHODIMP_(ULONG32) PlayerStateAdvise::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) PlayerStateAdvise::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP PlayerStateAdvise::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXClientAdviseSink*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXClientAdviseSink ) ) {
        AddRef();
        *object = (IHXClientAdviseSink*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

class PlayerVolumeAdvise : public IHXVolumeAdviseSink,
    public Subject
{
public:
    PlayerVolumeAdvise()
        : m_refCount( 0 )
    { }

    // IHXVolumeAdviseSink
    STDMETHOD(OnMuteChange) (THIS_ const BOOL bMute);
    STDMETHOD(OnVolumeChange) (THIS_ const UINT16 uVolume);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;
};

STDMETHODIMP PlayerVolumeAdvise::OnMuteChange( const BOOL )
{
    notify();
    return HXR_OK;
}

STDMETHODIMP PlayerVolumeAdvise::OnVolumeChange( const UINT16 )
{
    notify();
    return HXR_OK;
}

STDMETHODIMP_(ULONG32) PlayerVolumeAdvise::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) PlayerVolumeAdvise::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP PlayerVolumeAdvise::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXVolumeAdviseSink*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXVolumeAdviseSink ) ) {
        AddRef();
        *object = (IHXVolumeAdviseSink*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

class PlayerErrorSink : public IHXErrorSink,
    public Subject
{
public:
    PlayerErrorSink()
        : m_refCount( 0 )
    { }

    // Return last error string
    QString errorString() const { return m_errorstring; }

    // IHXErrorSink
    STDMETHOD(ErrorOccurred) (THIS_
        const UINT8     unSeverity,
        const ULONG32   ulHXCode,
        const ULONG32   ulUserCode,
        const char*     pUserString,
        const char*     pMoreInfoURL);

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;

    QString m_errorstring;
};

STDMETHODIMP PlayerErrorSink::ErrorOccurred(
        const UINT8     /*unSeverity*/,
        const ULONG32   ulHXCode,
        const ULONG32   /*ulUserCode*/,
        const char*     /*pUserString*/,
        const char*     /*pMoreInfoURL*/)
{
    // TODO tr

    bool reporterror = true;

    switch( ulHXCode )
    {
    case HXR_SOCK_CONNREFUSED:
        m_errorstring = "No network connection available.";
        break;
    case HXR_NET_CONNECT:
        m_errorstring = "Server not found.";
        break;
    case HXR_SERVER_DISCONNECTED:
        m_errorstring = "Connection lost.";
        break;
    case HXR_DOC_MISSING:
        m_errorstring = "File not found.";
        break;
    case HXR_OUTOFMEMORY:
        m_errorstring = "Out of memory.";
        break;
    case HXR_AUDIO_DRIVER:
        m_errorstring = "Resource unavailable.";
        break;
    case HXR_INVALID_FILE:
    case HXR_NO_RENDERER:
        m_errorstring = "Media type unsupported.";
        break;
    default:
        REPORT_ERROR( ERR_HELIX );
        qWarning() << "Helix Error Code" << HXErrorCodeToString( ulHXCode );
        reporterror = false;
        break;
    }

    // Notify observers
    if( reporterror ) {
        notify();
    }

    return HXR_OK;
}

STDMETHODIMP_(ULONG32) PlayerErrorSink::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) PlayerErrorSink::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP PlayerErrorSink::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)(IHXErrorSink*)this;
        return HXR_OK;
    } else if( IsEqualIID( riid, IID_IHXErrorSink ) ) {
        AddRef();
        *object = (IHXErrorSink*)this;
        return HXR_OK;
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}

HelixPlayer::HelixPlayer(IHXClientEngine* engine):
    m_engine(engine)
{
    if (HXR_OK == m_engine->CreatePlayer(m_player)) {

        // Install advise sinks
        m_progressadvise = new PlayerProgressAdvise;
        HX_ADDREF( m_progressadvise );

        m_progressadvise->attach( this );
        m_player->AddAdviseSink( m_progressadvise );

        m_stateadvise = new PlayerStateAdvise;
        HX_ADDREF( m_stateadvise );

        m_stateadvise->attach( this );
        m_player->AddAdviseSink( m_stateadvise );

        IHXAudioPlayer *audioplayer;
        if( m_player->QueryInterface( IID_IHXAudioPlayer, (void**)&audioplayer ) == HXR_OK ) {
            m_volume = audioplayer->GetAudioVolume();
            HX_RELEASE( audioplayer );
        } else {
            REPORT_ERROR( ERR_HELIX );
        }

        m_volumeadvise = new PlayerVolumeAdvise;
        HX_ADDREF( m_volumeadvise );

        m_volumeadvise->attach( this );
        m_volume->AddAdviseSink( m_volumeadvise );

        // Install player context
        m_sitesupplier = new HelixSiteSupplier( m_player );
        HX_ADDREF( m_sitesupplier );
        m_sitesupplier->attach( this );

        m_errorsink = new PlayerErrorSink;
        HX_ADDREF( m_errorsink );
        m_errorsink->attach( this );

        if( m_player->QueryInterface( IID_IHXErrorSinkControl, (void**)&m_errorcontrol ) == HXR_OK ) {
            m_errorcontrol->AddErrorSink( m_errorsink, HXLOG_EMERG, HXLOG_INFO );
        } else {
            REPORT_ERROR( ERR_HELIX );
        }

        m_context = new GenericContext( QList<IUnknown*>()
            << m_sitesupplier
            << m_errorsink );
        HX_ADDREF( m_context );
        m_player->SetClientContext( m_sitesupplier );
    }
}

HelixPlayer::~HelixPlayer()
{
    m_errorsink->detach( this );
    m_sitesupplier->detach( this );

    m_volumeadvise->detach( this );
    m_volume->RemoveAdviseSink( m_volumeadvise );

    m_stateadvise->detach( this );
    m_player->RemoveAdviseSink( m_stateadvise );

    m_progressadvise->detach( this );
    m_player->RemoveAdviseSink( m_progressadvise );

    m_engine->ClosePlayer(m_player);

    HX_RELEASE( m_context );
    HX_RELEASE( m_errorsink );
    HX_RELEASE( m_sitesupplier );
    HX_RELEASE( m_volumeadvise );
    HX_RELEASE( m_stateadvise );
    HX_RELEASE( m_progressadvise );

    HX_RELEASE( m_errorcontrol );
    HX_RELEASE( m_volume );
    HX_RELEASE( m_player );
}

void HelixPlayer::open( const QString& in )
{
    m_player->OpenURL(in.toLocal8Bit().constData());
}

void HelixPlayer::play()
{
    m_player->Begin();
}

void HelixPlayer::pause()
{
    if (m_player->IsLive())
        m_player->Stop();
    else
        m_player->Pause();
}

void HelixPlayer::stop()
{
    m_player->Stop();
}

QtopiaMedia::State HelixPlayer::playerState() const
{
    return m_stateadvise->state();
}

void HelixPlayer::seek( quint32 ms )
{
    if (!m_player->IsLive())
    {
        m_player->Seek( ms );

        // Fake a seek when player is paused
        if( m_stateadvise->state() == QtopiaMedia::Paused ) {
            m_progressadvise->setPosition( ms );

            PlaybackStatus::notify();
        }
    }
}

void HelixPlayer::setMuted( bool mute )
{
    m_volume->SetMute( mute );
}

bool HelixPlayer::isMuted() const
{
    return m_volume->GetMute();
}

void HelixPlayer::setVolume( int volume )
{
    m_volume->SetVolume( volume );
}

int HelixPlayer::volume() const
{
    return m_volume->GetVolume();
}

quint32 HelixPlayer::length() const
{
    return m_progressadvise->length();
}

quint32 HelixPlayer::position() const
{
    return m_progressadvise->position();
}

bool HelixPlayer::hasVideo() const
{
    return m_sitesupplier->hasVideo();
}

QWidget* HelixPlayer::createVideoWidget()
{
    if( m_sitesupplier->hasVideo() ) {
        // FIXME Direct painting flickers in QVFb
        // If supported use direct painter widget
        // if( DirectPainterVideoWidget::isSupported() ) {
        //    return new DirectPainterVideoWidget( m_sitesupplier->site()->surface() );
        // }
        return new GenericVideoWidget( m_sitesupplier->site()->surface() );
    }

    return 0;
}

QString HelixPlayer::errorString() const
{
    return m_errorsink->errorString();
}

QVariant HelixPlayer::value(QString const& name) const
{
    IHXPreferences *preferences;

    m_player->QueryInterface( IID_IHXPreferences, (void**)&preferences );

    IHXBuffer *buffer;
    preferences->ReadPref(name.toLatin1().constData(), buffer );

    QString s = (const char*)buffer->GetBuffer();

    HX_RELEASE( buffer );
    HX_RELEASE( preferences );

    return s;
}

void HelixPlayer::setValue(QString const& name, QVariant const& value)
{
    IHXPreferences *preferences;
    IHXCommonClassFactory *factory;

    m_player->QueryInterface( IID_IHXPreferences, (void**)&preferences );
    m_player->QueryInterface( IID_IHXCommonClassFactory, (void**)&factory );

    IHXBuffer *buffer;
    factory->CreateInstance( CLSID_IHXBuffer, (void**)&buffer );

    QString s = value.toString();
    buffer->Set( (const UCHAR*)s.toLatin1().data(), s.length() );

    preferences->WritePref(name.toLatin1().constData(), buffer );
    HX_RELEASE( buffer );

    HX_RELEASE( factory );
    HX_RELEASE( preferences );
}

void HelixPlayer::update( Subject* subject )
{
    if( subject == m_progressadvise ) {
        PlaybackStatus::notify();
    } else if( subject == m_stateadvise ) {
        PlayerState::notify();
    } else if( subject == m_volumeadvise ) {
        VolumeControl::notify();
    } else if( subject == m_sitesupplier ) {
        VideoRender::notify();
    } else if( subject == m_errorsink ) {
        ErrorReport::notify();
    }
    else {
        REPORT_ERROR( ERR_UNEXPECTED );
    }
}

