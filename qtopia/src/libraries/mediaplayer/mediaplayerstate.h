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
#ifndef MEDIA_PLAYER_STATE_H
#define MEDIA_PLAYER_STATE_H


#include <qobject.h>
#include "mutex.h"


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


class MediaPlayerState : public QObject {
Q_OBJECT
public:
    MediaPlayerState( QObject *parent, const char *name );
    ~MediaPlayerState();

    void loadPlugins();
    void unloadPlugins();

    bool fullscreen()		 { return isFullscreen; }
    bool scaled()		 { return isScaled; }
    bool looping()		 { return isLooping; }
    bool shuffled()		 { return isShuffled; }
    bool paused()		 { return isPaused; }
    bool playing()		 { return isPlaying; }
    bool seeking()		 { return isSeeking; }
    bool seekable()		 { return isSeekable; }
    long position()		 { return curPosition; }
    long length()		 { return curLength; }
    bool hasLength()		 { return (curLength > 1); }
    View view()			 { return curView; }
    QString skin()		 { return curSkin; }
    void closeView()		 { emit viewClosed(); }

    MediaPlayerDecoder* decoder( const QString& file = QString::null );
    MediaPlayerDecoder* streamingDecoder( const QString& url, const QString& mimetype );
    DecoderVersion decoderVersion() { return curDecoderVersion; }
public slots:
    void setFullscreen( bool b ) { if ( isFullscreen == b ) return; isFullscreen = b; emit fullscreenToggled(b); }
    void setScaled( bool b )     { if ( isScaled     == b ) return; isScaled = b;     emit scaledToggled(b); }
    void setLooping( bool b )    { if ( isLooping    == b ) return; isLooping = b;    emit loopingToggled(b); }
    void setShuffled( bool b )   { if ( isShuffled   == b ) return; isShuffled = b;   emit shuffledToggled(b); }
    void setPaused( bool b )	 { if ( isPaused     == b ) return; isPaused = b;     emit pausedToggled(b); }
    void setPlaying( bool b )	 { if ( isPlaying    == b ) return; isPlaying = b;    emit playingToggled(b); }
    void setSeeking( bool b )	 { if ( isSeeking    == b ) return; isSeeking = b;    emit seekingToggled(b); }
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
    void setSeeking()		 { setSeeking( TRUE ); }
    void setSeekable()		 { setSeekable( TRUE ); }
    void setPrevTab()		 { emit prevTab(); }
    void setNextTab()		 { emit nextTab(); }

    void toggleFullscreen()	 { setFullscreen( !isFullscreen ); }
    void toggleScaled()		 { setScaled( !isScaled ); }
    void toggleLooping()	 { setLooping( !isLooping ); }
    void toggleShuffled()	 { setShuffled( !isShuffled ); }
    void togglePaused()		 { setPaused( !isPaused ); }
    void togglePlaying()	 { setPlaying( !isPlaying ); }
    void toggleSeeking()	 { setSeeking( !isSeeking ); }
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
    void seekingToggled( bool );
    void seekableToggled( bool );
    void positionChanged( long ); // When the slider is moved
    void positionUpdated( long ); // When the media file progresses
    void lengthChanged( long );
    void viewChanged( View );
    void viewClosed();
    void skinChanged( const QString& );

    void prev();
    void next();
    void prevTab();
    void nextTab();

private:
    Mutex mutex;
    MediaPlayerDecoder *curDecoder;
    DecoderVersion curDecoderVersion;

    bool temporaryIsMuted;
    unsigned int temporaryMuteRefCount;

    bool isFullscreen;
    bool isScaled;
    bool isLooping;
    bool isShuffled;
    bool isPaused;
    bool isPlaying;
    bool isSeeking;
    bool isSeekable;
    long curPosition;
    long curLength;
    View curView;
    QString curSkin;

    void readConfig( Config& cfg );
    void writeConfig( Config& cfg ) const;
};


extern MediaPlayerState *mediaPlayerState;


#endif // MEDIA_PLAYER_STATE_H

