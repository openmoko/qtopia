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

#include "ficgta01multiplexer.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <qgsm0710multiplexer.h>
#include <qserialport.h>
#include <qtopialog.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>

QTOPIA_EXPORT_PLUGIN( Ficgta01MultiplexerPlugin )

// Define this to disable GSM 07.10 multiplexing, but still do wakeup handling.
#define FICGTA01_NO_MUX


// Size of GSM 07.10 frames to use with the multiplexer.
#ifndef FICGTA01_FRAME_SIZE
#define FICGTA01_FRAME_SIZE       64
#endif

#define N_TIHTC 17

Ficgta01MultiplexerPlugin::Ficgta01MultiplexerPlugin( QObject* parent )
    : QSerialIODeviceMultiplexerPlugin( parent )
{
}

Ficgta01MultiplexerPlugin::~Ficgta01MultiplexerPlugin()
{
}


bool Ficgta01MultiplexerPlugin::detect( QSerialIODevice *device )
{
// The FIC needs a special line discipline set on the device.
    QSerialPort *port = qobject_cast<QSerialPort *>( device );
    if (port) {
        int discipline = N_TIHTC;
        ::ioctl(port->fd(), TIOCSETD, &discipline);
    }
    device->discard();
    int rc;
    struct termios t;
    rc = tcgetattr(port->fd(), &t);
    t.c_cflag |= CRTSCTS;
    rc = tcsetattr(port->fd(), TCSANOW, &t);

    // Make the modem talk to us. It can be a bit rough to get
    // it initialized... So we will empty the current buffer
    // and then send ^Z\r\n and wait for an OK or AT from the modem. This is
    // mostly based on ideas from ogsmd
    device->readAll();
    int attempts = 0;
    for (; attempts < 10; ++attempts) {
        if (QSerialIODeviceMultiplexer::chat(device, QChar(0x1a))) {
            qLog(Modem) << "Attempts needed to initialize the modem" << attempts;
            break;
        }
    }

    if (attempts == 10) {
        qWarning() << "Initializing the modem failed. What should one do?... abort";
        abort();
    }


    // disable echoing of commands
    QSerialIODeviceMultiplexer::chat(device, "ATE0");
    device->readAll();


    // Issue the AT+CMUX command to determine if this device
    // uses GSM 07.10-style multiplexing.
#ifndef FICGTA01_NO_MUX
    return QGsm0710Multiplexer::cmuxChat( device, FICGTA01_FRAME_SIZE, true );
#else
    return true;
#endif
}

QSerialIODeviceMultiplexer *Ficgta01MultiplexerPlugin::create
        ( QSerialIODevice *device )
{
#ifndef FICGTA01_NO_MUX
    return new QGsm0710Multiplexer( device, FICGTA01_FRAME_SIZE, true );
#else
    return new QNullSerialIODeviceMultiplexer( device );
#endif
}
