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

#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <qtopia/applnk.h>

#include <qobject.h>
#include <qstring.h>
#include <qrect.h>
#include <qimage.h>
#include <qsize.h>

#define DEFAULT_LEVELS 4

class ImageIO : public QObject {
    Q_OBJECT
public:
    ImageIO( QObject* parent = 0, const char* name = 0 );
    
    ~ImageIO();
    
    enum Status { NORMAL, LOAD_ERROR, DEPTH_ERROR };

    // Load image from file
    // The image at the nth level is 1/2^n the size of the original image 
    // level    number of image samples, 1 .. n
    Status load( const DocLnk& lnk, int levels = DEFAULT_LEVELS );
    
    // Load image from image
    // The image at the nth level is 1/2^n the size of the original image 
    // level    number of image samples, 1 .. n
    Status load( const QImage& image, int levels = DEFAULT_LEVELS );
    
    // Return image format
    // Pre: image was loaded from file
    const char* format() const { return QImageIO::imageFormat( _lnk.file() ); }
    
    // Return true if save is supported for the current format
    // Pre: image was loaded from file 
    bool isSaveSupported() const;
    
    // Save image to file using original format
    // Return true if save successful
    // If save not supported, ignore overwrite and save using default format
    // Pre: image was loaded from file
    bool save( const QImage&, bool overwrite = true );
    
    // Return level closest to given reduction factor
    // Levels are numbered 0 .. n-1, 0 being the original image
    int level( double ) const;
    
    // Return reduction factor given level
    double factor( int ) const;
    
    // Return image at level
    QImage image( int level = 0 ) { return image_samples[ level ]; }
    
    // Return the visible portion of image at level within rect
    // rect    the desired area in the original image
    QImage image( const QRect& rect, int level = 0 ) const;
    
    // Return size of the original image
    QSize size() const { return image_samples[ 0 ].size(); }
    
signals:
    void changed();
    
private:
    DocLnk _lnk;
    int image_levels;

    QImage *image_samples;
};

#endif
