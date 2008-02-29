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

#include "installedpackagescanner.h"
#include "packagecontroller.h"
#include "packageinformationreader.h"

#include <qtopialog.h>
#include <qtopianamespace.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QEventLoop>

#include <qdebug.h>

InstalledPackageScanner::InstalledPackageScanner( QObject *parent )
    : QThread( parent )
    , aborted( false )
{
    pkgController = qobject_cast<AbstractPackageController*>(parent);
}

InstalledPackageScanner::~InstalledPackageScanner()
{
}

/*!
  Set the list of locations to be scanned to \a l

  Any duplicates in the list \a l are ignored.
*/
void InstalledPackageScanner::setLocations( const QStringList &l )
{
    locations.clear();
    for ( int loc = 0; loc < l.count(); ++loc )
        if ( !locations.contains( l[loc] ))
            locations.append( l[loc] );
}

void InstalledPackageScanner::run()
{
    setTerminationEnabled();
    eventLoop = new QEventLoop();

    QList< InstallControl::PackageInfo> pkgList;
    for ( int i = 0; i < locations.count() && !aborted; ++i )
    {
        if ( !locations[i].endsWith("/") )
            locations[i] += "/";
        QDir controlsDir( locations[i] );
        if ( !controlsDir.exists() )
        {
            qLog(Package) << controlsDir.path() << "does not exist";
            continue;  // could be unmounted, not an error
        }
        QFileInfoList flist = controlsDir.entryInfoList();
        for ( int i = 0; i < flist.count() && !aborted; ++i )
        {
            if ( !flist[i].filePath().endsWith( AbstractPackageController::INFORMATION_FILE ) ) 
                continue;
            pkgList.append( scan( flist[i].filePath() ) );
            eventLoop->processEvents();
        }
    }
    qSort( pkgList );
    foreach ( InstallControl::PackageInfo pkg, pkgList )
        pkgController->addPackage( pkg ); 
    delete eventLoop; // cant parent to this, due to thread
}

/*!
  Scan the control file.  If the control file does not exist
  the package state will be InstallControl::PartlyInstalled |
  InstallControl::Error

  Other error states possible are described by PackageInformationReader
  and InstallControl::PackageInfo
*/
InstallControl::PackageInfo InstalledPackageScanner::scan( const QString &controlPath )
{
        PackageInformationReader informationReader( controlPath );
        InstallControl::PackageInfo pkg = informationReader.package();
        if ( pkg.md5Sum.isEmpty() )
            pkg.md5Sum =controlPath.mid( controlPath.lastIndexOf("/") + 1, 32 );  
            
        QString md5Sum = pkg.md5Sum;
            
        if ( isPackageEnabled( md5Sum ) )
            pkg.isEnabled = true;
         else
            pkg.isEnabled = false;

        return pkg;
}

bool InstalledPackageScanner::isPackageEnabled( const QString &md5Sum ) const
{
    //TODO: This does not handle the case where the package is installed on a media card
    QDir installSystemBinPath( Qtopia::packagePath() + "/bin" );
    QFileInfoList links = installSystemBinPath.entryInfoList( QStringList(md5Sum + "*" ), QDir::System );
    foreach( QFileInfo link, links )
    {
        if ( link.symLinkTarget().endsWith( InstalledPackageController::DISABLED_TAG ) )
            return false;
    }
    return true;
}

bool InstalledPackageScanner::isPackageInstalled( const QString &md5Sum ) const
{
    QDir installSystemBinPath( Qtopia::packagePath() + "/controls" );
    QFileInfoList links = installSystemBinPath.entryInfoList( QStringList(md5Sum + "*" ) );
   
    if( links.count() == 0 )
        return false;
    else    
        return true;    
}


void InstalledPackageScanner::cancel()
{
    aborted = true;
    quit();
}
