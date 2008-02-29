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
#ifndef MEDIA_PLAYER_STATE_H
#define MEDIA_PLAYER_STATE_H


#include <qobject.h>


enum View {
    ListView,
    VideoView,
    AudioView
};


enum DecoderVersion {
    Decoder_Unknown,
    Decoder_1_5,
    Decoder_1_6
};


class MediaPlayerDecoder;
class Config;
class VideoWidget;
class AudioWidget;


class MediaPlayerState : public QObject {
Q_OBJECT
public:
    MediaPlayerState( QObject *parent, const char *name );
    ~MediaPlayerState();

    void loadPlugins();

    bool fullscreen()		 { return isFullscreen; }
    bool scaled()		 { return isScaled; }
    bool looping()		 { return isLooping; }
    bool shuffled()		 { return isShuffled; }
    bool paused()		 { return isPaused; }
    bool playing()		 { return isPlaying; }
    bool seekable()		 { return isSeekable; }
    long position()		 { return curPosition; }
    long length()		 { return curLength; }
    bool hasLength()		 { return (curLength > 1); }
    View view()			 { return curView; }
    QString skin()		 { return curSkin; }

    MediaPlayerDecoder* decoder( const QString& file = QString::null );
    MediaPlayerDecoder* streamingDecoder( const QString& url, const QString& mimetype );
    DecoderVersion decoderVersion() { return curDecoderVersion; }
public slots:
    AudioWidget* audioUI();
    VideoWidget* videoUI();

    void setFullscreen( bool b ) { if ( isFullscreen == b ) return; isFullscreen = b; emit fullscreenToggled(b); }
    void setScaled( bool b )     { if ( isScaled     == b ) return; isScaled = b;     emit scaledToggled(b); }
    void setLooping( bool b )    { if ( isLooping    == b ) return; isLooping = b;    emit loopingToggled(b); }
    void setShuffled( bool b )   { if ( isShuffled   == b ) return; isShuffled = b;   emit shuffledToggled(b); }
    void setPaused( bool b )	 { if ( isPaused     == b ) return; isPaused = b;     emit pausedToggled(b); }
    void setPlaying( bool b )	 { if ( isPlaying    == b ) return; isPlaying = b;    emit playingToggled(b); }
    void setSeekable( bool b )	 { if ( isSeekable   == b ) return; isSeekable = b;   emit seekableToggled(b); }
    void setPosition( long p )   { if ( curPosition  == p ) return; curPosition = p;  emit positionChanged(p); }
    void updatePosition( long p ){ if ( curPosition  == p ) return; curPosition = p;  emit positionUpdated(p); }
    void setLength( long l )	 { if ( curLength    == l ) return; curLength = l;    emit lengthChanged(l); }
    void setView( View v )	 { if ( curView      == v ) return; curView = v;      emit viewChanged(v); }
    void setSkin( const QString& s ) { if ( curSkin  == s ) return; curSkin = s;      emit skinChanged(s); }

    void setPrev()		 { emit prev(); }
    void setNext()		 { emit next(); }
    void setList()		 { setPlaying( FALSE ); setView( ListView ); }
    void setVideo()		 { setView( VideoView ); }
    void setAudio()		 { setView( AudioView ); }
    void setPlaying()		 { setPlaying( TRUE ); }
    void setSeekable()		 { setSeekable( TRUE ); }
    void setPrevTab()		 { emit prevTab(); }
    void setNextTab()		 { emit nextTab(); }

    void toggleFullscreen()	 { setFullscreen( !isFullscreen ); }
    void toggleScaled()		 { setScaled( !isScaled ); }
    void toggleLooping()	 { setLooping( !isLooping ); }
    void toggleShuffled()	 { setShuffled( !isShuffled ); }
    void togglePaused()		 { setPaused( !isPaused ); }
    void togglePlaying()	 { setPlaying( !isPlaying ); }
    void toggleSeekable()	 { setSeekable( !isSeekable ); }

    void startTemporaryMute();
    void stopTemporaryMute();
    void stopTemporaryMute( int delay );

signals:
    void decreaseVolume();
    void increaseVolume();
    void endDecreaseVolume();
    void endIncreaseVolume();

    void scanBackward();
    void scanForward();
    void endScanBackward();
    void endScanForward();

    void fullscreenToggled( bool );
    void scaledToggled( bool );
    void loopingToggled( bool );
    void shuffledToggled( bool );
    void playlistToggled( bool );
    void pausedToggled( bool );
    void playingToggled( bool );
    void seekableToggled( bool );
    void positionChanged( long ); // When the slider is moved
    void positionUpdated( long ); // When the media file progresses
    void lengthChanged( long );
    void viewChanged( View );
    void skinChanged( const QString& );

    void prev();
    void next();
    void prevTab();
    void nextTab();

private:
    AudioWidget* aw;
    VideoWidget* vw;
    MediaPlayerDecoder *curDecoder;
    DecoderVersion curDecoderVersion;

    bool isFullscreen;
    bool isScaled;
    bool isLooping;
    bool isShuffled;
    bool isPaused;
    bool isPlaying;
    bool isSeekable;
    long curPosition;
    long curLength;
    View curView;
    QString curSkin;

    unsigned int temporaryLeftVol;
    unsigned int temporaryRightVol;
    bool temporaryIsMuted;
    unsigned int temporaryMuteRefCount;

    void readConfig( Config& cfg );
    void writeConfig( Config& cfg ) const;
};


extern MediaPlayerState *mediaPlayerState;


#endif // MEDIA_PLAYER_STATE_H

