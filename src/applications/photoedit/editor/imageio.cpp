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

#include "imageio.h"

#include <cmath>

#include <qfile.h>
#include <qfileinfo.h>

#include <QImageWriter>

ImageIO::ImageIO( QObject* parent )
    : QObject( parent ), image_levels( 0 ), image_samples( 0 )
{ }

ImageIO::~ImageIO()
{
    // Delete loaded image samples
    delete[] image_samples;
}

ImageIO::Status ImageIO::load( const QContent& lnk, int levels )
{
    static const int maxSize = 2097152;
    static const int maxArea = 1920000;
    _lnk = lnk;

    QImageReader reader( lnk.fileName() );

    QImage image;

    bool scaled = false;

    if( reader.supportsOption( QImageIOHandler::Size ) )
    {
        QSize size = reader.size();

        int area = size.width() * size.height();

        if( area > maxArea )
        {
            if( reader.supportsOption( QImageIOHandler::ScaledSize ) )
            {
                size *= sqrt( qreal( maxArea ) / qreal( area ) );

                reader.setScaledSize( size );

                scaled = true;
            }
            else
            {
                _status = SIZE_ERROR;

                return _status;
            }
        }
    }
    else if( QFileInfo( lnk.file() ).size() > maxSize )
    {
        _status = SIZE_ERROR;

        return _status;
    }

    QByteArray format = reader.format();

    if( reader.read( &image ) )
    {
        _status = load( image, levels );

        _format = format;

        if( scaled && _status == NORMAL )
            _status = REDUCED_SIZE;
    }
    else
        _status = LOAD_ERROR;

    return _status;
}

ImageIO::Status ImageIO::load( const QImage& image, int levels )
{
#define SUPPORTED_DEPTH QImage::Format_ARGB32
    _format = QByteArray();

    // Remove previously loaded image samples
    delete[] image_samples;

    // Create image levels
    image_samples = new QImage[ image_levels = levels ];

    if( image.isNull() ) {
        // Notify of change to image
        emit changed();

        _status = LOAD_ERROR;

        return LOAD_ERROR;
    }

    // Load the original image
    switch( image.format() )
    {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        image_samples[ 0 ] = image;
        break;
    default:
        {
            // Try to convert image to supported depth
            image_samples[ 0 ] = image.convertToFormat( QImage::Format_ARGB32 );
            if( image_samples[ 0 ].isNull() ) {
                // Notify of change to image
                emit changed();

                _status = DEPTH_ERROR;

                return DEPTH_ERROR;
            }
        }
    }

    // For each level after the first
    for( int i = 1; i < levels; ++i ) {
        // Load image half the size of the previous image
        QImage *prev_sample = image_samples + i - 1;
        int width = prev_sample->width() / 2;
        int height = prev_sample->height() / 2;
        image_samples[ i ] = prev_sample->scaled( width ? width : 1, height ? height : 1 );
    }

    // Notify of change to image
    emit changed();

    _status = NORMAL;

    return NORMAL;
}

bool ImageIO::isSaveSupported() const
{
    return QImageWriter::supportedImageFormats().contains( _format );
}

bool ImageIO::isReadOnly() const
{
    QFileInfo origFile( _lnk.fileName() );
    if ( !origFile.exists() || !origFile.isWritable() || _status != NORMAL )
        return true;

    return false;
}

bool ImageIO::save( const QImage& image, bool overwrite )
{
#define DEFAULT_FORMAT "PNG"
#define DEFAULT_MIME_TYPE "image/png"

    // If saving supported, save using original format
    // Otherwise, save using default format
    QString filename( _lnk.fileName() );
    if( isSaveSupported() ) {
        // If overwriting, save image to current file
        // Otherwise, save image as new file
        if( overwrite ) {
            if( image.save( filename, _format.constData() ) ) {
                // Generate link changed signal to notify of change to image
                _lnk.commit();

                return true;
            }
        } else {
            // Generate unique file name
            QContent lnk;
            // Perserve name and category
            lnk.setName( _lnk.name() );
            lnk.setType( _lnk.type() );
            lnk.setMedia( _lnk.media() );
            lnk.setCategories( _lnk.categories() );
            // Save image to disk
            QIODevice *device = lnk.open( QIODevice::WriteOnly );
            if( device && image.save( device, _format.constData() ) ) {
                device->close();
                delete device;
                lnk.commit();

                return true;
            }
            else if( device )
            {
                delete device;

                lnk.removeFiles();
            }
        }
    } else {
        // Generate unique file name
            QContent lnk;
            // Perserve name and category
            lnk.setName( _lnk.name() );
            lnk.setType( DEFAULT_MIME_TYPE );
            lnk.setMedia( _lnk.media() );
            lnk.setCategories( _lnk.categories() );
            // Save image to disk
            QIODevice *device = lnk.open( QIODevice::WriteOnly );
            if( device && image.save( device, DEFAULT_FORMAT ) ) {
                device->close();
                delete device;
                lnk.commit();

                return true;
            }
            else if( device )
            {
                delete device;

                lnk.removeFiles();
            }
    }

    return false;
}

int ImageIO::level( double x ) const
{
    // Determine level using inverse reduction function log2 1/x
    int n = (int)( log( 1.0 / x ) / log( 2.0 ) );
    // Limit level to within range
    if( n < 0 ) n = 0;
    else if( n >= image_levels ) n = image_levels - 1;

    return n;
}

double ImageIO::factor( int n ) const
{
    // Determine factor using reduction function 1/2^n
    return 1.0 / pow( 2, n );
}

// Return image at level
QImage ImageIO::image(int level)
{
    return image_levels < 1 ? QImage() : image_samples[ level ];
}

QImage ImageIO::image( const QRect& rect, int level ) const
{
    if ( image_levels < 1 ) {
        return QImage();
    }

    // Reduce desired area using reduction function
    QRect area( rect );
    double reduction_factor = factor( level );
    area.setTop( (int)( area.top() * reduction_factor ) );
    area.setLeft( (int)( area.left() * reduction_factor ) );
    area.setBottom( (int)( area.bottom() * reduction_factor ) );
    area.setRight( (int)( area.right() * reduction_factor ) );

    QRect sample( image_samples[ level ].rect() );
    // If area is wider than image, reduce area to fit width
    if( area.left() < sample.left() ) area.setLeft( sample.left() );
    if( area.right() > sample.right() ) area.setRight( sample.right() );

    // If area is taller than image, reduce area to fit height
    if( area.top() < sample.top() ) area.setTop( sample.top() );
    if( area.bottom() > sample.bottom() ) area.setBottom( sample.bottom() );

    return image_samples[ level ].copy( area );
}
