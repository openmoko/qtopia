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

#ifndef NOKIAKBDHANDLER_H
#define NOKIAKBDHANDLER_H

#ifdef QT_QWS_NOKIA

#include <QObject>
#include <QWSKeyboardHandler>
#include <QTimer>

class QSocketNotifier;

struct NokiaKeys {
    uint code;
    uint keyCode;
    uint FnKeyCode;
    ushort unicode;
    ushort shiftUnicode;
    ushort fnUnicode;
    
};

class NokiaKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    NokiaKbdHandler();
    ~NokiaKbdHandler();
    virtual const NokiaKeys *keyMap() const;

private:
    QSocketNotifier *m_notify;
    QSocketNotifier *powerNotify;
    QTimer *keytimer;

    int  kbdFD;
    int powerFd;
    bool shift;
    int numKeyPress;
    bool keyFunction;
    bool controlButton;

    int getKeyCode(int code, bool isFunc);
    int getUnicode(int code);
    
private Q_SLOTS:

    void readKbdData();
    void readPowerKbdData();
    void timerUpdate();
};

#endif // QT_QWS_NOKIA

#endif // NOKIAKBDHANDLER_H
