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
#include <QtopiaIpcAdaptor>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>

#include <sys/ioctl.h>

QTOPIA_TASK(Ficgta01Hardware, Ficgta01Hardware);

Ficgta01Hardware::Ficgta01Hardware()
    : vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
      vsoUsbCable("/Hardware/UsbGadget")
{
    adaptor = new QtopiaIpcAdaptor("QPE/Neo1973Hardware");

    qLog(Hardware) << "ficgta01hardware";

    vsoPortableHandsfree.setAttribute("Present", false);
    vsoPortableHandsfree.sync();

    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(headphonesInserted(bool)),
                              this, SLOT(headphonesInserted(bool)));

    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(cableConnected(bool)),
                              this, SLOT(cableConnected(bool)));

}

Ficgta01Hardware::~Ficgta01Hardware()
{
}


void Ficgta01Hardware::headphonesInserted(bool b)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << b;
    vsoPortableHandsfree.setAttribute("Present", b);
    vsoPortableHandsfree.sync();
}

void Ficgta01Hardware::cableConnected(bool b)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << b;
    vsoUsbCable.setAttribute("cableConnected", b);
    vsoUsbCable.sync();
}



void Ficgta01Hardware::shutdownRequested()
{
    qLog(PowerManagement)<< __PRETTY_FUNCTION__;

    QFile powerFile;
    QFile btPower;

    if ( QFileInfo("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on").exists()) {
//ficgta01
        powerFile.setFileName("/sys/bus/platform/devices/gta01-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/gta01-pm-bt.0/power_on");
    } else {
//ficgta02
        powerFile.setFileName("/sys/bus/platform/devices/neo1973-pm-gsm.0/power_on");
        btPower.setFileName("/sys/bus/platform/devices/neo1973-pm-bt.0/power_on");
    }

    if( !powerFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerFile);
        out << "0";
        powerFile.close();
    }

        if( !btPower.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&btPower);
        out <<  "0";
        powerFile.close();
    }


    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

#endif // QT_QWS_Ficgta01

