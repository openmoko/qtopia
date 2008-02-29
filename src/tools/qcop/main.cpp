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

#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif

#include <qtopia/qpeapplication.h>
#include <qstringlist.h>
#include <qdatastream.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef Q_OS_WIN32
#include <pwd.h>
#include <unistd.h>
#endif

// No tr() anywhere in this file

static void usage()
{
    fprintf( stderr, "Usage: qcop [-l username] channel command [parameters]\n" );
}

static void syntax( const QString &where, const QString &what )
{
    fprintf( stderr, "Syntax error in %s: %s\n", where.latin1(), what.latin1() );
    exit(1);
}

int main( int argc, char *argv[] )
{
    if ( argc > 1 ) {
	QString opt = argv[1];
	if ( opt == "-l" ) {
	    if ( argc < 5 ) {
		usage();
		exit(1);
	    }
#ifndef Q_OS_WIN32
	    const char *username = argv[2];
	    struct passwd *pwd = getpwnam( username );
	    if ( !pwd ) {
		fprintf( stderr, "Unknown user %s\n", username );
		exit(1);
	    }
	    int uid =  pwd->pw_uid;

	    if ( setuid( uid ) != 0 ) {
		fprintf( stderr, "Could not run as user %s\n", username );
		exit(1);
	    }
	    setenv( "LOGNAME", username, 1 );
#else
	    setenv("LOGNAME", argv[2], 1);
#endif
	    
	    argc -= 2;
	    for ( int i = 1; i < argc; i++ ) {
		argv[i] = argv[i+2];
	    }
	}
		      
    }

    QApplication app( argc, argv );
    
    if ( argc < 3 ) {
	usage();
	exit(1);
    }

    QString channel = argv[1];
    QString command = argv[2];
    command.stripWhiteSpace();

    int paren = command.find( "(" );
    if ( paren <= 0 )
	syntax( "command", command );

    QString params = command.mid( paren + 1 );
    if ( params[(int)params.length()-1] != ')' )
	syntax( "command", command );

    params.truncate( params.length()-1 );
#ifndef QT_NO_COP
    QCopEnvelope env(channel.latin1(), command.latin1());

    int argIdx = 3;

    QStringList paramList = QStringList::split( ",", params );
    QStringList::Iterator it;
    for ( it = paramList.begin(); it != paramList.end(); ++it ) {
	QString arg = argv[argIdx];
	if ( *it == "QString" ) {
	    env << arg;
	} else if ( *it == "int" ) {
	    env << arg.toInt();
	} else {
	    syntax( "parameter type", *it );
	}
	argIdx++;
    }
#endif

    QTimer::singleShot( 0, &app, SLOT(quit()) );
    return app.exec();
}

