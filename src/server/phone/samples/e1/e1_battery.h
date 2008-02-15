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

#ifndef _E1_BATTERY_H_
#define _E1_BATTERY_H_

#include <QObject>
#include <QValueSpaceItem>

class E1Battery : public QObject
{
Q_OBJECT
public:
    E1Battery(QObject *parent = 0);
    virtual ~E1Battery();

    int charge();

signals:
    void chargeChanged(int);

protected:
    virtual void timerEvent(QTimerEvent *);

private slots:
    void batteryChanged();

private:
    void startcharge();
    void stopcharge();
    int m_timerId;
    QValueSpaceItem m_battery;
    int m_charge;
};

#endif // _E1_BATTERY_H_

