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
#include <qpe/qpeapplication.h>
#define QTOPIA_INTERNAL_LANGLIST
#include <qpe/qlibrary.h>
#include <qpe/mediaplayerplugininterface.h>
#include <qpe/config.h>
#include <qpe/global.h>
#include <qvaluelist.h>
#include <qobject.h>
#include <qtimer.h>
#include <qdir.h>
#include "audiodevice.h"
#include "mediaplayerstate.h"
#include "audiowidget.h"
#include "videowidget.h"

#ifdef QT_NO_COMPONENT
// Plugins which are compiled in when no plugin architecture available
#include "libmad/libmadpluginimpl.h"
#include "libmpeg3/libmpeg3pluginimpl.h"
#include "wavplugin/wavpluginimpl.h"
#endif


#define MediaPlayerDebug(x)	qDebug x
//#define MediaPlayerDebug(x)


MediaPlayerState::MediaPlayerState( QObject *parent, const char *name )
    : QObject( parent, name ), aw( 0 ), vw( 0 ), curDecoder( 0 ), temporaryMuteRefCount( 0 )
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
    isFullscreen = cfg.readBoolEntry( "FullScreen" );
    isScaled = cfg.readBoolEntry( "Scaling" );
    isLooping = cfg.readBoolEntry( "Looping" );
    isShuffled = cfg.readBoolEntry( "Shuffle" );
    usePlaylist = cfg.readBoolEntry( "UsePlayList" );
    curSkin = cfg.readEntry( "Skin", "default" );
    isPlaying = FALSE;
    isPaused = FALSE;
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
    cfg.writeEntry("UsePlayList", usePlaylist );
    cfg.writeEntry("Skin", curSkin );
}


AudioWidget* MediaPlayerState::audioUI()
{
    if ( !aw ) {
	aw = new AudioWidget( 0, curSkin, "audioUI" );
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
	vw = new VideoWidget( 0, curSkin, "videoUI" );
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
#endif
    MediaPlayerPluginInterface *iface;
    MediaPlayerDecoder *decoder;
    MediaPlayerEncoder *encoder;
};


static QValueList<MediaPlayerPlugin> pluginList;


// Find the first decoder which supports this type of file
MediaPlayerDecoder *MediaPlayerState::decoder( const QString& file )
{
    if ( file.isNull() )
	return curDecoder;
    MediaPlayerDecoder *tmpDecoder = NULL;
    QValueList<MediaPlayerPlugin>::Iterator it;
    for ( it = pluginList.begin(); it != pluginList.end(); ++it ) {
	if ( (*it).decoder->isFileSupported( file ) ) {
	    tmpDecoder = (*it).decoder;
	    break;
	}
    }
    return curDecoder = tmpDecoder; 
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

    QString path = QPEApplication::qpeDir() + "/plugins/codecs";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaPlayerPluginInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );

	MediaPlayerDebug(( "querying: %s", QString( path + "/" + *it ).latin1() ));

	if ( lib->queryInterface( IID_MediaPlayerPlugin, (QUnknownInterface**)&iface ) == QS_OK ) {

	    MediaPlayerDebug(( "loading: %s", QString( path + "/" + *it ).latin1() ));

	    MediaPlayerPlugin plugin;
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
		QString tfn = QPEApplication::qpeDir()+"/i18n/"+lang+"/"+type+".qm";
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
    
    MediaPlayerPlugin plugin0;
    plugin0.iface = new LibMpeg3PluginImpl;
    plugin0.decoder = plugin0.iface->decoder();
    plugin0.encoder = plugin0.iface->encoder();
    pluginList.append( plugin0 );

    MediaPlayerPlugin plugin1;
    plugin1.iface = new LibMadPluginImpl;
    plugin1.decoder = plugin1.iface->decoder();
    plugin1.encoder = plugin1.iface->encoder();
    pluginList.append( plugin1 );

    MediaPlayerPlugin plugin2;
    plugin2.iface = new WavPluginImpl;
    plugin2.decoder = plugin2.iface->decoder();
    plugin2.encoder = plugin2.iface->encoder();
    pluginList.append( plugin2 );
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

