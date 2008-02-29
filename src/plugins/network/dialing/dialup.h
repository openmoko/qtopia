/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef DIALUPIMPL_H
#define DIALUPIMPL_H

#include <qtopianetworkinterface.h>
#include <qtopianetwork.h>
#include <qvaluespace.h>
#include <scriptthread.h>

#ifdef QTOPIA_CELL
#include <qphonecallmanager.h>
#include <qnetworkregistration.h>
#include <qcommservicemanager.h>
#endif

class DialupImpl : public QtopiaNetworkInterface
{
    Q_OBJECT
public:
    DialupImpl( const QString& confFile );
    virtual ~DialupImpl();

    virtual Status status();

    virtual void initialize();
    virtual void cleanup();
    virtual bool start( const QVariant options = QVariant() );
    virtual bool stop();
    virtual QString device() const;
    virtual bool setDefaultGateway();

    virtual QtopiaNetwork::Type type() const;

    virtual QtopiaNetworkConfiguration * configuration();

    virtual void setProperties(
            const QtopiaNetworkProperties& properties);

protected:
    bool isAvailable() const;
    bool isActive() const;

    void timerEvent( QTimerEvent* e );
private:
    enum { Initialize, Connect, Monitoring, Disappearing } state;

private:
    void updateTrigger( QtopiaNetworkInterface::Error code = QtopiaNetworkInterface::NoError, const QString& desc = QString() );

private:
    QtopiaNetworkConfiguration *configIface;
    Status ifaceStatus;
    mutable QString deviceName;
    QString pppIface;

    int tidStateUpdate;
    int logIndex;
    int trigger;

private slots:
    void updateState();
#ifdef QTOPIA_CELL
    void connectNotification( const QPhoneCall&, QPhoneCall::Notification, const QString& );
    void registrationStateChanged();
    void phoneCallStateChanged( const QPhoneCall& );
private:
    QTelephony::RegistrationState regState;
    QCommServiceManager* commManager;
    QPhoneCallManager* callManager;
    QPhoneCall dataCall;
    QNetworkRegistration *netReg;
    bool pppdProcessBlocked;
#endif
private:
    QValueSpaceObject* netSpace;
    ScriptThread thread;
    bool delayedGatewayInstall;
};

#endif
