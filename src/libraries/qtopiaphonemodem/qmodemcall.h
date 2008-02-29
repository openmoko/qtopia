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

#ifndef QMODEMCALL_H
#define QMODEMCALL_H

#include <qphonecall.h>
#include <qmodemcallprovider.h>

class QSerialSocket;
class QModemCallPrivate;
class QModemDataCallPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemCall : public QPhoneCallImpl
{
    Q_OBJECT
public:
    QModemCall( QModemCallProvider *provider, const QString& identifier,
                const QString& callType );
    ~QModemCall();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

    uint modemIdentifier() const;
    void setModemIdentifier( uint id );

    QModemCallProvider *provider() const;

    void setConnected();

    void setState( QPhoneCall::State value );

private slots:
    void dialRequestDone( bool ok );
    void dialCheck();
    void clccDialCheck( bool ok, const QAtResult& result );
    void acceptDone( bool ok );
    void joinRequestDone( bool ok );
    void detachRequestDone( bool ok );
    void holdRequestDone( bool ok );
    void activateSingleRequestDone( bool ok );
    void activateMultipleRequestDone( bool ok );
    void vtsRequestDone( bool ok, const QAtResult& result );

private:
    QModemCallPrivate *d;

};

class QTOPIAPHONEMODEM_EXPORT QModemDataCall : public QModemCall
{
    Q_OBJECT
public:
    QModemDataCall( QModemCallProvider *provider, const QString& identifier,
                    const QString& callType );
    ~QModemDataCall();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private slots:
    void fclass( bool ok, const QAtResult& result );
    void selectBearer();
    void sendBearer();
    void bearerDone( bool ok );
    void testBearer( bool ok );
    void bearers( bool ok, const QAtResult& result );
    void performDial();
    void dialRequestDone( bool ok );
    void acceptDone( bool ok );
    void dtrLowered();
    void sendSwitchBack();
    void sendHangup();
    void hangupDone();
    void remoteHangupDone();
    void carrierChanged( bool value );
    void incoming( QSerialSocket *sock );
    void channelReadyRead();
    void socketReadyRead();
    void socketClosed();
    void pppCallActive();
    void pppCallInactive();
    void pppStateUpdate( QPhoneCall::DataState value );
    void raiseDtr();
    void contextSetupDone( bool ok, const QAtResult& result );

public:
    void setState( QPhoneCall::State value );

private:
    QModemDataCallPrivate *d;

    void createChannels();
    void discardInput();
    void createServer();
    void destroyServer();
};

#endif // QMODEMCALL_H
