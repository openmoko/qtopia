/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
    ImageUI( ImageProcessor*, QWidget* parent = 0, Qt::WFlags f = 0 );

    // Enable preview if true
    void setEnabled( bool b ) { enabled = b; }

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

    QPixmap preview;
    QPoint viewport_center, preview_position;
};

#endif
