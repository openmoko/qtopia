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

#ifndef _NEOBATTERY_H_
#define _NEOBATTERY_H_

#include <QObject>
#include <QSocketNotifier>

class QPowerSourceProvider;
class QValueSpaceItem;

class NeoBattery : public QObject
{
Q_OBJECT
public:
    NeoBattery(QObject *parent = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    int percentCharge;
    bool charging;
    bool cableEnabled;
    bool APMEnabled() const;
 
    QPowerSourceProvider *ac;
    QPowerSourceProvider *backup;
    QPowerSourceProvider *battery;

    QValueSpaceItem *vsUsbCable;
    bool isCharging();
    
private Q_SLOTS:
    
    void updateApmStatus();

    void cableChanged();
    void checkChargeState();
    void apmFileChanged(const QString & file);
};

#endif // _NEOBATTERY_H_
