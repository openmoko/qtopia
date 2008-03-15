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

#ifndef N800HARDWARE_H
#define N800HARDWARE_H

#ifdef QT_QWS_N800

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>

class QSocketNotifier;
class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class N800Hardware : public QObject
{
    Q_OBJECT

public:
    N800Hardware();
    ~N800Hardware();

private:
    bool charging;
    int percent;
    int visualCharge;
    int chargeId;

    QValueSpaceObject vsoPortableHandsfree;

    QSocketNotifier *m_notifyDetect;
    int detectFd;

    QProcess *mountProc;
    QString sdCardDevice;


    QBootSourceAccessoryProvider *bootSource;

    QPowerSourceProvider *batterySource;
    QPowerSourceProvider *wallSource;

    void setLeds(int charge);

private slots:

    void shutdownRequested();

    void chargingChanged(bool charging);
    void chargeChanged(int charge);
};

#endif // QT_QWS_N800

#endif // N800HARDWARE_H
