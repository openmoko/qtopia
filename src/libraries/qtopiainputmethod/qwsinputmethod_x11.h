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

#define X11_KEYBOARD_APP(Keyboard, widget)                           \
int main(int argc, char** argv) {                                    \
        QtopiaApplication app(argc, argv);                           \
        Keyboard keyboard;                                           \
        QWidget* wid = keyboard.widget;                              \
        wid->setFocusPolicy(Qt::NoFocus);                            \
        Atom keyboardAtom = XInternAtom(QX11Info::display(), "_E_VIRTUAL_KEYBOARD", False); \
        unsigned char data = 1;                                      \
        XChangeProperty(QX11Info::display(),                         \
                        wid->winId(), keyboardAtom,                  \
                        XA_CARDINAL, 32, PropModeReplace, &data, 1); \
        wid->show();                                                 \
        return app.exec();                                           \
}


        

#endif
