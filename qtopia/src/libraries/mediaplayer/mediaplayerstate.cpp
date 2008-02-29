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
#include <qtopia/qpeapplication.h>
#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/mediaplayerplugininterface.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/pluginloader.h>
#include <qvaluelist.h>
#include <qobject.h>
#include <qtimer.h>
#include <qdir.h>
#include "audiodevice.h"
#include "mediaplayerstate.h"


/*!
  \class MediaPlayerState mediaplayerstate.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class stores and notifies about changes in various
    states and options associated with the mediaplayer apps.
*/


#ifdef QT_NO_COMPONENT
// Plugins which are compiled in when no plugin architecture available
#include "../../3rdparty/plugins/codecs/libffmpeg/libffmpegpluginimpl.h"
#endif


#define MediaPlayerDebug(x)	qDebug x
//#define MediaPlayerDebug(x)


#ifdef QTOPIA_PHONE
#  define DEFAULT_SKIN  "phone" // No tr
#else
#  define DEFAULT_SKIN  "default" // No tr
#endif


MediaPlayerState::MediaPlayerState( QObject *parent, const char *name )
    : QObject( parent, name ), curDecoder( 0 ),
	curDecoderVersion( Decoder_Unknown ), temporaryMuteRefCount(0),
    isFullscreen(false), isScaled(false), isLooping(false), isShuffled(false),
    isPaused(false), isPlaying(false), isSeeking(false), isSeekable(false),
    curPosition(0), curLength(0)
{
    Config cfg( "MediaPlayer" );
    readConfig( cfg );
}


MediaPlayerState::~MediaPlayerState()
{
    Config cfg( "MediaPlayer" );
    writeConfig( cfg );
}
    

void MediaPlayerState::readConfig( Config& cfg )
{
    cfg.setGroup( "Options" );
    isFullscreen = cfg.readBoolEntry( "FullScreen", FALSE );
    isScaled = cfg.readBoolEntry( "Scaling", TRUE );
    isLooping = cfg.readBoolEntry( "Looping", FALSE );
    isShuffled = cfg.readBoolEntry( "Shuffle", FALSE );
    curSkin = cfg.readEntry( "Skin", DEFAULT_SKIN );
    isPlaying = FALSE;
    isPaused = FALSE;
    isSeekable = TRUE;
    curPosition = 0;
    curLength = 0;
    curView = ListView;
}


void MediaPlayerState::writeConfig( Config& cfg ) const
{
    cfg.setGroup("Options");
    cfg.writeEntry("FullScreen", isFullscreen );
    cfg.writeEntry("Scaling", isScaled );
    cfg.writeEntry("Looping", isLooping );
    cfg.writeEntry("Shuffle", isShuffled );
    cfg.writeEntry("Skin", curSkin );
}


struct MediaPlayerPlugin {
#ifndef QT_NO_COMPONENT
    MediaPlayerPluginInterface *iface;
#endif
    MediaPlayerDecoder *decoder;
    MediaPlayerEncoder *encoder;
    DecoderVersion version;
};


static QValueList<MediaPlayerPlugin> pluginList;
static PluginLoader *loader = 0;


// Find the first decoder which supports this type of file
MediaPlayerDecoder *MediaPlayerState::decoder( const QString& file )
{
    AutoLockUnlockMutex lock(&mutex);
    if ( file.isNull() )
	return curDecoder;
    QValueList<MediaPlayerPlugin>::Iterator it;
    for ( it = pluginList.begin(); it != pluginList.end(); ++it ) 
	if ( (*it).decoder->isFileSupported( file ) ) {
	    curDecoderVersion = (*it).version;
	    return curDecoder = (*it).decoder;
	}
    curDecoderVersion = Decoder_Unknown;
    return curDecoder = NULL;
}


