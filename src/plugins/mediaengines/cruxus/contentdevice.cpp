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

#include <QContent>

#include "contentdevice.h"


class ContentDevicePrivate
{
public:
    QContent*   content;
    QIODevice*  contentDevice;
    QMediaPipe* outputPipe;
    QMediaDevice::Info  outputInfo;
};

/*!
    \class ContentDevice
    \internal
*/

ContentDevice::ContentDevice(QString const& filePath):
    d(new ContentDevicePrivate)
{
    // chop URL modifier
    int marker = filePath.indexOf("://");
    QString urlStripped = filePath.mid(marker + 3);

    d->content = new QContent(marker != -1 ? urlStripped : filePath, false);
    d->contentDevice = 0;
    d->outputPipe = 0;

    d->outputInfo.type = QMediaDevice::Info::Raw;
}

ContentDevice::~ContentDevice()
{
    delete d->contentDevice;
    delete d->content;
    delete d;
}

QMediaDevice::Info const& ContentDevice::dataType() const
{
    return d->outputInfo;
}

bool ContentDevice::connectToInput(QMediaDevice*)
{
    qWarning("ContentDevice::connectToInput(); ContentDevice is a sink - inputs are invalid");

    return false;
}

void ContentDevice::disconnectFromInput(QMediaDevice*)
{
    qWarning("ContentDevice::disconnectFromInput(); ContentDevice is a sink - inputs are invalid");
}

bool ContentDevice::open(QIODevice::OpenMode)
{
    bool        rc = false;

    if (d->contentDevice)
        rc = true;
    else
    {
        d->contentDevice = d->content->open(QIODevice::ReadOnly);
        if (d->contentDevice) {
            if (QIODevice::open(QIODevice::ReadOnly | QIODevice::Unbuffered))
            {
                d->outputInfo.dataSize = d->contentDevice->isSequential() ? -1 : d->contentDevice->size();

                connect(d->contentDevice, SIGNAL(aboutToClose()), SIGNAL(aboutToClose()));
                connect(d->contentDevice, SIGNAL(bytesWritten(qint64)), SIGNAL(bytesWritten(qint64)));
                connect(d->contentDevice, SIGNAL(readyRead()), SIGNAL(readyRead()));

                rc = true;
            }
            else {
                delete d->contentDevice;
                d->contentDevice = 0;
            }
        }
    }

    return rc;
}

void ContentDevice::close()
{
    if (d->contentDevice) {
        d->contentDevice->close();

        delete d->contentDevice;
        d->contentDevice = 0;

        QIODevice::close();
    }
}

bool ContentDevice::isSequential() const
{
    if (d->contentDevice)
        return d->contentDevice->isSequential();

    qWarning("Calling isSequential() on an invalid ContentDevice");

    return false;
}

bool ContentDevice::seek(qint64 pos)
{
    if (d->contentDevice)
        return d->contentDevice->seek(pos);

    qWarning("Calling seek() on an invalid ContentDevice");

    return false;
}

qint64 ContentDevice::pos() const
{
    if (d->contentDevice)
        return d->contentDevice->pos();

    qWarning("Calling pos() on an invalid ContentDevice");

    return 0;
}

//protected:
qint64 ContentDevice::readData(char *data, qint64 maxlen)
{
    if (d->contentDevice)
        return d->contentDevice->read(data, maxlen);

    qWarning("Calling readData() on an invalid ContentDevice");

    return 0;
}

qint64 ContentDevice::writeData(const char *, qint64)
{
    return 0;
}


