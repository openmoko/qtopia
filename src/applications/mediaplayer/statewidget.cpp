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

#include "statewidget.h"
#include "keyhold.h"

#include <media.h>

static const int KEY_SELECT_HOLD = Qt::Key_unknown + Qt::Key_Select;

StateWidget::StateWidget( PlayerControl* control, QWidget* parent )
    : QWidget( parent ), m_control( control )
{
    static const int HOLD_THRESHOLD = 500;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin( 0 );

    m_label = new QMediaStateLabel;
    layout->addWidget( m_label );

    setLayout( layout );

    connect( control, SIGNAL(stateChanged(PlayerControl::State)),
        this, SLOT(setState(PlayerControl::State)) );

    m_holdtimer = new QTimer( this );
    connect( m_holdtimer, SIGNAL(timeout()),
        this, SLOT(setStopped()) );
    m_holdtimer->setInterval( HOLD_THRESHOLD );
    m_holdtimer->setSingleShot( true );

    new KeyHold( Qt::Key_Select, KEY_SELECT_HOLD, HOLD_THRESHOLD, this, this );
}

void StateWidget::setState( PlayerControl::State state )
{
    switch( state )
    {
    case PlayerControl::Playing:
        m_label->setState( QtopiaMedia::Playing );
        break;
    case PlayerControl::Paused:
        m_label->setState( QtopiaMedia::Paused );
        break;
    case PlayerControl::Stopped:
        m_label->setState( QtopiaMedia::Stopped );
        break;
    }
}

void StateWidget::setStopped()
{
    m_control->setState( PlayerControl::Stopped );
}

void StateWidget::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() )
    {
    case Qt::Key_Select:
        e->accept();
        togglePlaying();
        break;
    case KEY_SELECT_HOLD:
        e->accept();
        setStopped();
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
}

void StateWidget::mousePressEvent( QMouseEvent* )
{
    m_holdtimer->start();
}

void StateWidget::mouseReleaseEvent( QMouseEvent* )
{
    if( m_holdtimer->isActive() ) {
        togglePlaying();
        m_holdtimer->stop();
    }
}

void StateWidget::togglePlaying()
{
    switch( m_control->state() )
    {
    case PlayerControl::Playing:
        m_control->setState( PlayerControl::Paused );
        break;
    default:
        m_control->setState( PlayerControl::Playing );
        break;
    }
}
