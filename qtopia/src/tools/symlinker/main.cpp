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

#include <qtopia/qpeapplication.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qstringlist.h>

#include <stdlib.h>
#include <unistd.h> //symlink() 
#include <sys/stat.h> // mkdir()

#include <sys/vfs.h>
#include <mntent.h>

//#define IPKGSIM

#ifdef IPKGSIM
static const char *mainDir = QPEApplication::qpeDir() + "/usr/lib/ipkg";
static const char *infoDir = QPEApplication::qpeDir() + "/usr/lib/ipkg/info/";
static const char *listDir = QPEApplication::qpeDir() + "/usr/lib/ipkg/externinfo/";
#else
static const char *mainDir = "/usr/lib/ipkg";
static const char *infoDir = "/usr/lib/ipkg/info/";
static const char *listDir = "/usr/lib/ipkg/externinfo/";
#endif

static void createSymlinks( const QString &location, const QString &package )
{
    QFile inFile( location + infoDir + package + ".list" );
    mkdir( mainDir, 0777 );
    mkdir( listDir, 0777 );

    QFile outFile( listDir + package + ".list");

    //qDebug( "createSymlinks %s -> %s", inFile.name().ascii(), outFile.name().ascii() );
    

    
    if ( inFile.open(IO_ReadOnly) && outFile.open(IO_WriteOnly)) {   
	QTextStream in(&inFile);
	QTextStream out(&outFile);

	QString s;
	while ( !in.eof() ) {        // until end of file...
	    s = in.readLine();       // line of text excluding '\n'
	    //qDebug( "Read: %s", s.ascii() );
	    // for s, do link/mkdir.
	    if ( s.right(1) == "/" ) {
		//qDebug("do mkdir for %s", s.ascii());
		mkdir( s.ascii(), 0777 );
		//possible optimization: symlink directories
		//that don't exist already. -- Risky.
	    } else {
		//qDebug("do symlink for %s", s.ascii());
		QFileInfo ffi( s ); 
		//Don't try to symlink if a regular file already exists.
		if ( !ffi.exists() || ffi.isSymLink() ) {
		    symlink( QDir::cleanDirPath(location+s).ascii(), s.ascii() );
		    out << s << "\n";
		} else {
		    qDebug( "%s  exists already, not symlinked", s.ascii() );
		}
	    }
	}
	inFile.close();
	outFile.close();
    }
}



static void removeSymlinks( const QString &package )
{
    QFile inFile( listDir + package + ".list" );

    if ( inFile.open(IO_ReadOnly) ) {   
	QTextStream in(&inFile);

	QString s;
	while ( !in.eof() ) {        // until end of file...
	    s = in.readLine();       // line of text excluding '\n'
	    //qDebug("remove symlink %s", s.ascii());
	    QFileInfo ffi( s ); 
	    //Confirm that it's still a symlink.
	    if ( ffi.isSymLink() )
		unlink( s.ascii() );
	    else
		qDebug( "Not removed %s", s.ascii() );
	}
	inFile.close();
	inFile.remove();
    }
}



/*
  Slightly hacky: we can't use StorageInfo, since we don't have a
  QApplication. We look for filesystems that have the directory
  /usr/lib/ipkg/info, and assume that they are removable media
  with packages installed. This is safe even if eg. /usr is on a 
  separate filesystem, since then we would be testing for
  /usr/usr/lib/ipkg/info, which should not exist. 
 */

static void updateSymlinks()
{
    QDir lists( listDir );
    QStringList knownPackages = lists.entryList( "*.list" ); // No tr
    
    struct mntent *me;
    FILE *mntfp = setmntent( "/etc/mtab", "r" );

    if ( mntfp ) {
	while ( (me = getmntent( mntfp )) != 0 ) {
	    QString root = me->mnt_dir;
	    if ( root == "/" ) 
		continue;

	    QString info = root + infoDir;
	    QDir infoDir( info );
	    //qDebug( "looking at %s", info.ascii() );
	    if ( infoDir.isReadable() ) {
		const QFileInfoList *packages = infoDir.entryInfoList( "*.list" ); // No tr
		QFileInfoListIterator it( *packages );
		QFileInfo *fi;
		while (( fi = *it )) {
		    ++it;
		    if ( knownPackages.contains( fi->fileName() ) ) {
			//qDebug( "found %s and we've seen it before", fi->fileName().latin1() );
			knownPackages.remove( fi->fileName() );
		    } else {
			//it's a new one
			createSymlinks( root + "/", fi->baseName() );
		    }
		    
		}		
		
	    }
	}
	endmntent( mntfp );
    }
    
    for ( QStringList::Iterator it = knownPackages.begin(); 
	  it != knownPackages.end(); ++it ) {
	// strip ".info" off the end.
	removeSymlinks( (*it).left((*it).length()-5) );
    }
}



int main( int argc, char *argv[] )
{
    QApplication a( argc, argv, QApplication::Tty );

    QString command = argc > 1 ? argv[1] : "update"; // No tr
    
    if ( command == "update" ) // No tr
	updateSymlinks();
    else if ( command == "create" && argc > 3 ) // No tr
	createSymlinks( argv[2], argv[3] );
    else if ( command == "remove"  && argc > 2 ) // No tr
	removeSymlinks( argv[2] );
    else
	qWarning( "Argument error" );
}
