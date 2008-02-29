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

#include "imageui.h"

#include "imageprocessor.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qcolor.h>

ImageUI::ImageUI( ImageProcessor* ip, QWidget* parent, const char* name,
    WFlags f )
    : QWidget( parent, name, f | Qt::WResizeNoErase | Qt::WRepaintNoErase ),
    image_processor( ip ), enabled( true )
{
    // When image changed in image processor update view
    connect( image_processor, SIGNAL( changed() ), 
        this, SLOT( updateViewport() ) );
}

QRect ImageUI::viewport( const QRect& rect ) const
{
    QRect area( rect );
    // Displace area by position of viewport
    area.moveBy( _viewport.x(), _viewport.y() );    
    return area;
}

void ImageUI::reset()
{
    _viewport.moveCenter( _space.center() );
    viewport_center = image_processor->unmap( _viewport.center() );
    emit changed();
    update();
}

QRegion ImageUI::region() const
{
    QRect region( preview.rect() );
    region.moveBy( preview_position.x(), preview_position.y() );
    return region;
}

void ImageUI::moveViewportBy( int dx, int dy )
{
    _viewport.moveBy( dx, dy );
    viewport_center = image_processor->unmap( _viewport.center() );
    
    // Reload preview from image processor
    update();
}

void ImageUI::updateViewport()
{
    // Update space dimensions
    _space = QRect( QPoint( 0, 0 ), image_processor->size() );
    
    // Initialize viewport
    _viewport.moveCenter( image_processor->map( viewport_center ) );
    if( _viewport.width() > _space.width() )
        _viewport.moveCenter( 
            QPoint( _space.center().x(), _viewport.center().y() ) );
    if( _viewport.height() > _space.height() )
        _viewport.moveCenter( 
            QPoint( _viewport.center().x(), _space.center().y() ) );
    containViewport();
    
    // Emit changed signal
    emit changed();

    // Reload preview from image processor
    update();
}

void ImageUI::paintEvent( QPaintEvent* )
{
#define BACKGROUND_COLOR Qt::lightGray
#define PATTERN QBrush::BDiagPattern
#define PATTERN_COLOR QColor( Qt::lightGray ).light( 110 )
 
    QPainter painter( &widget_buffer );
    
    // Draw background onto widget buffer
    painter.fillRect( widget_buffer.rect(), BACKGROUND_COLOR );
    painter.fillRect( widget_buffer.rect(), 
        QBrush( PATTERN_COLOR, PATTERN ) );
        
    if( enabled ) {
        preview = image_processor->preview( _viewport );
        
        // Update image position
        preview_position.setX( ( width() - preview.width() ) / 2 );
        preview_position.setY( ( height() - preview.height() ) / 2 );

        // Draw preview on widget buffer
        painter.drawPixmap( preview_position, preview );
    }
         
    emit updated();
}

void ImageUI::resizeEvent( QResizeEvent* )
{
    // Resize widget buffer
    widget_buffer.resize( width(), height() );

    // Resize viewport while maintaining center and contain viewport
    QPoint viewport_center( _viewport.center() );
    _viewport.setWidth( width() );
    _viewport.setHeight( height() );
    _viewport.moveCenter( viewport_center );
    containViewport();
    
    emit changed();
    
    // Reload preview from image processor
    update();
}

void ImageUI::containViewport()
{
    // If viewport is thinner than space, contain horizontally
    if( _viewport.width() <= _space.width() ) {
        // If viewport beyond the left bound, move within the left
        if( _viewport.left() < _space.left() )
            _viewport.moveBy( _space.left() - _viewport.left(), 0 );
        // Otherwise, if viewport beyond the right bound, move within the right
        else if( _viewport.right() > _space.right() )
            _viewport.moveBy( _space.right() - _viewport.right(), 0 );
    }
    
    // If viewport is shorter than space, contain vertically
    if( _viewport.height() <= _space.height() ) {
        // If viewport beyond the upper bound, move within the upper
        if( _viewport.top() < _space.top() )
            _viewport.moveBy( 0, _space.top() - _viewport.top() );
        // Otherwise, if viewport beyond the lower bound, move within the lower
        else if( _viewport.bottom() > _space.bottom() )
            _viewport.moveBy( 0, _space.bottom() - _viewport.bottom() );
    }
}
