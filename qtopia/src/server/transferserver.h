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
#include <qserversocket.h>
#include <qsocket.h>
#include <qdir.h>
#include <qfile.h>
#include <qbuffer.h>
#include <qlist.h>

class QFileInfo;
class QProcess;
class ServerPI;

class TransferServer : public QServerSocket
{
    Q_OBJECT

public:
    TransferServer( Q_UINT16 port, QObject *parent = 0, const char* name = 0 );
    virtual ~TransferServer();

    void newConnection( int socket );
    void authorizeConnections();

protected slots:
    void closed(ServerPI *);

private:
    QList<ServerPI> connections;
};

class SyncAuthentication : QObject
{
    Q_OBJECT

public:
    static int isAuthorized(QHostAddress peeraddress);
    static bool checkPassword(const QString& pw);
    static bool checkUser(const QString& user);

    static QString serverId();
    static QString loginName();
    static QString ownerName();
};


class ServerDTP : public QSocket
{
    Q_OBJECT

public:
    ServerDTP( QObject *parent = 0, const char* name = 0 );
    ~ServerDTP();

    enum Mode{ Idle = 0, SendFile, SendGzipFile, SendBuffer,
	       RetrieveFile, RetrieveGzipFile, RetrieveBuffer };

    void sendFile( const QString fn );
    void sendFile( const QString fn, const QHostAddress& host, Q_UINT16 port );
    void sendGzipFile( const QString &fn, const QStringList &archiveTargets );
    void sendGzipFile( const QString &fn, const QStringList &archiveTargets,
		       const QHostAddress& host, Q_UINT16 port );
    void sendByteArray( const QByteArray& array );
    void sendByteArray( const QByteArray& array, const QHostAddress& host, Q_UINT16 port );

    void retrieveFile( const QString fn, int fileSize );
    void retrieveFile( const QString fn, const QHostAddress& host, Q_UINT16 port, int fileSize );
    void retrieveGzipFile( const QString &fn );
    void retrieveGzipFile( const QString &fn, const QHostAddress& host, Q_UINT16 port );
    void retrieveByteArray();
    void retrieveByteArray( const QHostAddress& host, Q_UINT16 port );

    Mode dtpMode() { return mode; }
    QByteArray buffer() { return buf.buffer(); }
    QString fileName() const { return file.name(); }

    void setSocket( int socket );

signals:
    void completed();
    void failed();

private slots:
    void connectionClosed();
    void connected();
    void bytesWritten( int bytes );
    void readyRead();
    void writeTargzBlock();
    void targzDone();
    void extractTarDone();

private:

    unsigned long bytes_written;
    Mode mode;
    QFile file;
    QBuffer buf;
    QProcess *createTargzProc;
    QProcess *retrieveTargzProc;
    int recvFileSize;
};

class ServerSocket : public QServerSocket
{
    Q_OBJECT

public:
    ServerSocket( Q_UINT16 port, QObject *parent = 0, const char* name = 0 )
	: QServerSocket( port, 1, parent, name ) {}

    void newConnection( int socket ) { emit newIncomming( socket ); }
signals:
    void newIncomming( int socket );
};

class ServerPI : public QSocket
{
    Q_OBJECT

    enum State { Connected, Wait_USER, Wait_PASS, Ready, Forbidden };
    enum Transfer { SendFile = 0, RetrieveFile = 1, SendByteArray = 2, RetrieveByteArray = 3 };

public:
    ServerPI( int socket, QObject *parent = 0, const char* name = 0 );
    virtual ~ServerPI();

    bool verifyAuthorised();

signals:
    void connectionClosed(ServerPI *);

protected slots:
    void read();
    void send( const QString& msg );
    void process( const QString& command );
    void connectionClosed();
    void dtpCompleted();
    void dtpFailed();
    void dtpError( int );
    void newConnection( int socket );

protected:
    bool checkReadFile( const QString& file );
    bool checkWriteFile( const QString& file );
    bool parsePort( const QString& pw );
    bool backupRestoreGzip( const QString &file, QStringList &targets );
    bool backupRestoreGzip( const QString &file );

    bool sendList( const QString& arg );
    void sendFile( const QString& file );
    void retrieveFile( const QString& file );

    QString permissionString( QFileInfo *info );
    QString fileListing( QFileInfo *info );
    QString absFilePath( const QString& file );

    void timerEvent( QTimerEvent *e );

private:
    State state;
    Q_UINT16 peerport;
    QHostAddress peeraddress;
    bool passiv;
    bool wait[4];
    ServerDTP *dtp;
    ServerSocket *serversocket;
    QString waitfile;
    QDir directory;
    QByteArray waitarray;
    QString renameFrom;
    QString lastCommand;
    int waitsocket;
    int storFileSize;
};
