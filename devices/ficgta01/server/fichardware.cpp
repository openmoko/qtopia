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

#include <linux/input.h>

struct Ficgta01Input {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

QTOPIA_TASK(Ficgta01Hardware, Ficgta01Hardware);

Ficgta01Hardware::Ficgta01Hardware()
    : m_vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree")
{
    qWarning() << "ficgta01hardware plugin";

    m_vsoPortableHandsfree.setAttribute("Present", false);

    m_detectFd = ::open("/dev/input/event0", O_RDONLY|O_NDELAY, 0);
    if (m_detectFd >= 0) {
      m_auxNotify = new QSocketNotifier(m_detectFd, QSocketNotifier::Read, this);
      connect(m_auxNotify, SIGNAL(activated(int)), this, SLOT(readAuxKbdData()));
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

    int n = read(m_detectFd, &event, sizeof(Ficgta01Input));
    if(n != (int)sizeof(Ficgta01Input) || event.type != EV_SW)
        return;

    qWarning("keypressed: type=%03d, code=%03d, value=%03d (%s)",
              event.type, event.code,event.value,((event.value)!=0) ? "Down":"Up");

    // Only handle the headphone insert. value=0 (Up,Insert), value=1 (Down,Remove)
    switch(event.code) {
    case SW_HEADPHONE_INSERT:
        m_vsoPortableHandsfree.setAttribute("Present", event.value != 0x01);
        m_vsoPortableHandsfree.sync();
        break;
    default:
        break;
    };
}


void Ficgta01Hardware::shutdownRequested()
{
    qLog(PowerManagement) << __PRETTY_FUNCTION__;

    QtopiaIpcEnvelope e("QPE/AudioVolumeManager/Ficgta01VolumeService", "setAmpMode(bool)");
    e << false;

    QFile powerFile;
    QFile btPower;

    // Detect if we have a gta01 or gta02.
    if (QFileInfo("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on").exists()) {
        powerFile.setFileName("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/gta01-pm-bt.0/power_on");
    } else {
        powerFile.setFileName("/sys/bus/platform/devices/neo1973-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/neo1973-pm-bt.0/power_on");
    }

    if(!powerFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "File not opened";
    } else {
        QTextStream out(&powerFile);
        out << "0";
        powerFile.close();
    }

    if(!btPower.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "File not opened";
    } else {
        QTextStream out(&btPower);
        out <<  "0";
        powerFile.close();
    }

    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

#endif // QT_QWS_Ficgta01

