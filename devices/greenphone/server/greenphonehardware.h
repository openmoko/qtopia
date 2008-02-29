/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef GREENPHONEHARDWARE_H
#define GREENPHONEHARDWARE_H

#ifdef QT_QWS_GREENPHONE

#include <QObject>

#include <qvaluespace.h>

class QSocketNotifier;
class QProcess;
class QtopiaIpcAdaptor;
class QSpeakerPhoneAccessoryProvider;
class QBootSourceAccessoryProvider;

class GreenphoneHardware : public QObject
{
    Q_OBJECT

public:
    GreenphoneHardware();
    ~GreenphoneHardware();

private:
    QValueSpaceObject vsoBattery;
    bool charging;
    int percent;
    int visualCharge;
    int chargeId;

    QValueSpaceObject vsoPortableHandsfree;

    QSocketNotifier *m_notifyDetect;
    int detectFd;

    QProcess *mountProc;

    QtopiaIpcAdaptor *adaptor;

    QSpeakerPhoneAccessoryProvider *speakerPhone;
    QBootSourceAccessoryProvider *bootSource;

    void setCharging(bool charge);
    void setBatteryLevel(int level);
    void setLeds();

    void forceShutdown();

    void timerEvent(QTimerEvent *e);

private slots:
    void readDetectData(quint32 devices = 0);

    void delayedRead();

    void shutdownRequested();

    void mountSD();
    void unmountSD();

    void onSpeakerModified();
};

#endif // QT_QWS_GREENPHONE

#endif // GREENPHONEHARDWARE_H
