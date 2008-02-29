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

#ifndef __MEDIASERVER_DOMAINMANAGER_H
#define __MEDIASERVER_DOMAINMANAGER_H

#include <qaudiodomain.h>

namespace mediaserver
{

class DomainManagerCallback
{
public:
    virtual ~DomainManagerCallback();

    virtual void domainChange(QStringList const& activeDomains,
                              QStringList const& inactiveDomains) = 0;
};


class DomainManagerPrivate;

class DomainManager
{
public:
    ~DomainManager();

    bool activateDomain(QString const& name,
                        QAudioCapabilities capabilities = Output);
    void deactivateDomain(QString const& name);

    bool isActiveDomain(QString const& name);
    bool isActionableDomain(QString const& name);
    int priorityForDomain(QString const& name);

    QStringList availableDomains();
    QStringList activeDomains();
    QStringList inactiveDomains();

    void addCallback(DomainManagerCallback* callback);
    void removeCallback(DomainManagerCallback* callback);

    static DomainManager* instance();

private:
    DomainManager();

    DomainManagerPrivate*   d;
};

}

#endif  // __MEDIASERVER_DOMAINMANAGER_H
