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

#ifndef __BTPINHELPER_H__
#define __BTPINHELPER_H__

#include <qbluetoothpasskeyagent.h>

class QPasswordDialog;

class BTPinHelper : public QBluetoothPasskeyAgent {
    Q_OBJECT
public:
    BTPinHelper(QObject *parent = 0);
    ~BTPinHelper();

    virtual void requestPasskey(QBluetoothPasskeyRequest &req);
    virtual void cancelRequest(const QString &localDevice,
                               const QBluetoothAddress &remoteAddr);
    virtual void release();

private slots:
    void pairingCreated(const QBluetoothAddress &addr);

private:
    QPasswordDialog *m_passDialog;
};

#endif
