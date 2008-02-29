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

#include "ficgta01kbdhandler.h"

#ifdef QT_QWS_FICGTA01
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QtCore/QDebug>


#include <QtopiaServiceRequest>
#include <QValueSpaceObject>
#include <QValueSpaceItem>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <QProcess>

struct Ficgta01Input {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

Ficgta01KbdHandler::Ficgta01KbdHandler()
{

    qWarning( "Loaded Ficgta01 keypad plugin");
    setObjectName( "Ficgta01 Keypad Handler" );
//event1 is ts

    kbdFDaux = ::open("/dev/input/event0", O_RDONLY, 0);
    int err=0;

    if (kbdFDaux >= 0) {
      auxNotify = new QSocketNotifier( kbdFDaux, QSocketNotifier::Read, this );
      connect( auxNotify, SIGNAL(activated(int)), this, SLOT(readAuxKbdData()));
    } else {
      qWarning("Cannot open /dev/input/event0 for keypad (%s)", strerror(errno));
      err = 1;
    }

    kbdFDpower = ::open("/dev/input/event2", O_RDONLY, 0);
    if (kbdFDpower >= 0) {
      powerNotify = new QSocketNotifier( kbdFDpower, QSocketNotifier::Read, this );
      connect( powerNotify, SIGNAL(activated(int)), this, SLOT(readPowerKbdData()));
    } else {
      qWarning("Cannot open /dev/input/event2 for keypad (%s)", strerror(errno));
      err = 1;
    }

    if(err !=0)
        return;
    shift = false;

}

/*Ficgta01KbdHandler::~Ficgta01KbdHandler()
{
}
*/
void Ficgta01KbdHandler::readAuxKbdData()
{

    Ficgta01Input event;
bool useRepeat=true;

    int n = read(kbdFDaux, &event, sizeof(Ficgta01Input));
    if(n != (int)sizeof(Ficgta01Input)) {
//      qWarning("A keypressed: n=%03d",n);
     return;
    }

    if(event.type == 0)
        return;

    int unicode = 0xffff;
    bool isPress = (event.value);
    int qtKeyCode = 0;

         qWarning("keypressed AUX: type=%03d, code=%03d, value=%03d (%s)",
              event.type, event.code,event.value,((event.value)!=0) ? "Down":"Up");
//x14a

    switch(event.code)
    {
    case 0xA9:
        qtKeyCode = Qt::Key_F7;
        break;

    case 0x77: // ??
        //  type=001, code=119, value=001 (Down)
        return; //drop any other bits
        break;
    default:
        //       qWarning("Dropped!");
        return; //drop any other bits
        break;
    }


     processKeyEvent(unicode, qtKeyCode, Qt::NoModifier, isPress, false);

    if(isPress!=0 && event.code != 0x02 && useRepeat) {
        //      qWarning("Start manual repeat");
        beginAutoRepeat(unicode, qtKeyCode, Qt::NoModifier);
    } else {
        //       qWarning("End manual repeat");
        if(!useRepeat)
            endAutoRepeat();
        else
            useRepeat = true;
    }
}

void Ficgta01KbdHandler::readPowerKbdData()
{
    Ficgta01Input event;

    int n = read(kbdFDpower, &event, sizeof(Ficgta01Input));
    if(n != (int)sizeof(Ficgta01Input)) {
//      qWarning("P keypressed: n=%03d",n);
      return;
    }

    int unicode = 0xffff;
    int qtKeyCode = 0;
    bool isPress = (event.value);

   qWarning("keypressed power: type=%03d, code=%03d, value=%03d (%s)",
            event.type, event.code,event.value,((event.value)!=0) ? "Down":"Up");

    switch(event.code)
    {
    case 0x74:
        qtKeyCode = Qt::Key_Hangup;
        break;
    default:
        //       qWarning("Dropped!");
        return; //drop any other bits
        break;
    }



    processKeyEvent(unicode, qtKeyCode, Qt::NoModifier, isPress, false);

    if(isPress!=0) {
        //       qWarning("Start manual repeat");
        beginAutoRepeat(unicode, qtKeyCode, Qt::NoModifier);
    } else {
//        qWarning("End manual repeat");
        endAutoRepeat();
    }
}




#endif // QT_QWS_FICGTA01
