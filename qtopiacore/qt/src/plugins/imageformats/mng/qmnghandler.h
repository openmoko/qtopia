/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMNGHANDLER_H
#define QMNGHANDLER_H

#include <QtGui/qimageiohandler.h>

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QMngHandlerPrivate;

class QMngHandler : public QImageIOHandler
{
    public:
    QMngHandler();
    ~QMngHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage *image);
    virtual bool write(const QImage &image);
    virtual int currentImageNumber() const;
    virtual int imageCount() const;
    virtual bool jumpToImage(int imageNumber);
    virtual bool jumpToNextImage();
    virtual int loopCount() const;
    virtual int nextImageDelay() const;
    static bool canRead(QIODevice *device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant & value);
    virtual bool supportsOption(ImageOption option) const;

    private:
    Q_DECLARE_PRIVATE(QMngHandler)
    QMngHandlerPrivate *d_ptr;
};

#endif // QMNGHANDLER_H
