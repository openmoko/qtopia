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

#include "singleview_p.h"

#include "image.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/contextbar.h>

#include <qpainter.h>

SingleView::SingleView( QWidget* parent, const char* name, WFlags f )
    : QWidget( parent, name, f | Qt::WResizeNoErase | Qt::WRepaintNoErase ),
    dialog_mode( false ), right_pressed( false )
{
#ifndef QTOPIA_PHONE
    // Enable stylus press events
    QPEApplication::setStylusOperation( this, QPEApplication::RightOnHold );
#endif

    // Accept focus
    setFocusPolicy( QWidget::StrongFocus );
}

#ifdef QTOPIA_PHONE
void SingleView::enableDialogMode()
{
    dialog_mode = true;
}
#endif

void SingleView::setImage( const DocLnk& lnk )
{
    // Clear buffer, update image and update display
    image_buffer.resize( 0, 0 );
    image = lnk;
    QWidget::update();
}

void SingleView::paintEvent( QPaintEvent* )
{
    // If image not loaded, load image into buffer
    if( image_buffer.isNull() && !image.file().isNull() ) {
        // Scale image to fit within the current widget dimensions
        // while keeping the original width:height ratio
        // Load scaled image to buffer
        image_buffer = Image::loadScaled( image.file(), width(), height() );
        // Update image position
        image_position.setX( ( width() - image_buffer.width() ) / 2 );
        image_position.setY( ( height() - image_buffer.height() ) / 2 );
    }
    
    QPainter painter( this );
    
    // If image has transparency, fill background and draw image
    // Otherwise, draw image and fill remaining background
    if( image_buffer.hasAlpha() ) {
        painter.fillRect( rect(), Qt::white );
        painter.drawPixmap( image_position, image_buffer );
    } else {
        painter.drawPixmap( image_position, image_buffer );
        QRect region( image_buffer.rect() );
        region.moveBy( image_position.x(), image_position.y() );
        painter.setClipRegion( QRegion( rect() ).subtract( region ) );
        painter.setClipping( true );
        painter.fillRect( rect(), Qt::white );
    }
}

void SingleView::resizeEvent( QResizeEvent* )
{
    // Update image position
    image_position.setX( ( width() - image_buffer.width() ) / 2 );
    image_position.setY( ( height() - image_buffer.height() ) / 2 );
}

void SingleView::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() ) {
    // If select key, emit select signal
#ifdef QTOPIA_PHONE
    case Qt::Key_Select:
#endif
    case Qt::Key_Enter:
        emit selected();
        break;
    // Move forward one image
    case Qt::Key_Right:
        emit forward();
        break;
    // Move backward one image
    case Qt::Key_Left:
        emit back();
        break;
#ifdef QTOPIA_PHONE
    // If cancel key, emit cancel signal
    case Qt::Key_Back:
        if( dialog_mode ) { 
            emit canceled(); 
            e->accept(); 
        } else e->ignore();
        break;
#endif
    default:
        e->ignore();
    }
}

#ifndef QTOPIA_PHONE
void SingleView::mouseReleaseEvent( QMouseEvent* e )
{
    // If left button clicked, emit selected
    if( !right_pressed && e->button() == Qt::LeftButton ) emit selected();
}
#endif

void SingleView::mousePressEvent( QMouseEvent* e )
{
#ifdef QTOPIA_PHONE
    Q_UNUSED(e);
    emit selected();
#else
    right_pressed = false;
    // If right button pressed, emit held
    if( e->button() == Qt::RightButton ) {
        right_pressed = true;
        emit held( e->globalPos() );
    }
#endif
}
