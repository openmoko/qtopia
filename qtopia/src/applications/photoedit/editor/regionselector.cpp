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

#include "regionselector.h"

#include "imageui.h"

#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/contextbar.h>

#include <qaction.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

RegionSelector::RegionSelector( ImageUI* iui, const char* name, WFlags f )
    : QWidget( iui, name, f | Qt::WResizeNoErase | Qt::WRepaintNoErase ), 
    image_ui( iui ), enabled( false )
{
#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() ) {
#endif
        current_state = MARK;
#ifdef QTOPIA_PHONE
    } else current_state = MOVE;
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
    // If selection enabled, add labels to context bar
    // Otherwise, remove labels from context bar
    if( enabled ) {
        if( Global::mousePreferred() ) {
            // Disable context menu
            ContextBar::setLabel( this, ContextMenu::key(), ContextBar::NoLabel );
        } else {
            setStateLabel();
            ContextBar::setLabel( this, Qt::Key_Select, ContextBar::Select );
        }
        ContextBar::setLabel( this, Qt::Key_Back, ContextBar::Cancel );
    } else {
        ContextBar::clearLabel( this, ContextMenu::key() );
        if( !Global::mousePreferred() ) ContextBar::clearLabel( this, Qt::Key_Select );
        ContextBar::clearLabel( this, Qt::Key_Back );
    }
#endif
}

void RegionSelector::reset()
{
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100

    // Reset region
#ifdef QTOPIA_PHONE
    if( Global::mousePreferred() ) {
#endif
        region_start = QPoint();
        _region = QRect();
        current_state = MARK;
#ifdef QTOPIA_PHONE
    } else {
        // Set default region
        _region = QRect( 0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT );
        _region.moveCenter( rect().center() );
        // Set current state to move
        current_state = MOVE;
        // If enabled toggle state label
        if( enabled ) setStateLabel();
    }
#endif
}

void RegionSelector::paintEvent( QPaintEvent* )
{
#ifdef QTOPIA_PHONE
static const QPixmap top_left( Resource::loadPixmap("top_left") );
static const QPixmap top_right( Resource::loadPixmap("top_right" ) );
static const QPixmap bottom_left( Resource::loadPixmap("bottom_left") );
static const QPixmap bottom_right( Resource::loadPixmap("bottom_right") );
static const QPixmap crosshair( Resource::loadPixmap("crosshair") );

#define CROSSHAIR_CENTER 7
#define CORNER_WIDTH 7
#define CORNER_HEIGHT 7
#define INSET 1
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
        
        // Draw box around current selection region
        painter.drawRect( _region.normalize() );
        
#ifdef QTOPIA_PHONE
        if( !Global::mousePreferred() ) {
            QPoint center( _region.center() );
            switch( current_state ) {
            // If current state is move, draw crosshair in center of region
            case MOVE:
                painter.drawPixmap( center.x() - CROSSHAIR_CENTER, center.y() - CROSSHAIR_CENTER, crosshair );
                break;
            // If current state is size, draw corners around inner edge of region
            case SIZE:
                painter.drawPixmap( _region.left() + INSET, _region.top() + INSET, top_left );
                painter.drawPixmap( _region.right() - CORNER_WIDTH, _region.top() + INSET, top_right );
                painter.drawPixmap( _region.left() + INSET, _region.bottom() - CORNER_HEIGHT, bottom_left );
                painter.drawPixmap( _region.right() - CORNER_WIDTH, _region.bottom() - CORNER_HEIGHT, bottom_right );
                break;
            default:
                // Ignore
                break;
            }
        }
#endif
        painter.setClipRegion( image_ui->region().subtract( _region.normalize() ) );
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
#define STEP 4

    if( enabled && !Global::mousePreferred() ) {
        if( e->key() == ContextMenu::key() ) {
            // Toggle current state
            switch( current_state ) {
            case MOVE:
                current_state = SIZE;
                setStateLabel();
                update();
                break;
            case SIZE:
                current_state = MOVE;
                setStateLabel();
                update();
                break;
            default:
                // Ignore
                break;
            }
        } else if( e->key() == Key_Select ) {
            emit selected();
        } else {
            switch( current_state ) {
            // Move region
            case MOVE:
                switch( e->key() ) {
                case Key_Left:
                    moveBy( -STEP, 0 );
                    update();
                    break;
                case Key_Right:
                    moveBy( STEP, 0 );
                    update();
                    break;
                case Key_Up:
                    moveBy( 0, -STEP );
                    update();
                    break;
                case Key_Down:
                    moveBy( 0, STEP );
                    update();
                    break;
                default:
                    // Ignore
                    e->ignore();
                    break;
                }
                break;
            // Size region
            case SIZE:
                switch( e->key() ) {
                case Key_Left:
                    sizeBy( -STEP, 0 );
                    update();
                    break;
                case Key_Right:
                    sizeBy( STEP, 0 );
                    update();
                    break;
                case Key_Up:
                    sizeBy( 0, STEP );
                    update();
                    break;
                case Key_Down:
                    sizeBy( 0, -STEP );
                    update();
                    break;
                default:
                    // Ignore
                    e->ignore();
                    break;
                }
                break;
            default:
                // Ignore
                break;
            }
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
}

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
        default:
            // Ignore
            break;
        }
    }
}

