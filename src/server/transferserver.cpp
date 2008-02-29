/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#define _XOPEN_SOURCE

#include <qtopia/qpeglobal.h>
#include <qtopia/qpeapplication.h>

#ifndef Q_OS_WIN32
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <shadow.h>

extern "C" {
#include <uuid/uuid.h>
#define UUID_H_INCLUDED
}

#else
#include <stdlib.h>
#include <time.h>
#endif


#if defined(_OS_LINUX_)
#include <shadow.h>
#endif

#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qregexp.h>
//#include <qtopia/qcopchannel_qws.h>
#include <qtopia/process.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/private/contact.h>
#include <qtopia/quuid.h>
#include <qtopia/version.h>
#ifdef Q_WS_QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include "transferserver.h"
#include <qtopia/qprocess.h>

const int block_size = 51200;

TransferServer::TransferServer( Q_UINT16 port, QObject *parent,
        const char* name)
    : QServerSocket( port, 1, parent, name )
{
    connections.setAutoDelete( TRUE );
    if ( !ok() )
	qWarning( "Failed to bind to port %d", port );
}

void TransferServer::authorizeConnections()
{
    QListIterator<ServerPI> it(connections);
    while ( it.current() ) {
	if ( !it.current()->verifyAuthorised() ) {
	    disconnect( it.current(), SIGNAL(connectionClosed(ServerPI *)), this, SLOT( closed(ServerPI *)) );
	    connections.removeRef( it.current() );
	} else
	    ++it;
    }
}

void TransferServer::closed(ServerPI *item)
{
    connections.removeRef(item);
}

TransferServer::~TransferServer()
{
}

void TransferServer::newConnection( int socket )
{
    ServerPI *ptr = new ServerPI( socket, this );
    connect( ptr, SIGNAL(connectionClosed(ServerPI *)), this, SLOT( closed(ServerPI *)) );
    connections.append( ptr );
}

QString SyncAuthentication::serverId()
{
    return Global::deviceId();
}

QString SyncAuthentication::ownerName()
{
    return Global::ownerName();
    QString vfilename = Global::applicationFileName("addressbook",
                "businesscard.vcf");
    if (QFile::exists(vfilename)) {
	Contact c;
	c = Contact::readVCard( vfilename )[0];
	return c.fullName();
    }

    return "";
}

QString SyncAuthentication::loginName()
{
    struct passwd *pw = 0L;
#ifndef Q_OS_WIN32
    pw = getpwuid( geteuid() );
    return QString::fromLocal8Bit( pw->pw_name );
#else
    //### revise
    return QString();
#endif
}

int SyncAuthentication::isAuthorized(QHostAddress peeraddress)
{
    Config cfg( QPEApplication::qpeDir()+"/etc/Security.conf", Config::File );
    cfg.setGroup("Sync");
    QString allowedstr = cfg.readEntry("auth_peer","192.168.0.0");
    QHostAddress allowed;
    allowed.setAddress(allowedstr);
    uint auth_peer = allowed.ip4Addr();
    uint auth_peer_bits = cfg.readNumEntry("auth_peer_bits",16);
    uint mask = auth_peer_bits >= 32 // shifting by 32 is not defined
	? 0xffffffff : (((1<<auth_peer_bits)-1)<<(32-auth_peer_bits));
    return (peeraddress.ip4Addr() & mask) == auth_peer;
}

bool SyncAuthentication::checkUser( const QString& user )
{
    if ( user.isEmpty() ) return FALSE;
    QString euser = loginName();
    return user == euser;
}

