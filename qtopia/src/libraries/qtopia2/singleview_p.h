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

#ifndef SINGLEVIEW_P_H
#define SINGLEVIEW_P_H

#include <qtopia/applnk.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>

class SingleView : public QWidget
{
    Q_OBJECT
public:
    SingleView( QWidget* parent = 0, const char* name = 0, WFlags f = 0 );
    
#ifdef QTOPIA_PHONE
    // BCI: To overcome limitations in ImageSelectorDialog
    // Enable caputre of the back key
    void enableDialogMode();
#endif
    
signals:
    // Select key pressed
    void selected();
    
    // Only Qtopia PDA
    // Stylus held
    void held( const QPoint& );
    
    // Right key presse
    void forward();
    
    // Left key pressed
    void back();

    // Only Qtopia Phone
    // Back key pressed
    void canceled();

public slots:
    // Clear buffer and reload current image
    void setImage( const DocLnk& );

protected:
    // Draw scaled image onto widget
    void paintEvent( QPaintEvent* );

    // Update image position
    void resizeEvent( QResizeEvent* );

    // Naivgate through visible collection
    void keyPressEvent( QKeyEvent* );
    
#ifndef QTOPIA_PHONE
    // Emit selected signal
    void mouseReleaseEvent( QMouseEvent* );
#endif

    // Emit held signal
    void mousePressEvent( QMouseEvent* );

private:
    bool dialog_mode, right_pressed;

    DocLnk image;
    QPixmap image_buffer;
    QPoint image_position;
};

#endif // SINGLEVIEW_P_H
