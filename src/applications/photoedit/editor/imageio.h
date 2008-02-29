/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <qcontent.h>

#include <qobject.h>
#include <qrect.h>
#include <qimage.h>
#include <qsize.h>

#include <QByteArray>
#include <QImageReader>

#define DEFAULT_LEVELS 4

class ImageIO : public QObject
{
    Q_OBJECT
public:
    ImageIO( QObject* parent = 0 );

    ~ImageIO();

    enum Status { NORMAL, LOAD_ERROR, DEPTH_ERROR, REDUCED_SIZE, SIZE_ERROR };

    // Load image from file
    // The image at the nth level is 1/2^n the size of the original image
    // level    number of image samples, 1 .. n
    Status load( const QContent& lnk, int levels = DEFAULT_LEVELS );

    // Load image from image
    // The image at the nth level is 1/2^n the size of the original image
    // level    number of image samples, 1 .. n
    Status load( const QImage& image, int levels = DEFAULT_LEVELS );

    // Return image format
    // Pre: image was loaded from file
    QByteArray format() const { return _format; }

    // Return true if save is supported for the current format
    // Pre: image was loaded from file
    bool isSaveSupported() const;

    // Return true if the original file is read-only
    // Pre: image was loaded from file
    bool isReadOnly() const;

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
    QImage image( int level = 0 );

    // Return the visible portion of image at level within rect
    // rect    the desired area in the original image
    QImage image( const QRect& rect, int level = 0 ) const;

    // Return size of the original image
    QSize size() const { return image_samples[ 0 ].size(); }

signals:
    void changed();

private:
    QContent _lnk;
    int image_levels;

    QByteArray _format;
    QImage *image_samples;
    Status _status;
};

#endif
