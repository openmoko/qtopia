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

#include "devicebuttontask.h"
#include "pressholdgate.h"
#include <QtopiaApplication>
#include <QDeviceButtonManager>
#include <qtopiaipcenvelope.h>
#include <QValueSpaceItem>

/*!
  \class DeviceButtonTask
  \ingroup QtopiaServer
  \brief The DeviceButtonTask class performs configurable actions when special
         purpose buttons are pressed or held.

  The DeviceButtonTask class provides the server backend for the QDeviceButton
  and QDeviceButtonManager APIs.  The DeviceButtonTask will issue the actions
  for all configured button mappings without a specified context.

  The DeviceButtonTask class provides the \c {DeviceButton} task.

  \sa QDeviceButton, QDeviceButtonManager
 */

/*! \internal */
DeviceButtonTask::DeviceButtonTask()
:   vs( 0 ),
    ph( 0 )
{
    vs = new QValueSpaceItem( "/UI", this );
    ph = new PressHoldGate(this);
    ph->setHardFilter(true);
    connect(ph,SIGNAL(activate(int,bool)),this,SLOT(doActivate(int,bool)));
    QWSServer::addKeyboardFilter(this);
}

/*! \internal */
bool DeviceButtonTask::filter(int, int keycode, int modifiers,
                              bool press, bool autoRepeat)
{
    if(!modifiers) {
        if( !keyLocked() && !((QtopiaApplication*)qApp)->keyboardGrabbed()) {
            // First check to see if QDeviceButtonManager knows something
            // about this button:
            if ( ph->filterDeviceButton(keycode,press,autoRepeat) ) {
                QWSServer::screenSaverActivate(false);
                return true;
            }
        }

        if ( keycode == Qt::Key_F29 ) { // Lock key
            if ( press )
                QtopiaIpcEnvelope e("QPE/System", "showHomeScreenAndToggleKeylock()");
            return true;
        }
    }

    return false;
}

/*! \internal */
bool DeviceButtonTask::keyLocked()
{
    return vs->value( "KeyLock" ).toBool();
}

/*! \internal */
void DeviceButtonTask::doActivate(int keycode, bool held)
{
    const QDeviceButton* button =
        QDeviceButtonManager::instance().buttonForKeycode(keycode);
    if ( button ) {
        QtopiaServiceRequest sr;
        if ( held ) {
            sr = button->heldAction();
        } else {
            sr = button->pressedAction();
        }

        // A button with no action defined, will return a null
        // QtopiaServiceRequest.  Don't attempt to send/do anything with this
        // as it will crash
        if ( !sr.isNull() ) {
            QString app = QtopiaService::app(sr.service());
            sr.send();
        }

        emit activated(keycode, held);
    }
}

/*!
  \fn void DeviceButtonTask::activated(int keycode, bool wasHeld)

  Emitted whenever the special-function button \a keycode was pressed.
  \a wasHeld will be true if the button was held and false if the button was
  pressed.

  This signal will be emitted \bold after the button action has been performed.
 */

QTOPIA_TASK(DeviceButton, DeviceButtonTask);
QTOPIA_TASK_PROVIDES(DeviceButton, DeviceButtonTask);
