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

#ifndef __MEDIASERVER_DOMAINMANAGERSERVICE_H
#define __MEDIASERVER_DOMAINMANAGERSERVICE_H

#include <QtopiaAbstractService>
#include <qaudiodomain.h>

#include "domainmanager.h"

class DomainManagerServicePrivate;

class DomainManagerService :
    public QtopiaAbstractService,
    public mediaserver::DomainManagerCallback
{
    Q_OBJECT

public:
    DomainManagerService();
    ~DomainManagerService();

    void domainChange(QStringList const& newDomains, QStringList const& oldDomains);

public slots:
    void activateDomain(QString const& name, QAudioCapabilities capabilities);
    void deactivateDomain(QString const& name);

signals:
    void activeDomainChanged(QStringList const& activeDomains,
                             QStringList const& inactiveDomains);

private:
    DomainManagerServicePrivate*  d;
};


#endif  // __MEDIASERVER_DOMAINMANAGERSERVICE_H
