/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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

#include "regionselector.h"

#include "imageui.h"

#include <qtopia/resource.h>
#include <qtopia/contextbar.h>

#include <qaction.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

RegionSelector::RegionSelector( ImageUI* iui, const char* name, WFlags f )
    : QWidget( iui, name, f | Qt::WResizeNoErase | Qt::WRepaintNoErase ), 
    image_ui( iui ), enabled( false ), current_state( MARK )
{
#ifdef QTOPIA_PHONE
    // Construct context menu
    context_menu = new ContextMenu( this );
    
    // Construct cancel item
    context_menu->insertItem( Resource::loadIconSet( "reset" ),
        tr( "Reset" ), this, SLOT( reset() ) );
    
    // Diable context menu
    context_menu->removeFrom( this );
#endif

    // Update display when image ui updated
    connect( image_ui, SIGNAL( updated() ), this, SLOT( update() ) );
}

QRect RegionSelector::region() const
{
    if( !_region.isNull() && 
        _region.left() != _region.right() &&
        _region.top() != _region.bottom() &&
        !image_ui->region().intersect( _region ).isEmpty() )
        return _region.normalize();
    return QRect();
}

void RegionSelector::setEnabled( bool b )
{
    enabled = b;
    
#ifdef QTOPIA_PHONE
    // If selection enabled, enable context menu and context bar
    // Otherwise, disable context menu and context bar
    if( enabled ) {
        context_menu->addTo( this );
        ContextBar::setLabel( this, Qt::Key_Select, ContextBar::Select );
        ContextBar::setLabel( this, Qt::Key_Back, ContextBar::Cancel );
    } else {
        context_menu->removeFrom( this );
        ContextBar::clearLabel( this, Qt::Key_Select );
        ContextBar::clearLabel( this, Qt::Key_Back );
    }
#endif
}

void RegionSelector::reset()
{
    // Reset region
    region_start = QPoint();
    _region = QRect();
#ifdef QTOPIA_PHONE
    // Move crosshair position to center of widget
    crosshair_position = QPoint( width() / 2, height() / 2 );
#endif
    current_state = MARK;
}

void RegionSelector::paintEvent( QPaintEvent* )
{
#ifdef QTOPIA_PHONE
static const char *mark_crosshair_xpm[] = {
    "15 15 2 1",
    "x c #ffffff",
    ". c None",
    "...xxxxxxxxx...",
    "..xxxxxxxxxxx..",
    ".xxxxxxxxxxxxx.",
    "xxx.........xxx",
    "xxx.........xxx",
    "xxx.........xxx",
    "xxx....x....xxx",
    "xxx...xxx...xxx",
    "xxx....x....xxx",
    "xxx.........xxx",
    "xxx.........xxx",
    "xxx.........xxx",
    ".xxxxxxxxxxxxx.",
    "..xxxxxxxxxxx..",
    "...xxxxxxxxx...",
};

static const char *moving_crosshair_xpm[] = {
    "15 15 2 1",
    "x c #ffffff",
    ". c None",
    "...xxx...xxx...",
    "...xxx...xxx...",
    "...xxx...xxx...",
    "xxxxxx...xxxxxx",
    "xxxxxx...xxxxxx",
    "xxxxxx...xxxxxx",
    "...............",
    "...............",
    "...............",
    "xxxxxx...xxxxxx",
    "xxxxxx...xxxxxx",
    "xxxxxx...xxxxxx",
    "...xxx...xxx...",
    "...xxx...xxx...",
    "...xxx...xxx...",
};
static const QPixmap mark_crosshair( mark_crosshair_xpm );
static const QPixmap moving_crosshair( moving_crosshair_xpm );
#define CROSSHAIR_CENTER 8
#endif

#define PAINTER_COLOR Qt::white
#define SPACE_FILL_PATTERN QBrush::Dense6Pattern

    QPainter painter;

    // If selection enabled, draw selected region onto widget
    // Otherwise, draw image ui onto widget
    if( enabled ) {
        QPixmap buffer( image_ui->pixmap() );
        painter.begin( &buffer );
        painter.setPen( PAINTER_COLOR );
        painter.setRasterOp( Qt::XorROP );

#ifdef QTOPIA_PHONE
        QPixmap crosshair;
        
        switch( current_state ) {
        case MARK:
            crosshair = mark_crosshair;
            break;
        case MOVING:
            crosshair = moving_crosshair;
            break;
        }

        // Draw crosshair
        painter.drawPixmap( crosshair_position.x() - CROSSHAIR_CENTER,
            crosshair_position.y() - CROSSHAIR_CENTER, crosshair );

#endif   
       // Draw box around current selection region
       painter.drawRect( _region.normalize() );
        
#ifdef QTOPIA_PHONE
        QRect crosshair_region( crosshair.rect() );
        crosshair_region.moveBy( crosshair_position.x() - CROSSHAIR_CENTER,
            crosshair_position.y() - CROSSHAIR_CENTER );
#endif
        
        painter.setClipRegion( 
            QRegion( image_ui->region().subtract( _region.normalize() )            
#ifdef QTOPIA_PHONE
            .subtract( crosshair_region )
#endif
            ) );
        painter.setClipping( true );
        
        // Gray out area surrounding current selection region
        painter.fillRect( rect(), QBrush( PAINTER_COLOR, SPACE_FILL_PATTERN ) );
        painter.end();
        
        painter.begin( this );
        painter.drawPixmap( 0, 0, buffer );
    } else {
        painter.begin( this );
        painter.drawPixmap( 0, 0, image_ui->pixmap() );
    }
}

