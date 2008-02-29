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

#ifndef LANIMPL_H
#define LANIMPL_H

#include <qtopianetworkinterface.h>
#include <qtopianetwork.h>
#include <qvaluespace.h>
#include <scriptthread.h>

#ifndef NO_WIRELESS_LAN
class RoamingMonitor;
#endif

class LanImpl : public QtopiaNetworkInterface
{
    Q_OBJECT
public:
    LanImpl( const QString& confFile );
    virtual ~LanImpl();

    virtual Status status();

    virtual void initialize();
    virtual void cleanup();
    virtual bool start( const QVariant options );
    virtual bool stop();
    virtual QString device() const;
    virtual bool setDefaultGateway();

    virtual QtopiaNetwork::Type type() const;

    virtual QtopiaNetworkConfiguration * configuration();

    virtual void setProperties(
            const QtopiaNetworkProperties& properties);

protected:
    bool isAvailable() const;
    bool isPCMCIADevice( const QString& dev) const;
    bool isActive() const;

private slots:
#ifndef NO_WIRELESS_LAN
    void reconnectWLAN();
#endif
    void updateState();

private:
    void installDNS(bool);
    void updateTrigger( QtopiaNetworkInterface::Error code = QtopiaNetworkInterface::NoError, const QString& desc = QString() );

    QtopiaNetworkConfiguration *configIface;
    Status ifaceStatus;
    mutable QString deviceName;

#ifndef NO_WIRELESS_LAN
    RoamingMonitor* roaming;
#endif
    QValueSpaceObject *netSpace;
    ScriptThread thread;
    bool delayedGatewayInstall;
    int trigger;
};

#endif
