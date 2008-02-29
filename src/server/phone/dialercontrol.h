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

#ifndef DIALERCONTROL_H
#define DIALERCONTROL_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

#include <qvaluespace.h>
#include <qphonecallmanager.h>
#include <qcalllist.h>
#include <QPhoneProfileManager>
class DialerControlPrivate;
class QCBSMessage;
class QDialOptions;

// Will be renamed CallControl
class DialerControl : public QObject
{
    Q_OBJECT
public:
    static DialerControl * instance();

    virtual ~DialerControl();

    // Check for various phone states.
    bool hasIncomingCall() const;
    bool isDialing() const;

    QList<QPhoneCall> allCalls() const;
    QList<QPhoneCall> activeCalls() const;
    QList<QPhoneCall> callsOnHold() const;
    uint activeCallsCount() const;
    uint callsOnHoldCount() const;
    bool hasActiveCalls() const;
    bool hasCallsOnHold() const;
    bool isConnected() const;

    bool callerIdNeeded(const QString &n) const;

    // Get incoming number
    QPhoneCall incomingCall() const;

    QPhoneCallManager &callManager() const;
    QCallList &callList();

    // Missed call stuff
    int missedCallCount() const;
public slots:
    void resetMissedCalls();
signals:
    void missedCount(int);
    void activeCount(int);

    // Raised when a change of state occurs.
    void stateChanged();

    void pendingTonesChanged( const QPhoneCall &call );
    void requestFailed(const QPhoneCall &,QPhoneCall::Request);

    //signals for each event of a call
    void callCreated( const QPhoneCall &call );
    void callMissed( const QPhoneCall &call );
    void callDropped( const QPhoneCall &call );
    void callPutOnHold( const QPhoneCall &call );
    void callConnected( const QPhoneCall &call );
    void callIncoming( const QPhoneCall &call );
    void callDialing( const QPhoneCall &call );

    void autoAnswerCall();

    void modifyDial( QDialOptions& options, bool& handledAlready );

public slots:
    QPhoneCall createCall( const QString& callType = "Voice" ); // No tr

    //Dials a number if there are no currently active calls
    void dial( const QString &number, bool sendcallerid, const QString& callType = "Voice", const QUniqueId &contact = QUniqueId() );       // No tr

    // End the current call.
    void endCall();

    // End a call with a specific modem identifier.
    void endCall( int id );

    // Activate a call with a specific modem identifier.
    void activateCall( int id );

    // End all calls.
    void endAllCalls();

    // End the held call.
    void endHeldCalls();

    // Accept an incoming connection.
    void accept();

    // Put the current connection on hold.
    void hold();

    // Make the connections on hold active, putting the
    // active connections (if any) on hold.
    void unhold();

    // Join the active and on hold connections to create a
    // three way multi-party conference call.
    void join();

    // Send "busy" to an incoming call to indicate that the
    // user does not want to answer it.  "endCall()" will also
    // do this if there is no active calls.
    void sendBusy();

    // Join the active and on hold connections and hangup.
    void transfer();

    // Deflect the incoming call to a new number.
    void deflect( const QString& number );

    void recordCall( const QPhoneCall &call );

protected:
    virtual void timerEvent(QTimerEvent *);
private:
    QList<QPhoneCall> findCalls(QPhoneCall::State st) const;
    QPhoneCall active() const;
    void missedCall(const QPhoneCall &);
    DialerControl();

    QCallList mCallList;
    QPhoneCallManager *mCallManager;
    int aaTid;
    QPhoneProfileManager *mProfiles;

    int missedCalls;
    QValueSpaceObject phoneValueSpace;
    void doActiveCalls();
    int activeCallCount;

private slots:
    void newCall( const QPhoneCall& call );
    void callStateChanged( const QPhoneCall& call );
};

#endif