#ifdef QTOPIA_PHONE
void RegionSelector::keyPressEvent( QKeyEvent* e )
{
#define MOVE_STEP 5

    // If selection enabled
    if( enabled ) {
        switch( e->key() ) {
        // If left key pressed, move current position one unit left
        case Key_Left:
            moveCrosshairBy( -MOVE_STEP, 0 );
            if( current_state == MOVING ) 
                _region = QRect( region_start, crosshair_position );
            update();
            break;
        // If right key pressed, move current position one unit right
        case Key_Right:
            moveCrosshairBy( MOVE_STEP, 0 );
            if( current_state == MOVING )
                _region = QRect( region_start, crosshair_position );
            update();
            break;
        // If up key pressed, move current position one unit up
        case Key_Up:
            moveCrosshairBy( 0, -MOVE_STEP );
            if( current_state == MOVING )
                _region = QRect( region_start, crosshair_position );
            update();
            break;
        // If down key pressed, move current position one unit down
        case Key_Down:
            moveCrosshairBy( 0, MOVE_STEP );
            if( current_state == MOVING )
                _region = QRect( region_start, crosshair_position );
            update();
            break;
        // If select key pressed
        case Key_Select:
            switch( current_state ) {
            // If region is being marked
            case MARK:
                // Update region start and end position with current position
                region_start = crosshair_position;
                _region = QRect( region_start, region_start );
                current_state = MOVING;
                update();
                break;
            // If region is moving, emit selected
            case MOVING:
                emit selected();
                break;
            }
            break;
        default:
            // Ignore
            e->ignore();
            break;
        }
    } else {
        if( e->key() == Qt::Key_Back ) emit pressed();
        e->ignore();
    }
}
#endif

void RegionSelector::mousePressEvent( QMouseEvent* e )
{
#define LAG 15

#ifdef QTOPIA_PHONE
    // Update crosshair position
    crosshair_position = e->pos();
    if( current_state == MOVING )
        _region = QRect( region_start, crosshair_position );
    
    // Update display
    update();
#else
    switch( e->button() ) {
    // If stylus has been pressed
    case Qt::LeftButton:
        // If selection enabled update press position with stylus position
        // Otherwise, emit pressed
        if( enabled ) {
            region_start = e->pos();
            lag_area = QRect( region_start - QPoint( LAG, LAG ),
                region_start + QPoint( LAG, LAG ) );
        } else emit pressed();
        break;
    default:
        // Ignore
        break;
    }
#endif
}

#ifndef QTOPIA_PHONE
void RegionSelector::mouseReleaseEvent( QMouseEvent* e )
{
    if( enabled ) {
        switch( current_state ) {
        // If region is being marked
        case MARK:
            // If stylus released within the selected region, emit selected
            // Otherwise, emit canceled signal
            if( _region.normalize().contains( e->pos() ) ) emit selected();
            else emit canceled();
            break;
        // If region is moving, change to mark
        case MOVING:
            current_state = MARK;
            break;
        }
    }
}
#endif

#ifndef QTOPIA_PHONE
void RegionSelector::mouseMoveEvent( QMouseEvent* e )
{
    if( enabled ) {
        switch( current_state ) {
        case MARK:
            // If stylus has moved outside lag area, change to moving
            if( !lag_area.contains( e->pos() ) ) current_state = MOVING;
            break;
        case MOVING:
            // Update region
            int x = e->pos().x(), y = e->pos().y();
    
            // Contain region within widget
            if( x < rect().left() ) x = rect().left();
            if( x > rect().right() ) x = rect().right();
            if( y < rect().top() ) y = rect().top();
            if( y > rect().bottom() ) y = rect().bottom();
    
            // Update region end with current stylus position
            _region = QRect( region_start, QPoint( x, y ) );

            // Update display
            update();
            break;
        }
    }
}
#endif

#ifdef QTOPIA_PHONE
void RegionSelector::moveCrosshairBy( int x, int y )
{
    crosshair_position.setX( crosshair_position.x() + x );
    crosshair_position.setY( crosshair_position.y() + y );
    
    if( crosshair_position.x() < 0 ) crosshair_position.setX( 0 );
    else if( crosshair_position.x() > width() ) 
        crosshair_position.setX( width() );
    
    if( crosshair_position.y() < 0 ) crosshair_position.setY( 0 );
    else if( crosshair_position.y() > height() ) 
        crosshair_position.setY( height() );
}
#endif

