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

#include "regionselector.h"

#include "imageui.h"


#include <qsoftmenubar.h>
#include <qtopianamespace.h>

#include <qaction.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qregion.h>

#include <QMouseEvent>
#include <QKeyEvent>

RegionSelector::RegionSelector( ImageUI* iui, Qt::WFlags f )
    : QWidget( iui, f ), image_ui( iui ), enabled( false )
{
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() ) {
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
        return _region.normalized();
    return QRect();
}

void RegionSelector::setEnabled( bool b )
{
    enabled = b;

#ifdef QTOPIA_PHONE
    // If selection enabled, add labels to context bar
    // Otherwise, remove labels from context bar
    if( enabled ) {
        if( Qtopia::mousePreferred() ) {
            // Disable context menu
            QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), QSoftMenuBar::NoLabel );
        } else {
            setStateLabel();
            QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::Select );
        }
        QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Cancel );
    } else {
        QSoftMenuBar::clearLabel( this, QSoftMenuBar::menuKey() );
        if( !Qtopia::mousePreferred() ) QSoftMenuBar::clearLabel( this, Qt::Key_Select );
        QSoftMenuBar::clearLabel( this, Qt::Key_Back );
    }
#endif
}

void RegionSelector::reset()
{
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100

    // Reset region
#ifdef QTOPIA_PHONE
    if( Qtopia::mousePreferred() ) {
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
static const QPixmap top_left( ":image/photoedit/top_left" );
static const QPixmap top_right( ":image/photoedit/top_right" );
static const QPixmap bottom_left( ":image/photoedit/bottom_left" );
static const QPixmap bottom_right( ":image/photoedit/bottom_right" );
static const QPixmap crosshair( ":image/photoedit/crosshair" );

#define CROSSHAIR_CENTER 7
#define CORNER_WIDTH 7
#define CORNER_HEIGHT 7
#define INSET 1
#endif

#define PAINTER_COLOR Qt::white
#define SPACE_FILL_PATTERN Qt::Dense6Pattern
#define PEN_WIDTH 1

    QPainter painter( this );
    painter.setPen( PAINTER_COLOR );

    // If selection enabled, draw selected region onto widget
    // Otherwise, draw image ui onto widget
    if( enabled ) {
        // painter.setRasterOp( Qt::XorROP );

        // Draw box around current selection region
        painter.drawRect( _region.normalized().adjusted( 0, 0, -PEN_WIDTH, -PEN_WIDTH ) );

#ifdef QTOPIA_PHONE
        if( !Qtopia::mousePreferred() ) {
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
        QRegion region = image_ui->region().subtract( _region.normalized() );
        if( !region.isEmpty() ) {
            painter.setClipRegion( region );
            painter.setClipping( true );

            // Gray out area surrounding current selection region
            painter.fillRect( rect(), QBrush( PAINTER_COLOR, SPACE_FILL_PATTERN ) );
        }
    }
}

#ifdef QTOPIA_PHONE
void RegionSelector::keyPressEvent( QKeyEvent* e )
{
#define STEP 4

    if( enabled && !Qtopia::mousePreferred() ) {
        if( e->key() == QSoftMenuBar::menuKey() ) {
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
        } else if( e->key() == Qt::Key_Select ) {
            emit selected();
        } else {
            switch( current_state ) {
            // Move region
            case MOVE:
                switch( e->key() ) {
                case Qt::Key_Left:
                    moveBy( -STEP, 0 );
                    update();
                    break;
                case Qt::Key_Right:
                    moveBy( STEP, 0 );
                    update();
                    break;
                case Qt::Key_Up:
                    moveBy( 0, -STEP );
                    update();
                    break;
                case Qt::Key_Down:
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
                case Qt::Key_Left:
                    sizeBy( -STEP, 0 );
                    update();
                    break;
                case Qt::Key_Right:
                    sizeBy( STEP, 0 );
                    update();
                    break;
                case Qt::Key_Up:
                    sizeBy( 0, STEP );
                    update();
                    break;
                case Qt::Key_Down:
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
            if( _region.normalized().contains( e->pos() ) ) emit selected();
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
        QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), "photoedit/resize", QString() );
        break;
    case SIZE:
        QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), "photoedit/move", QString() );
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

    _region.translate( dx, dy );
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

