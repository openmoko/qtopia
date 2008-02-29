/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "keyhold.h"

#include <QtGui>

KeyHold::KeyHold( int key, int keyHold, int threshold, QObject* target, QObject* parent )
    : QObject( parent ), m_key( key ), m_keyHold( keyHold ), m_threshold( threshold ), m_target( target )
{
    m_target->installEventFilter( this );

    m_countdown = new QTimer( this );
    m_countdown->setSingleShot( true );
    connect( m_countdown, SIGNAL(timeout()), this, SLOT(generateKeyHoldPress()) );
}

// ### Installing ignores auto repeat

bool KeyHold::eventFilter( QObject*, QEvent* e )
{
static bool enabled = true;

    if( enabled ) {
        switch( e->type() )
        {
        case QEvent::KeyPress:
            {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( ke->key() == m_key ) {
                if( !ke->isAutoRepeat() ) {
                    // Start hold countdown
                    m_countdown->start( m_threshold );
                }
                return true;
            }
            }
            break;
        case QEvent::KeyRelease:
            {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( ke->key() == m_key ) {
                if( !ke->isAutoRepeat() ) {
                    // If countdown active, generate key press and key release
                    // Otherwise, generate key hold release
                    if( m_countdown->isActive() ) {
                        m_countdown->stop();
                        enabled = false;
                        QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_key, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                        event = QKeyEvent( QEvent::KeyRelease, m_key, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                        enabled = true;
                    } else {
                        QKeyEvent event = QKeyEvent( QEvent::KeyRelease, m_keyHold, Qt::NoModifier );
                        QCoreApplication::sendEvent( m_target, &event );
                    }
                }
                return true;
            }
            }
            break;
        default:
            // Ignore
            break;
        }
    }

    return false;
}

void KeyHold::generateKeyHoldPress()
{
    QKeyEvent event = QKeyEvent( QEvent::KeyPress, m_keyHold, Qt::NoModifier );
    QCoreApplication::sendEvent( m_target, &event );
}
