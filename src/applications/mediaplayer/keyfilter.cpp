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

#include <QKeyEvent>

#include "keyfilter.h"

namespace mediaplayer
{

KeyFilter::KeyFilter( QObject* subject, QObject* target, QObject* parent )
    : QObject( parent ), m_target( target )
{
    subject->installEventFilter( this );
}

void KeyFilter::addKey( int key )
{
    m_keys.insert( key );
}

bool KeyFilter::eventFilter( QObject*, QEvent* e )
{
    // Guard against recursion
    static QEvent* d = 0;

    if( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if( d != e && !ke->isAutoRepeat() && m_keys.contains( ke->key() ) ) {
            QKeyEvent event = QKeyEvent( e->type(), ke->key(), Qt::NoModifier );
            QCoreApplication::sendEvent( m_target, d = &event );
            d = 0;
            return true;
        }
    }

    return false;
}

} // ns mediaplayer
