/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "wavdecoder.h"

#include "wavplugin.h"



// {{{ WavPluginPrivate
class WavPluginPrivate
{
public:
    QStringList mimeTypes;
    QStringList fileExtensions;
};
// }}}

/*!
    \class WavPlugin
    \brief The WavPlugin class is used to identify the capabilities of a media plugin and
    to construct a Decoder for the Wave data file format.
    \sa QMediaCodecPlugin, QMediaDecoder, WavDecoder
*/

// {{{ WavPlugin
/*!
    Construct a WavPlugin.
*/

WavPlugin::WavPlugin():
    d(new WavPluginPrivate)
{
    d->mimeTypes << "audio/x-wav";
    d->fileExtensions << "wav";
}

/*!
    Destroy the WavPlugin object.
*/

WavPlugin::~WavPlugin()
{
    delete d;
}

/*!
    Return the Name of this plugin.
*/

QString WavPlugin::name() const
{
    return QLatin1String("Wave decoder");
}

/*!
    Return the comment string for this plugin.
*/

QString WavPlugin::comment() const
{
    return QString();
}

/*!
    Return the mimeTypes that this plugin handles.
*/

QStringList WavPlugin::mimeTypes() const
{
    return d->mimeTypes;
}

/*!
    Return the recognized file extensions for this plugin.
*/

QStringList WavPlugin::fileExtensions()  const
{
    return d->fileExtensions;
}

/*!
    Return the version information for this plugin.
*/

double WavPlugin::version() const
{
    return 0.01;
}

/*!
    Return whether this plugin is capable of encoding data.
*/

bool WavPlugin::canEncode() const
{
    return false;
}

/*!
    Return whether this plugin is capable of decoding data.
*/

bool WavPlugin::canDecode() const
{
    return true;
}

/*!
    Return an encoder for the \a mimeType.
*/

QMediaEncoder* WavPlugin::encoder(QString const& mimeType)
{
    Q_UNUSED(mimeType);

    return 0;
}

/*!
    Return a decoder for the \a mimeType.
*/

QMediaDecoder* WavPlugin::decoder(QString const& mimeType)
{
    Q_UNUSED(mimeType);

    return new WavDecoder;
}
// }}}

QTOPIA_EXPORT_PLUGIN(WavPlugin);

