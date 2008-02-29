/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "greenphonekbdhandler.h"

#ifdef QT_QWS_GREENPHONE
#include <QScreen>
#include <QSocketNotifier>

#include "qscreen_qws.h"
#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qnamespace.h"

#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>

#define VTACQSIG SIGUSR1
#define VTRELSIG SIGUSR2

static int vtQws = 0;

GreenphoneKbdHandler::GreenphoneKbdHandler()
{
    qLog(Input) << "Loaded Greenphone keypad plugin";
    setObjectName( "Greenphone Keypad Handler" );
    kbdFD = ::open("/dev/tty0", O_RDONLY|O_NDELAY, 0);
    if (kbdFD >= 0) {
        qLog(Input) << "Opened tty0 as keypad input";
        m_notify = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
        connect( m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    } else {
        qWarning("Cannot open tty0 for keypad (%s)", strerror(errno));
        return;
    }

    tcgetattr(kbdFD, &origTermData);
    struct termios termdata;
    tcgetattr(kbdFD, &termdata);

    ioctl(kbdFD, KDSKBMODE, K_RAW);

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(kbdFD, TCSANOW, &termdata);

    connect(QApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(handleTtySwitch(int)));
    QApplication::instance()->watchUnixSignal(VTACQSIG, true);
    QApplication::instance()->watchUnixSignal(VTRELSIG, true);

    struct vt_mode vtMode;
    ioctl(kbdFD, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTRELSIG;
    vtMode.acqsig = VTACQSIG;
    ioctl(kbdFD, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(kbdFD, VT_GETSTATE, &vtStat);
    vtQws = vtStat.v_active;
}

GreenphoneKbdHandler::~GreenphoneKbdHandler()
{
    if (kbdFD >= 0) {
        ioctl(kbdFD, KDSKBMODE, K_XLATE);
        tcsetattr(kbdFD, TCSANOW, &origTermData);
        ::close(kbdFD);
        kbdFD = -1;
    }
}

void GreenphoneKbdHandler::handleTtySwitch(int sig)
{
    if (sig == VTACQSIG) {
        if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
            qwsServer->enablePainting(true);
            qt_screen->restore();
            qwsServer->resumeMouse();
            qwsServer->refresh();
        }
    } else if (sig == VTRELSIG) {
        qwsServer->enablePainting(false);
        qt_screen->save();
        if(ioctl(kbdFD, VT_RELDISP, 1) == 0) {
            qwsServer->suspendMouse();
        } else {
            qwsServer->enablePainting(true);
        }
    }
}

void GreenphoneKbdHandler::readKbdData()
{
    unsigned char  buf[81];
    unsigned short key_code;
    unsigned short Phkey;
    unsigned short unicode;
    unsigned int   key_code2;
    int            modifiers;

    int n = ::read(kbdFD, buf, 80);

    for ( int loop = 0; loop < n; loop++ ) {
        key_code = (unsigned short)buf[loop];
        key_code2 = 0;
        unicode = 0xffff;
        modifiers = 0;
        qLog(Input) << "keypressed: code=" << key_code << " (" << (((key_code & 0x80)==0) ? "Down":"Up") << ")";
        Phkey = key_code & 0x7F;

        switch(Phkey)
        {
            case 0x2e: key_code2 = Qt::Key_0; unicode  = 0x30; break;
            case 0x02: key_code2 = Qt::Key_1; unicode  = 0x31; break;
            case 0x03: key_code2 = Qt::Key_2; unicode  = 0x32; break;
            case 0x04: key_code2 = Qt::Key_3; unicode  = 0x33; break;
            case 0x05: key_code2 = Qt::Key_4; unicode  = 0x34; break;
            case 0x06: key_code2 = Qt::Key_5; unicode  = 0x35; break;
            case 0x08: key_code2 = Qt::Key_6; unicode  = 0x36; break;
            case 0x09: key_code2 = Qt::Key_7; unicode  = 0x37; break;
            case 0x0a: key_code2 = Qt::Key_8; unicode  = 0x38; break;
            case 0x0b: key_code2 = Qt::Key_9; unicode  = 0x39; break;

            case 0x1e:
                       key_code2 = Qt::Key_Asterisk;
                       unicode  = 0x2A;
                       break;
            case 0x20:
                       key_code2 = Qt::Key_NumberSign;
                       unicode  = 0x23;
                       break;

            case 0x32:
                       key_code2 = Qt::Key_Call;
                       unicode = 0xffff;
                       break;
            case 0x16:
                       key_code2 = Qt::Key_Hangup;
                       unicode = 0xffff;
                       break;

            case 0x19:
                       key_code2 = Qt::Key_Context1;
                       unicode = 0xffff;
                       break;
            case 0x26:
                       key_code2 = Qt::Key_Back;
                       unicode = 0xffff;
                       break;

            case 0x12:
                       key_code2 = Qt::Key_Up;
                       unicode = 0xffff;
                       break;
            case 0x24:
                       key_code2 = Qt::Key_Down;
                       unicode = 0xffff;
                       break;
            case 0x21:
                       key_code2 = Qt::Key_Left;
                       unicode = 0xffff;
                       break;
            case 0x17:
                       key_code2 = Qt::Key_Right;
                       unicode = 0xffff;
                       break;

            case 0x22:
                       key_code2 = Qt::Key_Select;
                       unicode = 0xffff;
                       break;

            // Keys on left hand side of device
            // Volume Up
            case 0x07:
                       key_code2 = Qt::Key_F5;
                       unicode = 0xffff;
                       break;
            // Volume Down
            case 0x14: key_code2 = Qt::Key_F6;
                       unicode = 0xffff;
                       break;

            // Keys on right hand side of device
            // Key +
            case 0x31:
                       key_code2 = Qt::Key_F7;
                       unicode = 0xffff;
                       break;
            // Key -
            case 0x30: key_code2 = Qt::Key_F8;
                       unicode = 0xffff;
                       break;
            // Camera
            case 0x23: key_code2 = Qt::Key_F4;
                       unicode = 0xffff;
                       break;

            // Lock key on top of device
            case 0x36:
                       key_code2 = Qt::Key_F29;
                       unicode = 0xffff;
                       break;

            // Key on headphones
            case 0x33:
                       key_code2 = Qt::Key_F28;
                       unicode = 0xffff;
                       break;
        }

        qLog(Input) << "processKeyEvent(): key=" << key_code2 << ", unicode=" << unicode;
        processKeyEvent(unicode, key_code2, (Qt::KeyboardModifiers)modifiers, !(key_code & 0x80), false);
        if((key_code & 0x80) == 0) {
            // Down
            beginAutoRepeat(unicode, key_code2, (Qt::KeyboardModifiers)modifiers);
        } else {
            // Released
            endAutoRepeat();
        }
    }
}

#endif // QT_QWS_GREENPHONE
