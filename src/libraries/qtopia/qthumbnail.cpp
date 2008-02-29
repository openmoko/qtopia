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

/*!
  \class QThumbnail
  \brief The QThumbnail class is used to generate thumbnails of large images.

  Create a QThumbnail with an image file name then use \c pixmap() to generate thumbnails of different sizes.

  For example:

  \code
    QPainter painter( this );

    QThumbnail thumbnail( "image.jpg" );
    painter.drawPixmap( offset, thumbnail.pixmap( size() ) );
  \endcode

  In addition \c actualSize() can be used to determine the actual size of a thumbnail for a requested size and aspect mode.

  For some image formats QThumbnail will efficiently generate thumbnails by scaling the image as it is being loaded. For other formats the whole image will be loaded and then scaled.

  \ingroup multimedia
*/

#include "qthumbnail.h"

#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QImageIOHandler>
#include <QVariant>
#include <qglobal.h>
#include <QDebug>

#if QT_VERSION < 0x040200
// factory loader
# include <qcoreapplication.h>
# include <private/qfactoryloader_p.h>

// image handlers
#ifndef QT_NO_IMAGEFORMAT_BMP
# include <private/qbmphandler_p.h>
#endif
#ifndef QT_NO_IMAGEFORMAT_XBM
# include <private/qxbmhandler_p.h>
#endif
#ifndef QT_NO_IMAGEFORMAT_XPM
# include <private/qxpmhandler_p.h>
#endif
# ifndef QT_NO_IMAGEFORMAT_PNG
#   include <private/qpnghandler_p.h>
# endif

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QImageIOHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/imageformats")))

