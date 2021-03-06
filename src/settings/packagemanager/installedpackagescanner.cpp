/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

        if ( isPackageEnabled( pkg ) )
            pkg.isEnabled = true;
        else
            pkg.isEnabled = false;

        return pkg;
}

bool InstalledPackageScanner::isPackageEnabled( const InstallControl::PackageInfo &pkgInfo )
{
    QString md5Sum = pkgInfo.md5Sum;
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

bool InstalledPackageScanner::isPackageInstalled( const InstallControl::PackageInfo &pkgInfo )
{
    QDir controlsDir( AbstractPackageController::INSTALLED_INFO_FILES_LOC );
    QFileInfoList flist = controlsDir.entryInfoList();

    InstallControl::PackageInfo pkgInfoControl;
    for ( int i = 0; i < flist.count(); ++i )
    {
        if ( !flist[i].filePath().endsWith( AbstractPackageController::INFORMATION_FILE ) )
            continue;
        pkgInfoControl = scan( flist[i].filePath() );
        if ( pkgInfo == pkgInfoControl )
            return true;
    }
    return false;
}


void InstalledPackageScanner::cancel()
{
    aborted = true;
    quit();
}
