/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#ifdef QTOPIA_PHONE
    // Disable context menu
    QSoftMenuBar::setLabel( this, QSoftMenuBar::menuKey(), QSoftMenuBar::NoLabel );
#endif
}

#ifdef QTOPIA_PHONE
void Slider::keyPressEvent( QKeyEvent* e )
{
    if( !Qtopia::mousePreferred() ) {
        if( e->key() == Qt::Key_Select && hasEditFocus() )
            emit selected();

    }
    QSlider::keyPressEvent( e );
}
#endif
