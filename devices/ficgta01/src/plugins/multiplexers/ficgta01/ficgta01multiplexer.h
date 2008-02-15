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

#ifndef FICGTA01MUTLIPLEXER_H
#define FICGTA01MUTLIPLEXER_H

#include <qserialiodevicemultiplexerplugin.h>
#include <qserialiodevicemultiplexer.h>

class Ficgta01MultiplexerPlugin : public QSerialIODeviceMultiplexerPlugin
{
    Q_OBJECT
public:
    Ficgta01MultiplexerPlugin( QObject* parent = 0 );
    ~Ficgta01MultiplexerPlugin();

    bool detect( QSerialIODevice *device );
    QSerialIODeviceMultiplexer *create( QSerialIODevice *device );
};

#endif /* FICGTA01MUTLIPLEXER_H */
