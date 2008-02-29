/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSVGIOHANDLER_H
#define QSVGIOHANDLER_H

#include <QtGui/qimageiohandler.h>

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QSvgIOHandlerPrivate;

class QSvgIOHandler : public QImageIOHandler
{
public:
    QSvgIOHandler();
    ~QSvgIOHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage *image);
    static bool canRead(QIODevice *device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant & value);
    virtual bool supportsOption(ImageOption option) const;

private:
    QSvgIOHandlerPrivate *d;
};

#endif // QSVGIOHANDLER_H
