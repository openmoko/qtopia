/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef SIPTELEPHONYSERVICE_H
#define SIPTELEPHONYSERVICE_H

#include <qtelephonyservice.h>
#include <qpresence.h>
#include <qnetworkregistration.h>
#include <qtelephonyconfiguration.h>
#include <qphonecallprovider.h>
#include "dissipate2/sipregister.h"

class SipLayer;
class SipCallProvider;

class SipTelephonyService : public QTelephonyService
{
    Q_OBJECT
public:
    SipTelephonyService( const QString& service, SipLayer *layer,
                         QObject *parent = 0 );
    ~SipTelephonyService();

    void initialize();

    SipLayer *layer() const { return _layer; }

private:
    SipLayer *_layer;
};

class SipPresence : public QPresence
{
    Q_OBJECT
public:
    SipPresence( SipTelephonyService *service );
    ~SipPresence();

public slots:
    bool startMonitoring( const QString& uri );
    bool stopMonitoring( const QString& uri );
    void setLocalPresence( QPresence::Status status );

private slots:
    void monitorChanged( const QString& uri, bool available );

private:
    SipTelephonyService *service;
};

class SipNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    SipNetworkRegistration( SipTelephonyService *service );
    ~SipNetworkRegistration();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id,
                             const QString& technology );
    void requestAvailableOperators();

private slots:
    void registrationChanged( SipRegister::RegisterState state );

private:
    SipTelephonyService *service;
};

class SipConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    SipConfiguration( SipTelephonyService *service );
    ~SipConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private:
    SipTelephonyService *service;
};

class SipCallObject : public QPhoneCallImpl
{
    Q_OBJECT
public:
    SipCallObject( SipCallProvider *provider, const QString& identifier, const QString& callType );
    virtual ~SipCallObject();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private slots:
    void stateChanged( const QString& id, QPhoneCall::State state,
                       const QString& uri );
    void callingName( const QString& id, const QString& uri );
    void remoteHold( const QString& id, const QString& value );

private:
    SipTelephonyService *service;
};

class SipCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
public:
    SipCallProvider( SipTelephonyService *service );
    ~SipCallProvider();

    SipTelephonyService *service() const { return _service; }

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );

private slots:
    void stateChanged( const QString& id, QPhoneCall::State state,
                       const QString& uri );

private:
    SipTelephonyService *_service;
};

#endif /* SIPTELEPHONYSERVICE_H */
