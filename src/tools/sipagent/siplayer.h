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

#ifndef SIPLAYER_H
#define SIPLAYER_H

#include <qobject.h>
#include <qstring.h>
#include <qtimer.h>
#include <qmap.h>
#include <QNetworkInterface>
#include <qphonecall.h>

#include "dissipate2/sipclient.h"
#include "dissipate2/sipregister.h"
#include "dissipate2/sipcall.h"

class MediaSessionClient;
class QHostAddress;

class SipLayerCallInfo : public QObject
{
    Q_OBJECT
public:
    SipLayerCallInfo( const QString& id, SipCallMember *member );
    ~SipLayerCallInfo();

    QString id() const { return _id; }
    SipCallMember *member() const { return _member; }
    SipCall *call() const { return _call; }

    MediaSessionClient *session() const { return _session; }
    void setSession( MediaSessionClient *session );

    QPhoneCall::State prevState() const { return _prevState; }
    void setPrevState( QPhoneCall::State state ) { _prevState = state; }

    bool incomingInvite() const { return _incomingInvite; }
    void setIncomingInvite( bool value ) { _incomingInvite = value; }

    static SipLayerCallInfo *fromSession( MediaSessionClient *session );
    static SipLayerCallInfo *fromCall( SipCall *call );
    static SipLayerCallInfo *fromMember( SipCallMember *member );

private:
    QString _id;
    SipCallMember *_member;
    SipCall *_call;
    MediaSessionClient *_session;
    QPhoneCall::State _prevState;
    bool _incomingInvite;
};

class SipLayer : public QObject
{
    Q_OBJECT
public:
    SipLayer( QObject *parent );
    ~SipLayer();

    SipClient& client() const { return *_client; }

    bool online();
    bool busy();
    bool isRegistered();
    SipRegister::RegisterState registrationState();

public:
    void updateGeneralConfig();
    void updateRegistrationConfig();
    void registerToProxy();
    void deregisterFromProxy();
    void setOnline( bool value );
    void setBusy( bool value );

    void dial( const QString& id, const QString& uri, bool callerid );
    void accept( const QString& id );
    void hangup( const QString& id );
    void tone( const QString& id, const QString& tones );
    void hold( const QString& id );
    void activate( const QString& id );

    void startMonitor( const QString& uri );
    void stopMonitor( const QString& uri );

signals:
    void onlineChanged();
    void monitorChanged( const QString& uri, bool available );
    void registrationChanged( SipRegister::RegisterState state );
    void stateChanged( const QString& id, QPhoneCall::State state,
                       const QString& uri );
    void callingName( const QString& id, const QString& name );
    void remoteHold( const QString& id, const QString& value );

private slots:
    void callListUpdated();
    void registrationStatusUpdated();
    void delayedRegTimerTimeout();
    void handleMemberAuth( SipCallMember *member );
    void incomingSubscribe( SipCallMember *member, bool sendSubscribe );
    void incomingCall( SipCall *call, const QString& body );
    void callStatusUpdated( SipCallMember *member );
    void sendLocalParameters( const QString& sdp );
    void onHoldChanged();
    void remoteOnHoldChanged();
#if QT_VERSION < 0x040200
    void networkChanged( QString iface, QHostAddress newAddress );
#else
    void networkChanged( QString iface, const QNetworkInterface& newAddress );
#endif

private:
    SipClient *_client;
    SipRegister *registration;
    QString prevuser;
    QString prevserver;
    int atom;
    bool isOnline;
    bool isBusy;
    QStringList monitoring;
    SipRegister::RegisterState prevstate;
    QTimer *delayedRegTimer;
    int subscribeExpires;
    MediaSessionClient *activeSession;
    bool pendingUpdate;
    bool pendingDeregister;
    bool pendingRegister;

    void notify( SipCallMember *member );
    void setMonitor( const QString& uri );
    void clearMonitor( const QString& uri, bool resubscribe = false );
    void setAllMonitors();
    void clearAllMonitors();
    SipLayerCallInfo *tagCall( SipCall *call, const QString& id );
    SipLayerCallInfo *callFromIdentifier( const QString& id );
    void hangupMedia( SipLayerCallInfo *call );
#if QT_VERSION < 0x040200
    void updateAddress( const QHostAddress& address );
#else
    void updateAddress( const QNetworkInterface& address );
#endif
    bool updateRegistrationConfigInner();
    void registerToProxyInner();
};

#endif /* SIPLAYER_H */
