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

#include "qmediadevice.h"

#include "qmediapipe.h"


/*!
    \class QMediaPipe
    \preliminary
    \brief The QMediaPipe class connects QMediaDevices in media processing
    chain.

    \sa QMediaDevice, QMediaDecoder
*/

// {{{ QMediaPipePrivate
class QMediaPipePrivate
{
public:
    QMediaDevice*  inputDevice;
    QMediaDevice*  outputDevice;
};
// }}}


// {{{ QMediaPipe

/*!
    Construct a QMediaPipe with the given \a inputDevice as source and the \a
    outputDevice as destination for all data. The QMediaPipe will be parented
    to \a parent.
*/

QMediaPipe::QMediaPipe
(
 QMediaDevice* inputDevice,
 QMediaDevice* outputDevice,
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

    d->inputDevice->connectOutputPipe(this);
    d->outputDevice->connectInputPipe(this);
}

/*!
    Destroy the QMediaPipe object, disconnecting it from the media processing
    chain.
*/

QMediaPipe::~QMediaPipe()
{
    d->inputDevice->disconnectOutputPipe(this);
    d->outputDevice->disconnectInputPipe(this);

    delete d;
}

/*!
    Read \a data of maximim length \a maxlen, this function should normally be
    called by the outputDevice.
*/

qint64 QMediaPipe::read(char *data, qint64 maxlen)
{
    return d->inputDevice->read(data, maxlen);
}

/*!
    Write \a data of length \a len, this function should normally be called by the
    inputDevice.
*/

qint64 QMediaPipe::write(const char *data, qint64 len)
{
    return d->outputDevice->write(data, len);
}

/*!
    \fn void QMediaPipe::readyRead();

    Signal emitted when the pipe has data available from the inputDevice.
*/

/*!
    \fn void QMediaPipe::bytesWritten(qint64 bytes);

    Signal emitted when the pipe has written \a bytes of data to the outputDevice.
*/

// }}}
