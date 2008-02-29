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

#ifndef __QBLUETOOTHABSTRACTSERVER_P_H__
#define __QBLUETOOTHABSTRACTSERVER_P_H__

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

#include <QObject>

class QBluetoothSocketEngine;
class QBluetoothAbstractServer;
class QSocketNotifier;
class QBluetoothAbstractSocket;

class QBluetoothAbstractServerPrivate : public QObject
{
    Q_OBJECT

public:
    QBluetoothAbstractServerPrivate(QBluetoothAbstractServer *parent);
    ~QBluetoothAbstractServerPrivate();

    int m_fd;

    bool m_isListening;
    QString m_errorString;
    int m_maxConnections;
    QList<QBluetoothAbstractSocket *> m_pendingConnections;

    QBluetoothAbstractServer *m_parent;
    QSocketNotifier *m_readNotifier;
    QBluetoothSocketEngine *m_engine;

public slots:
    void incomingConnection();
};

#define SERVER_DATA(Class) Class##Private * const m_data = static_cast<Class##Private *>(QBluetoothAbstractServer::m_data)

#endif
