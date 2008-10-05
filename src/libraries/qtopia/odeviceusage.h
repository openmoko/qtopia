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

#ifndef ODEVICEUSAGE_H
#define ODEVICEUSAGE_H

#include <qtopiaglobal.h>
#include <QObject>

class ODevicePrivate;

/**
 * Openmoko device usage. Talks to ompowerd and maybe later to the
 * FSO frameworkd
 */
class QTOPIA_EXPORT ODeviceUsage : public QObject {
    Q_OBJECT
public:
    ODeviceUsage(const QString& domain, const QString& user, QObject* parent);
    ~ODeviceUsage();

    void lock();
    void unlock();

private:
    void doLock();
    void doUnlock();

private:
    QString m_domain;
    QString m_user;
    int m_lockCount;
    ODevicePrivate* d;
};

#endif
