/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsvgiohandler.h"
#include "qsvgrenderer.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qvariant.h"
#include "qdebug.h"

class QSvgIOHandlerPrivate
{
public:
    QSvgIOHandlerPrivate()
        : r(new QSvgRenderer())
    {}
    ~QSvgIOHandlerPrivate()
    {
        delete r;
    }

    QSvgRenderer *r;
    QSize         defaultSize;
    QSize         currentSize;
};

QSvgIOHandler::QSvgIOHandler()
    : d(new QSvgIOHandlerPrivate())
{

}


QSvgIOHandler::~QSvgIOHandler()
{
    delete d;
}


bool QSvgIOHandler::canRead() const
{
    QByteArray contents = device()->peek(80);

    return contents.contains("<svg");
}


QByteArray QSvgIOHandler::name() const
{
    return "svg";
}


bool QSvgIOHandler::read(QImage *image)
{

    d->r->load(device()->readAll());
    d->defaultSize = QSize(d->r->viewBox().width(), d->r->viewBox().height());
    if (d->currentSize.isEmpty())
        d->currentSize = d->defaultSize;
    if (!d->r->isValid())
        return false;
    *image = QImage(d->currentSize, QImage::Format_ARGB32_Premultiplied);
    image->fill(0x00000000);
    QPainter p(image);
    d->r->render(&p);
    p.end();
    return true;
}


QVariant QSvgIOHandler::option(ImageOption option) const
{
    switch(option) {
    case Size:
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


void QSvgIOHandler::setOption(ImageOption option, const QVariant & value)
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


bool QSvgIOHandler::supportsOption(ImageOption option) const
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

bool QSvgIOHandler::canRead(QIODevice *device)
{
    QByteArray contents = device->peek(80);
    return contents.contains("<svg");
}
