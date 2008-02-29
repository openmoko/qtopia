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
#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <qdplugindefs.h>

#include <QObject>

class ConnectionManagerPrivate;

class ConnectionManager : public QObject
{
    Q_OBJECT
public:
    ConnectionManager( QObject *parent = 0 );
    ~ConnectionManager();

    QDDevPlugin *currentDevice();

    enum TimerAction {
        TryConnect,
        WaitForLink,
        WaitForCon,
    };

    void stop();
    int state();

signals:
    void setConnectionState( int state );

private slots:
    void setConnectionState( QDConPlugin *connection, int state );
    void setLinkState( QDLinkPlugin *link, int state );

private:
    void timerEvent( QTimerEvent *e );
    void nextLink();
    void nextConnection();

    ConnectionManagerPrivate *d;
};

#endif
