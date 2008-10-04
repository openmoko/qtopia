/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
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

    void focusOutEvent(QFocusEvent *event);
};

#endif
