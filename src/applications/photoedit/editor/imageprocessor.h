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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "matrix.h"

#include <qobject.h>
#include <qpoint.h>
#include <qsize.h>
#include <qpixmap.h>
#include <qimage.h>

class ImageIO;

class ImageProcessor : public QObject {
    Q_OBJECT
public:
    ImageProcessor( ImageIO*, QObject* parent = 0, const char* name = 0 );
    
    // Crop image to area within rect
    void crop( const QRect& );
    
    // Return point on transformed image given point on original
    QPoint map( const QPoint& ) const;
    
    // Return point on original image given point on transformed
    QPoint unmap( const QPoint& ) const;
    
    // Return a preview of the image with transformations applied at rect
    const QPixmap& preview( const QRect& rect ) const;
    
    // Return the image with transformations applied
    QImage image() const;
    
    // Return the image with transformation applied scaled to within size
    // The width to height ratio of the transformed image is preserved
    QImage image( const QSize& size ) const;
    
    // Return size of image with the current transformations applied
    QSize size() const;
    
    // Return true if transformed image is different from original
    bool isChanged() const;
    
signals:
    // Image has changed
    void changed();
    
public slots:
    // Set zoom factor for image
    // factor    0 < value
    void setZoom( double factor );

    // Set brightness factor for the image
    // factor    0 <= value <= 1
    void setBrightness( double factor );
    
    // Rotate image clockwise 90 degrees
    void rotate();
    
private slots:
    // Reset transformations
    void reset();

private:
    // Return rect on transformed image given rect on original
    QRect map( const QRect& ) const;

    // Return rect on original image given rect on transformed
    QRect unmap( const QRect& ) const;
    
    QImage transform( const QImage&, const QRect& ) const;

    ImageIO *image_io;
    
    QRect viewport;
    
    double brightness_factor;
    double zoom_factor;
    Matrix transformation_matrix;
    
    mutable QPixmap _preview;
};

#endif