void RegionSelector::mouseMoveEvent( QMouseEvent* e )
{
    if( enabled ) {
        switch( current_state ) {
        case MARK:
            // If stylus has moved outside lag area, change to moving
            if( !lag_area.contains( e->pos() ) ) current_state = MOVING;
            break;
        case MOVING:
            {
            // Update region
            int x = e->pos().x(), y = e->pos().y();
    
            // Contain region within widget
            if( x < rect().left() ) x = rect().left();
            if( x > rect().right() ) x = rect().right();
            if( y < rect().top() ) y = rect().top();
            if( y > rect().bottom() ) y = rect().bottom();
    
            // Update region end with current stylus position
            _region = QRect( region_start, QPoint( x, y ) );
            }
            // Update display
            update();
            break;
        default:
            // Ignore
            break;
        }
    }
}

#ifdef QTOPIA_PHONE
void RegionSelector::setStateLabel()
{
    switch( current_state ) {
    case MOVE:
        ContextBar::setLabel( this, ContextMenu::key(), "photoedit/resize", QString::null );
        break;
    case SIZE:
        ContextBar::setLabel( this, ContextMenu::key(), "photoedit/move", QString::null );
        break;
    default:
        // Ignore
        break;
    }
}

void RegionSelector::moveBy( int dx, int dy )
{
    // Contain region within widget
    if( _region.left() + dx < rect().left() )
        dx = rect().left() - _region.left();
    if( _region.right() + dx > rect().right() )
        dx = rect().right() - _region.right();
    if( _region.top() + dy < rect().top() )
        dy = rect().top() - _region.top();
    if( _region.bottom() + dy > rect().bottom() )
        dy = rect().bottom() - _region.bottom();
        
    _region.moveBy( dx, dy );
}

void RegionSelector::sizeBy( int dw, int dh )
{
#define MIN_WIDTH 20
#define MIN_HEIGHT 20

    _region.setLeft( _region.left() - dw );
    _region.setRight( _region.right() + dw );
    _region.setTop( _region.top() - dh );
    _region.setBottom( _region.bottom() + dh );
    
    // Limit to minimum
    if( _region.width() < MIN_WIDTH ) {
        _region.setLeft( _region.left() + dw );
        _region.setRight( _region.right() - dw );
    }
    if( _region.height() < MIN_HEIGHT ) {
        _region.setTop( _region.top() + dh );
        _region.setBottom( _region.bottom() - dh );
    }
     
    // Contain region within widget
    if( _region.left() < rect().left() )
        _region.setLeft( rect().left() );
    if( _region.right() > rect().right() )
        _region.setRight( rect().right() );
    if( _region.top() < rect().top() )
        _region.setTop( rect().top() );
    if( _region.bottom() > rect().bottom() )
        _region.setBottom( rect().bottom() );
}
#endif

