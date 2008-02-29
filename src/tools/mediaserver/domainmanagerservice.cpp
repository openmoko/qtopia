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


#include "domainmanagerservice.h"

using namespace mediaserver;

#define DOMAIN_MANAGER_CHANNEL  "DomainManager"

class DomainManagerServicePrivate
{
public:
    DomainManager*  domainManager;
};


DomainManagerService::DomainManagerService():
    QtopiaAbstractService(DOMAIN_MANAGER_CHANNEL),
    d(new DomainManagerServicePrivate)
{
    d->domainManager = DomainManager::instance();

    d->domainManager->addCallback(this);
}

DomainManagerService::~DomainManagerService()
{
    d->domainManager->removeCallback(this);

    delete d;
}

void DomainManagerService::domainChange(QStringList const& activeDomains, QStringList const& inactiveDomains)
{
    emit activeDomainChanged(activeDomains, inactiveDomains);
}

//public slots:
void DomainManagerService::activateDomain(QString const& name, QAudioCapabilities capabilities)
{
    d->domainManager->activateDomain(name, capabilities);
}

void DomainManagerService::deactivateDomain(QString const& name)
{
    d->domainManager->deactivateDomain(name);
}

