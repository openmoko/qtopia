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

#if !defined(Q_OS_WIN32) && defined(SINGLE_EXEC)
#include "singleexec_quicklaunch.h"

#ifdef _OS_LINUX_
QPEApplication *app = 0;
QWidget *mainWindow = 0;

QPEAppMap *qpeAppMap() { 
    static QPEAppMap *am = 0; 
    if ( !am ) am = new QPEAppMap(); 
    return am; 
} 

#include <stdio.h>
char **argv0 = 0;
int argv_lth;
#ifndef SPT_BUFSIZE
#define SPT_BUFSIZE     2048
#endif
#include <stdarg.h>
void setproctitle (const char *fmt,...) {
    int        i;
    char       buf[SPT_BUFSIZE];
    va_list    ap;

    if (!argv0)
	return;

    va_start(ap, fmt);
    (void) vsnprintf(buf, SPT_BUFSIZE, fmt, ap);
    va_end(ap);

    i = strlen (buf);
    if (i > argv_lth - 2) {
	i = argv_lth - 2;
	buf[i] = '\0';
    }
    memset(argv0[0], '\0', argv_lth);       /* clear the memory area */
    (void) strcpy (argv0[0], buf);

    argv0[1] = NULL;
}
#endif

QuickLauncher::QuickLauncher() : QObject() {
    qlChannel = new QCopChannel( "QPE/QuickLauncher", this);
    connect( qlChannel, SIGNAL(received(const QCString&,const QByteArray&)),
	    this, SLOT(message(const QCString&,const QByteArray&)) );
}

void QuickLauncher::exec (int argc, char **argv ) {
    QString executableName(argv[0]); 
    executableName = executableName.right(executableName.length() 
	    - executableName.findRev('/') - 1); 

    QPEAppMap *qpeAppMap(); 
    if ( qpeAppMap()->contains(executableName) ) {
	if (!app) // Makes it quicker to fail for non-quicklaunched apps
	    app = new QPEApplication( argc, argv ); 
	mainWindow = (*qpeAppMap())[executableName](0,0,0); 
    } else {
	qFatal("Could not find application: %s", executableName.latin1());
    }
    if ( mainWindow ) { 
	if ( mainWindow->metaObject()->slotNames(true).contains("setDocument(const QString&)") ) 
	    app->showMainDocumentWidget( mainWindow ); 
	else 
	    app->showMainWidget( mainWindow ); 
    } else {
	qFatal("Could not create view for: %s", executableName.latin1());
    }
}

void QuickLauncher::message(const QCString &msg, const QByteArray & data) {
    QStrList argList;

    if ( msg == "execute(QStrList)" ) {
	delete qlChannel;
	QDataStream stream( data, IO_ReadOnly );
	QStrList argList;
	stream >> argList;
	qDebug( "QuickLauncher execute: %s", argList.at(0) );
	doQuickLaunch( argList );
	delete this;
    } else if ( msg == "execute(QString)" ) {
	delete qlChannel;
	QDataStream stream( data, IO_ReadOnly );
	QString arg;
	stream >> arg;
	qDebug( "QuickLauncher execute: %s", arg.latin1() );
	QStrList argList;
	argList.append( arg.utf8() );
	doQuickLaunch( argList );
	delete this;
    }
}

void QuickLauncher::doQuickLaunch( QStrList &argList ) {
    static int myargc = argList.count();
    static char **myargv = new char *[myargc + 1];
    for ( int j = 0; j < myargc; j++ ) {
	myargv[j] = new char [strlen(argList.at(j))+1];
	strcpy( myargv[j], argList.at(j) );
    }
    myargv[myargc] = NULL;
#ifdef _OS_LINUX_
    // Change name of process
    setproctitle(myargv[0]);
#endif
    connect(app, SIGNAL(lastWindowClosed()), app, SLOT(hideOrQuit()));
    app->exit_loop();
    //	app->initApp( myargc, myargv );
    exec( myargc, myargv );
}

#endif
