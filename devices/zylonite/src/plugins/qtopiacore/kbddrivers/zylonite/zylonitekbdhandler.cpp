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

#include "zylonitekbdhandler.h"

#ifdef QT_QWS_ZYLONITE
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

struct ZyloniteInput {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

ZyloniteKbdHandler::ZyloniteKbdHandler()
{
    qWarning( "***Loaded Zylonite keypad plugin!");
    setObjectName( "Zylonite Keypad Handler" );
    kbdFD = ::open("/dev/input/event1", O_RDONLY, 0);
    if (kbdFD >= 0) {
      qWarning("Opened event1 as keypad input");
      m_notify = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
      connect( m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    } else {
      qWarning("Cannot open event1 for keypad (%s)", strerror(errno));
      return;
    }
    shift=false;
}

ZyloniteKbdHandler::~ZyloniteKbdHandler()
{
}

void ZyloniteKbdHandler::readKbdData()
{
    ZyloniteInput event;

    int n = read(kbdFD, &event, sizeof(ZyloniteInput));
    if(n !=16)
      return;

    qWarning("keypressed: code=%03d (%s)",event.code,((event.value)!=0) ? "Down":"Up");

    int modifiers=0;
    int unicode=0xffff;
    int key_code=0;

    switch(event.code)
    {
      case 0xA2:
        key_code = ((!shift) ? Qt::Key_0      : Qt::Key_Plus      );
        unicode  = ((!shift) ? 0x30           : 0x2B              );
        break;
      case 0x70:
        key_code = ((!shift) ? Qt::Key_1      : Qt::Key_At        );
        unicode  = ((!shift) ? 0x31           : 0x40              );
        break;
      case 0x72:
        key_code = ((!shift) ? Qt::Key_2      : Qt::Key_Ampersand );
        unicode  = ((!shift) ? 0x32           : 0x26              );
        break;
      case 0x74:
        key_code = ((!shift) ? Qt::Key_3      : Qt::Key_At        );
        unicode  = ((!shift) ? 0x33           : 0x3F              );
        break;
      case 0x80:
        key_code = ((!shift) ? Qt::Key_4      : Qt::Key_Minus     );
        unicode  = ((!shift) ? 0x34           : 0x2D              );
        break;
      case 0x82:
        key_code = ((!shift) ? Qt::Key_5      : Qt::Key_Apostrophe);
        unicode  = ((!shift) ? 0x35           : 0x27              );
        break;
      case 0x84:
        key_code = ((!shift) ? Qt::Key_6      : Qt::Key_Slash     );
        unicode  = ((!shift) ? 0x36           : 0x5C              );
        break;
      case 0x90:
        key_code = ((!shift) ? Qt::Key_7      : Qt::Key_Colon     );
        unicode  = ((!shift) ? 0x37           : 0x3A              );
        break;
      case 0x92:
        key_code = ((!shift) ? Qt::Key_8      : Qt::Key_Semicolon );
        unicode  = ((!shift) ? 0x38           : 0x3B              );
        break;
      case 0x94:
        key_code = ((!shift) ? Qt::Key_9      : Qt::Key_QuoteDbl  );
        unicode  = ((!shift) ? 0x39           : 0x22              );
        break;
      case 0x0:
        key_code = Qt::Key_A;
        unicode  = ((!shift) ? 0x61           : 0x41              );
        break;
      case 0x10:
        key_code = Qt::Key_B;
        unicode  = ((!shift) ? 0x62           : 0x42              );
        break;
      case 0x20:
        key_code = Qt::Key_C;
        unicode  = ((!shift) ? 0x63           : 0x43              );
        break;
      case 0x30:
        key_code = Qt::Key_D;
        unicode  = ((!shift) ? 0x64           : 0x44              );
        break;
      case 0x40:
        key_code = Qt::Key_E;
        unicode  = ((!shift) ? 0x65           : 0x45              );
        break;
      case 0x50:
        key_code = Qt::Key_F;
        unicode  = ((!shift) ? 0x66           : 0x46              );
        break;
      case 0x01:
        key_code = Qt::Key_G;
        unicode  = ((!shift) ? 0x67           : 0x47              );
        break;
      case 0x11:
        key_code = Qt::Key_H;
        unicode  = ((!shift) ? 0x68           : 0x48              );
        break;
      case 0x21:
        key_code = Qt::Key_I;
        unicode  = ((!shift) ? 0x69           : 0x49              );
        break;
      case 0x31:
        key_code = Qt::Key_J;
        unicode  = ((!shift) ? 0x6A           : 0x4A              );
        break;
      case 0x41:
        key_code = Qt::Key_K;
        unicode  = ((!shift) ? 0x6B           : 0x4B              );
        break;
      case 0x51:
        key_code = Qt::Key_L;
        unicode  = ((!shift) ? 0x6C           : 0x4C              );
        break;
      case 0x02:
        key_code = Qt::Key_M;
        unicode  = ((!shift) ? 0x6D           : 0x4D              );
        break;
      case 0x12:
        key_code = Qt::Key_N;
        unicode  = ((!shift) ? 0x6E           : 0x4E              );
        break;
      case 0x22:
        key_code = Qt::Key_O;
        unicode  = ((!shift) ? 0x6F           : 0x4F              );
        break;
      case 0x32:
        key_code = Qt::Key_P;
        unicode  = ((!shift) ? 0x70           : 0x50              );
        break;
      case 0x42:
        key_code = Qt::Key_Q;
        unicode  = ((!shift) ? 0x71           : 0x51              );
        break;
      case 0x52:
        key_code = Qt::Key_R;
        unicode  = ((!shift) ? 0x72           : 0x52              );
        break;
      case 0x03:
        key_code = Qt::Key_S;
        unicode  = ((!shift) ? 0x73           : 0x53              );
        break;
      case 0x13:
        key_code = Qt::Key_T;
        unicode  = ((!shift) ? 0x74           : 0x54              );
        break;
      case 0x23:
        key_code = Qt::Key_U;
        unicode  = ((!shift) ? 0x75           : 0x55              );
        break;
      case 0x33:
        key_code = Qt::Key_V;
        unicode  = ((!shift) ? 0x76           : 0x56              );
        break;
      case 0x43:
        key_code = Qt::Key_W;
        unicode  = ((!shift) ? 0x77           : 0x57              );
        break;
      case 0x53:
        key_code = Qt::Key_X;
        unicode  = ((!shift) ? 0x78           : 0x58              );
        break;
      case 0x24:
        key_code = Qt::Key_Y;
        unicode  = ((!shift) ? 0x79           : 0x59              );
        break;
      case 0x34:
        key_code = Qt::Key_Z;
        unicode  = ((!shift) ? 0x7A           : 0x5A              );
        break;
      case 0xA4:
        key_code = ((!shift) ? Qt::Key_NumberSign : Qt::Key_Period);
        unicode  = ((!shift) ? 0x23           : 0x2E              );
        break;
      case 0xA0:
        key_code = ((!shift) ? Qt::Key_Asterisk   : Qt::Key_NumberSign );
        unicode  = ((!shift) ? 0x2A           : 0x2C              );
        break;
      case 0x25:
        key_code = Qt::Key_Space;
        unicode  = 0x20;
        break;
      case 0x06:
        key_code = Qt::Key_Up;
        unicode  = 0xffff; 
        break;
      case 0x16:
        key_code = Qt::Key_Down;
        unicode  = 0xffff; 
        break;
      case 0x26:
        key_code = Qt::Key_Left;
        unicode  = 0xffff;
        break;
      case 0x36:
        key_code = Qt::Key_Right;
        unicode  = 0xffff;
        break;
      case 0x46:
        key_code = Qt::Key_Select;
        unicode  = 0xffff;
        break;
      case 0x61:
        key_code = Qt::Key_Backspace;
        unicode  = 0xffff; 
        break;
      case 0x60:
        key_code = Qt::Key_Call;
        unicode  = 0xffff;
        break;
      case 0x55:
        key_code = Qt::Key_Hangup;
        unicode  = 0xffff;
        break;
      case 0x62:
        key_code = Qt::Key_Context1;
        unicode  = 0xffff; 
        break;
      case 0x63:
        key_code = Qt::Key_Back;
        unicode  = 0xffff;
        break;
      case 0x05:
        key_code = Qt::Key_Home;
        unicode  = 0xffff;
        break;
      case 0x15:
        key_code = Qt::Key_Shift;
        unicode  = 0xffff; modifiers |= Qt::ShiftModifier;
        if(event.value==0) break;
        if(shift) {
          shift = FALSE;
          qWarning("Caps Off!");
        } else {
          shift = TRUE;
          qWarning("Caps On!");
        }
        break;
      case 0x1C:
        key_code = ((!shift) ? Qt::Key_Backspace : Qt::Key_Enter );
        unicode  = 0xffff;
        break;
      case 0x19:
        key_code = Qt::Key_F1;
        unicode  = 0xffff; 
        break;
      case 0x1A:
        key_code = Qt::Key_F2;
        unicode  = 0xffff; 
        break;
      case 0x1B:
        key_code = Qt::Key_F3;
        unicode  = 0xffff; 
        break;
    }
    if(shift) modifiers |= Qt::ShiftModifier;
    processKeyEvent(unicode, key_code, (Qt::KeyboardModifiers)modifiers,
        event.value!=0, false);
    if(event.value!=0) {
      beginAutoRepeat(unicode, key_code, (Qt::KeyboardModifiers)modifiers);
    } else {
      endAutoRepeat();
    }
}

#endif // QT_QWS_ZYLONITE
