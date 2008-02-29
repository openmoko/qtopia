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

#ifndef _QABSTRACTCALLPOLICYMANAGER_H_
#define _QABSTRACTCALLPOLICYMANAGER_H_

#include <QObject>
#include <qtelephonynamespace.h>
#include "qtopiaserverapplication.h"

class QAbstractCallPolicyManager : public QObject
{
    Q_OBJECT
public:
    QAbstractCallPolicyManager(QObject *parent = 0) : QObject(parent) {}

    enum CallHandling
    {
        CannotHandle,
        CanHandle,
        MustHandle,
        NeverHandle
    };

    virtual QString callType() const = 0;
    virtual QString trCallType() const = 0;
    virtual QString callTypeIcon() const = 0;
    virtual QTelephony::RegistrationState registrationState() const = 0;
    virtual QAbstractCallPolicyManager::CallHandling handling(const QString& number) = 0;
    virtual bool isAvailable(const QString& number) = 0;
    virtual QString registrationMessage() const = 0;
    virtual QString registrationIcon() const = 0;

signals:
    void registrationChanged(QTelephony::RegistrationState state);
};

QTOPIA_TASK_INTERFACE(QAbstractCallPolicyManager);

#endif // _QABSTRACTCALLPOLICYMANAGER_H_
