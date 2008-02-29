/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef MULTIPORTMULTIPLEXER_H
#define MULTIPORTMULTIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>

class MultiPortMultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    MultiPortMultiplexerPlugin( QObject *parent = 0 )
	: QSerialIODeviceMultiplexerPlugin( parent ) {}

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

#endif // MULTIPORTMULTIPLEXER_H
