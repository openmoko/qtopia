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

#include "navigator.h"

#include "imageui.h"

#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

Navigator::Navigator( ImageUI* iui, QWidget* parent, const char* name, 
    WFlags f )
    : QWidget( parent, name, f | Qt::WResizeNoErase | Qt::WRepaintNoErase ),
    image_ui( iui )
{
    setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum ) );
    setFocusPolicy( QWidget::StrongFocus );
    
    // When image ui has changed update viewport and space
    connect( image_ui, SIGNAL( changed() ), this, SLOT( updateNavigator() ) );
    
    // Update display when image ui updated
    connect( image_ui, SIGNAL( updated() ), this, SLOT( update() ) );
    
    // When navigator moved notify image ui of movement
    connect( this, SIGNAL( viewportMoved( int, int ) ), 
        image_ui, SLOT( moveViewportBy( int, int ) ) );
}

void Navigator::updateNavigator()
{
    // Retrive and set space
    actual_space = QRect( QPoint( 0, 0 ), image_ui->space() );
    // Retrive and set viewport
    actual_viewport = image_ui->viewport(); 
    
    // Update display
    if( !actual_viewport.contains( actual_space ) ) calculateReduced();
    update();
}

void Navigator::keyPressEvent( QKeyEvent* e )
{
#define MOVE_STEP 1

    // Record direction and start viewport movement
    switch( e->key() ) {
    case Qt::Key_Right:
        moveViewportBy( MOVE_STEP, 0 );
        break;
    case Qt::Key_Left:
        moveViewportBy( -MOVE_STEP, 0 );
        break;
    case Qt::Key_Up:
        moveViewportBy( 0, -MOVE_STEP );
        break;
    case Qt::Key_Down:
        moveViewportBy( 0, MOVE_STEP );
        break;
    default:
        // Ignore
        e->ignore();
        break;
    }
}

void Navigator::mousePressEvent( QMouseEvent* e )
{
    // If stylus pressed
    if( e->button() == Qt::LeftButton ) {
        // If pressed within vieport
        if( reduced_viewport.contains( e->pos() ) ) {
            // Record positon and allow viewport to be moved
            mouse_position = e->pos();
            moving_viewport = true;
        }
    }
}

void Navigator::mouseReleaseEvent( QMouseEvent* /*e*/ )
{
    // Disallow viewport to be moved
    moving_viewport = false;
}

void Navigator::mouseMoveEvent( QMouseEvent* e )
{
    // If viewport can be moved and mouse is in space
    if( moving_viewport ) {
        // Calculate displacement and move viewport
        QPoint delta( e->pos() - mouse_position );
        moveViewportBy( delta.x(), delta.y() );
        mouse_position = e->pos();
    }
}

void Navigator::paintEvent( QPaintEvent* )
{
#define PAINTER_COLOR Qt::white
#define SPACE_FILL_PATTERN QBrush::Dense6Pattern

    QPainter painter( this );

    // Paint image ui onto widget
    painter.drawPixmap( QPoint( 0, 0 ), image_ui->pixmap(), geometry() );
        
    if( actual_space.contains( actual_viewport ) &&
        actual_space != actual_viewport ) {
        // Draw navigator onto widget
        painter.setPen( PAINTER_COLOR );
        painter.setRasterOp( Qt::XorROP );
        
        // Draw reduced viewport
        painter.setBrush( QBrush() );
        painter.drawRect( reduced_viewport );
        
        // Draw reduced space border
        painter.setClipRegion( QRegion( rect() ).subtract( reduced_viewport ) );
        painter.setClipping( true );
        painter.setBrush( QBrush( PAINTER_COLOR, SPACE_FILL_PATTERN ) );
        painter.drawRect( reduced_space );
    }
}

void Navigator::resizeEvent( QResizeEvent* )
{
    // Update display
    if( !actual_viewport.contains( actual_space ) ) calculateReduced();
    update();
}

QSize Navigator::sizeHint() const
{
#define PREFERRED_WIDTH 65
#define PREFERRED_HEIGHT 65

    return QSize( PREFERRED_WIDTH, PREFERRED_HEIGHT );
}

void Navigator::calculateReduced()
{
#define BORDER 5
#define REDUCTION_RATIO( dw, dh, sw, sh ) \
    ( (dw)*(sh) > (dh)*(sw) ? (double)(dh)/(double)(sh) : \
    (double)(dw)/(double)(sw) )
    
    // If viewport is wider than space, reduce viewport to fit width
    // Otherwise if viewport is taller than space, reduce viewport to fit height
    if( actual_viewport.width() > actual_space.width() ) {
        actual_viewport.setLeft( actual_space.left() );
        actual_viewport.setRight( actual_space.right() );
    } else if( actual_viewport.height() > actual_space.height() ) {
        actual_viewport.setTop( actual_space.top() );
        actual_viewport.setBottom( actual_space.bottom() );
    }
    
    // Reduce viewport to fit within widget dimensions
    reduction_ratio = REDUCTION_RATIO( width() - BORDER, height() - BORDER, 
        actual_space.width(), actual_space.height() );
    // Reduce and center space 
    QWMatrix reduction_matrix( reduction_ratio, 0, 0, reduction_ratio, 
        0, 0 );
        
    reduced_space = QRect( actual_space.topLeft() * reduction_ratio,
        actual_space.bottomRight() * reduction_ratio );
    
    centered_origin.setX( width() - reduced_space.width() );
    centered_origin.setY( height() - reduced_space.height() );
    reduced_space.moveBy( centered_origin.x(), centered_origin.y() );
    
    // Reduce and center viewport by same amount
    reduced_viewport = QRect( actual_viewport.topLeft() * reduction_ratio,
        actual_viewport.bottomRight() * reduction_ratio );
    reduced_viewport.moveBy( centered_origin.x(), centered_origin.y() );
}

void Navigator::moveViewportBy( int dx, int dy )
{
    // Scale dx dy up by reversing reduction
    dx = (int)( dx / reduction_ratio );
    dy = (int)( dy /reduction_ratio );

    // If viewport is wider than space, don't move horizontally
    // Otherwise, restrict dx to within actual space
    if( actual_viewport.width() >= actual_space.width() ) dx = 0;
    else {
        if( dx + actual_viewport.right() > actual_space.right() )
            dx = actual_space.right() - actual_viewport.right();
        else if( dx + actual_viewport.left() < actual_space.left() )
            dx = actual_space.left() - actual_viewport.left();
    }

    // If viewport is taller than space, don't move vertically 
    // Otherwise, restrict dy to within actual space
    if( actual_viewport.height() >= actual_space.height() ) dy = 0;
    else {
        if( dy + actual_viewport.bottom() > actual_space.bottom() )
            dy = actual_space.bottom() - actual_viewport.bottom();
        else if( dy + actual_viewport.top() < actual_space.top() ) 
            dy = actual_space.top() - actual_viewport.top();
    }
        
    // If viewport needs to be move, move viewports
    if( dx | dy ) {
        actual_viewport.moveBy( dx, dy );
        
        // Calculate reduced viewport
        reduced_viewport = QRect( actual_viewport.topLeft() * reduction_ratio,
            actual_viewport.bottomRight() * reduction_ratio );
        reduced_viewport.moveBy( centered_origin.x(), centered_origin.y() );
    
        // Emit viewport moved signal
        emit viewportMoved( dx, dy );
    
        // Update display
        update();
    }
}
