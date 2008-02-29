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

#ifndef MODEMEMULATORSERVICE_H
#define MODEMEMULATORSERVICE_H

#include <qtopiaabstractservice.h>

class AtSessionManager;
class QValueSpaceObject;

class ModemEmulatorService : public QtopiaAbstractService
{
    Q_OBJECT
public:
    ModemEmulatorService( AtSessionManager *parent );
    ~ModemEmulatorService();

public slots:
    void addSerialPort( const QString& deviceName );
    void addSerialPort( const QString& deviceName, const QString& options );
    void addTcpPort( int tcpPort, bool localHostOnly );
    void addTcpPort( int tcpPort, bool localHostOnly, const QString& options );
    void removeSerialPort( const QString& deviceName );
    void removeTcpPort( int tcpPort );

private slots:
    void updateValueSpace();

private:
    AtSessionManager *sessions;
    QValueSpaceObject *info;
    int pendingTcpPort;
    QString pendingSerialPort;
};

#endif
