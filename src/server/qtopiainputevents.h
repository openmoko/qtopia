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

#ifndef _QTOPIAINPUTEVENTS_H
#define _QTOPIAINPUTEVENTS_H

#include <qobject.h>
#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#ifdef Q_WS_QWS
typedef QWSServer::KeyboardFilter QtopiaKeyboardFilter;
#else
class QtopiaKeyboardFilter
{
public:
    virtual ~QtopiaKeyboardFilter() {}
    virtual bool filter(int unicode, int keycode, int modifiers,
                        bool isPress, bool autoRepeat)=0;
};
#endif

class QtopiaInputEvents
{
private:
    QtopiaInputEvents() {}
    ~QtopiaInputEvents() {}

public:
    static void sendKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                             bool isPress, bool autoRepeat);
    static void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat);

    static void addKeyboardFilter(QtopiaKeyboardFilter *f);
    static void removeKeyboardFilter();

    static void suspendMouse();
    static void resumeMouse();

    static void openMouse();
    static void openKeyboard();
};

#endif // _QTOPIAINPUTEVENTS_H
