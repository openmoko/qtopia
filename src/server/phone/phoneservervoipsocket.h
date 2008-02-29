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

#ifndef PHONESERVERVOIP_H
#define PHONESERVERVOIP_H

#include <qtelephonyservice.h>
#include <qphonecallprovider.h>
#include <qtelephonyconfiguration.h>
#include <qnetworkregistration.h>
#include <qpresence.h>
#include <qmap.h>

class QPhoneSocket;
class PhoneServerVoIPService;
class PhoneServerVoIPCallProvider;

class PhoneServerVoIPCall : public QPhoneCallImpl
{
    Q_OBJECT
    friend class PhoneServerVoIPCallProvider;
public:
    PhoneServerVoIPCall( PhoneServerVoIPCallProvider *provider, const QString& callType, const QString& identifier );
    virtual ~PhoneServerVoIPCall();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private:
    void emitNotification( QPhoneCall::Notification type, const QString& value )
    {
        emit notification( type, value );
    }

    QPhoneSocket *handler() const;

    void send( const QString& cmd );
    void send( const QString& cmd, const QString& arg1 );
    void send( const QString& cmd, const QString& arg1, const QString& arg2 );
    void send( const QString& cmd, const QString& arg1, const QString& arg2,
               const QString& arg3 );
};

class PhoneServerVoIPCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
public:
    PhoneServerVoIPCallProvider( PhoneServerVoIPService *service );
    ~PhoneServerVoIPCallProvider();

    PhoneServerVoIPService *service() const { return _service; }

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );

private slots:
    void received( const QStringList& cmd );

private:
    PhoneServerVoIPService *_service;
};

class PhoneServerVoIPConfiguration : public QTelephonyConfiguration
{
    Q_OBJECT
public:
    PhoneServerVoIPConfiguration( PhoneServerVoIPService *service );
    ~PhoneServerVoIPConfiguration();

public slots:
    void update( const QString& name, const QString& value );
    void request( const QString& name );

private slots:
    void received( const QStringList& cmd );

private:
    PhoneServerVoIPService *service;
};

class PhoneServerVoIPNetworkRegistration : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    PhoneServerVoIPNetworkRegistration( PhoneServerVoIPService *service );
    ~PhoneServerVoIPNetworkRegistration();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id,
                             const QString& technology );
    void requestAvailableOperators();

private slots:
    void received( const QStringList& cmd );

private:
    PhoneServerVoIPService *service;
    QTelephony::RegistrationState registrationState;
    QString operatorName;

    void updateOperatorInfo();
};

class PhoneServerVoIPPresence : public QPresence
{
    Q_OBJECT
public:
    PhoneServerVoIPPresence( PhoneServerVoIPService *service );
    ~PhoneServerVoIPPresence();

public slots:
    bool startMonitoring( const QString& uri );
    bool stopMonitoring( const QString& uri );
    void setLocalPresence( QPresence::Status status );

private slots:
    void received( const QStringList& cmd );

private:
    PhoneServerVoIPService *service;

    void setPresence( QPresence::Status status );
};

class PhoneServerVoIPService : public QTelephonyService
{
    Q_OBJECT
public:
    PhoneServerVoIPService( const QString& service, QObject *parent = 0 );
    ~PhoneServerVoIPService();

    void initialize();

    QPhoneSocket *handler() const { return _handler; }

    static bool handlerPresent();

signals:
    void handlerStarted();
    void handlerReceived( const QStringList& cmd );
    void handlerClosed();

private slots:
    void testCommand( const QStringList& cmd );
    void incoming( QPhoneSocket *socket );
    void closed();
    void launchHandler();

private:
    QPhoneSocket *server;
    QPhoneSocket *_handler;
    QStringList supportedCommands;
    QStringList ourCommands;
};

#endif /* PHONESERVERVOIP_H */
