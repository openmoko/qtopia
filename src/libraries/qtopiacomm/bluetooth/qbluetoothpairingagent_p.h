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
#ifndef __QBLUETOOTHQBluetoothPairingAgent_P_H__
#define __QBLUETOOTHQBluetoothPairingAgent_P_H__

#include <qbluetoothaddress.h>

#include <QObject>

class QWaitWidget;
class QBluetoothLocalDevice;

class QBluetoothPairingAgent : public QObject
{
    Q_OBJECT
public:
    enum Result {
        Success,
        Failed,
        Cancelled
    };

    enum Operation {
        Pair,
        Unpair
    };

    QBluetoothPairingAgent( Operation op, const QBluetoothAddress &addr, const QBluetoothLocalDevice &local, QObject *parent = 0 );
    ~QBluetoothPairingAgent();

    static QBluetoothPairingAgent::Result pair( const QBluetoothAddress &addr, const QBluetoothLocalDevice &local );
    static QBluetoothPairingAgent::Result unpair( const QBluetoothAddress &addr, const QBluetoothLocalDevice &local );

    QBluetoothAddress address() const;
    Operation operation() const;
    QBluetoothPairingAgent::Result result() const;

    void showErrorDialog();

public slots:
    void start();   // asynchronous
    QBluetoothPairingAgent::Result exec();    // synchronous

private slots:
    void startPairing();
    void pairingFailed( const QBluetoothAddress &addr );
    void pairingCompleted( const QBluetoothAddress &addr );
    void unpairingCompleted( const QBluetoothAddress &addr );

    void waitWidgetCancelled();

signals:
    void pairingCompleted( const QBluetoothAddress &addr,
                           QBluetoothPairingAgent::Result result );
    void unpairingCompleted( const QBluetoothAddress &addr,
                             QBluetoothPairingAgent::Result result );

private:
    void finished( Result result );

    QBluetoothLocalDevice *m_local;
    QBluetoothAddress m_addr;
    Operation m_op;
    QWaitWidget *m_waitWidget;
    Result m_result;
    bool m_busy;
    bool m_delayedStart;
};


#endif
