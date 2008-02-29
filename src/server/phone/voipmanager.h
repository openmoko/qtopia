/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _VOIPMANAGER_H_
#define _VOIPMANAGER_H_

#include <QObject>
#include <qnetworkregistration.h>
#include <qpresence.h>
#include <qcommservicemanager.h>

class VoIPManagerPrivate;

class VoIPManager : public QObject
{
    Q_OBJECT
public:
    static VoIPManager * instance();

    QTelephony::RegistrationState registrationState() const;
    QPresence::Status localPresence() const;
    void startMonitoring();
    bool isAvailable(const QString &uri);

signals:
    void registrationChanged(QTelephony::RegistrationState);
    void localPresenceChanged(QPresence::Status);
    void monitoredPresenceChanged(const QString&, bool available);

private slots:
    void registrationStateChanged();
    void localPresenceChanged();
    void monitoredPresence(const QString&, QPresence::Status);
    void servicesChanged();

private:
    VoIPManager();
    VoIPManagerPrivate *d;

    void serviceStarted();
    void serviceStopped();
};

#endif // _VOIPMANAGER_H_
