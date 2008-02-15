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

#ifndef PHONESTATUS_H
#define PHONESTATUS_H

#include <qtopiaglobal.h>

#include <QObject>

class QPhoneStatusPrivate;

class QTOPIAPHONE_EXPORT QPhoneStatus : public QObject
{
    Q_OBJECT
public:
    explicit QPhoneStatus(QObject *parent);
    ~QPhoneStatus();

    enum StatusItem {
        // int values
        BatteryLevel=1,
        SignalLevel=2,
        MissedCalls=3,
        NewMessages=4,
        ActiveCalls=5,

        // string values
        OperatorName=100,
        Profile=101,

        // bool values
        Roaming=200,
        Locked=201,
        Alarm=202,
        CallDivert=203,
        NetworkRegistered=204,
        VoIPRegistered=205,
        Presence=206
    };

    QVariant value(StatusItem);

signals:
    void incomingCall(const QString &number, const QString &name);
    void statusChanged();

private slots:
    void phoneStatusChanged();

private:
    QPhoneStatusPrivate *d;
};

#endif
