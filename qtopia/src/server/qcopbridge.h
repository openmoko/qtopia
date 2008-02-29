/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __qcopbridge_h__
#define __qcopbridge_h__

#include <qtopia/qpeglobal.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qdir.h>
#include <qfile.h>
#include <qbuffer.h>

class QFileInfo;
class QCopBridgePI;
class QCopChannel;
class QTimer;

class QCopBridge : public QServerSocket
{
    Q_OBJECT

public:
    QCopBridge( Q_UINT16 port, QObject *parent = 0, const char* name = 0 );
    virtual ~QCopBridge();

    void newConnection( int socket );
    void closeOpenConnections();
    void authorizeConnections();

public slots:
    void closed( QCopBridgePI *pi );
    void desktopMessage( const QCString &call, const QByteArray & );

signals:
    void connectionClosed( const QHostAddress & );
    
protected:
    void timerEvent( QTimerEvent * );
    
private:
    QCopChannel *desktopChannel;
    QCopChannel *cardChannel;
    QList<QCopBridgePI> openConnections;
    bool sendSync;
};


class QCopBridgePI : public QSocket
{
    Q_OBJECT

    enum State { Connected, Wait_USER, Wait_PASS, Ready, Forbidden };

public:
    QCopBridgePI( int socket, QObject *parent = 0, const char* name = 0 );
    virtual ~QCopBridgePI();

    void sendDesktopMessage( const QString &msg );
    void sendDesktopMessage( const QCString &msg, const QByteArray& );
    void startSync() { sendSync = TRUE; }
    bool verifyAuthorised();

signals:
    void connectionClosed( QCopBridgePI *);
    
protected slots:
    void read();
    void send( const QString& msg );
    void process( const QString& command );
    void myConnectionClosed();

private:
    State state;
    Q_UINT16 peerport;
    QHostAddress peeraddress;
    bool sendSync;
    QTimer *timer;
};

#endif
