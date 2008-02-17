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

#ifndef GREENPHONEKBDHANDLER_H
#define GREENPHONEKBDHANDLER_H

#ifdef QT_QWS_GREENPHONE

#include <QObject>
#include <QWSKeyboardHandler>

#include <termios.h>
#include <linux/kd.h>

class QSocketNotifier;

class GreenphoneKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    GreenphoneKbdHandler();
    ~GreenphoneKbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
//    unsigned int    m_repeatKeyCode;
//    unsigned short  m_unicode;
    struct termios origTermData;
//    QTimer*     m_timer;

private Q_SLOTS:
    void readKbdData();
    void handleTtySwitch(int sig);
//    void repeat();
};

#endif // QT_QWS_GREENPHONE

#endif // GREENPHONEKBDHANDLER_H
