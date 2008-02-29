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
#include "pluginlist.h"

#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qtopia/pluginloader.h>
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
    loader = new PluginLoader( "codecs" );
    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaRecorderPluginInterface *iface = 0;
	qDebug( "querying: %s", (*it).latin1() );
	if ( loader->queryInterface( *it, IID_MediaRecorderPlugin, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    addFormats( iface );
	}
    }
    if ( pluginList.count() ) 
	qDebug( "%i encoders found", pluginList.count() );
    else
	qDebug( "No encoders found" );
#else
    // No component support - load the compiled-in encoders.
    loader = 0;
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
#ifndef QT_NO_COMPONENT
	    loader->releaseInterface( (*mit).iface );
#else
	    (*mit).iface->release();
#endif
	}
    }
    pluginList.clear();
#ifndef QT_NO_COMPONENT
    delete loader;
#endif
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


void MediaRecorderPluginList::addFormats( MediaRecorderPluginInterface *iface )
{
    MediaRecorderPlugin plugin;
    MediaRecorderEncoder *encoder;
    int nformats, format;

    encoder = iface->encoder();
    nformats = encoder->pluginNumFormats();

    for ( format = 0; format < nformats; ++format ) {

	plugin.iface = iface;
	plugin.encoder = encoder;
	plugin.format = encoder->pluginFormatTag( format );
	plugin.formatName = encoder->pluginFormatName( format );
	pluginList.append( plugin );

        qDebug( "recorder found for %s [%s]",
	        plugin.encoder->pluginMimeType().latin1(),
		plugin.format.latin1());

	iface = 0;
    }
}


MediaPlayerPluginList::MediaPlayerPluginList()
{
    loader = NULL;
    curDecoder = NULL;
    curDecoderVersion = Decoder_Unknown;
    loadPlugins();
}


MediaPlayerPluginList::~MediaPlayerPluginList()
{
#ifndef QT_NO_COMPONENT
    QValueList<MediaPlayerPlugin>::Iterator mit;
    for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	loader->releaseInterface( (*mit).iface );
    }
    delete loader;
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
    if ( !loader )
	loader = new PluginLoader( "codecs" );

    QValueList<MediaPlayerPlugin>::Iterator mit;
    for ( mit = pluginList.begin(); mit != pluginList.end(); ++mit ) {
	loader->releaseInterface( (*mit).iface );
    }
    pluginList.clear();

    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	MediaPlayerPluginInterface *iface = 0;

	qDebug( "querying: %s", (*it).latin1() );

	DecoderVersion ver = Decoder_Unknown;
	if ( loader->queryInterface( *it, IID_MediaPlayerPlugin_1_6, (QUnknownInterface**)&iface ) == QS_OK && iface != 0 ) 
	    ver = Decoder_1_6;
	else if ( loader->queryInterface( *it, IID_MediaPlayerPlugin, (QUnknownInterface**)&iface ) == QS_OK && iface != 0 )
	    ver = Decoder_1_5;

	if ( ver != Decoder_Unknown ) {

	    MediaPlayerPlugin plugin;
	    plugin.version = ver;
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

