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
#ifndef __PAIRINGAGENT_H__
#define __PAIRINGAGENT_H__

#include <qbluetoothaddress.h>
#include <QObject>

class QBluetoothLocalDevice;

class PairingAgent : public QObject
{
    Q_OBJECT

public:
    PairingAgent(QBluetoothLocalDevice *local, QObject *parent = 0);
    ~PairingAgent();

    void start(const QBluetoothAddress &remoteAddress);
    inline bool wasCanceled() const { return m_canceled; }
    inline QBluetoothAddress remoteAddress() const { return m_address; }

signals:
    void done(bool error);

public slots:
    void cancel();

private slots:
    void pairingCreated(const QBluetoothAddress &addr);
    void pairingFailed(const QBluetoothAddress &addr);
    void beginPairing();

private:
    void finish(bool error);

    QBluetoothLocalDevice *m_local;
    QBluetoothAddress m_address;
    bool m_running;
    bool m_canceled;
    bool m_delayedPairing;

    Q_DISABLE_COPY(PairingAgent)
};

#endif