static QImageIOHandler *createReadHandler(QIODevice *device, const QByteArray &format)
{
    QByteArray form = format.toLower();
    QImageIOHandler *handler = 0;

    // check if we have plugins that support the image format
    QFactoryLoader *l = loader();
    QStringList keys = l->keys();
    const qint64 pos = device->pos();
    for (int i = 0; i < keys.count(); ++i) {
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
        if (plugin && plugin->capabilities(device, form) & QImageIOPlugin::CanRead) {
            handler = plugin->create(device, form);
            break;
        }
        device->seek(pos);
    }

    // check if we have built-in support for the format name
    if (!handler && !format.isEmpty()) {
#ifndef QT_NO_IMAGEFORMAT_PNG
        if (form == QLatin1String("png")) {
            handler = new QPngHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_BMP
        if (form == QLatin1String("bmp")) {
            handler = new QBmpHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_XPM
        if (form == QLatin1String("xpm")) {
            handler = new QXpmHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_XBM
        if (form == QLatin1String("xbm")) {
            handler = new QXbmHandler;
            handler->setOption(QImageIOHandler::SubType, form);
        }
#else
        {
        }
#endif
    }

    // check if any of our built-in formats can read images from the device
    if (!handler) {
        //QByteArray subType;
#ifndef QT_NO_IMAGEFORMAT_PNG
        if (QPngHandler::canRead(device)) {
            handler = new QPngHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_BMP
        if (QBmpHandler::canRead(device)) {
            handler = new QBmpHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_XPM
        if (QXpmHandler::canRead(device)) {
            handler = new QXpmHandler;
        } else
#endif
#ifndef QT_NO_IMAGEFORMAT_XBM
        if (QXbmHandler::canRead(device)) {
            handler = new QXbmHandler;
        }
#else
        {
        }
#endif
    }

    if (!handler)
        return 0;

    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#endif //QT_VERSION < 040200

class QThumbnailPrivate
{
public:
#if QT_VERSION < 0x040200
    QIODevice *file;
    QImageIOHandler *handler;
#else
    QImageReader *reader;
#endif
};

#define JPEG_DECOMPRESSION_QUALITY 10

/*!
  Constructs the object for the given image \a fileName.
*/
QThumbnail::QThumbnail( const QString& fileName )
{
    d = new QThumbnailPrivate;

#if QT_VERSION < 0x040200
    d->file = new QFile( fileName );
    d->file->open( QIODevice::ReadOnly );

    d->handler = createReadHandler( d->file, QByteArray() );
    // Enable fast decompression if supported
    if( d->handler && d->handler->supportsOption( QImageIOHandler::Quality ) ) {
        d->handler->setOption( QImageIOHandler::Quality, JPEG_DECOMPRESSION_QUALITY );
    }
#else
    d->reader = new QImageReader( fileName );

    // Enable fast decompression if supported
    if ( d->reader->supportsOption( QImageIOHandler::Quality ) ) {
        //only jpeg supports this at the moment
        QString format = d->reader->format().toLower();
        if ( format == QLatin1String("jpeg") || format == QLatin1String("jpg") )
            d->reader->setQuality( JPEG_DECOMPRESSION_QUALITY );
    }
#endif //QT_VERSION >= 0x040200
}

/*!
  Constructs the object for the image on the given io \a device.
*/
QThumbnail::QThumbnail( QIODevice *device )
{
    d = new QThumbnailPrivate;

#if QT_VERSION < 0x040200
    d->file = device;
    d->file->open( QIODevice::ReadOnly );

    d->handler = createReadHandler( d->file, QByteArray() );
    // Enable fast decompression if supported
    if( d->handler && d->handler->supportsOption( QImageIOHandler::Quality ) ) {
        d->handler->setOption( QImageIOHandler::Quality, JPEG_DECOMPRESSION_QUALITY );
    }
#else
    d->reader = new QImageReader( device );

    // Enable fast decompression if supported
    if ( d->reader->supportsOption( QImageIOHandler::Quality ) ) {
        //only jpeg supports this at the moment
        QString format = d->reader->format().toLower();
        if ( format == QLatin1String("jpeg") || format == QLatin1String("jpg") )
            d->reader->setQuality( JPEG_DECOMPRESSION_QUALITY );
    }
#endif //QT_VERSION >= 0x040200
}

/*!
  Destroys the thumbnail.
*/
QThumbnail::~QThumbnail()
{
#if QT_VERSION < 0x040200
    delete d->handler;
    delete d->file;
#else
    delete d->reader;
#endif
    delete d;
}

/*!
  Returns the actual size of the thumbnail for the requested \a size and \a
  mode. An invalid size is returned if the image is not able to be read.
  The whole image may be loaded if the image handler does not support the
  QImageIOHandler::Size option.

  If \a size is invalid the function returns the size as defined by
  QImageIOHandler::Size. This can be used to prevent that the image is scaled
  up when calling QThumbnail::pixmap().

  \sa pixmap()
*/
QSize QThumbnail::actualSize( const QSize& size, Qt::AspectRatioMode mode )
{
#if QT_VERSION < 0x040200
    if( !d->handler ) {
        return QSize();
    }

    d->file->reset();
#endif

    if( mode == Qt::IgnoreAspectRatio ) {
        return size;
    }

    QSize actual;

#if QT_VERSION < 0x040200
    if( d->handler->supportsOption( QImageIOHandler::Size ) ) {
        actual = d->handler->option( QImageIOHandler::Size ).toSize();
        if ( size.isValid() )
            actual.scale( size, mode );
    } else {
        // Load image to determine size
        QImage image;
        d->handler->read( &image );
        actual = image.size();
        if ( size.isValid() )
            actual.scale( size, mode );
    }
#else
    if( d->reader->supportsOption( QImageIOHandler::Size ) ) {
        actual = d->reader->size();
        if (size.isValid())
            actual.scale( size, mode );
    } else {
        // Load image to determine size
        QImage image = d->reader->read();
        actual = image.size();
        if (size.isValid())
            actual.scale( size, mode );
    }
#endif

    return actual;
}

/*!
  Returns a pixmap with the requested \a size and \a mode. A null pixmap
  is returned if the image cannot be loaded.

  The pixmap will be generated by scaling the image as it is being loaded
  where possible. This is reliant on the support for the
  QImageIOHandler::ScaledSize option in the image handler. If the option is
  not supported the whole image will be loaded and then scaled.

  \sa actualSize()
*/
QPixmap QThumbnail::pixmap( const QSize& size, Qt::AspectRatioMode mode )
{
#if QT_VERSION < 0x040200
    if( !d->handler ) {
        return QPixmap();
    }

    d->file->reset();
#endif

    // If supported, use handler to scale image
    // Otherwise, load then scale image
    bool notScaled = true;
#if QT_VERSION < 0x040200
    if( d->handler->supportsOption( QImageIOHandler::ScaledSize ) ) {
        if( mode == Qt::IgnoreAspectRatio ) {
            d->handler->setOption( QImageIOHandler::ScaledSize, size );
            notScaled = false;
        } else {
            if( d->handler->supportsOption( QImageIOHandler::Size ) ) {
                QSize actual = d->handler->option( QImageIOHandler::Size ).toSize();
                actual.scale( size, mode );
                d->handler->setOption( QImageIOHandler::ScaledSize, actual );
                notScaled = false;
            }
        }
    }

    QImage thumbnail;
    d->handler->read( &thumbnail );
#else
    if ( d->reader->supportsOption( QImageIOHandler::ScaledSize ) ) {
        if ( mode == Qt::IgnoreAspectRatio ) {
            d->reader->setScaledSize( size );
            notScaled = false;
        } else if (d->reader->supportsOption( QImageIOHandler::Size ) ) {
            QSize actual = d->reader->size();
            actual.scale( size, mode );
            d->reader->setScaledSize( actual );
            notScaled = false;
        }
    }

    QImage thumbnail = d->reader->read();
#endif

    if( notScaled ) {
        thumbnail = thumbnail.scaled( size, mode, Qt::FastTransformation );
    }

    return QPixmap::fromImage( thumbnail );
}
