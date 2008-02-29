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

#include "pictureiohandler.h"
#include <qtopiaglobal.h>
#include <QPicture>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QVariant>
#include <QIODevice>
#include <QByteArray>
#include <QDebug>

class QtopiaPicturePlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList QtopiaPicturePlugin::keys() const
{
    return QStringList() << QLatin1String("pic");
}

QImageIOPlugin::Capabilities QtopiaPicturePlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "pic")
        return Capabilities(CanRead);
    else
        return 0;


    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable())
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QtopiaPicturePlugin::create(QIODevice *device, const QByteArray &format) const
{
    PictureIOHandler *hand = new PictureIOHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

QTOPIA_EXPORT_QT_PLUGIN(QtopiaPicturePlugin)

//===========================================================================

class PictureIOHandlerPrivate
{
public:
    PictureIOHandlerPrivate()
        : picture(new QPicture()), loaded(false)
    {}
    ~PictureIOHandlerPrivate()
    {
        delete picture;
    }

    bool load(QIODevice *device);

    QPicture *picture;
    QSize defaultSize;
    QSize currentSize;
    bool loaded;
};

bool PictureIOHandlerPrivate::load(QIODevice *device)
{
    if (loaded)
        return true;

    if (picture->load(device)) {
        defaultSize = picture->boundingRect().size();
        if (currentSize.isEmpty())
            currentSize = defaultSize;
        loaded = true;
    }

    return loaded;
}

PictureIOHandler::PictureIOHandler()
    : d(new PictureIOHandlerPrivate())
{
}

PictureIOHandler::~PictureIOHandler()
{
    delete d;
}

bool PictureIOHandler::canRead() const
{
    QByteArray contents = device()->peek(8);

    return contents.contains("QPIC");
}

QByteArray PictureIOHandler::name() const
{
    return "pic";
}

bool PictureIOHandler::read(QImage *image)
{
    if (d->load(device())) {
        *image = QImage(d->currentSize, QImage::Format_ARGB32_Premultiplied);
        image->fill(0x00000000);
        QPainter p(image);
        QRect br = d->picture->boundingRect();
        if (br.width() > 0 && br.height() > 0)
            p.scale(qreal(d->currentSize.width())/br.width(), qreal(d->currentSize.height())/br.height());
        p.drawPicture(0, 0, *d->picture);
        p.end();
        return true;
    }

    return false;
}

QVariant PictureIOHandler::option(ImageOption option) const
{
    switch(option) {
    case Size:
        d->load(device());
        return d->defaultSize;
        break;
    case ScaledSize:
        return d->currentSize;
        break;
    default:
        break;
    }
    return QVariant();
}

void PictureIOHandler::setOption(ImageOption option, const QVariant & value)
{
    switch(option) {
    case Size:
        d->defaultSize = value.toSize();
        d->currentSize = value.toSize();
        break;
    case ScaledSize:
        d->currentSize = value.toSize();
        break;
    default:
        break;
    }
}

bool PictureIOHandler::supportsOption(ImageOption option) const
{
    switch(option)
    {
    case Size:
    case ScaledSize:
        return true;
    default:
        break;
    }
    return false;
}

bool PictureIOHandler::canRead(QIODevice *device)
{
    QByteArray contents = device->peek(8);
    return contents.contains("QPIC");
}