// Find the first decoder which can play this streamed URL
MediaPlayerDecoder *MediaPlayerState::streamingDecoder( const QString& url, const QString& mimetype )
{
    AutoLockUnlockMutex lock(&mutex);
    QValueList<MediaPlayerPlugin>::Iterator it;
    for ( it = pluginList.begin(); it != pluginList.end(); ++it ) 
	if ( (*it).version == Decoder_1_6 ) {
	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)(*it).decoder;
	    if ( decoder->supportsStreaming() ) 
		if ( decoder->canStreamURL( url, mimetype ) ) {
		    curDecoderVersion = (*it).version;
		    return curDecoder = decoder;
		}
	}
    curDecoderVersion = Decoder_Unknown;
    return curDecoder = NULL;
}


void MediaPlayerState::unloadPlugins()
{
    AutoLockUnlockMutex lock(&mutex);
#ifndef QT_NO_COMPONENT
    if ( loader ) {
	QValueList<MediaPlayerPlugin>::Iterator mit;
	for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	    loader->releaseInterface( (*mit).iface );
	}
	pluginList.clear();
	delete loader;
	loader = 0;
    }
#else
    pluginList.clear();
#endif
}


void MediaPlayerState::loadPlugins()
{
    unloadPlugins();
    AutoLockUnlockMutex lock(&mutex);
#ifndef QT_NO_COMPONENT
    loader = new PluginLoader( "codecs" );

    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaPlayerPluginInterface *iface = 0;

	MediaPlayerDebug(( "querying: %s", (*it).latin1() )); // No tr

	DecoderVersion ver = Decoder_Unknown;

	// Some old plugins have dodgy queryInterface implementations which always anwser QS_OK
	const QUuid IID_evilPlugin( 0x00000666, 0x0666, 0x0666, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x66 );
	if ( loader->queryInterface( *it, IID_evilPlugin, (QUnknownInterface**)&iface ) == QS_OK ) {
	    qDebug("Detected old evil plugin which answers to any old interface Id!");
	    loader->queryInterface( *it, IID_MediaPlayerPlugin, (QUnknownInterface**)&iface );
	    ver = Decoder_1_5;
	} else if ( loader->queryInterface( *it, IID_MediaPlayerPlugin_1_6, (QUnknownInterface**)&iface ) == QS_OK ) 
	    ver = Decoder_1_6;
	else if ( loader->queryInterface( *it, IID_MediaPlayerPlugin, (QUnknownInterface**)&iface ) == QS_OK )
	    ver = Decoder_1_5;

	if ( ver != Decoder_Unknown ) {
	    MediaPlayerDebug(( "loading: %s", (*it).latin1() )); // No tr

	    MediaPlayerPlugin plugin;
	    plugin.version = ver;
	    plugin.iface = iface;
	    plugin.decoder = plugin.iface->decoder();
	    plugin.encoder = plugin.iface->encoder();
	    pluginList.append( plugin );
	}
    }
#else
    MediaPlayerPlugin *plugin1 = new MediaPlayerPlugin();
    LibFFMpegPluginImpl *mpeg = new LibFFMpegPluginImpl();
    plugin1->decoder = mpeg->decoder();
    plugin1->encoder = mpeg->encoder();
    pluginList.append( *plugin1 );
#endif
    if ( pluginList.count() ) 
	MediaPlayerDebug(( "%i decoders found", pluginList.count() )); // No tr
    else
	MediaPlayerDebug(( "No decoders found" )); // No tr
}


void MediaPlayerState::startTemporaryMute()
{
    if ( temporaryMuteRefCount == 0 )
	AudioDevice::setMuted(true);
    temporaryMuteRefCount++;
}


void MediaPlayerState::stopTemporaryMute( int delay )
{
    QTimer::singleShot( delay, this, SLOT( stopTemporaryMute() ) );
}


void MediaPlayerState::stopTemporaryMute()
{
    temporaryMuteRefCount--;
    if ( temporaryMuteRefCount <= 0 ) {
	if ( !temporaryIsMuted )
	    AudioDevice::setMuted(false);
	temporaryMuteRefCount = 0;
    }
}

