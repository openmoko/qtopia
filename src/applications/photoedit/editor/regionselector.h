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

#ifndef REGIONSELECTOR_H
#define REGIONSELECTOR_H

#include <qsoftmenubar.h>

#include <qwidget.h>
#include <qrect.h>
#include <qpoint.h>

class ImageUI;

class RegionSelector: public QWidget {
    Q_OBJECT
public:
    RegionSelector( ImageUI*, Qt::WFlags f = 0 );

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

    // Control region selector
    void keyPressEvent( QKeyEvent* );

    // Control region selector
    void mousePressEvent( QMouseEvent* );

    // Control region selector
    void mouseReleaseEvent( QMouseEvent* );

    // Control region selector
    void mouseMoveEvent( QMouseEvent* );

private:
    // Only keypad mode
    // Set state label in context bar to current state
    void setStateLabel();

    // Only keypad mode
    // Move region center limited to widget dimensions
    void moveBy( int dx, int dy );

    // Only keypad mode
    // Size region limited to widget dimensions
    void sizeBy( int dw, int dh );

    ImageUI *image_ui;

    bool enabled;
    enum {
        // Stylus operation
        MARK, MOVING,
        // Keypad operation
        MOVE, SIZE
    } current_state, previous_state;

    QPoint region_start;
    QRect lag_area, _region;
};

#endif
