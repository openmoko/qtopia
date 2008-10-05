/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#ifndef QWSInputMethodX11_h
#define QWSInputMethodX11_h

#include <qtopiaglobal.h>

#include <QObject>
#include <QPoint>

/**
 * Currently this is a dummy QWSInputMethod. It can just send key events
 * using fakekey. No special input method handling is supported for now...
 */
class QTOPIA_VISIBILITY QWSInputMethod : public QObject {
    Q_OBJECT
public:
    QWSInputMethod();

    void sendCommitString(const QString&);
    void sendPreeditString(const QString&, int length) { Q_UNUSED(length) }
};

struct QWSServer {
    static void sendKeyEvent(int unicode, int keycode, int mod, bool isPress, bool autoRepeat);
};

#endif
