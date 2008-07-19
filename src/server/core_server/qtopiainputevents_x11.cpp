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

#include "qtopiainputevents.h"
#include "qwsinputmethod_x11.h"

#ifdef Q_WS_X11


void QtopiaInputEvents::sendKeyEvent
    (int unicode, int keycode, Qt::KeyboardModifiers modifiers,
     bool isPress, bool autoRepeat)
{
    QWSServer::sendKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

void QtopiaInputEvents::processKeyEvent
    (int unicode, int keycode, Qt::KeyboardModifiers modifiers,
    bool isPress, bool autoRepeat)
{
    // processKeyEvent() is the same as sendKeyEvent() for X11.
    // Physical keyboards and input methods will typically be
    // handled via XIM, rather than direct key faking.
    sendKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

void QtopiaInputEvents::addKeyboardFilter(QtopiaKeyboardFilter *f)
{
    Q_UNUSED(f);
}

void QtopiaInputEvents::removeKeyboardFilter()
{
}

void QtopiaInputEvents::suspendMouse()
{
}

void QtopiaInputEvents::resumeMouse()
{
}

void QtopiaInputEvents::openMouse()
{
}

void QtopiaInputEvents::openKeyboard()
{
}

#endif // Q_WS_X11
