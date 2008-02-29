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

#ifndef IAXCALLPROVIDER_H
#define IAXCALLPROVIDER_H

#include <qphonecallprovider.h>

#define IAXAGENT_MAX_CALLS      16

class IaxCallProvider;
class IaxTelephonyService;

class IaxPhoneCall : public QPhoneCallImpl
{
    Q_OBJECT
public:
    IaxPhoneCall( IaxCallProvider *provider, const QString& identifier,
                  const QString& callType, int callNo );
    virtual ~IaxPhoneCall();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void tone( const QString& tones );
    void transfer( const QString& number );

    void stateEvent( struct iaxc_ev_call_state *e );

public:
    IaxCallProvider *provider;
    int callNo;
};

class IaxCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
public:
    IaxCallProvider( IaxTelephonyService *service );
    ~IaxCallProvider();

    void stateEvent( struct iaxc_ev_call_state *e );
    void putActiveOnHold();
    void endStateTransaction();
    void updateCallerIdConfig();
    QString callUri() const;

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );

private:
    IaxTelephonyService *iaxservice;

    IaxPhoneCall *fromCallNo( int callNo );
};

#endif // IAXCALLPROVIDER_H
