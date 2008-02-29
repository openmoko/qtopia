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

#include <qcontent.h>

#include "contentdevice.h"


class ContentDevicePrivate
{
public:
    QContent*   content;
    QIODevice*  contentDevice;
    QMediaPipe* outputPipe;
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
}

ContentDevice::~ContentDevice()
{
    delete d->contentDevice;
    delete d->content;
    delete d;
}

void ContentDevice::connectInputPipe(QMediaPipe* inputPipe)
{
    Q_UNUSED(inputPipe);

    qWarning("ContentDevice is a Source, setting an input pipe is invalid");
}

void ContentDevice::connectOutputPipe(QMediaPipe* outputPipe)
{
    d->outputPipe = outputPipe;
}

void ContentDevice::disconnectInputPipe(QMediaPipe* inputPipe)
{
    Q_UNUSED(inputPipe);

    qWarning("ContentDevice is a Source, setting an input pipe is invalid");
}

void ContentDevice::disconnectOutputPipe(QMediaPipe* outputPipe)
{
    d->outputPipe = 0;
}

void ContentDevice::setValue(QString const&, QVariant const&)
{
}

QVariant ContentDevice::value(QString const&)
{
    return QVariant();
}

bool ContentDevice::open(QIODevice::OpenMode)
{
    d->contentDevice = d->content->open(QIODevice::ReadOnly);

    return d->contentDevice == 0 ? false : QIODevice::open(QIODevice::ReadOnly);
}

void ContentDevice::close()
{
    if (d->contentDevice)
        d->contentDevice->close();
}

bool ContentDevice::isSequential() const
{
    if (d->contentDevice)
        return d->contentDevice->isSequential();

    qWarning("Calling isSequential on an invalid ContentDevice");

    return false;
}

//protected:
qint64 ContentDevice::readData(char *data, qint64 maxlen)
{
    if (d->contentDevice)
        return d->contentDevice->read(data, maxlen);

    qWarning("Calling readData on an invalid ContentDevice");

    return 0;
}

qint64 ContentDevice::writeData(const char *, qint64)
{
    return 0;
}


