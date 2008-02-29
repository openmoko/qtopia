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

#ifndef QIMAGEREADER_H
#define QIMAGEREADER_H

#include <QtCore/qbytearray.h>
#include <QtGui/qimageiohandler.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QColor;
class QIODevice;
class QImage;
class QRect;
class QSize;
class QStringList;

class QImageReaderPrivate;
class Q_GUI_EXPORT QImageReader
{
public:
    enum ImageReaderError {
        UnknownError,
        FileNotFoundError,
        DeviceError,
        UnsupportedFormatError,
        InvalidDataError
    };

    QImageReader();
    explicit QImageReader(QIODevice *device, const QByteArray &format = QByteArray());
    explicit QImageReader(const QString &fileName, const QByteArray &format = QByteArray());
    ~QImageReader();

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    QSize size() const;

    QStringList textKeys() const;
    QString text(const QString &key) const;

    void setClipRect(const QRect &rect);
    QRect clipRect() const;

    void setScaledSize(const QSize &size);
    QSize scaledSize() const;

    void setQuality(int quality);
    int quality() const;

    void setScaledClipRect(const QRect &rect);
    QRect scaledClipRect() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    bool supportsAnimation() const;

    bool canRead() const;
    QImage read();
    bool read(QImage *image);

    bool jumpToNextImage();
    bool jumpToImage(int imageNumber);
    int loopCount() const;
    int imageCount() const;
    int nextImageDelay() const;
    int currentImageNumber() const;
    QRect currentImageRect() const;

    ImageReaderError error() const;
    QString errorString() const;

    bool supportsOption(QImageIOHandler::ImageOption option) const;

    static QByteArray imageFormat(const QString &fileName);
    static QByteArray imageFormat(QIODevice *device);
    static QList<QByteArray> supportedImageFormats();

private:
    Q_DISABLE_COPY(QImageReader)
    QImageReaderPrivate *d;
};

QT_END_HEADER

#endif // QIMAGEREADER_H
