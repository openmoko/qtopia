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

#ifndef QPOWER_H
#define QPOWER_H

#include <qtopiaglobal.h>
#include <QObject>

class QTOPIABASE_EXPORT QPowerStatus
{
public:
    QPowerStatus() {
        ac = Offline;
        bs = NotPresent;
        bbs = NotPresent;
        percentRemain = -1;
        secsRemain = -1;
        percentAccurate = false;
    }
    QPowerStatus(const QPowerStatus &other)
        : ac(other.ac), bs(other.bs), bbs(other.bbs),
          percentRemain(other.percentRemain), secsRemain(other.secsRemain),
          percentAccurate(other.percentAccurate) {}

    enum ACStatus { Offline, Online, Backup, Unknown };
    ACStatus acStatus() const { return ac; }

    enum BatteryStatus { High=0x01, Low=0x02, VeryLow=0x04, Critical=0x08,
                         Charging=0x10, NotPresent=0x20 };
    BatteryStatus batteryStatus() const { return bs; }
    BatteryStatus backupBatteryStatus() const { return bbs; }

    bool batteryPercentAccurate() const { return percentAccurate; }
    int batteryPercentRemaining() const { return percentRemain; }
    int batteryTimeRemaining() const { return secsRemain; }

    bool operator!=( const QPowerStatus &ps ) {
        return  (ps.ac != ac) || (ps.bs != bs) || (ps.bbs != bbs) ||
                (ps.percentRemain != percentRemain) ||
                (ps.secsRemain != secsRemain );
    }

private:
    ACStatus ac;
    BatteryStatus bs;
    BatteryStatus bbs;
    int percentRemain;
    int secsRemain;
    bool percentAccurate;

    friend class QPowerStatusManager;
};


class QTOPIABASE_EXPORT QPowerStatusManager : public QObject
{
Q_OBJECT
public:
    explicit QPowerStatusManager(QObject *parent = 0);

    static QPowerStatus readStatus();
    static bool APMEnabled();

signals:
    void powerStatusChanged(const QPowerStatus &newStatus);

protected:
    virtual void connectNotify(const char *);
    virtual void timerEvent(QTimerEvent *);

    static bool getProcApmStatus(int &ac, int &bs, int &bf, int &pc, int &sec);
    static void getStatus();

private:
    static QPowerStatus *ps;
    QPowerStatus oldStatus;
    int timerId;
};

QTOPIABASE_EXPORT int qpe_sysBrightnessSteps();
QTOPIABASE_EXPORT void qpe_setBrightness(int bright);

#endif

