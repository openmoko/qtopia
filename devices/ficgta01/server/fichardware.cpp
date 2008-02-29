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

#ifdef QT_QWS_FICGTA01

#include "fichardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>
#include <standarddevicefeatures.h>
#include <ui/standarddialogs.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

struct Ficgta01Input {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

QTOPIA_TASK(Ficgta01Hardware, Ficgta01Hardware);

Ficgta01Hardware::Ficgta01Hardware()
      :vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree")
{

    qWarning()<<"ficgta01hardware plugin";

    vsoPortableHandsfree.setAttribute("Present", false);


    detectFd = ::open("/dev/input/event0", O_RDONLY|O_NDELAY, 0);
     if (detectFd >= 0) {
      auxNotify = new QSocketNotifier( detectFd, QSocketNotifier::Read, this );
      connect( auxNotify, SIGNAL(activated(int)), this, SLOT(readAuxKbdData()));
    } else {
      qWarning("Cannot open /dev/input/event0 for keypad (%s)", strerror(errno));
    }
}

Ficgta01Hardware::~Ficgta01Hardware()
{
}

void Ficgta01Hardware::readAuxKbdData()
{

    Ficgta01Input event;

    int n = read(detectFd, &event, sizeof(Ficgta01Input));
    if(n != (int)sizeof(Ficgta01Input)) {
        return;
    }

    if(event.type != 5)
        return;

    switch(event.code) {
    case 0x02: //headphone insert
    {
        //  type=005, code=002, value=000 (Up) //insert
        //  type=005, code=002, value=001 (Down) //out

        if(event.value != 1) {
            vsoPortableHandsfree.setAttribute("Present", true);
            vsoPortableHandsfree.sync();
        } else {
            vsoPortableHandsfree.setAttribute("Present", false);
            vsoPortableHandsfree.sync();
        }
    }
    break;
    };
}


void Ficgta01Hardware::shutdownRequested()
{
    qLog(PowerManagement)<<" Ficgta01Hardware::shutdownRequested";


    QFile powerFile("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on");
    if( !powerFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerFile);
        out << "0";
        powerFile.close();
    }

    QFile btPower("/sys/bus/platform/devices/gta01-pm-bt.0/power_on");
        if( !btPower.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&btPower);
        out <<  "0";
        powerFile.close();
    }

    system( "amixer -q sset \"Amp Mode\" \"Off\"");


    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

#endif // QT_QWS_Ficgta01

