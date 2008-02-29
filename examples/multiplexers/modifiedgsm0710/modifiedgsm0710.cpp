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

#include "modifiedgsm0710.h"
#include <qgsm0710multiplexer.h>

bool ModifiedGsm0710MultiplexerPlugin::detect( QSerialIODevice *device )
{
    // Determine if the module supports a frame size of 256.
    return QGsm0710Multiplexer::cmuxChat( device, 256 );
}

QSerialIODeviceMultiplexer *ModifiedGsm0710MultiplexerPlugin::create
        ( QSerialIODevice *device )
{
    // Create the GSM 07.10 multiplexer with a frame size of 256.
    return new QGsm0710Multiplexer( device, 256 );
}

QTOPIA_EXPORT_PLUGIN( ModifiedGsm0710MultiplexerPlugin )
