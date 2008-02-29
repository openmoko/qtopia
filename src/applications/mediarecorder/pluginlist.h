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
#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <qlist.h>
#include <qlibrary.h>
#include <mediarecorderplugininterface.h>


class QPluginManager;


class MediaRecorderPlugin
{
public:
    MediaRecorderCodecPlugin *plugin;
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
    QList<MediaRecorderPlugin> pluginList;
    QPluginManager *loader;

    void addFormats( MediaRecorderCodecPlugin *iface );

};

#endif

