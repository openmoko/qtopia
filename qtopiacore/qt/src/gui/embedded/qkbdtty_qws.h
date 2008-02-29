/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKBDTTY_QWS_H
#define QKBDTTY_QWS_H

#include <QtGui/qkbdpc101_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_TTY

class QWSTtyKbPrivate;

class QWSTtyKeyboardHandler : public QWSPC101KeyboardHandler
{
public:
    explicit QWSTtyKeyboardHandler(const QString&);
    virtual ~QWSTtyKeyboardHandler();

protected:
    virtual void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat);

private:
    QWSTtyKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_TTY

#endif // QT_NO_QWS_KEYBOARD

QT_END_HEADER

#endif // QKBDTTY_QWS_H
