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

#include "imageprocessor.h"

#include "imageio.h"
#include "math.h"


#define LIMIT( X, Y, Z ) ( (X) > (Y) ? (X) > (Z) ? (Z) : (X) : (Y) )

inline QRect operator*( const QRect& rect, double d )
{
    QRect r( rect );
    r.setTop( (int)floor( r.top() * d ) );
    r.setLeft( (int)floor( r.left() * d ) );
    r.setBottom( (int)floor( r.bottom() * d ) );
    r.setRight( (int)floor( r.right() * d ) );
    return r;
}

inline QRect operator/( const QRect& rect, double d )
{
    QRect r( rect );
    r.setTop( (int)ceil( r.top() / d ) );
    r.setLeft( (int)ceil( r.left() / d ) );
    r.setBottom( (int)ceil( r.bottom() / d ) );
    r.setRight( (int)ceil( r.right() / d ) );
    return r;
}

inline QImage scale( const QImage& image, int width, int height )
{
#define PRECISION 16
   
    QImage buffer( width, height, image.depth() );
    buffer.setAlphaBuffer( image.hasAlphaBuffer() );
    
    if( width && height ) {
        // Calculate mapping factors
        uint factor_x = ( image.width() << PRECISION ) / width;
        uint factor_y = ( image.height() << PRECISION ) / height;
     
        QRgb **src = ( QRgb** )image.jumpTable();
        QRgb **dest = ( QRgb** )buffer.jumpTable();
        // For each pixel in buffer
        uint y = 0;
        for( int j = 0; j < buffer.height(); ++j ) {
            uint yd = y >> PRECISION;
            uint x = 0;
            for( int i = 0; i < buffer.width(); ++i ) {
                // Calculate position in image
                // Copy pixel value from image into buffer
                dest[ j ][ i ] = src[ yd ][ x >> PRECISION ];
                x += factor_x;
            }
            y += factor_y;
        }
    }

   return buffer;
}

ImageProcessor::ImageProcessor( ImageIO* iio, QObject* parent, 
    const char* name )
    : QObject( parent, name ), image_io( iio ), brightness_factor( 0 ),
    zoom_factor( 1.0 )
{ 
    // Clear transformations when image changes in image io
    connect( image_io, SIGNAL( changed() ), this, SLOT( reset() ) );
}

void ImageProcessor::crop( const QRect& rect )
{
    // Crop image to area within rect
    viewport = unmap( rect ).intersect( viewport );
    
    // Notify of change to image
    emit changed();
}

QPoint ImageProcessor::map( const QPoint& point ) const
{
    QPoint p( point );
    // Apply transformations to point
    p = transformation_matrix.map( p );
    // Calculate displacement to make transformed image positive
    QRect space( transformation_matrix.map( viewport ).normalize() );
    // Apply displacement to point
    // Scale point by zoom factor
    p.setX( (int)floor( ( p.x() - space.x() ) * zoom_factor ) );
    p.setY( (int)floor( ( p.y() - space.y() ) * zoom_factor ) );
    return p;
}

QPoint ImageProcessor::unmap( const QPoint& point ) const
{
    // Sacle point by inverse of zoom factor
    QPoint p( (int)ceil( point.x() / zoom_factor ), 
        (int)ceil( point.y() / zoom_factor ) );
    // Calculate displacement to make transformed image positive
    QRect space( transformation_matrix.map( viewport ).normalize() );
    // Reverse displacement of point
    p.setX( p.x() + space.x() );
    p.setY( p.y() + space.y() );
    // Apply the inverse transformations to point
    return transformation_matrix.inverse().map( p );
}

const QPixmap& ImageProcessor::preview( const QRect& rect ) const
{
    // Reverse transformations to rect
    // Limit area to within viewport
    QRect area( unmap( rect ).intersect( viewport ) );
    
    // Retrive sample from image io
    QImage sample = image_io->image( area, image_io->level( zoom_factor ) );

    // Apply transformations to image
    sample = transform( sample, sample.rect() );
    
    // Scale up
    area = map( area );
    return _preview = scale( sample, area.width(), area.height() );
}

QImage ImageProcessor::image() const
{
    return transform( image_io->image(), viewport );
}

QImage ImageProcessor::image( const QSize& target ) const
{
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )
    
    // Determine reduction ratio for tranformed image
    QSize transformed( transformation_matrix.map( viewport ).normalize().
        size() );
    double reduction_ratio = REDUCTION_RATIO( target.width(), target.height(),
        transformed.width(), transformed.height() );
    // Determine image level closest to reduction ratio
    int closest_level = image_io->level( reduction_ratio );
    
    // Reduce viewport by the factor of the closest level
    double level_factor = image_io->factor( closest_level );
    QRect reduced_viewport( viewport.topLeft() * level_factor,
        viewport.bottomRight() * level_factor );
    
    // Retrive image at closest level and apply transformations
    QImage sample( transform( image_io->image( closest_level ),
        reduced_viewport ) );
        
    // Scale image up to size
    double scale_factor = reduction_ratio / level_factor;
    return sample.smoothScale( (int)( sample.width() * scale_factor ), 
        (int)( sample.height() * scale_factor ) );
}

