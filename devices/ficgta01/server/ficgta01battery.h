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
class QValueSpaceItem;

class Ficgta01Battery : public QObject
{
Q_OBJECT
public:
    Ficgta01Battery(QObject *parent = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    int percentCharge;
    bool charging;
    bool cableEnabled;

    QPowerSourceProvider *ac;
    QPowerSourceProvider *backup;
    QPowerSourceProvider *battery;

    int getBatteryLevel();
    bool batteryIsFull();
    QValueSpaceItem *vsUsbCable;
    
private Q_SLOTS:
    
    void updateFicStatus();

    void cableChanged();
    void checkChargeState();
    
};

#endif

#endif // _FICGTA01BATTERY_H_
