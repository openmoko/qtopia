/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "slideshowui.h"

#include <qthumbnail.h>

#include <qpainter.h>
#include <qcolor.h>
#include <qfontmetrics.h>

#include <QKeyEvent>

SlideShowUI::SlideShowUI( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), display_name( false )
{ }

void SlideShowUI::setImage( const QContent& lnk )
{
    // Clear buffer, update image, update fitted name and update display
    image_buffer = QPixmap();
    image = lnk;
    if( display_name ) calculateFittedName();
    update();
}

void SlideShowUI::paintEvent( QPaintEvent* )
{
#define NAME_POSX 2
#define NAME_POSY 2
#define NAME_COLOR QColor( 162, 190, 0 )
#define SHADOW_OFFSET 1

    // If image not loaded, load image into buffer
    if( image_buffer.isNull() && !image.file().isNull() ) {
        // Scale image to fit within the current widget dimensions
        // while keeping the original width:height ratio
        // Load scaled image to buffer
        QThumbnail thumbnail( image.file() );
        image_buffer = thumbnail.pixmap( size() );
        // Update image position
        image_position.setX( ( width() - image_buffer.width() ) / 2 );
        image_position.setY( ( height() - image_buffer.height() ) / 2 );
    }

    QPainter painter( this );

    // If image has transparency, fill background and draw image
    // Otherwise, draw image and fill remaining background
    if( image_buffer.hasAlpha() ) {
        painter.fillRect( rect(), Qt::black );
        painter.drawPixmap( image_position, image_buffer );
    } else {
        painter.drawPixmap( image_position, image_buffer );
        QRect region( image_buffer.rect() );
        region.translate( image_position.x(), image_position.y() );
        painter.setClipRegion( QRegion( rect() ).subtract( region ) );
        painter.setClipping( true );
        painter.fillRect( rect(), Qt::black );
        painter.setClipping( false );
    }

    // If display name, draw fitted name onto widget
    if( display_name ) {
        // Draw shadow
        painter.setPen( Qt::black );
        painter.drawText( NAME_POSX + SHADOW_OFFSET,
           painter.fontInfo().pointSize() + NAME_POSY + SHADOW_OFFSET,
           fitted_name );
        // Draw fitted name
        painter.setPen( NAME_COLOR );
        painter.drawText( NAME_POSX, painter.fontInfo().pointSize() + NAME_POSY,
            fitted_name );
    }
}

void SlideShowUI::resizeEvent( QResizeEvent* )
{
    // Update image position
    image_position.setX( ( width() - image_buffer.width() ) / 2 );
    image_position.setY( ( height() - image_buffer.height() ) / 2 );

    // If display name, calculate fitted name
    if( display_name ) calculateFittedName();
}

#ifdef QTOPIA_PHONE
void SlideShowUI::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == Qt::Key_Back ) {
        emit pressed();
        e->accept();
    }
}
#endif

void SlideShowUI::mousePressEvent( QMouseEvent* )
{
    emit pressed();
}

void SlideShowUI::calculateFittedName()
{
    fitted_name = image.name();

    QFontMetrics font_metrics( font() );
    // If name is larger than the current width, reduce name
    if( font_metrics.width( fitted_name ) > width() - NAME_POSX ) {
        fitted_name = "...";
        int i = 0;
        // Add to fitted name while fitted name is less than current width
            while( !image.name()[ i ].isNull() &&
                font_metrics.width( fitted_name + image.name()[i] ) <
                    width() - NAME_POSX )
                fitted_name += image.name()[i++];
        fitted_name.remove( 0, 3 );
        fitted_name += "...";
    }
}
