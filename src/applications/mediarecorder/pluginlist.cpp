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
#include "pluginlist.h"

#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qdir.h>


#ifdef QT_NO_COMPONENT
// Plugins that are compiled in when no plugin architecture available.
#include "../../plugins/codecs/wavrecord/wavrecordimpl.h"
#include "../../plugins/codecs/wavplugin/wavpluginimpl.h"
#include "../../3rdparty/plugins/codecs/libffmpeg/libffmpegpluginimpl.h"
#endif


MediaRecorderPluginList::MediaRecorderPluginList()
{
#ifndef QT_NO_COMPONENT
    // Load the plugin list from the "/plugins/codecs" directory.
    QString path = QPEApplication::qpeDir() + "plugins/codecs";
    QDir dir( path, "lib*.so" );
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaRecorderPluginInterface *iface = 0;
	QLibrary *lib = new QLibrary( path + "/" + *it );

	qDebug( "querying: %s", (path + "/" + *it).latin1() );
	if ( lib->queryInterface( IID_MediaRecorderPlugin, (QUnknownInterface**)&iface ) == QS_OK && iface ) {

	    addFormats( lib, iface );

	} else {
	    delete lib;
	}
    }
#else
    // No component support - load the compiled-in encoders.
    MediaRecorderPluginInterface *iface = new WavRecorderPluginImpl();
    addFormats( iface );
#endif
}


MediaRecorderPluginList::~MediaRecorderPluginList()
{
    QValueList<MediaRecorderPlugin>::Iterator mit;
    for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	if ((*mit).iface) {
	    delete (*mit).encoder;
	    (*mit).iface->release();
#ifndef QT_NO_COMPONENT
	    (*mit).library->unload();
	    delete (*mit).library;
#endif
	}
    }
    pluginList.clear();
}


MediaRecorderEncoder *MediaRecorderPluginList::fromType( const QString& type, const QString& tag )
{
    uint numPlugins;
    uint plugin;
    MediaRecorderEncoder *encoder;
    QString format;

    numPlugins = count();

    for ( plugin = 0; plugin < numPlugins; ++plugin ) {
	encoder = pluginList[plugin].encoder;
	format = pluginList[plugin].format;
	if ( type == encoder->pluginMimeType() && tag == format ) {
	    return encoder;
	}
    }

    return 0;
}


int MediaRecorderPluginList::indexFromType( const QString& type, const QString& tag )
{
    uint numPlugins;
    uint plugin;
    MediaRecorderEncoder *encoder;
    QString format;

    numPlugins = count();

    for ( plugin = 0; plugin < numPlugins; ++plugin ) {
	encoder = pluginList[plugin].encoder;
	format = pluginList[plugin].format;
	if ( type == encoder->pluginMimeType() && tag == format ) {
	    return (int)plugin;
	}
    }

    return -1;
}


#ifndef QT_NO_COMPONENT
void MediaRecorderPluginList::addFormats( QLibrary *library, MediaRecorderPluginInterface *iface )
#else
void MediaRecorderPluginList::addFormats( MediaRecorderPluginInterface *iface )
#endif
{
    MediaRecorderPlugin plugin;
    MediaRecorderEncoder *encoder;
    int nformats, format;

    encoder = iface->encoder();
    nformats = encoder->pluginNumFormats();

    for ( format = 0; format < nformats; ++format ) {

    #ifndef QT_NO_COMPONENT
	plugin.library = library;
    #endif
	plugin.iface = iface;
	plugin.encoder = encoder;
	plugin.format = encoder->pluginFormatTag( format );
	plugin.formatName = encoder->pluginFormatName( format );
	pluginList.append( plugin );

        qDebug( "recorder found for %s [%s]",
	        plugin.encoder->pluginMimeType().latin1(),
		plugin.format.latin1());

	library = 0;
	iface = 0;
    }
}


MediaPlayerPluginList::MediaPlayerPluginList()
{
    curDecoder = NULL;
    curDecoderVersion = Decoder_Unknown;
    loadPlugins();
}


MediaPlayerPluginList::~MediaPlayerPluginList()
{
#ifndef QT_NO_COMPONENT
    QValueList<MediaPlayerPlugin>::Iterator mit;
    for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	(*mit).iface->release();
	(*mit).library->unload();
	delete (*mit).library;
    }
#endif
}


MediaPlayerDecoder *MediaPlayerPluginList::fromFile( const QString& file )
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


void MediaPlayerPluginList::loadPlugins()
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

	qDebug( "querying: %s", QString( path + "/" + *it ).latin1() );

	DecoderVersion ver = Decoder_Unknown;
	if ( lib->queryInterface( IID_MediaPlayerPlugin_1_6, (QUnknownInterface**)&iface ) == QS_OK && iface != 0 ) 
	    ver = Decoder_1_6;
	else if ( lib->queryInterface( IID_MediaPlayerPlugin, (QUnknownInterface**)&iface ) == QS_OK && iface != 0 )
	    ver = Decoder_1_5;

	if ( ver != Decoder_Unknown ) {
	    qDebug( "loading: %s", QString( path + "/" + *it ).latin1() );

	    MediaPlayerPlugin plugin;
	    plugin.version = ver;
	    plugin.library = lib;
	    plugin.iface = iface;
	    plugin.decoder = plugin.iface->decoder();
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
    pluginList.append( plugin0 );

    MediaPlayerPlugin plugin1;
    plugin1.iface = new WavPluginImpl;
    plugin1.decoder = plugin1.iface->decoder();
    pluginList.append( plugin2 );
#endif
    if ( pluginList.count() ) 
	qDebug( "%i decoders found", pluginList.count() );
    else
	qDebug( "No decoders found" );
}

