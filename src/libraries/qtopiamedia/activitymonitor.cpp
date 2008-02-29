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

#include "activitymonitor_p.h"

/*!
    \class ActivityMonitor
    \internal
*/

/*!
    \fn void ActivityMonitor::update()
    \internal
*/
void ActivityMonitor::update()
{
    if( isActive() ) {
        m_updateCalled = true;
    } else {
        m_active = true;
        emit active();

        // Start polling for activity
        m_timer = startTimer( m_interval );
    }
}

/*!
    \fn void ActivityMonitor::timerEvent( QTimerEvent* e )
    \internal
*/
void ActivityMonitor::timerEvent( QTimerEvent* e )
{
    if( e->timerId() == m_timer ) {
        if( m_updateCalled ) {
            m_updateCalled = false;
        } else {
            // Stop polling for activity
            killTimer( m_timer );

            m_active = false;
            emit inactive();
        }
    }
}
