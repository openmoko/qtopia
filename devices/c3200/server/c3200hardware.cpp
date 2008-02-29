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

#ifdef QT_QWS_C3200

#include "c3200hardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>

#include <qwindowsystem_qws.h>

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

struct detect_device_t {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
};

QTOPIA_TASK(C3200Hardware, C3200Hardware);

C3200Hardware::C3200Hardware()
      :vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree")
{
    //StandardDeviceFeatures::disableBatteryMonitor();
    detectFd = ::open("/dev/input/event0", O_RDONLY | O_NDELAY, 0);
    if (detectFd >= 0) {
        qLog(Hardware) << "Opened keypad as detect input";
        m_notifyDetect = new QSocketNotifier(detectFd, QSocketNotifier::Read, this);
        connect(m_notifyDetect, SIGNAL(activated(int)), this, SLOT(readDetectData()));
    } else {
        qWarning("Cannot open log for detect (%s)", strerror(errno));
    }
}

C3200Hardware::~C3200Hardware()
{
    if (detectFd >= 0) {
        ::close(detectFd);
        detectFd = -1;
    }
}

void C3200Hardware::readDetectData()
{
    detect_device_t detectData;

    int n = read(detectFd, &detectData, sizeof(detectData));
    if(n !=16)
      return;

    if((detectData.type==0) && (detectData.code==0) && (detectData.value==0)) 
        return;
    if(detectData.type==1) return;
    qWarning("event: type=%03d code=%03d value=%03d",detectData.type, 
              detectData.code,detectData.value);
  
    if((detectData.type==5) && (detectData.code==2) && (detectData.value==1)) {
        qLog(Hardware) << "Headset plugged in";
	vsoPortableHandsfree.setAttribute("Present", true);
        system("amixer set \'Speaker Function\' Off");
        system("amixer set \'Jack Function\' \'Headset\'");
        return;  
    } 
    if((detectData.type==5) && (detectData.code==2) && (detectData.value==0)) {
        qLog(Hardware) << "Headset unplugged";
	vsoPortableHandsfree.setAttribute("Present", false);
        system("amixer set \'Speaker Function\' On");
        system("amixer set \'Jack Function\' \'Headphone\'");
        return;  
    } 
    if((detectData.type==5) && (detectData.code==1) && (detectData.value==1)) {
        qLog(Hardware) << "Screen closed";
        system("echo 1>/sys/class/backlight/corgi-bl/brightness");
        return;  
    }
    if((detectData.type==5) && (detectData.code==1) && (detectData.value==0)) {
        qLog(Hardware) << "Screen opened";
        system("echo 10>/sys/class/backlight/corgi-bl/brightness");
        QWSServer::instance()->refresh();
        return;  
    }
}
void C3200Hardware::shutdownRequested()
{
    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}
#endif // QT_QWS_C3200
