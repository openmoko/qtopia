/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "qmediacodecplugin.h"


/*!
    \class QMediaCodecPlugin
    \preliminary
    \brief The QMediaCodecPlugin class represents a codec factory in Qtopia's
    media engine.

    \sa QMediaDecoder, QMediaDevice
*/


/*!
    Destroy the QMediaCodecPlugin object
*/

QMediaCodecPlugin::~QMediaCodecPlugin()
{
}

/*!
    \fn QString QMediaCodecPlugin::name() const;

    Return the name of the Codec package.
*/

/*!
    \fn QString QMediaCodecPlugin::comment() const;

    Return a general comment about this Codec package, there is no specified
    format.
*/

/*!
    \fn QStringList QMediaCodecPlugin::mimeTypes() const;

    Return a QStringList of mime-types that are supported by this Codec
    package.
*/

/*!
    \fn QStringList QMediaCodecPlugin::fileExtensions() const;

    Return a QStringList of the file extensions that are known to this
    Codec package.
*/

/*!
    \fn double QMediaCodecPlugin::version() const;

    Return version information for this Codec package, it is specific to the
    implmentation.
*/

/*!
    \fn bool QMediaCodecPlugin::canEncode() const;

    Return an indication of whether this Codec package supports the encoding
    of data.
*/

/*!
    \fn bool QMediaCodecPlugin::canDecode() const;

    Return an indication of whether this Codec package supports the decoding
    of data.
*/

/*!
    \fn QMediaEncoder* QMediaCodecPlugin::encoder(QString const& mimeType);

    Return a QMediaEncoder suitable for the specified \a mimeType.
*/

/*!
    \fn QMediaDecoder* QMediaCodecPlugin::decoder(QString const& mimeType);

    Return a QMediaDecoder suitable for the specified \a mimeType.
*/

