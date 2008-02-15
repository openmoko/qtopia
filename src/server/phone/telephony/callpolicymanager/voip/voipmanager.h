/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _VOIPMANAGER_H_
#define _VOIPMANAGER_H_

#include <QObject>
#include <qnetworkregistration.h>
#include <qpresence.h>
#include <qcommservicemanager.h>
#include "qabstractcallpolicymanager.h"

class VoIPManagerPrivate;

class VoIPManager : public QAbstractCallPolicyManager
{
    Q_OBJECT
public:
    VoIPManager(QObject *parent=0);
    static VoIPManager * instance();

    QString callType() const;
    QString trCallType() const;
    QString callTypeIcon() const;
    QAbstractCallPolicyManager::CallHandling handling(const QString& number);
    QString registrationMessage() const;
    QString registrationIcon() const;

    QTelephony::RegistrationState registrationState() const;
    QPresence::Status localPresence() const;
    void startMonitoring();
    bool isAvailable(const QString &uri);

signals:
    void localPresenceChanged(QPresence::Status);
    void monitoredPresenceChanged(const QString&, bool available);

private slots:
    void registrationStateChanged();
    void localPresenceChanged();
    void monitoredPresence(const QString&, QPresence::Status);
    void servicesChanged();
    void hideMessageTimeout();

private:
    VoIPManagerPrivate *d;

    void serviceStarted();
    void serviceStopped();
};

QTOPIA_TASK_INTERFACE(VoIPManager);

#endif // _VOIPMANAGER_H_
