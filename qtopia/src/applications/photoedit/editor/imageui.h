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

#ifndef IMAGEUI_H
#define IMAGEUI_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qregion.h>

class ImageProcessor;

class ImageUI : public QWidget {
    Q_OBJECT
public:
    ImageUI( ImageProcessor*, QWidget* parent = 0, const char* name = 0, 
        WFlags f = 0 );
        
    // Enable preview if true
    void setEnabled( bool b ) { enabled = b; }
        
    // Return a pixmap representing the widget
    const QPixmap& pixmap() const { return widget_buffer; }
    
    // Return current size of image
    QSize space() const { return _space.size(); }
    
    // Return current viewport
    QRect viewport() const { return _viewport; }
    
    // Return viewport contained within rect
    QRect viewport( const QRect& ) const;
    
    // Return position and dimensions of viewport in widget
    QRegion region() const;
    
    // Reset viewport centering on space
    void reset();

signals:
    // Image dimensions has changed
    void changed();

    // Preview has been updated
    void updated();
    
public slots:
    // Move viewport and update preview
    void moveViewportBy( int dx, int dy );
    
private slots:
    // Update viewport and reload preview
    void updateViewport();
    
protected:
    // Update widget buffer with preview from image processor
    void paintEvent( QPaintEvent* );

    // Resize viewport and update preview position
    void resizeEvent( QResizeEvent* );

private:
    // Paint preview onto buffer
    void updateBuffer();

    // Contain viewport within space
    void containViewport();

    ImageProcessor *image_processor;
    
    bool enabled;

    QRect _space, _viewport;
    
    QPixmap preview, widget_buffer;
    QPoint viewport_center, preview_position;
};

#endif
