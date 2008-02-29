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
    QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), QSoftMenuBar::NoLabel );
}

void Slider::keyPressEvent( QKeyEvent* e )
{
    if( ( e->key() == Qt::Key_Select || e->key() == Qt::Key_Back ) && hasEditFocus() )
        emit selected();
    else
        QSlider::keyPressEvent( e );
}
