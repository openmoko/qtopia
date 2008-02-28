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

#ifndef SLIDESHOWUI_H
#define SLIDESHOWUI_H

#include <qcontent.h>

#include <qwidget.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qstring.h>

class SlideShowUI : public QWidget {
    Q_OBJECT
public:
    SlideShowUI( QWidget* parent = 0, Qt::WFlags f = 0 );

signals:
    // Stylus pressed
    void pressed();

public slots:
    // Set image to display
    void setImage( const QContent& );

    // If true, display name of image
    void setDisplayName( bool b ) { display_name = b; }

protected:
    // Draw scaled image onto widget
    void paintEvent( QPaintEvent* );

    // Update image position
    void resizeEvent( QResizeEvent* );

    void keyPressEvent( QKeyEvent* );

    // Transform stylus presses into signals
    void mousePressEvent( QMouseEvent* );

private:
    bool display_name;

    QContent image;
    QPixmap image_buffer;
    QPoint image_position;
};

#endif
