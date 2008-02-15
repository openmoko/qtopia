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

#ifndef KERNELKEYMAPKBDHANDLER_H
#define KERNELKEYMAPKBDHANDLER_H

//#ifdef QT_QWS_KERNELKEYMAP

#include <QObject>
#include <QWSKeyboardHandler>

#include <termios.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

class QSocketNotifier;

typedef struct QWSKeyMap {
    uint key_code;
    ushort unicode;
    ushort shift_unicode;
    ushort ctrl_unicode;
};

class KernelkeymapKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    KernelkeymapKbdHandler();
    ~KernelkeymapKbdHandler();
    void readKeyboardMap();
    void readUnicodeMap();
    void handleKey(unsigned char code);

    virtual const QWSKeyMap *keyMap() const;
    
private:
    QSocketNotifier *m_notify;
    int  kbdFD;
    struct termios origTermData;

    unsigned short acm[E_TABSZ];
    unsigned char kernel_map[(1<<KG_CAPSSHIFT)][NR_KEYS];

    int current_map;	

private Q_SLOTS:
    void readKeyboardData();

    void handleTtySwitch(int sig);
protected:
      Qt::KeyboardModifiers modifiers;
    int prevuni;
    int prevkey;
  
};

//#endif // QT_QWS_KERNELKEYMAP

#endif // KERNELKEYMAPKBDHANDLER_H
