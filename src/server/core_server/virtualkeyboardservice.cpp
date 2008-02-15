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

#include "virtualkeyboardservice.h"
#include <qtopiaipcenvelope.h>
#include "qtopiainputevents.h"
#include "qtopiaserverapplication.h"

/*!
  \service VirtualKeyboardService VirtualKeyboard
  \ingroup QtopiaServer::Task
  \brief The VirtualKeyboardService class provides a service to simulate keypresses on a device.

  The \c {VirtualKeyboard} service provided by the VirtualKeyboardService class
  allows applications to cause the equivalent of physical keypresses on a
  device.

  The VirtualKeyboardService class provides the \c {VirtualKeyboard} task.
  It is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  \internal
  Construct a new VirtualKeyboardService instance with the provided \a parent.
 */
VirtualKeyboardService::VirtualKeyboardService( QObject *parent )
    : QtopiaAbstractService( "VirtualKeyboard", parent )
{
    publishAll();
}

/*!
  \internal
  Destroys the VirtualKeyboardService instance.
 */
VirtualKeyboardService::~VirtualKeyboardService()
{
}

/*!
  Simulate the press and release of \a key.  This is equivalent to calling the
  QtopiaInputEvents::processKeyEvent() method for a press and then a release.

  This slot corresponds to the QCop service message \c{VirtualKeyboard::keyPress(int)}.
 */
void VirtualKeyboardService::keyPress( int key )
{
    QtopiaInputEvents::processKeyEvent(QChar(key).unicode(), key, 0, true, false);
    QtopiaInputEvents::processKeyEvent(QChar(key).unicode(), key, 0, false, false);
}

/*!
    Send a key event consisting of \a unicode, \a keycode,
    \a modifiers, \a isPress, and \a autoRepeat.

    This slot corresponds to the QCop service message
    \c{VirtualKeyboard::sendKeyEvent(int,int,int,bool,bool)}.
*/
void VirtualKeyboardService::sendKeyEvent
    ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    QtopiaInputEvents::sendKeyEvent
        ( unicode, keycode, (Qt::KeyboardModifiers)modifiers,
          isPress, autoRepeat );
}

/*!
    Process a key event consisting of \a unicode, \a keycode,
    \a modifiers, \a isPress, and \a autoRepeat.

    This slot corresponds to the QCop service message
    \c{VirtualKeyboard::processKeyEvent(int,int,int,bool,bool)}.
*/
void VirtualKeyboardService::processKeyEvent
    ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    QtopiaInputEvents::processKeyEvent
        ( unicode, keycode, (Qt::KeyboardModifiers)modifiers,
          isPress, autoRepeat );
}

QTOPIA_TASK(VirtualKeyboard, VirtualKeyboardService);
