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
#include <qtopia/qpeapplication.h>
#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/qlibrary.h>
#include <qtopia/mediaplayerplugininterface.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qvaluelist.h>
#include <qobject.h>
#include <qtimer.h>
#include <qdir.h>
#include "audiodevice.h"
#include "mediaplayerstate.h"
#include "audiowidget.h"
#include "videowidget.h"
#include "maindocumentwidgetstack.h"

#ifdef QT_NO_COMPONENT
// Plugins which are compiled in when no plugin architecture available
#include "../../3rdparty/plugins/codecs/libffmpeg/libffmpegpluginimpl.h"
#endif


#define MediaPlayerDebug(x)	qDebug x
//#define MediaPlayerDebug(x)


MediaPlayerState::MediaPlayerState( QObject *parent, const char *name )
    : QObject( parent, name ), aw( 0 ), vw( 0 ), curDecoder( 0 ),
	curDecoderVersion( Decoder_Unknown ), temporaryMuteRefCount( 0 )
{
    Config cfg( "MediaPlayer" );
    readConfig( cfg );

    // Start loading up the UIs up in the background if they haven't been already
    QTimer::singleShot( 2000, this, SLOT( audioUI() ) );
    QTimer::singleShot( 5000, this, SLOT( videoUI() ) );
}


MediaPlayerState::~MediaPlayerState()
{
    Config cfg( "MediaPlayer" );
    writeConfig( cfg );
    delete aw;
    delete vw;
}


void MediaPlayerState::readConfig( Config& cfg )
{
    cfg.setGroup( "Options" );
    isFullscreen = cfg.readBoolEntry( "FullScreen", FALSE );
    isScaled = cfg.readBoolEntry( "Scaling", TRUE );
    isLooping = cfg.readBoolEntry( "Looping", FALSE );
    isShuffled = cfg.readBoolEntry( "Shuffle", FALSE );
    curSkin = cfg.readEntry( "Skin", "default" );
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


AudioWidget* MediaPlayerState::audioUI()
{
    if ( !aw ) {
	aw = new AudioWidget( mainDocumentWindow, curSkin, "audioUI" );
	connect( aw,  SIGNAL( moreClicked() ),         this, SIGNAL( increaseVolume() ) );
	connect( aw,  SIGNAL( lessClicked() ),         this, SIGNAL( decreaseVolume() ) );
	connect( aw,  SIGNAL( moreReleased() ),        this, SIGNAL( endIncreaseVolume() ) );
	connect( aw,  SIGNAL( lessReleased() ),        this, SIGNAL( endDecreaseVolume() ) );
	connect( aw,  SIGNAL( forwardClicked() ),      this, SIGNAL( scanForward() ) );
	connect( aw,  SIGNAL( backwardClicked() ),     this, SIGNAL( scanBackward() ) );
	connect( aw,  SIGNAL( forwardReleased() ),     this, SIGNAL( endScanForward() ) );
	connect( aw,  SIGNAL( backwardReleased() ),    this, SIGNAL( endScanBackward() ) );
    }
    return aw;
}


VideoWidget* MediaPlayerState::videoUI()
{
    if ( !vw ) {
	vw = new VideoWidget( mainDocumentWindow, curSkin, "videoUI" );
	connect( vw,  SIGNAL( forwardClicked() ),      this, SIGNAL( scanForward() ) );
	connect( vw,  SIGNAL( backwardClicked() ),     this, SIGNAL( scanBackward() ) );
	connect( vw,  SIGNAL( forwardReleased() ),     this, SIGNAL( endScanForward() ) );
	connect( vw,  SIGNAL( backwardReleased() ),    this, SIGNAL( endScanBackward() ) );
    }
    return vw;
}


struct MediaPlayerPlugin {
#ifndef QT_NO_COMPONENT
    QLibrary *library;
    MediaPlayerPluginInterface *iface;
#endif
    MediaPlayerDecoder *decoder;
    MediaPlayerEncoder *encoder;
    DecoderVersion version;
};


static QValueList<MediaPlayerPlugin> pluginList;


// Find the first decoder which supports this type of file
MediaPlayerDecoder *MediaPlayerState::decoder( const QString& file )
{
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


void MediaPlayerState::loadPlugins()
{
#ifndef QT_NO_COMPONENT
    QValueList<MediaPlayerPlugin>::Iterator mit;
    for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	(*mit).iface->release();
	(*mit).library->unload();
	delete (*mit).library;
    }
    pluginList.clear();

    QString path = QPEApplication::qpeDir() + "plugins/codecs";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaPlayerPluginInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );

	MediaPlayerDebug(( "querying: %s", QString( path + "/" + *it ).latin1() ));

	DecoderVersion ver = Decoder_Unknown;

	// Some old plugins have dodgy queryInterface implementations which always anwser QS_OK
	const QUuid IID_evilPlugin( 0x00000666, 0x0666, 0x0666, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x66 );
	if ( lib->queryInterface( IID_evilPlugin, (QUnknownInterface**)&iface ) == QS_OK ) {
	    qDebug("Detected old evil plugin which answers to any old interface Id!");
	    lib->queryInterface( IID_MediaPlayerPlugin, (QUnknownInterface**)&iface );
	    ver = Decoder_1_5;
	} else if ( lib->queryInterface( IID_MediaPlayerPlugin_1_6, (QUnknownInterface**)&iface ) == QS_OK ) 
	    ver = Decoder_1_6;
	else if ( lib->queryInterface( IID_MediaPlayerPlugin, (QUnknownInterface**)&iface ) == QS_OK )
	    ver = Decoder_1_5;

	if ( ver != Decoder_Unknown ) {
	    MediaPlayerDebug(( "loading: %s", QString( path + "/" + *it ).latin1() ));

	    MediaPlayerPlugin plugin;
	    plugin.version = ver;
	    plugin.library = lib;
	    plugin.iface = iface;
	    plugin.decoder = plugin.iface->decoder();
	    plugin.encoder = plugin.iface->encoder();
	    pluginList.append( plugin );

	    QString type = (*it).left( (*it).find(".") );
	    QStringList langs = Global::languageList();
	    for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
		QString lang = *lit;
		QTranslator * trans = new QTranslator(qApp);
		QString tfn = QPEApplication::qpeDir()+"i18n/"+lang+"/"+type+".qm";
		if ( trans->load( tfn ))
		    qApp->installTranslator( trans );
		else
		    delete trans;
	    }

	} else {
	    delete lib;
	}
    }
#else
    pluginList.clear();
    
    MediaPlayerPlugin *plugin1 = new MediaPlayerPlugin();
    LibFFMpegPluginImpl *mpeg = new LibFFMpegPluginImpl();
    plugin1->decoder = mpeg->decoder();
    plugin1->encoder = mpeg->encoder();
    pluginList.append( *plugin1 );

#endif
    if ( pluginList.count() ) 
	MediaPlayerDebug(( "%i decoders found", pluginList.count() ));
    else
	MediaPlayerDebug(( "No decoders found" ));
}


void MediaPlayerState::startTemporaryMute()
{
    if ( temporaryMuteRefCount == 0 ) {
	//qDebug("start mute");
	AudioDevice::getVolume( temporaryLeftVol, temporaryRightVol, temporaryIsMuted );
	if ( !temporaryIsMuted )
	    AudioDevice::setVolume( 0, 0, FALSE ); // temporaryLeftVol, temporaryRightVol, TRUE );
    }
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
	//qDebug("stop mute");
	if ( !temporaryIsMuted )
	    AudioDevice::setVolume( temporaryLeftVol, temporaryRightVol, FALSE );
	temporaryMuteRefCount = 0;
    }
}

