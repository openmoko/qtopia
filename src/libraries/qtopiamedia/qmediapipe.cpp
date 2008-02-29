/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qiodevice.h>

#include "qmediapipe.h"


/*!
    \class QMediaPipe
    \internal
*/

// {{{ QMediaPipePrivate
class QMediaPipePrivate
{
public:
    QIODevice*  inputDevice;
    QIODevice*  outputDevice;
};
// }}}


// {{{ QMediaPipe
QMediaPipe::QMediaPipe
(
 QIODevice* inputDevice,
 QIODevice* outputDevice,
 QObject*   parent
):
    QObject(parent),
    d(new QMediaPipePrivate)
{
    d->inputDevice = inputDevice;
    d->outputDevice = outputDevice;

    connect(d->inputDevice, SIGNAL(readyRead()),
            this, SIGNAL(readyRead()));

    connect(d->outputDevice, SIGNAL(bytesWritten(qint64)),
            this, SIGNAL(bytesWritten(qint64)));
}

QMediaPipe::~QMediaPipe()
{
    delete d;
}

qint64 QMediaPipe::read(char *data, qint64 maxlen)
{
    return d->inputDevice->read(data, maxlen);
}

qint64 QMediaPipe::write(const char *data, qint64 len)
{
    return d->outputDevice->write(data, len);
}
// }}}
