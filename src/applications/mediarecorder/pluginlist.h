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
#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <qvaluelist.h>
#include <qtopia/qlibrary.h>
#include <qtopia/mediarecorderplugininterface.h>
#include <qtopia/mediaplayerplugininterface.h>


class PluginLoader;


class MediaRecorderPlugin
{
public:
    MediaRecorderPluginInterface *iface;
    MediaRecorderEncoder *encoder;
    QString format;
    QString formatName;
};


class MediaRecorderPluginList
{
public:
    MediaRecorderPluginList();
    ~MediaRecorderPluginList();

    // Walk the encoder list.
    uint count() const { return pluginList.count(); }
    MediaRecorderEncoder *at( uint n ) const { return pluginList[n].encoder; }
    QString formatAt( uint n ) const { return pluginList[n].format; }
    QString formatNameAt( uint n ) const { return pluginList[n].formatName; }

    // Get the plugin with a specific MIME type and format tag.
    MediaRecorderEncoder *fromType( const QString& type, const QString& tag );

    // Get the index of a plugin with a specific MIME type and format tag.
    int indexFromType( const QString& type, const QString& tag );

private:
    QValueList<MediaRecorderPlugin> pluginList;
    PluginLoader *loader;

    void addFormats( MediaRecorderPluginInterface *iface );

};


enum DecoderVersion {
    Decoder_Unknown,
    Decoder_1_5,
    Decoder_1_6
};


class MediaPlayerPlugin
{
public:
    MediaPlayerPluginInterface *iface;
    MediaPlayerDecoder *decoder;
    DecoderVersion version;
};


class MediaPlayerPluginList
{
public:
    MediaPlayerPluginList();
    ~MediaPlayerPluginList();

    // Walk the decoder list.
    uint count() const { return pluginList.count(); }
    MediaPlayerDecoder *at( uint n ) const { return pluginList[n].decoder; }

    // Get the plugin that can play a specific file.
    MediaPlayerDecoder *fromFile( const QString& file );

private:
    QValueList<MediaPlayerPlugin> pluginList;
    MediaPlayerDecoder *curDecoder;
    DecoderVersion curDecoderVersion;
    PluginLoader *loader;

    void loadPlugins();

};


#endif

