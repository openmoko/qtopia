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

#include "s3c24a0kbdhandler.h"

#ifdef QT_QWS_S3C24A0
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

struct S3c24a0Input {
    unsigned short key;
    unsigned short status;
};

S3c24a0KbdHandler::S3c24a0KbdHandler()
{
    qWarning( "***Loaded S3c24a0 keypad plugin!");
    setObjectName( "S3c24a0 Keypad Handler" );
    kbdFD = ::open("/dev/keypad/0", O_RDONLY, 0);
    if (kbdFD >= 0) {
      qWarning("Opened /dev/keypad/0 as keypad input");
      m_notify = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
      connect( m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    } else {
      qWarning("Cannot open /dev/keypad/0 for keypad (%s)", strerror(errno));
      return;
    }
    shift=false;
}

S3c24a0KbdHandler::~S3c24a0KbdHandler()
{
}

void S3c24a0KbdHandler::readKbdData()
{
    S3c24a0Input event;

    int n = read(kbdFD, &event, sizeof(S3c24a0Input));
    if(n != (int)sizeof(S3c24a0Input)) {
      qWarning("keypressed: n=%03d",n);
      return;
    }  

    qWarning("keypressed: key=%03d, status=%03d",event.key, event.status);

    int modifiers=0;
    int unicode=0xffff;
    int key_code=0;

    switch(event.key)
    {
      case 0x00: //SW1
        key_code = Qt::Key_Context1;
        unicode  = 0xffff; 
        break;
      case 0x01: //SW2
        key_code = Qt::Key_Context1;
        unicode  = 0xffff;
        break;
      case 0x02: //SW3
        key_code = Qt::Key_Up;
        unicode  = 0xffff;
        break;
      case 0x03: //SW4
        key_code = Qt::Key_Back;
        unicode  = 0xffff;
        break;
      case 0x04: //SW5
        key_code = Qt::Key_Back;
        unicode  = 0xffff;
        break;
      case 0x05: //SW6
        key_code = Qt::Key_Call;
        unicode  = 0xffff; 
        break;
      case 0x06: //SW7
        key_code = Qt::Key_Left;
        unicode  = 0xffff; 
        break;
      case 0x07: //SW8
        key_code = Qt::Key_Select;
        unicode  = 0xffff;
        break;
      case 0x08: //SW9
        key_code = Qt::Key_Right;
        unicode  = 0xffff;
        break;
      case 0x09: //SW10
        key_code = Qt::Key_Hangup;
        unicode  = 0xffff;
        break;
      case 0x0A: //SW11
        key_code = Qt::Key_Home;
        unicode  = 0xffff;
        break;
      case 0x0B: //SW12
        key_code = Qt::Key_F1;
        unicode  = 0xffff; 
        break;
      case 0x0C: //SW13
        key_code = Qt::Key_Down; 
        unicode  = 0xffff; 
        break;
      case 0x0D: //SW14
        key_code = Qt::Key_Asterisk;
        unicode  = 0x2A; 
        break;
      case 0x0E: //SW15
        key_code = Qt::Key_NumberSign; 
        unicode  = 0x23; 
        break;
      case 0x0F: //SW16
        key_code = Qt::Key_0;
        unicode  = 0x30; 
        break;
      case 0x10: //SW17
        key_code = Qt::Key_1;
        unicode  = 0x31;
        break;
      case 0x11: //SW18
        key_code = Qt::Key_2;
        unicode  = 0x32;
        break;
      case 0x12: //SW19
        key_code = Qt::Key_3;
        unicode  = 0x33;
        break;
      case 0x13: //SW20
        key_code = Qt::Key_4;
        unicode  = 0x34;
        break;
      case 0x14: //SW21
        key_code = Qt::Key_5;
        unicode  = 0x35;
        break;
      case 0x15: //SW22
        key_code = Qt::Key_6;
        unicode  = 0x36;
        break;
      case 0x16: //SW23
        key_code = Qt::Key_7;
        unicode  = 0x37;
        break;
      case 0x17: //SW24
        key_code = Qt::Key_8;
        unicode  = 0x38;
        break;
      case 0x18: //SW25
        key_code = Qt::Key_9;
        unicode  = 0x39;
        break;
    }
    processKeyEvent(unicode, key_code, (Qt::KeyboardModifiers)modifiers,
        event.status!=0, false);

    if(event.status!=0) {
      beginAutoRepeat(unicode, key_code, (Qt::KeyboardModifiers)modifiers);
    } else {
      endAutoRepeat();
    }
}

#endif // QT_QWS_S3C24A0
