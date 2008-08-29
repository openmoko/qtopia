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

#include "slider.h"

#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <QKeyEvent>

Slider::Slider( int min, int max, int step, int value, QWidget* parent )
    : QSlider( Qt::Horizontal, parent )
{
    setMinimum( min );
    setMaximum( max );
    setPageStep( step );
    setValue( value );
    // Disable context menu
    QSoftMenuBar::menuFor( this );
}

void Slider::keyPressEvent( QKeyEvent* e )
{
    switch (e->key()) {
    case Qt::Key_Select:
    case Qt::Key_Back:
        QSlider::keyPressEvent( e );

        emit selected();
    case Qt::Key_Up:
    case Qt::Key_Down:
        e->accept();
        break;
    default:
        QSlider::keyPressEvent( e );
    }
}

void Slider::focusOutEvent(QFocusEvent *event)
{
    switch (event->reason()) {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
        hide();
    default:
        QSlider::focusOutEvent(event);
    }
}
