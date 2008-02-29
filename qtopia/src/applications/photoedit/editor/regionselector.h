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

#ifndef REGIONSELECTOR_H
#define REGIONSELECTOR_H

#include <qtopia/contextmenu.h>

#include <qwidget.h>
#include <qrect.h>
#include <qpoint.h>
#include <qbitmap.h>

class ImageUI;

class RegionSelector: public QWidget {
    Q_OBJECT
public:
    RegionSelector( ImageUI*, const char* name = 0, WFlags f = 0 );
    
    // Return currently selected region
    QRect region() const;
    
    // Enable region selection if true
    void setEnabled( bool );
        
signals:
    // Stylus or back key has been pressed
    // Only emitted when selection is disabled 
    void pressed();

    // Region has been selected
    // Only emitted when selection is enabled
    void selected();
    
    // Only when mouse preferred
    // Selection has been canceled
    // Only emitted when selection is enabled
    void canceled();
    
public slots:
    // Reset region selection
    void reset();
    
protected:
    // Draw widget
    void paintEvent( QPaintEvent* );
    
#ifdef QTOPIA_PHONE
    // Control region selector
    void keyPressEvent( QKeyEvent* );
#endif

    // Control region selector
    void mousePressEvent( QMouseEvent* );
    
    // Control region selector
    void mouseReleaseEvent( QMouseEvent* );

    // Control region selector
    void mouseMoveEvent( QMouseEvent* );
    
private:
#ifdef QTOPIA_PHONE
    // Only keypad mode
    // Set state label in context bar to current state
    void setStateLabel();

    // Only keypad mode
    // Move region center limited to widget dimensions
    void moveBy( int dx, int dy );
    
    // Only keypad mode
    // Size region limited to widget dimensions
    void sizeBy( int dw, int dh );
#endif

    ImageUI *image_ui;
    
    bool enabled;
    enum {
        // Stylus operation 
        MARK, MOVING,
        // Keypad operation
        MOVE, SIZE
    } current_state;

    QPoint region_start;
    QRect lag_area, _region;
};

#endif
