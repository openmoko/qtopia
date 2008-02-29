/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <qtopiaapplication.h>
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

QSXE_APP_KEY

//#define IPKGSIM

#ifdef IPKGSIM
static const char *mainDir = Qtopia::qtopiaDir() + "/usr/lib/ipkg";
static const char *infoDir = Qtopia::qtopiaDir() + "/usr/lib/ipkg/info/";
static const char *listDir = Qtopia::qtopiaDir() + "/usr/lib/ipkg/externinfo/";
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

    if ( inFile.open(QIODevice::ReadOnly) && outFile.open(QIODevice::WriteOnly)) {
        QTextStream in(&inFile);
        QTextStream out(&outFile);

        QString s;
        while ( !in.atEnd() ) {        // until end of file...
            s = in.readLine();       // line of text excluding '\n'
            // for s, do link/mkdir.
            if ( s.right(1) == "/" ) {
                mkdir( s.toAscii(), 0777 );
                //possible optimization: symlink directories
                //that don't exist already. -- Risky.
            } else {
                QFileInfo ffi( s );
                //Don't try to symlink if a regular file already exists.
                if ( !ffi.exists() || ffi.isSymLink() ) {
                    symlink( QDir::cleanPath(location+s).toAscii(), s.toAscii() );
                    out << s << "\n";
                } else {
                    qWarning( "%s  exists already, not symlinked", s.toAscii().constData() );
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

    if ( inFile.open(QIODevice::ReadOnly) ) {
        QTextStream in(&inFile);

        QString s;
        while ( !in.atEnd() ) {        // until end of file...
            s = in.readLine();       // line of text excluding '\n'
            QFileInfo ffi( s );
            //Confirm that it's still a symlink.
            if ( ffi.isSymLink() )
                unlink( s.toAscii() );
            else
                qWarning( "Not removed %s", s.toAscii().constData() );
        }
        inFile.close();
        inFile.remove();
    }
}



/*
  Slightly hacky: we can't use QStorageMetaInfo, since we don't have a
  QApplication. We look for filesystems that have the directory
  /usr/lib/ipkg/info, and assume that they are removable media
  with packages installed. This is safe even if eg. /usr is on a
  separate filesystem, since then we would be testing for
  /usr/usr/lib/ipkg/info, which should not exist.
 */

static void updateSymlinks()
{
    QDir lists( listDir );
    QStringList knownPackages = lists.entryList( QStringList("*.list") ); // No tr

    struct mntent *me;
    FILE *mntfp = setmntent( "/proc/mounts", "r" );

    if ( mntfp ) {
        while ( (me = getmntent( mntfp )) != 0 ) {
            QString root = me->mnt_dir;
            if ( root == "/" )
                continue;

            QString info = root + infoDir;
            QDir infoDir( info );
            if ( infoDir.isReadable() ) {
                const QFileInfoList packages = infoDir.entryInfoList( QStringList("*.list") ); // No tr
                foreach (QFileInfo fi, packages)
                {
                    if ( knownPackages.contains( fi.fileName() ) ) {
                        knownPackages.removeAll( fi.fileName() );
                    } else {
                        //it's a new one
                        createSymlinks( root + "/", fi.baseName() );
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
