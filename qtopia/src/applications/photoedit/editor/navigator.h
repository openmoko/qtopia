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

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <qwidget.h>
#include <qsize.h>
#include <qrect.h>
#include <qpoint.h>

class ImageUI;

class Navigator : public QWidget {
    Q_OBJECT
public:
    Navigator( ImageUI*, QWidget* parent = 0, const char* name = 0, 
        WFlags f = 0 );

signals:
    // Viewport has moved
    void viewportMoved( int dx, int dy );
    
private slots:
    // Retrive and update viewport and space from image ui
    void updateNavigator();
    
protected:
    // Move viewport in the direction of the arrow key pressed
    void keyPressEvent( QKeyEvent* );
    
    // Allow movement of viewport when stylus moves
    void mousePressEvent( QMouseEvent* );
    
    // Disallow movement of view viewport
    void mouseReleaseEvent( QMouseEvent* );
    
    // Move viewport
    void mouseMoveEvent( QMouseEvent* );
    
    // Paint navigator onto widget
    void paintEvent( QPaintEvent* );
    
    // Resize naviagtor and update display
    void resizeEvent( QResizeEvent* );
    
    // Return prefered size of navigator
    QSize sizeHint() const;

private:
    // Reduce space and viewport to fit on widget
    void calculateReduced();
    
    // Move viewports within space
    // dx    x reduced displacement
    // dy    y reduced displacement
    void moveViewportBy( int dx, int dy );
    
    ImageUI *image_ui;
    
    QPoint mouse_position;
    bool moving_viewport;
    
    QRect actual_space;
    QRect actual_viewport;
    
    double reduction_ratio;
    QRect reduced_space;
    QRect reduced_viewport;
    QPoint centered_origin;
};

#endif
