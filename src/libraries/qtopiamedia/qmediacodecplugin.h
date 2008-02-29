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

#ifndef __QTOPIA_MEDIASERVER_MEDIACODECPLUGIN_H
#define __QTOPIA_MEDIASERVER_MEDIACODECPLUGIN_H

#include <QStringList>

#include <qtopiaglobal.h>

class QMediaEncoder;
class QMediaDecoder;

class QTOPIAMEDIA_EXPORT QMediaCodecPlugin
{
public:
    virtual ~QMediaCodecPlugin();

    virtual QString name() const = 0;
    virtual QString comment() const = 0;
    virtual QStringList mimeTypes() const = 0;
    virtual QStringList fileExtensions() const = 0;

    virtual double version() const = 0;

    virtual bool canEncode() const = 0;
    virtual bool canDecode() const = 0;

    virtual QMediaEncoder* encoder(QString const& mimeType) = 0;
    virtual QMediaDecoder* decoder(QString const& mimeType) = 0;
};


Q_DECLARE_INTERFACE(QMediaCodecPlugin, "com.trolltech.qtopia.MediaCodecPlugin/1.0");

#endif  // __QTOPIA_MEDIASERVER_MEDIACODECPLUGIN_H
