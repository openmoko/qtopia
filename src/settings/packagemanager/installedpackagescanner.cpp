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
    for ( int i = 0; i < locations.count() && !aborted; ++i )
    {
        if ( !locations[i].endsWith("/") )
            locations[i] += "/";
        QDir locDir( locations[i] );
        if ( !locDir.exists() )
        {
            qLog(Package) << locDir.path() << "does not exist";
            continue;  // could be unmounted, not an error
        }
        QFileInfoList flist = locDir.entryInfoList();
        for ( int i = 0; i < flist.count() && !aborted; ++i )
        {
            if ( flist[i].fileName().startsWith(".")
                    || !flist[i].isDir() ) continue;
            scan( flist[i].filePath() );
            eventLoop->processEvents();
        }
    }
    delete eventLoop; // cant parent to this, due to thread
}

/*!
  Scan the location for information about a package.  If the location is
  a directory (and not "." or "..", or anything else starting with a ".")
  then it assumed to be a package.  If the file "control" does not exist
  in the directory then the package state will be
  InstallControl::PartlyInstalled | InstallControl::Error

  Other error states possible are described by PackageInformationReader
  and InstallControl::PackageInfo
*/
void InstalledPackageScanner::scan( const QString &loc )
{
    QString controlFile = loc + "/" + AbstractPackageController::INFORMATION_FILE;
    qLog(Package) << "getting info for" << controlFile;
    if ( QFile::exists( controlFile ))
    {
        PackageInformationReader informationReader( controlFile );
        pkgController->addPackage( informationReader.package() );
    }
}

void InstalledPackageScanner::cancel()
{
    aborted = true;
    quit();
}
