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

#ifndef ZYLONITEHARDWARE_H
#define ZYLONITEHARDWARE_H

#ifdef QT_QWS_ZYLONITEA1

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>

class QSocketNotifier;
class QtopiaIpcAdaptor;
class QSpeakerPhoneAccessoryProvider;

class ZyloniteHardware : public QObject
{
    Q_OBJECT

public:
    ZyloniteHardware();
    ~ZyloniteHardware();

private:
    QValueSpaceObject vsoPortableHandsfree;

    QSocketNotifier *m_notifyDetect;
    int detectFd;

    QProcess *mountProc;
    QString sdCardDevice;

    QSpeakerPhoneAccessoryProvider *speakerPhone;

private slots:
    void readDetectData();
    void shutdownRequested();

    void mountSD();
    void unmountSD();
    void fsckFinished(int, QProcess::ExitStatus);
    void mountFinished(int, QProcess::ExitStatus);
};

#endif // QT_QWS_ZYLONITEA1

#endif // ZYLONITEHARDWARE_H