bool SyncAuthentication::checkPassword( const QString& password )
{
#ifdef ALLOW_UNIX_USER_FTP
    // First, check system password...

    struct passwd *pw = 0;
    struct spwd *spw = 0;

    pw = getpwuid( geteuid() );
    spw = getspnam( pw->pw_name );

    QString cpwd = QString::fromLocal8Bit( pw->pw_passwd );
    if ( cpwd == "x" && spw )
	cpwd = QString::fromLocal8Bit( spw->sp_pwdp );

    // Note: some systems use more than crypt for passwords.
    QString cpassword = QString::fromLocal8Bit( crypt( password.local8Bit(), cpwd.local8Bit() ) );
    if ( cpwd == cpassword )
	return TRUE;
#endif

    static int lastdenial=0;
    static int denials=0;
    int now = time(0);

    // Detect old Qtopia Desktop (no password)
    if ( password.isEmpty() ) {
	if ( denials < 1 || now > lastdenial+600 ) {
	    QMessageBox unauth(
		tr("Sync Connection"),
		tr("<p>An unauthorized system is requesting access to this device."
		    "<p>If you are using a version of Qtopia Desktop older than 1.5.1, "
		    "please upgrade."),
		QMessageBox::Warning,
		QMessageBox::Cancel, QMessageBox::NoButton, QMessageBox::NoButton,
		0, QString::null, TRUE, WStyle_StaysOnTop);
	    unauth.setButtonText(QMessageBox::Cancel, tr("Deny"));
	    unauth.exec();

	    denials++;
	    lastdenial=now;
	}
	return FALSE;
    }

    // Second, check sync password...

    static int lock=0;
    if ( lock ) return FALSE;

    ++lock;
    if ( password.left(6) == "Qtopia" ) {
	Config cfg( QPEApplication::qpeDir()+"/etc/Security.conf", Config::File );
	cfg.setGroup("Sync");
	QStringList pwds = cfg.readListEntry("Passwords",' ');
	for (QStringList::ConstIterator it=pwds.begin(); it!=pwds.end(); ++it) {
#ifndef Q_OS_WIN32	 
	    QString cpassword = QString::fromLocal8Bit(
		crypt( password.mid(8).local8Bit(), (*it).left(2).latin1() ) );
#else
	    // ### revise
	    QString cpassword("");
#endif
	    if ( *it == cpassword ) {
		lock--;
		return TRUE;
	    }
	}

	// Unrecognized system. Be careful...
	QMessageBox unrecbox(
	    tr("Sync Connection"),
	    tr(	"<p>An unrecognized system is requesting access to this device."
		"<p>If you have just initiated a Sync for the first time, this is normal."),
		QMessageBox::Warning,
		QMessageBox::Cancel, QMessageBox::Yes, QMessageBox::NoButton,
		0, QString::null, TRUE, WStyle_StaysOnTop);
	unrecbox.setButtonText(QMessageBox::Cancel, tr("Deny"));
	unrecbox.setButtonText(QMessageBox::Yes, tr("Allow"));

	if ( (denials > 2 && now < lastdenial+600)
	    || unrecbox.exec() != QMessageBox::Yes)
	{
	    denials++;
	    lastdenial=now;
	    lock--;
	    return FALSE;
	} else {
	    const char salty[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/.";
	    char salt[2];
	    salt[0]= salty[rand() % (sizeof(salty)-1)];
	    salt[1]= salty[rand() % (sizeof(salty)-1)];
#ifndef Q_OS_WIN32
	    QString cpassword = QString::fromLocal8Bit(
		crypt( password.mid(8).local8Bit(), salt ) );
#else
	    //### revise
	    QString cpassword("");
#endif
	    denials=0;
	    pwds.prepend(cpassword);
	    cfg.writeEntry("Passwords",pwds,' ');
	    lock--;
	    return TRUE;
	}
    }
    lock--;

    return FALSE;
}


ServerPI::ServerPI( int socket, QObject *parent, const char* name )
    : QSocket( parent, name ) , dtp( 0 ), serversocket( 0 ), waitsocket( 0 ),
      storFileSize(-1)
{
    state = Connected;

    setSocket( socket );

    peerport = peerPort();
    peeraddress = peerAddress();

#ifndef INSECURE
    if ( !SyncAuthentication::isAuthorized(peeraddress) ) {
	state = Forbidden;
	startTimer( 0 );
    } else
#endif
    {
	connect( this, SIGNAL( readyRead() ), SLOT( read() ) );
	connect( this, SIGNAL( connectionClosed() ), SLOT( connectionClosed() ) );

	passiv = FALSE;
	for( int i = 0; i < 4; i++ )
	    wait[i] = FALSE;

	send( "220 Qtopia " QPE_VERSION " FTP Server" ); // No tr
	state = Wait_USER;

	dtp = new ServerDTP( this );
	connect( dtp, SIGNAL( completed() ), SLOT( dtpCompleted() ) );
	connect( dtp, SIGNAL( failed() ), SLOT( dtpFailed() ) );
	connect( dtp, SIGNAL( error( int ) ), SLOT( dtpError( int ) ) );


	directory = QDir::currentDirPath();

	static int p = 1024;

	while ( !serversocket || !serversocket->ok() ) {
	    delete serversocket;
	    serversocket = new ServerSocket( ++p, this );
	}
	connect( serversocket, SIGNAL( newIncomming( int ) ),
		 SLOT( newConnection( int ) ) );
    }
}

ServerPI::~ServerPI()
{
    close();
    dtp->close();
    delete dtp;
    delete serversocket;
}

bool ServerPI::verifyAuthorised()
{
    if ( !SyncAuthentication::isAuthorized(peerAddress()) ) {
	state = Forbidden;
	return FALSE;
    }
    return TRUE;
}

void ServerPI::connectionClosed()
{
    // qDebug( "Debug: Connection closed" );
    emit connectionClosed(this);
}

void ServerPI::send( const QString& msg )
{
    QTextStream os( this );
    os << msg << endl;
    //qDebug( "Reply: %s", msg.latin1() );
}

void ServerPI::read()
{
    while ( canReadLine() )
	process( readLine().stripWhiteSpace() );
}

bool ServerPI::checkReadFile( const QString& file )
{
    QString filename;

    if ( file[0] != "/" )
	filename = directory.path() + "/" + file;
    else
	filename = file;

    QFileInfo fi( filename );
    return ( fi.exists() && fi.isReadable() );
}

bool ServerPI::checkWriteFile( const QString& file )
{
    QString filename;

    if ( file[0] != "/" )
	filename = directory.path() + "/" + file;
    else
	filename = file;

    QFileInfo fi( filename );

    if ( fi.exists() )
	if ( !QFile( filename ).remove() )
	    return FALSE;
    return TRUE;
}

void ServerPI::process( const QString& message )
{
    //qDebug( "Command: %s", message.latin1() );

    // split message using "," as separator
    QStringList msg = QStringList::split( " ", message );
    if ( msg.isEmpty() ) return;

    // command token
    QString cmd = msg[0].upper();

    // argument token
    QString arg;
    if ( msg.count() >= 2 )
	arg = msg[1];

    // full argument string
    QString args;
    if ( msg.count() >= 2 ) {
        QStringList copy( msg );
        // FIXME: for Qt3
        // copy.pop_front()
        copy.remove( copy.begin() );
        args = copy.join( " " );
    }

    //qDebug( "args: %s", args.latin1() );

    // we always respond to QUIT, regardless of state
    if ( cmd == "QUIT" ) {
	send( "211 Good bye!" ); // No tr
	close();
	return;
    }

    // connected to client
    if ( Connected == state )
	return;

    // waiting for user name
    if ( Wait_USER == state ) {

	if ( cmd != "USER" || msg.count() < 2 || !SyncAuthentication::checkUser( arg ) ) {
	    send( "530 Please login with USER and PASS" ); // No tr
	    return;
	}
	send( "331 User name ok, need password" ); // No tr
	state = Wait_PASS;
	return;
    }

    // waiting for password
    if ( Wait_PASS == state ) {

	if ( cmd != "PASS" || !SyncAuthentication::checkPassword( arg ) ) {
	    send( "530 Please login with USER and PASS" ); // No tr
	    return;
	}
	send( "230 User logged in, proceed" ); // No tr
	state = Ready;
	return;
    }

    // ACCESS CONTROL COMMANDS

    // Only an ALLO sent immediately before STOR is valid.
    if ( cmd != "STOR" )
	storFileSize = -1;

    // account (ACCT)
    if ( cmd == "ACCT" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" ); // No tr
    }

    // change working directory (CWD)
    else if ( cmd == "CWD" ) {

	if ( !args.isEmpty() ) {
	    if ( directory.cd( args, TRUE ) )
		send( "250 Requested file action okay, completed" ); // No tr
	    else
		send( "550 Requested action not taken" ); // No tr
	}
	else
	    send( "500 Syntax error, command unrecognized" ); // No tr
    }

    // change to parent directory (CDUP)
    else if ( cmd == "CDUP" ) {
	if ( directory.cdUp() )
	    send( "250 Requested file action okay, completed" ); // No tr
	else
	    send( "550 Requested action not taken" ); // No tr
    }

    // structure mount (SMNT)
    else if ( cmd == "SMNT" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" ); // No tr
    }

    // reinitialize (REIN)
    else if ( cmd == "REIN" ) {
	// even wu-ftp does not support it
	send( "502 Command not implemented" ); // No tr
    }


    // TRANSFER PARAMETER COMMANDS


    // data port (PORT)
    else if ( cmd == "PORT" ) {
	if ( parsePort( arg ) )
	    send( "200 Command okay" ); // No tr
	else
	    send( "500 Syntax error, command unrecognized" ); // No tr
    }

    // passive (PASV)
    else if ( cmd == "PASV" ) {
	passiv = TRUE;
	send( "227 Entering Passive Mode (" // No tr
	      + address().toString().replace( QRegExp( "\\." ), "," ) + ","
	      + QString::number( ( serversocket->port() ) >> 8 ) + ","
	      + QString::number( ( serversocket->port() ) & 0xFF ) +")" );
    }

    // representation type (TYPE)
    else if ( cmd == "TYPE" ) {
	if ( arg.upper() == "A" || arg.upper() == "I" )
	    send( "200 Command okay" ); // No tr
	else
	    send( "504 Command not implemented for that parameter" ); // No tr
    }

    // file structure (STRU)
    else if ( cmd == "STRU" ) {
	if ( arg.upper() == "F" )
	    send( "200 Command okay" ); // No tr
	else
	    send( "504 Command not implemented for that parameter" ); // No tr
    }

    // transfer mode (MODE)
    else if ( cmd == "MODE" ) {
	if ( arg.upper() == "S" )
	    send( "200 Command okay" ); // No tr
	else
	    send( "504 Command not implemented for that parameter" ); // No tr
    }


    // FTP SERVICE COMMANDS


    // retrieve (RETR)
    else if ( cmd == "RETR" )
	if ( !args.isEmpty() && checkReadFile( absFilePath( args ) )
	     || backupRestoreGzip( absFilePath( args ) ) ) {
	    send( "150 File status okay" ); // No tr
	    sendFile( absFilePath( args ) );
	}
	else {
	    qDebug("550 Requested action not taken");
	    send( "550 Requested action not taken" ); // No tr
	}

    // store (STOR)
    else if ( cmd == "STOR" )
	if ( !args.isEmpty() && checkWriteFile( absFilePath( args ) ) ) {
	    send( "150 File status okay" ); // No tr
	    retrieveFile( absFilePath( args ) );
	}
	else
	    send( "550 Requested action not taken" ); // No tr

    // store unique (STOU)
    else if ( cmd == "STOU" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // append (APPE)
    else if ( cmd == "APPE" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // allocate (ALLO)
    else if ( cmd == "ALLO" ) {
	storFileSize = args.toInt();
	send( "200 Command okay" ); // No tr
    }

    // restart (REST)
    else if ( cmd == "REST" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // rename from (RNFR)
    else if ( cmd == "RNFR" ) {
	renameFrom = QString::null;
	if ( args.isEmpty() )
	    send( "500 Syntax error, command unrecognized" ); // No tr
	else {
	    QFile file( absFilePath( args ) );
	    if ( file.exists() ) {
		send( "350 File exists, ready for destination name" ); // No tr
		renameFrom = absFilePath( args );
	    }
	    else
		send( "550 Requested action not taken" ); // No tr
	}
    }

    // rename to (RNTO)
    else if ( cmd == "RNTO" ) {
	if ( lastCommand != "RNFR" )
	    send( "503 Bad sequence of commands" ); // No tr
	else if ( args.isEmpty() )
	    send( "500 Syntax error, command unrecognized" ); // No tr
	else {
	    QDir dir( absFilePath( args ) );
	    if ( dir.rename( renameFrom, absFilePath( args ), TRUE ) )
		send( "250 Requested file action okay, completed." ); // No tr
	    else
		send( "550 Requested action not taken" ); // No tr
	}
    }

    // abort (ABOR)
    else if ( cmd.contains( "ABOR" ) ) {
	dtp->close();
	if ( dtp->dtpMode() != ServerDTP::Idle )
	    send( "426 Connection closed; transfer aborted" ); // No tr
	else
	    send( "226 Closing data connection" ); // No tr
    }

    // delete (DELE)
    else if ( cmd == "DELE" ) {
	if ( args.isEmpty() )
	    send( "500 Syntax error, command unrecognized" ); // No tr
	else {
	    QFile file( absFilePath( args ) ) ;
	    if ( file.remove() ) {
		send( "250 Requested file action okay, completed" ); // No tr
		QCopEnvelope e("QPE/System", "linkChanged(QString)" );
		e << file.name();
	    } else {
		send( "550 Requested action not taken" ); // No tr
	    }
	}
    }

    // remove directory (RMD)
    else if ( cmd == "RMD" ) {
	if ( args.isEmpty() )
	    send( "500 Syntax error, command unrecognized" ); // No tr
	else {
	    QDir dir;
	    if ( dir.rmdir( absFilePath( args ), TRUE ) )
		send( "250 Requested file action okay, completed" ); // No tr
	    else
		send( "550 Requested action not taken" ); // No tr
	}
    }

    // make directory (MKD)
    else if ( cmd == "MKD" ) {
	if ( args.isEmpty() ) {
	    qDebug(" Error: no arg");
	    send( "500 Syntax error, command unrecognized" ); // No tr
	}
	else {
	    QDir dir;
	    if ( dir.mkdir( absFilePath( args ), TRUE ) )
		send( "250 Requested file action okay, completed." ); // No tr
	    else
		send( "550 Requested action not taken" ); // No tr
	}
    }

    // print working directory (PWD)
    else if ( cmd == "PWD" ) {
	send( "257 \"" + directory.path() +"\"" );
    }

    // list (LIST)
    else if ( cmd == "LIST" ) {
	if ( sendList( absFilePath( args ) ) )
	    send( "150 File status okay" ); // No tr
	else
	    send( "500 Syntax error, command unrecognized" ); // No tr
    }

    // size (SIZE)
    else if ( cmd == "SIZE" ) {
	QString filePath = absFilePath( args );
	QFileInfo fi( filePath );
	bool gzipfile = backupRestoreGzip( filePath );
	if ( !fi.exists() && !gzipfile )
	    send( "500 Syntax error, command unrecognized" ); // No tr
	else {
	    if ( !gzipfile )
		send( "213 " + QString::number( fi.size() ) );
	    else {
		Process duproc( QString("du") );
		duproc.addArgument("-s");
		QString in, out;
		if ( !duproc.exec(in, out) ) {
		    qDebug("du process failed; just sending back 1K");
		    send( "213 1024");
		}
		else {
		    QString size = out.left( out.find("\t") );
		    int guess = size.toInt()/5;
		    if ( filePath.contains("doc") ) // No tr
			guess *= 1000;
		    qDebug("sending back gzip guess of %d", guess);
		    send( "213 " + QString::number(guess) );
		}
	    }
	}
    }
    // name list (NLST)
    else if ( cmd == "NLST" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // site parameters (SITE)
    else if ( cmd == "SITE" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // system (SYST)
    else if ( cmd == "SYST" ) {
	send( "215 UNIX Type: L8" ); // No tr
    }

    // status (STAT)
    else if ( cmd == "STAT" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // help (HELP )
    else if ( cmd == "HELP" ) {
	send( "502 Command not implemented" ); // No tr
    }

    // noop (NOOP)
    else if ( cmd == "NOOP" ) {
	send( "200 Command okay" ); // No tr
    }

    // not implemented
    else
	send( "502 Command not implemented" ); // No tr

    lastCommand = cmd;
}

bool ServerPI::backupRestoreGzip( const QString &file )
{
    return (file.find( "backup" ) != -1 && // No tr
	    file.findRev( ".tgz" ) == (int)file.length()-4 );
}

bool ServerPI::backupRestoreGzip( const QString &file, QStringList &targets )
{
  if ( file.find( "backup" ) != -1 && // No tr
       file.findRev( ".tgz" ) == (int)file.length()-4 ) {
      QFileInfo info( file );
      targets = info.dirPath( TRUE );
      qDebug("ServerPI::backupRestoreGzip for %s = %s", file.latin1(),
	     targets.join(" ").latin1() );
      return true;
  }
  return false;
}

void ServerPI::sendFile( const QString& file )
{
    if ( passiv ) {
	wait[SendFile] = TRUE;
	waitfile = file;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else {
      QStringList targets;
      if ( backupRestoreGzip( file, targets ) )
	  dtp->sendGzipFile( file, targets, peeraddress, peerport );
      else dtp->sendFile( file, peeraddress, peerport );
    }
}

void ServerPI::retrieveFile( const QString& file )
{
    if ( passiv ) {
	wait[RetrieveFile] = TRUE;
	waitfile = file;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else {
      QStringList targets;
      if ( backupRestoreGzip( file, targets ) )
	dtp->retrieveGzipFile( file, peeraddress, peerport );
      else
	dtp->retrieveFile( file, peeraddress, peerport, storFileSize );
    }
}

bool ServerPI::parsePort( const QString& pp )
{
    QStringList p = QStringList::split( ",", pp );
    if ( p.count() != 6 ) return FALSE;

    // h1,h2,h3,h4,p1,p2
    peeraddress = QHostAddress( ( p[0].toInt() << 24 ) + ( p[1].toInt() << 16 ) +
				( p[2].toInt() << 8 ) + p[3].toInt() );
    peerport = ( p[4].toInt() << 8 ) + p[5].toInt();
    return TRUE;
}

void ServerPI::dtpCompleted()
{
    send( "226 Closing data connection, file transfer successful" ); // No tr
    if ( dtp->dtpMode() == ServerDTP::RetrieveFile ) {
	QString fn = dtp->fileName();
	if ( fn.right(8)==".desktop" && fn.find("/Documents/")>=0 ) {
	    QCopEnvelope e("QPE/System", "linkChanged(QString)" );
	    e << fn;
	}
    }
    waitsocket = 0;
    dtp->close();
    storFileSize = -1;
}

void ServerPI::dtpFailed()
{
    dtp->close();
    waitsocket = 0;
    send( "451 Requested action aborted: local error in processing" ); // No tr
    storFileSize = -1;
}

void ServerPI::dtpError( int )
{
    dtp->close();
    waitsocket = 0;
    send( "451 Requested action aborted: local error in processing" ); // No tr
    storFileSize = -1;
}

bool ServerPI::sendList( const QString& arg )
{
    QByteArray listing;
    QBuffer buffer( listing );

    if ( !buffer.open( IO_WriteOnly ) )
	return FALSE;

    QTextStream ts( &buffer );
    QString fn = arg;

    if ( fn.isEmpty() )
	fn = directory.path();

    QFileInfo fi( fn );
    if ( !fi.exists() ) return FALSE;

    // return file listing
    if ( fi.isFile() ) {
	ts << fileListing( &fi ) << endl;
    }

    // return directory listing
    else if ( fi.isDir() ) {
	QDir dir( fn );
	const QFileInfoList *list = dir.entryInfoList( QDir::All | QDir::Hidden );

	QFileInfoListIterator it( *list );
	QFileInfo *info;

	unsigned long total = 0;
	while ( ( info = it.current() ) ) {
	    if ( info->fileName() != "." && info->fileName() != ".." )
		total += info->size();
	    ++it;
	}

	ts << "total " << QString::number( total / 1024 ) << endl; // No tr

	it.toFirst();
	while ( ( info = it.current() ) ) {
	    if ( info->fileName() == "." || info->fileName() == ".." ) {
		++it;
		continue;
	    }
	    ts << fileListing( info ) << endl;
	    ++it;
	}
    }

    if ( passiv ) {
	waitarray = buffer.buffer();
	wait[SendByteArray] = TRUE;
	if ( waitsocket )
	    newConnection( waitsocket );
    }
    else
	dtp->sendByteArray( buffer.buffer(), peeraddress, peerport );
    return TRUE;
}

QString ServerPI::fileListing( QFileInfo *info )
{
    if ( !info ) return QString::null;
    QString s;

    // type char
    if ( info->isDir() )
	s += "d";
    else if ( info->isSymLink() )
	s += "l";
    else
	s += "-";

    // permisson string
    s += permissionString( info ) + " ";

    // number of hardlinks
    int subdirs = 1;

    if ( info->isDir() )
	subdirs = 2;
    // FIXME : this is to slow
    //if ( info->isDir() )
    //subdirs = QDir( info->absFilePath() ).entryList( QDir::Dirs ).count();

    s += QString::number( subdirs ).rightJustify( 3, ' ', TRUE ) + " ";

    // owner
    QString o = info->owner();
    if ( o.isEmpty() )
	o = QString::number(info->ownerId());
    s += o.leftJustify( 8, ' ', TRUE ) + " ";

    // group
    QString g = info->group();
    if ( g.isEmpty() )
	g = QString::number(info->groupId());
    s += g.leftJustify( 8, ' ', TRUE ) + " ";

    // file size in bytes
    s += QString::number( info->size() ).rightJustify( 9, ' ', TRUE ) + " ";

    // last modified date
    QDate date = info->lastModified().date();
    QTime time = info->lastModified().time();
    s += date.monthName( date.month() ) + " "
	 + QString::number( date.day() ).rightJustify( 2, ' ', TRUE ) + " "
	 + QString::number( time.hour() ).rightJustify( 2, '0', TRUE ) + ":"
	 + QString::number( time.minute() ).rightJustify( 2,'0', TRUE ) + " ";

    // file name
    s += info->fileName();

    return s;
}

QString ServerPI::permissionString( QFileInfo *info )
{
    if ( !info ) return QString( "---------" );
    QString s;

    // user
    if ( info->permission( QFileInfo::ReadUser ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteUser ) ) s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeUser ) ) s += "x";
    else s += "-";

    // group
    if ( info->permission( QFileInfo::ReadGroup ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteGroup ) )s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeGroup ) ) s += "x";
    else s += "-";

    // exec
    if ( info->permission( QFileInfo::ReadOther ) ) s += "r";
    else s += "-";
    if ( info->permission( QFileInfo::WriteOther ) ) s += "w";
    else s += "-";
    if ( info->permission( QFileInfo::ExeOther ) ) s += "x";
    else s += "-";

    return s;
}

void ServerPI::newConnection( int socket )
{
    //qDebug( "New incomming connection" );

    if ( !passiv ) return;

    if ( wait[SendFile] ) {
	QStringList targets;
	if ( backupRestoreGzip( waitfile, targets ) )
	    dtp->sendGzipFile( waitfile, targets );
	else
	    dtp->sendFile( waitfile );
	dtp->setSocket( socket );
    }
    else if ( wait[RetrieveFile] ) {
      qDebug("check retrieve file");
      if ( backupRestoreGzip( waitfile ) )
	dtp->retrieveGzipFile( waitfile );
      else
	dtp->retrieveFile( waitfile, storFileSize );
      dtp->setSocket( socket );
    }
    else if ( wait[SendByteArray] ) {
	dtp->sendByteArray( waitarray );
	dtp->setSocket( socket );
    }
    else if ( wait[RetrieveByteArray] ) {
	qDebug("retrieve byte array");
	dtp->retrieveByteArray();
	dtp->setSocket( socket );
    }
    else
	waitsocket = socket;

    for( int i = 0; i < 4; i++ )
	wait[i] = FALSE;
}

QString ServerPI::absFilePath( const QString& file )
{
    if ( file.isEmpty() ) return file;

    QString filepath( file );
    if ( file[0] != "/" )
	filepath = directory.path() + "/" + file;

    return filepath;
}


void ServerPI::timerEvent( QTimerEvent * )
{
    connectionClosed();
}


ServerDTP::ServerDTP( QObject *parent, const char* name)
  : QSocket( parent, name ), mode( Idle ), createTargzProc( 0 ),
    retrieveTargzProc( 0 )
{

  connect( this, SIGNAL( connected() ), SLOT( connected() ) );
  connect( this, SIGNAL( connectionClosed() ), SLOT( connectionClosed() ) );
  connect( this, SIGNAL( bytesWritten( int ) ), SLOT( bytesWritten( int ) ) );
  connect( this, SIGNAL( readyRead() ), SLOT( readyRead() ) );

  createTargzProc = new QProcess( QString("tar"), this, "createTargzProc"); // No tr
  createTargzProc->setCommunication( QProcess::Stdout );
  createTargzProc->setWorkingDirectory( QDir::rootDirPath() );
  connect( createTargzProc, SIGNAL( processExited() ), SLOT( targzDone() ) );

  retrieveTargzProc = new QProcess( this, "retrieveTargzProc" );
  retrieveTargzProc->setCommunication( QProcess::Stdin );
  retrieveTargzProc->setWorkingDirectory( QDir::rootDirPath() );
  connect( retrieveTargzProc, SIGNAL( processExited() ),
	   SIGNAL( completed() ) );
  connect( retrieveTargzProc, SIGNAL( processExited() ),
	   SLOT( extractTarDone() ) );
}

ServerDTP::~ServerDTP()
{
    buf.close();
    if ( RetrieveFile == mode && file.isOpen() ) {
	// We're being shutdown before the client closed.
	file.close();
	if ( recvFileSize >= 0 && (int)file.size() != recvFileSize ) {
	    qDebug( "STOR incomplete" );
	    file.remove();
	}
    } else {
	file.close();
    }
    createTargzProc->kill();
}

void ServerDTP::extractTarDone()
{
    qDebug("extract done");
#ifndef QT_NO_COP
    QCopEnvelope e( "QPE/System", "restoreDone(QString)" );
    e << file.name();
#endif
}

void ServerDTP::connected()
{
  // send file mode
  switch ( mode ) {
  case SendFile :
    if ( !file.exists() || !file.open( IO_ReadOnly) ) {
      emit failed();
      mode = Idle;
      return;
    }

    //qDebug( "Debug: Sending file '%s'", file.name().latin1() );

    bytes_written = 0;
    if ( file.size() == 0 ) {
      //make sure it doesn't hang on empty files
      file.close();
      emit completed();
      mode = Idle;
    } else {
      // Don't write more if there is plenty buffered already.
      if ( bytesToWrite() <= block_size && !file.atEnd() ) {
	QCString s;
	s.resize( block_size );
	int bytes = file.readBlock( s.data(), block_size );
	writeBlock( s.data(), bytes );
      }
    }
    break;
  case SendGzipFile:
    if ( createTargzProc->isRunning() ) {
      // SHOULDN'T GET HERE, BUT DOING A SAFETY CHECK ANYWAY
      qWarning("Previous tar --gzip process is still running; killing it...");
      createTargzProc->kill();
    }

    bytes_written = 0;
    qDebug("==>start send tar process");
    if ( !createTargzProc->start() )
      qWarning("Error starting %s",
	       createTargzProc->arguments().join(" ").latin1());
    break;
  case SendBuffer:
    if ( !buf.open( IO_ReadOnly) ) {
      emit failed();
      mode = Idle;
      return;
    }

    // qDebug( "Debug: Sending byte array" );
    bytes_written = 0;
    while( !buf.atEnd() )
      putch( buf.getch() );
    buf.close();
    break;
  case RetrieveFile:
    // retrieve file mode
    if ( file.exists() && !file.remove() ) {
      emit failed();
      mode = Idle;
      return;
    }

    if ( !file.open( IO_WriteOnly) ) {
      emit failed();
      mode = Idle;
      return;
    }
    // qDebug( "Debug: Retrieving file %s", file.name().latin1() );
    break;
  case RetrieveGzipFile:
    qDebug("=-> starting tar process to receive .tgz file");
    break;
  case RetrieveBuffer:
    // retrieve buffer mode
    if ( !buf.open( IO_WriteOnly) ) {
      emit failed();
      mode = Idle;
      return;
    }
    // qDebug( "Debug: Retrieving byte array" );
    break;
  case Idle:
    qDebug("connection established but mode set to Idle; BUG!");
    break;
  }
}

void ServerDTP::connectionClosed()
{
    //qDebug( "Debug: Data connection closed %ld bytes written", bytes_written );

    // send file mode
    if ( SendFile == mode ) {
	if ( bytes_written == file.size() )
	    emit completed();
	else
	    emit failed();
    }

    // send buffer mode
    else if ( SendBuffer == mode ) {
	if ( bytes_written == buf.size() )
	    emit completed();
	else
	    emit failed();
    }

    // retrieve file mode
    else if ( RetrieveFile == mode ) {
	file.close();
	if ( recvFileSize >= 0 && (int)file.size() != recvFileSize ) {
	    qDebug( "STOR incomplete" );
	    file.remove();
	    emit failed();
	} else {
	    emit completed();
	}
    }

    else if ( RetrieveGzipFile == mode ) {
	qDebug("Done writing ungzip file; closing input");
	retrieveTargzProc->flushStdin();
	retrieveTargzProc->closeStdin();
    }

    // retrieve buffer mode
    else if ( RetrieveBuffer == mode ) {
	buf.close();
	emit completed();
    }

    mode = Idle;
}

void ServerDTP::bytesWritten( int bytes )
{
    bytes_written += bytes;

    // send file mode
    if ( SendFile == mode ) {

	if ( bytes_written == file.size() ) {
	    // qDebug( "Debug: Sending complete: %d bytes", file.size() );
	    file.close();
	    emit completed();
	    mode = Idle;
	}
	else if( !file.atEnd() ) {
	    QCString s;
	    s.resize( block_size );
	    int bytes = file.readBlock( s.data(), block_size );
	    writeBlock( s.data(), bytes );
	}
    }

    // send buffer mode
    if ( SendBuffer == mode ) {

	if ( bytes_written == buf.size() ) {
	    // qDebug( "Debug: Sending complete: %d bytes", buf.size() );
	    emit completed();
	    mode = Idle;
	}
    }
}

void ServerDTP::readyRead()
{
    // retrieve file mode
    if ( RetrieveFile == mode ) {
	QCString s;
	s.resize( bytesAvailable() );
	readBlock( s.data(), bytesAvailable() );
	file.writeBlock( s.data(), s.size() );
    }
    else if ( RetrieveGzipFile == mode ) {
	if ( !retrieveTargzProc->isRunning() )
	    retrieveTargzProc->start();

      	QByteArray s;
	s.resize( bytesAvailable() );
	readBlock( s.data(), bytesAvailable() );
	retrieveTargzProc->writeToStdin( s );
	qDebug("wrote %d bytes to ungzip ", s.size() );
    }
    // retrieve buffer mode
    else if ( RetrieveBuffer == mode ) {
	QCString s;
	s.resize( bytesAvailable() );
	readBlock( s.data(), bytesAvailable() );
	buf.writeBlock( s.data(), s.size() );
    }
}

void ServerDTP::writeTargzBlock()
{
    QByteArray block = createTargzProc->readStdout();
    writeBlock( block.data(), block.size() );
    qDebug("writeTargzBlock %d", block.size());
}

void ServerDTP::targzDone()
{
    qDebug("tar and gzip done");
    emit completed();
    mode = Idle;
    disconnect( createTargzProc, SIGNAL( readyReadStdout() ),
		    this, SLOT( writeTargzBlock() ) );
}

void ServerDTP::sendFile( const QString fn, const QHostAddress& host, Q_UINT16 port )
{
    file.setName( fn );
    mode = SendFile;
    connectToHost( host.toString(), port );
}

void ServerDTP::sendFile( const QString fn )
{
    file.setName( fn );
    mode = SendFile;
}

void ServerDTP::sendGzipFile( const QString &fn,
			      const QStringList &archiveTargets,
			      const QHostAddress& host, Q_UINT16 port )
{
    sendGzipFile( fn, archiveTargets );
    connectToHost( host.toString(), port );
}

void ServerDTP::sendGzipFile( const QString &fn,
			      const QStringList &archiveTargets  )
{
    mode = SendGzipFile;
    file.setName( fn );

    QStringList args = "targzip";
    //args += "-cv";
    args += archiveTargets;
    qDebug("sendGzipFile %s", args.join(" ").latin1() );
    createTargzProc->setArguments( args );
    connect( createTargzProc,
	     SIGNAL( readyReadStdout() ), SLOT( writeTargzBlock() ) );
}

void ServerDTP::retrieveFile( const QString fn, const QHostAddress& host, Q_UINT16 port, int fileSize )
{
    recvFileSize = fileSize;
    file.setName( fn );
    mode = RetrieveFile;
    connectToHost( host.toString(), port );
}

void ServerDTP::retrieveFile( const QString fn, int fileSize )
{
    recvFileSize = fileSize;
    file.setName( fn );
    mode = RetrieveFile;
}

void ServerDTP::retrieveGzipFile( const QString &fn )
{
    qDebug("retrieveGzipFile %s", fn.latin1());
    file.setName( fn );
    mode = RetrieveGzipFile;

    retrieveTargzProc->setArguments( "targunzip" );
    connect( retrieveTargzProc, SIGNAL( processExited() ),
	     SLOT( extractTarDone() ) );
}

void ServerDTP::retrieveGzipFile( const QString &fn, const QHostAddress& host, Q_UINT16 port )
{
    retrieveGzipFile( fn );
    connectToHost( host.toString(), port );
}

void ServerDTP::sendByteArray( const QByteArray& array, const QHostAddress& host, Q_UINT16 port )
{
    buf.setBuffer( array );
    mode = SendBuffer;
    connectToHost( host.toString(), port );
}

void ServerDTP::sendByteArray( const QByteArray& array )
{
    buf.setBuffer( array );
    mode = SendBuffer;
}

void ServerDTP::retrieveByteArray( const QHostAddress& host, Q_UINT16 port )
{
    buf.setBuffer( QByteArray() );
    mode = RetrieveBuffer;
    connectToHost( host.toString(), port );
}

void ServerDTP::retrieveByteArray()
{
    buf.setBuffer( QByteArray() );
    mode = RetrieveBuffer;
}

void ServerDTP::setSocket( int socket )
{
    QSocket::setSocket( socket );
    connected();
}

