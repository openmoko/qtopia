/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifdef QT_QWS_N810

#include "n800hardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QFile>
#include <QtGlobal>
#include <QStringList>
#include <QResizeEvent>

#include <QDesktopWidget>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>
#include <qpowersource.h>
#include <QMessageBox>

#include <QtopiaServiceRequest>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

#if defined(QTOPIA_DBUS_IPC)
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbusargument.h>
#include <qtdbus/qdbusinterface.h>

#endif

/*
/system/osso/dsm/display
/system/osso/dsm/leds

cat /sys/devices/platform/gpio-switch/headphone/state
disconnected


*/

QTOPIA_TASK(N800Hardware, N800Hardware);

N800Hardware::N800Hardware()
    : charging(false), percent(-1), chargeId(0),
        vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"), mountProc(NULL)
{

    bootSource = new QBootSourceAccessoryProvider( "N800", this );

    batterySource = new QPowerSourceProvider(QPowerSource::Battery, "N800Battery", this);
    batterySource->setAvailability(QPowerSource::Available);

    connect(batterySource, SIGNAL(chargingChanged(bool)),
            this, SLOT(chargingChanged(bool)));
    connect(batterySource, SIGNAL(chargeChanged(int)),
            this, SLOT(chargeChanged(int)));

    wallSource = new QPowerSourceProvider(QPowerSource::Wall, "N800Charger", this);

    QDBusConnection dbc = QDBusConnection::systemBus();

    if (!dbc.isConnected()) {
        qWarning() << "Unable to connect to D-BUS:" << dbc.lastError();
        return;
    }

    dbc.connect(QString(), "/org/freedesktop/Hal/devices/platform_slide",
                "org.freedesktop.Hal.Device", "Condition",
                this, SIGNAL(getCoverProperty(QString,QString)));

    getCoverProperty(QString(),  "cover");
}

N800Hardware::~N800Hardware()
{
    if (detectFd >= 0) {
        ::close(detectFd);
        detectFd = -1;
    }
}

void N800Hardware::chargingChanged(bool charging)
{
    if (charging)
        setLeds(101);
    else
        setLeds(batterySource->charge());
}

void N800Hardware::chargeChanged(int charge)
{
    if (!batterySource->charging())
        setLeds(charge);
}

void N800Hardware::setLeds(int charge)
{
    Q_UNUSED(charge);

}

void N800Hardware::shutdownRequested()
{
    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

void N800Hardware::getCoverProperty(QString str,  QString str2)
{
    if (str2 == "cover") {

        QString slideState;

        QFile slideStateFile("/sys/devices/platform/gpio-switch/slide/state");
        slideStateFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QTextStream in(&slideStateFile);
        in >> slideState;
        slideStateFile.close();

        bool okToFlip = true;

        if (okToFlip) {

            if ( slideState == "closed") {

                QtopiaServiceRequest svreq("RotationManager", "setCurrentRotation(int)");
                svreq << 270;
                svreq.send();
            } else {
// cover open rotate 0
                QtopiaServiceRequest svreq("RotationManager", "defaultRotation()");
                svreq.send();
            }

        }
    }
}



#endif // QT_QWS_N80
