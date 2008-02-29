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

#ifndef __QBLUETOOTHPASSKEYAGENT_H__
#define __QBLUETOOTHPASSKEYAGENT_H__

#include <QObject>

#include <qtopiaglobal.h>
class QBluetoothPasskeyAgent_Private;
class QBluetoothLocalDevice;
class QBluetoothPasskeyRequest;
class QString;
class QBluetoothAddress;

class QTOPIACOMM_EXPORT QBluetoothPasskeyAgent : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothPasskeyAgent(const QString &name, QObject *parent = 0);
    virtual ~QBluetoothPasskeyAgent();

    QString name() const;

    virtual void requestPasskey(QBluetoothPasskeyRequest &req) = 0;
    virtual void cancelRequest(const QString &localDevice,
                               const QBluetoothAddress &remoteAddr);
    virtual void release();

    enum Error {
        NoError,
        AlreadyExists,
        DoesNotExist,
        UnknownAddress,
        UnknownError };

    QBluetoothPasskeyAgent::Error lastError() const;

    bool registerDefault();
    bool unregisterDefault();
    bool registerDefault(const QString &localDevice);
    bool unregisterDefault(const QString &localDevice);

    bool registerForAddress(QBluetoothAddress &addr);
    bool unregisterForAddress(QBluetoothAddress &addr);
    bool registerForAddress(const QString &localDevice, QBluetoothAddress &addr);
    bool unregisterForAddress(const QString &localDevice, QBluetoothAddress &addr);

private:
    QBluetoothPasskeyAgent_Private *m_data;
    Q_DISABLE_COPY(QBluetoothPasskeyAgent)
};

#endif
