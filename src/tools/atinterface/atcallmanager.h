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

#ifndef ATCALLMANAGER_H
#define ATCALLMANAGER_H

#include <qphonecallmanager.h>
#include <qatresult.h>

class AtCallManagerPrivate;
class AtCommands;

class AtCallManager : public QObject
{
    Q_OBJECT
public:
    AtCallManager( QObject *parent = 0 );
    ~AtCallManager();

    // Broken out call states for reporting to higher layers.
    enum CallState
    {
        CallIdle,
        CallCalling,
        CallConnecting,
        CallActive,
        CallHold,
        CallWaiting,
        CallAlerting,
        CallBusy
    };

    // Result code that indicates that a response will be deferred.
    static const QAtResult::ResultCode Defer = (QAtResult::ResultCode)(-100);

    void setHandler( AtCommands *handler );

    QAtResult::ResultCode dial( const QString& dialString );
    QAtResult::ResultCode accept();
    QAtResult::ResultCode online();
    QAtResult::ResultCode hangup();
    QAtResult::ResultCode hangup( int callID );
    QAtResult::ResultCode hangupHeldCalls();
    QAtResult::ResultCode activateHeldCalls();
    QAtResult::ResultCode activate( int callID );
    QAtResult::ResultCode join();
    QAtResult::ResultCode transfer();
    QAtResult::ResultCode tone( const QString& value );

    QStringList formatCallList();

    bool ringing() const;
    bool callInProgress() const;

    static int clccCallType( const QString& callType );
    static int numCallType( const QString& callType );
    static QString strCallType( const QString& callType );

    enum CallSetup
    {
        NoCallSetup,
        IncomingCallSetup,
        OutgoingCallSetup,
        AlertingCallSetup
    };

    enum CallHoldState
    {
        NoCallsHeld,
        CallsActiveAndHeld,
        CallsHeldOnly
    };

signals:
    void stateChanged( int callID, AtCallManager::CallState state,
                       const QString& number, const QString& type );
    void deferredResult( AtCommands *handler, QAtResult::ResultCode result );
    void ring( const QString& number, const QString& type );
    void callWaiting( const QString& number, const QString& type );
    void noCarrier();
    void setOnCall( bool value );
    void setCallSetup( AtCallManager::CallSetup callSetup );
    void setCallHold( AtCallManager::CallHoldState callHold );

private slots:
    void newCall( QPhoneCall call );
    void callStateChanged( const QPhoneCall& call );
    void repeatRing();

private:
    AtCallManagerPrivate *d;
};

#endif // ATCALLMANAGER_H
