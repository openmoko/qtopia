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

#ifndef QPHONECALL_P_H
#define QPHONECALL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qphonecall.h>
#include <qphonecallprovider.h>

class QPhoneCallManagerPrivate;
class QtopiaIpcAdaptor;
class QTimer;

class QPhoneCallPrivate : public QObject
{
    Q_OBJECT
public:
    QPhoneCallPrivate( QPhoneCallManagerPrivate *manager,
                       const QString& service, const QString& type,
                       const QString& identifier );
    ~QPhoneCallPrivate();

    void ref() { ++count; }
    bool deref() { return !--count; }

public:
    // Emit a "stateChanged" signal for this call.
    void emitStateChanged();

    // Emit a "requestFailed" signal for this call.
    void emitRequestFailed( QPhoneCall::Request request );

    // Emit a "notification" signal for this call.
    void emitNotification( QPhoneCall::Notification type, const QString& value );

    void addPendingTones( const QString &tones );

private slots:
    void callStateChanged( const QString& identifier, QPhoneCall::State state,
                           const QString& number, const QString& service,
                           const QString& callType, int actions );
    void callRequestFailed
        ( const QString& identifier, QPhoneCall::Request request );
    void callNotification
        ( const QString& identifier, QPhoneCall::Notification type,
          const QString& value );
    void callFloorChanged
        ( const QString& identifier, bool haveFloor, bool floorAvailable );
    void callDataPort( const QString& identifier, int port );
    void sendPendingTones();

signals:
    void stateChanged( const QPhoneCall& conv );
    void floorChanged( const QPhoneCall& conv );
    void requestFailed( const QPhoneCall& conv, QPhoneCall::Request request );
    void pendingTonesChanged( const QPhoneCall &call );
    void notification( const QPhoneCall& conv, QPhoneCall::Notification type, const QString& value );

public:
    QPhoneCallManagerPrivate *manager;
    QString identifier;

    QTimer *toneTimer;

    QDateTime startTime, connectTime, endTime;
    QString fullNumber;
    QString number;
    QPhoneCall::State state;
    QPhoneCallImpl::Actions actions;
    QString service;
    QString callType;
    QUniqueId contact;

    QString pendingTones;
    bool hasBeenConnected;
    bool dialedCall;
    bool sentAllTones;
    bool haveFloor;
    bool floorAvailable;

    QIODevice *device;
    int dataPort;

    QtopiaIpcAdaptor *request;
    QtopiaIpcAdaptor *response;

    uint count;
};

#endif // QPHONECALL_P_H
