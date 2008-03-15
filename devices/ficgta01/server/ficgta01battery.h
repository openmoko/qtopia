/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _FICGTA01BATTERY_H_
#define _FICGTA01BATTERY_H_

#ifndef QT_ILLUME_LAUNCHER

#include <QObject>
#include <QSocketNotifier>

class QPowerSourceProvider;


class Ficgta01Battery : public QObject
{
Q_OBJECT
public:
    Ficgta01Battery(QObject *parent = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    bool APMEnabled() const;
    int percentCharge;

    QPowerSourceProvider *ac;
    QPowerSourceProvider *battery;
    QPowerSourceProvider *backup;

    QSocketNotifier *powerNotify;
    int  kbdFDpower;
    void openDetect();
    int getBatteryLevel();
    bool batteryIsFull();

private Q_SLOTS:
    
    void updateFicStatus();
    void apmFileChanged(const QString & file);
    void readPowerKbdData();
    
};

#endif

#endif // _FICGTA01BATTERY_H_
