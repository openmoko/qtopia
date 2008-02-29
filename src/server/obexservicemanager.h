/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __OBEXSERVICEMANAGER_H__
#define __OBEXSERVICEMANAGER_H__

#include <qglobal.h>
#include <QObject>
#include <QAtomic>
#include <QMap>

#ifdef QTOPIA_BLUETOOTH
class QBluetoothObexServer;
class QSDPService;
class QSDAPSearchResult;
class QBluetoothLocalDevice;
class QSDAP;
class BluetoothPushingService;
class ObexPushServiceProvider;
#endif

class QObexPushService;
class QCommDeviceSession;
class QObexPushClient;

#ifdef QTOPIA_INFRARED
class QIrObexServer;
class InfraredBeamingService;
#endif

class ObexServiceManager : public QObject
{
    Q_OBJECT

public:
    ObexServiceManager(QObject *parent = 0);
    ~ObexServiceManager();

    void setupConnection(QObexPushService *opush);
    void setupConnection(QObexPushClient *client, const QString &filename, const QString &mimetype);

signals:
    void receiveInitiated(int id, const QString &filename, const QString &mime);
    void sendInitiated(int id, const QString &filename, const QString &mime);
    void progress(int id, qint64 bytes, qint64 total);
    void completed(int id, bool error);

private slots:
    void putRequest(const QString &filename, const QString &mimetype);
    void getRequest(const QString &filename, const QString &mimetype);
    void requestComplete(bool error);
    void progress(qint64, qint64);
    void aboutToDelete();

#ifdef QTOPIA_INFRARED
    void newIrxferConnection();
    void irSessionEnded();
#endif

private:
#ifdef QTOPIA_BLUETOOTH
    ObexPushServiceProvider *m_pushServiceProvider;
    BluetoothPushingService *m_bluetoothService;
#endif

#ifdef QTOPIA_INFRARED
    QIrObexServer *m_irxfer;

    int m_numIrSessions;
    QCommDeviceSession *m_irSession;
    InfraredBeamingService *m_infraredService;
#endif

    QMap<QObject *, int> m_map;
    static QBasicAtomic idCounter;
    static int nextId();
};

#endif
