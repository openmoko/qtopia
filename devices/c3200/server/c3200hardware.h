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

#ifndef C3200HARDWARE_H
#define C3200HARDWARE_H

#ifdef QT_QWS_C3200

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>

class QSocketNotifier;
class QtopiaIpcAdaptor;

class C3200Hardware : public QObject
{
    Q_OBJECT

public:
    C3200Hardware();
    ~C3200Hardware();

private:
    QValueSpaceObject vsoPortableHandsfree;

    QSocketNotifier *m_notifyDetect;
    int detectFd;

private slots:
    void readDetectData();
    void shutdownRequested();
};

#endif // QT_QWS_C3200

#endif // C3200HARDWARE_H
