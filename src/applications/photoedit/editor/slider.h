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

#ifndef SLIDER_H
#define SLIDER_H

#include <qslider.h>

class Slider : public QSlider {
    Q_OBJECT
public:
    Slider( int min, int max, int step, int value, QWidget* parent );

signals:
    // Only Qtopia Phone
    // Value has been selected
    void selected();

protected:
    // Emit selected if select key pressed
    void keyPressEvent( QKeyEvent* );
};

#endif
