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

#ifndef ATSESSIONMANAGER_H
#define ATSESSIONMANAGER_H

#include "atfrontend.h"
#include <qstringlist.h>

class AtSessionManagerPrivate;
class AtCallManager;
class QSerialSocket;

class AtSessionManager : public QObject
{
    Q_OBJECT
    friend class AtCommands;
public:
    AtSessionManager( QObject *parent = 0 );
    ~AtSessionManager();

    bool addSerialPort( const QString& deviceName,
                        const QString& options = QString() );
    bool addTcpPort( int tcpPort, bool localHostOnly = true,
                     const QString& options = QString() );

    void removeSerialPort( const QString& deviceName );
    void removeTcpPort( int tcpPort );

    QStringList serialPorts() const;
    QList<int> tcpPorts() const;

    AtCallManager *callManager() const;

signals:
    void newSession( AtFrontEnd *session );
    void devicesChanged();

private slots:
    void incoming( QSerialSocket *socket );
    void serialPortDestroyed();

private:
    AtSessionManagerPrivate *d;

    void registerTaskIfNecessary();
    void unregisterTaskIfNecessary();
};

#endif // ATSESSIONMANAGER_H