QSize ImageProcessor::size() const
{
    int width, height;
    transformation_matrix.map( viewport.width(), viewport.height(),
        &width, &height );
    return QSize( QABS( (int)( width * zoom_factor ) ), 
        QABS( (int)( height * zoom_factor ) ) );
}

bool ImageProcessor::isChanged() const
{
    return transformation_matrix != Matrix() || brightness_factor ||
        viewport != QRect( QPoint( 0, 0 ), image_io->size() );
}

void ImageProcessor::setZoom( double factor )
{
    // Set zoom factor
    zoom_factor = factor;
    
    // Notify of change to image
    emit changed();
}

void ImageProcessor::setBrightness( double factor )
{
    // Set brightness factor
    brightness_factor = factor;
    
    // Notify of change to image
    emit changed();
}

void ImageProcessor::rotate()
{
#define COS_90 0
#define SIN_90 1

    // Multiply a rotation matrix of 90 deg clockwise to transformation matrix
    transformation_matrix *= Matrix( COS_90, SIN_90, -SIN_90, COS_90 );
    // Notify of change to image
    emit changed();
}

void ImageProcessor::reset()
{
    // Reset transformations and notify of change
    brightness_factor = 0;
    zoom_factor = 1.0;
    transformation_matrix = Matrix();
    viewport = QRect( QPoint( 0, 0 ), image_io->size() );
    emit changed();
}

QRect ImageProcessor::map( const QRect& rect ) const
{
    // Apply transformations to rect
    QRect r( transformation_matrix.map( rect ).normalize() );
    // Calculate displacement to make transformed image positive
    QRect space( transformation_matrix.map( viewport ).normalize() );
    // Apply displacement to rect
    r.moveBy( -space.x(), -space.y() );
    // Scale rect by zoom factor
    return r * zoom_factor;
}

QRect ImageProcessor::unmap( const QRect& rect ) const
{
    // Sacle rect by inverse of zoom factor
    QRect r( rect / zoom_factor );
    // Calculate displacement to make transformed image positive
    QRect space( transformation_matrix.map( viewport ).normalize() );
    // Reverse displacement of rect
    r.moveBy( space.x(), space.y() );
    // Apply the inverse transformations to rect
    return transformation_matrix.inverse().map( r ).normalize();
}

QImage ImageProcessor::transform( const QImage& image, const QRect& area ) const
{
#define RGB_MIN 0
#define RGB_MAX 255

    // Determine size of final image
    // Calculate displacement to make transformed image positive
    QRect space( transformation_matrix.map( area ).normalize() );
    
    // Construct image buffer
    QImage buffer( space.width(), space.height(), image.depth() );
    buffer.setAlphaBuffer( image.hasAlphaBuffer() );
        
    int brightness = (int)( RGB_MAX * brightness_factor );
    // For each pixel in area
    QRgb **src = ( QRgb** )image.jumpTable();
    QRgb **dest = ( QRgb** )buffer.jumpTable();
    int bottom = QMIN( area.bottom() + 1, image.height() );
    int right = QMIN( area.right() + 1, image.width() );
    
    // xd = a*x + c*y + dx
    // yd = b*x + d*y + dy
    int cx = area.left() * transformation_matrix.a() + area.top() * transformation_matrix.c() - space.x();
    int cy = area.left() * transformation_matrix.b() + area.top() * transformation_matrix.d() - space.y();
    for( int j = area.top(); j < bottom; ++j ) {
        int x = cx;
        int y = cy;
        for( int i = area.left(); i < right; ++i ) {
            // Determine location of pixel after transformations applied
            // Apply transfromations and store in image buffer
            QRgb pixel = src[ j ][ i ];
            int r = qRed( pixel ) + brightness;
            int g = qGreen( pixel ) + brightness; 
            int b = qBlue( pixel ) + brightness;
            dest[ y ][ x ] = ( pixel & 0xff000000 ) | 
                ( LIMIT( r, RGB_MIN, RGB_MAX ) << 16 ) | 
                ( LIMIT( g, RGB_MIN, RGB_MAX ) << 8 ) |
                LIMIT( b, RGB_MIN, RGB_MAX );
            x += transformation_matrix.a();
            y += transformation_matrix.b();
        }
        cx += transformation_matrix.c();
        cy += transformation_matrix.d();
    }
    
    return buffer;
}
