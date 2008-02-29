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

#include "imageio.h"

#include <cmath>

#include <qfile.h>
#include <qfileinfo.h>

#include <QList>
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
    _lnk = lnk;

    return load( QImage( lnk.file() ), levels );
}

ImageIO::Status ImageIO::load( const QImage& image, int levels )
{
#define SUPPORTED_DEPTH QImage::Format_ARGB32

    // Remove previously loaded image samples
    delete[] image_samples;

    // Create image levels
    image_samples = new QImage[ image_levels = levels ];

    if( image.isNull() ) {
        // Notify of change to image
        emit changed();
        return LOAD_ERROR;
    }

    // Load the original image
    if( image.depth() == SUPPORTED_DEPTH )
        image_samples[ 0 ] = image;
    else {
        // Try to convert image to supported depth
        image_samples[ 0 ] = image.convertToFormat( SUPPORTED_DEPTH );
        if( image_samples[ 0 ].isNull() ) {
            // Notify of change to image
            emit changed();
            return DEPTH_ERROR;
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

    return NORMAL;
}

bool ImageIO::isSaveSupported() const
{
    return QImageWriter::supportedImageFormats().contains( format() );
}

bool ImageIO::isReadOnly() const
{
    QFileInfo origFile( _lnk.file() );
    if ( origFile.exists() && !origFile.isWritable() )
        return true;

    return false;
}

bool ImageIO::save( const QImage& image, bool overwrite )
{
#define DEFAULT_FORMAT "PNG"
#define DEFAULT_EXTENSION ".png"

    // If saving supported, save using original format
    // Otherwise, save using default format
    QString filename( _lnk.file() );
    if( isSaveSupported() ) {
        // If overwriting, save image to current file
        // Otherwise, save image as new file
        if( overwrite ) {
            if( image.save( filename, QImageReader::imageFormat( filename ).data() ) ) {
                // Generate link changed signal to notify of change to image
                _lnk.commit();

                return true;
            }
        } else {
            // Generate unique file name
            QFileInfo file( filename );
            QString extension( "." + file.suffix() );
            QString new_file( filename.left(
                filename.length() - extension.length() ) );
            do new_file += "_new";
            while( QFile::exists( new_file + extension ) );
            // Create new doc link
            QContent lnk;
            // Perserve name and category
            lnk.setName( _lnk.name() );
            lnk.setCategories( _lnk.categories() );
            // Save image to disk
            new_file += extension;
            if( image.save( new_file, QImageReader::imageFormat( filename ).data() ) ) {
                lnk.setFile( new_file );
                lnk.commit();
                return true;
            }
        }
    } else {
        // Generate unique file name
        QFileInfo file( filename );
        QString new_file( filename.left(
            filename.length() - file.suffix().length() - 1 ) );
        do new_file += "_new";
        while( QFile::exists( new_file + DEFAULT_EXTENSION ) );
        // Create new doc link
        QContent lnk;
        // Preserve name and category
        lnk.setName( _lnk.name() );
        lnk.setCategories( _lnk.categories() );
        // Save image to disk
        new_file += DEFAULT_EXTENSION;
        if( image.save( new_file, DEFAULT_FORMAT ) ) {
            lnk.setFile( new_file );
            lnk.commit();
            return true;
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
