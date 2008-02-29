/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "imageio.h"

#include <cstring>

#include <qfile.h>
#include <qfileinfo.h>
#include <qstrlist.h>

#include <math.h>

ImageIO::ImageIO( QObject* parent, const char* name )
    : QObject( parent, name ), image_levels( 0 ), image_samples( 0 )
{ }

ImageIO::~ImageIO()
{
    // Delete loaded image samples
    delete[] image_samples;
}

ImageIO::Status ImageIO::load( const DocLnk& lnk, int levels )
{
    _lnk = lnk;
    
    return load( QImage( lnk.file() ), levels );
}

ImageIO::Status ImageIO::load( const QImage& image, int levels )
{
#define SUPPORTED_DEPTH 32

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
        image_samples[ 0 ] = image.convertDepth( SUPPORTED_DEPTH );
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
        image_samples[ i ] = prev_sample->smoothScale( width ? width : 1, height ? height : 1 );
    }
        
    // Notify of change to image
    emit changed();
   
    return NORMAL;
}

bool ImageIO::isSaveSupported() const
{
    const char *current = format();
    QStrList supported( QImageIO::outputFormats() );
    const char *type;
    for( type = supported.first(); type && strcmp( type, current ) != 0; 
        type = supported.next() );
    return type;
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
            if( image.save( filename, QImageIO::imageFormat( filename ) ) ) {
                // Generate link changed signal to notify of change to image
                _lnk.writeLink();
                return true;
            }
        } else {
            // Generate unique file name
            QFileInfo file( filename );
            QString extension( "." + file.extension( false ) );
            QString new_file( filename.left( 
                filename.length() - extension.length() ) );
            do new_file += "_new"; 
            while( QFile::exists( new_file + extension ) );
            // Create new doc link
            DocLnk lnk;
            // Perserve name and category
            lnk.setName( _lnk.name() );
            lnk.setCategories( _lnk.categories() );
            // Save image to disk
            new_file += extension;
            if( image.save( new_file, QImageIO::imageFormat( filename ) ) ) {
                lnk.setFile( new_file );
                lnk.writeLink();
                return true;
            }
        }
    } else {
        // Generate unique file name
        QFileInfo file( filename );
        QString new_file( filename.left( 
            filename.length() - file.extension( false ).length() - 1 ) );
        do new_file += "_new";
        while( QFile::exists( new_file + DEFAULT_EXTENSION ) );
        // Create new doc link
        DocLnk lnk;
        // Preserve name and category
        lnk.setName( _lnk.name() );
        lnk.setCategories( _lnk.categories() );
        // Save image to disk
        new_file += DEFAULT_EXTENSION;
        if( image.save( new_file, DEFAULT_FORMAT ) ) {
            lnk.setFile( new_file );
            lnk.writeLink();
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

QImage ImageIO::image( const QRect& rect, int level ) const
{
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
