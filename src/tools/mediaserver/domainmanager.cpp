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

#include <QList>
#include <QStringList>
#include <QSettings>
#include <QValueSpaceObject>

#include <qtopialog.h>

#include "domainmanager.h"


namespace mediaserver
{

// {{{ DomainManagerCallback
DomainManagerCallback::~DomainManagerCallback()
{
}
// }}}

// {{{ DomainManagerPrivate
class DomainManagerPrivate
{
public:
    QList<DomainManagerCallback*>   callbacks;
    QMap<QString, int>              availableDomains;
    QMap<QString, int>              activeDomains;
    QStringList                     inactiveDomains;
    QValueSpaceObject*              status;
};
// }}}

// {{{ DomainManager
DomainManager::~DomainManager()
{
    delete d->status;
    delete d;
}

bool DomainManager::activateDomain(QString const& name, QAudioCapabilities capabilities)
{
    Q_UNUSED(capabilities);

    qLog(Media) << "DomainManager::activateDomain()" << name;

    bool rc = true;

    QMap<QString, int>::iterator it = d->availableDomains.find(name);

    if (it != d->availableDomains.end())
    {
        int requestedPriority = it.value();

        // Check if something with more priority isn't already active
        for (it = d->activeDomains.begin(); it != d->activeDomains.end(); ++it)
        {
            if (d->availableDomains[it.key()] < requestedPriority)
            {
                rc = false;
                break;
            }
        }

        if (rc)
        {
            // Deactivate those with less priority
            for (it = d->activeDomains.begin(); it != d->activeDomains.end(); ++it)
            {
                if (requestedPriority < d->availableDomains[it.key()])
                {
                    if (!d->inactiveDomains.contains(it.key()))
                        d->inactiveDomains.append(it.key());
                }
            }
        }

        d->activeDomains[name]++;

        d->status->setAttribute("ActiveDomains", QStringList(d->activeDomains.keys()));
        d->status->setAttribute("InactiveDomains", d->inactiveDomains);

        foreach (DomainManagerCallback* callback, d->callbacks)
        {
            callback->domainChange(d->activeDomains.keys(),  d->inactiveDomains);
        }
    }
    else
        rc = false;

    return rc;
}

void DomainManager::deactivateDomain(QString const& name)
{
    qLog(Media) << "DomainManager::deactivateDomain()" << name;

    QMap<QString, int>::iterator it = d->activeDomains.find(name);

    if (it != d->activeDomains.end())
    {
        if (--it.value() == 0)
        {
            d->inactiveDomains.removeAll(name);
            d->activeDomains.erase(it);

            // { Check if domain(s) should be reactivated
            // Grab highest priority from active
            int targetPriority = 21;
            QMap<QString, int>::iterator it;
            for (it = d->activeDomains.begin(); it != d->activeDomains.end(); ++it)
            {
                int pri = d->availableDomains[name];
                if (pri < targetPriority)
                    pri = targetPriority;
            }

            // from inactive
            foreach (QString const& name, d->inactiveDomains)
            {
                int pri = d->availableDomains[name];
                if (pri < targetPriority)
                    targetPriority = pri;
            }

            // Activate
            foreach (QString const& name, d->inactiveDomains)
            {
                if (d->availableDomains[name] <= targetPriority)
                    d->inactiveDomains.removeAll(name);
            }
            // }

            d->status->setAttribute("ActiveDomains", QStringList(d->activeDomains.keys()));
            d->status->setAttribute("InactiveDomains", d->inactiveDomains);

            foreach (DomainManagerCallback* callback, d->callbacks)
            {
                callback->domainChange(d->activeDomains.keys(),  d->inactiveDomains);
            }
        }
    }
}

bool DomainManager::isActiveDomain(QString const& name)
{
    return d->activeDomains.find(name) != d->activeDomains.end() &&
           !d->inactiveDomains.contains(name);
}

int DomainManager::priorityForDomain(QString const& name)
{
    return d->availableDomains[name];
}

QStringList DomainManager::availableDomains()
{
    return d->availableDomains.keys();
}

QStringList DomainManager::activeDomains()
{
    return d->activeDomains.keys();
}

QStringList DomainManager::inactiveDomains()
{
    return d->inactiveDomains;
}

void DomainManager::addCallback(DomainManagerCallback* callback)
{
    d->callbacks.append(callback);
}

void DomainManager::removeCallback(DomainManagerCallback* callback)
{
    d->callbacks.removeAll(callback);
}

DomainManager* DomainManager::instance()
{
    static DomainManager   self;

    return &self;
}

// private
DomainManager::DomainManager():
    d(new DomainManagerPrivate)
{
    d->status = new QValueSpaceObject("/Media/Domains");

    // Read the domains from configuration
    QSettings    settings("Trolltech", "AudioDomains");

    settings.beginGroup("AvailableDomains");

    foreach (QString const& key, settings.allKeys())
    {
        d->availableDomains.insert(key, settings.value(key).toInt());
    }

    d->status->setAttribute("Available", settings.allKeys());
}
// }}}


}   // ns mediaserver



