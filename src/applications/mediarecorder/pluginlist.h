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
#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <qvaluelist.h>
#include <qtopia/qlibrary.h>
#include <qtopia/mediarecorderplugininterface.h>
#include <qtopia/mediaplayerplugininterface.h>


class MediaRecorderPlugin
{
public:
#ifndef QT_NO_COMPONENT
    QLibrary *library;
#endif
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

#ifndef QT_NO_COMPONENT
    void addFormats( QLibrary *library, MediaRecorderPluginInterface *iface );
#else
    void addFormats( MediaRecorderPluginInterface *iface );
#endif

};


enum DecoderVersion {
    Decoder_Unknown,
    Decoder_1_5,
    Decoder_1_6
};


class MediaPlayerPlugin
{
public:
#ifndef QT_NO_COMPONENT
    QLibrary *library;
#endif
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

    void loadPlugins();

};


#endif

