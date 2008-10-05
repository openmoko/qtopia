/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#include "odeviceusage.h"

#include <QDBusConnection>
#include <QDBusMessage>

ODeviceUsage::ODeviceUsage(const QString& domain, const QString& user, QObject* parent)
    : QObject(parent)
    , m_domain(domain)
    , m_user(user)
    , m_lockCount(0)
{}

ODeviceUsage::~ODeviceUsage()
{
    doUnlock();
}

void ODeviceUsage::lock()
{
    if (m_lockCount++ != 0)
        return;

    doLock();
}

void ODeviceUsage::unlock()
{
    // be friendly to avoid cover up unbalanced lock/unlock
    if (m_lockCount == 0)
        return;

    if (--m_lockCount != 0)
        return;

    doUnlock();
}

void ODeviceUsage::doLock()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.openmoko.Power", "/",
            "org.openmoko.Power.Core", "RequestResourceState");
    message << m_domain;
    message << m_user;
    message << QLatin1String("on");
    QDBusConnection::systemBus().send(message);
}

void ODeviceUsage::doUnlock()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.openmoko.Power", "/",
            "org.openmoko.Power.Core", "RemoveRequestedResourceState");
    message << m_domain;
    message << m_user;
    QDBusConnection::systemBus().send(message);
}
