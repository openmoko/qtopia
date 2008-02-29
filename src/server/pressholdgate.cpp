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

#include "pressholdgate.h"
#include <QDeviceButtonManager>
#include <QtopiaApplication>
#include <qwindowsystem_qws.h>

/*!
  \class PressHoldGate
  \brief The PressHoldGate class encapsulates the processing of a key with press and press-and-hold actions.
  \ingroup QtopiaServer
  \internal
 */

/*! \internal */
void PressHoldGate::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == held_tid ) {
        killTimer(held_tid);
        // button held
        if ( held_key ) {
            emit activate(held_key, true);
            held_key = 0;
        }
        held_tid = 0;
    }

    QObject::timerEvent(e);
}

bool PressHoldGate::filterKey(int keycode, bool pressed, bool pressable, bool holdable)
{
    bool filterout=true;
    if ( held_tid ) {
        killTimer(held_tid);
        held_tid = 0;
    }
    if ( !holdable ) {
        if ( !pressable ) {
            filterout=false;
        } else {
            if ( pressed ) {
                emit activate(keycode,false);
            }
        }
    } else if ( pressed ) {
        if ( held_key ) {
            // If QWSServer::sendKeyEvent is changed to QWSServer::processKeyEvent,
            // this would happen.
            filterout=false;
        } else {
            held_key = keycode;
            if ( holdable ) {
                held_tid = startTimer(500);
            }
        }
    } else if ( held_key ) {
        if ( pressable ) {
            held_key = 0;
            emit activate(keycode,false);
        } else {
            if ( hardfilter ) // send the press now...
                QWSServer::sendKeyEvent(0, keycode, 0, true, false);
            held_key = 0;
            filterout=false;
        }
    }
    return filterout;
}

/*!
 \internal
*/
bool PressHoldGate::filterDeviceButton(int keycode, bool press, bool autorepeat)
{
    const QDeviceButton* button =
        QDeviceButtonManager::instance().buttonForKeycode(keycode,context);

    if (button) {
        bool pressable=!button->pressedAction().isNull();
        bool holdable=!button->heldAction().isNull();
        if ( autorepeat || filterKey(keycode,press,pressable,holdable) )
            return true;
    }

    return false;
}

