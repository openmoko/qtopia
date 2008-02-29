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

#ifndef _ASTERISKMANAGER_H_
#define _ASTERISKMANAGER_H_

#include <QObject>
#include <qnetworkregistration.h>
#include <qcommservicemanager.h>
#include "qabstractcallpolicymanager.h"

class AsteriskManagerPrivate;

class AsteriskManager : public QAbstractCallPolicyManager
{
    Q_OBJECT
public:
    AsteriskManager( QObject *parent=0 );
    ~AsteriskManager();

    QString callType() const;
    QString trCallType() const;
    QString callTypeIcon() const;
    QTelephony::RegistrationState registrationState() const;
    QAbstractCallPolicyManager::CallHandling handling(const QString& number);
    bool isAvailable(const QString& number);
    QString registrationMessage() const;
    QString registrationIcon() const;

private slots:
    void registrationStateChanged();
    void servicesChanged();

private:
    AsteriskManagerPrivate *d;

    void serviceStarted();
    void serviceStopped();
};

QTOPIA_TASK_INTERFACE(AsteriskManager);

#endif // _ASTERISKMANAGER_H_
