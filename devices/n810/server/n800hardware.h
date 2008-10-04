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

#ifndef N800HARDWARE_H
#define N800HARDWARE_H

#ifdef QT_QWS_N810

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>
#include <QDBusConnection>
#include <QDBusInterface>

class QSocketNotifier;
class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class N800Hardware : public QObject
{
    Q_OBJECT

public:
    N800Hardware();
    ~N800Hardware();
    QDBusInterface *m_iface;
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
    void getCoverProperty(QString, QString);
};

#endif // QT_QWS_N800

#endif
