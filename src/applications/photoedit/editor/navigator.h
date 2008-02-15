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
    Navigator( ImageUI*, QWidget* parent = 0, Qt::WFlags f = 0 );

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

    // Return preferred size of navigator
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

    bool valid;
};

#endif
