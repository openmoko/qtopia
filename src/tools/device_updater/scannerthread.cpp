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

#include "scannerthread.h"
#include "packagescanner.h"

#include <QFileInfo>
#include <QDateTime>
#include <QDir>

// granularity of the progress bar, in files, ie update the indicator
// every this many files
#define FILE_INC_THRESHOLD 10

int ScannerThread::numberOfFiles = DEFAULT_PROGRESS_MAX;

/*!
  Handle either a qpd or qpk file, depending on \a suffix.  The details of the
  file are in the \a file.
*/
void ScannerThread::handleFile( const QString &suffix, const QFileInfo &file )
{
    bool newPackage = false;
    PackageItem *package;
    package = mParent->findPackageByName( file.baseName() );
    if ( !package )
    {
        package = new PackageItem();
        // qDebug() << "Found" << e.baseName();
        newPackage = true;
    }
    if ( suffix == "qpk" )
    {
        package->qpkPath = file.filePath();
        mParent->progressValue( mParent->rowCount( QModelIndex() ));
    }
    if ( suffix == "qpd" )
        package->qpdPath = file.filePath();
    if ( newPackage )
    {
        package->name = file.completeBaseName();
        QStringList nameParts = package->name.split( "_" );
        package->display = "<bad package name>  " + package->name;
        if ( nameParts.count() < 3 )
            qWarning( "bad package name" );
        else
            package->display = QString( "%1 for %2 - ver %3" ).arg( nameParts[0] )
                .arg( nameParts[2] ).arg( nameParts[1] );
        package->lastMod = file.lastModified();
        package->hasBeenUploaded = false;
        mParent->appendPackage( package );
    }
}

/*!
  \reimp
  Scan thru the current directory and all sub-directories looking for
  packages or descriptors.
*/
void ScannerThread::run()
{
    Q_ASSERT( mParent != NULL );
    QDir packageDir(".");
    QFileInfoList entries = packageDir.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );
    int fileCount = 0;
    int progressInc = FILE_INC_THRESHOLD;
    while ( entries.count() > 0 )
    {
        fileCount++;
        progressInc--;
        if ( progressInc == 0 )
        {
            // number of files changed dramatically betw runs, correct
            if ( fileCount > numberOfFiles )
                numberOfFiles = fileCount;
            // only update bar if not too close to the end
            if ( fileCount + FILE_INC_THRESHOLD < numberOfFiles )
                mParent->progressValue( (int)( DEFAULT_PROGRESS_MAX * fileCount / numberOfFiles ));
            progressInc = FILE_INC_THRESHOLD;
        }
        QFileInfo e = entries.takeFirst();
        if ( e.isDir() )
        {
            QDir sub( e.filePath() );
            QFileInfoList subEntries = sub.entryInfoList( QDir::AllEntries | QDir::NoDotAndDotDot );
            while ( subEntries.count() > 0 )
                entries.append( subEntries.takeFirst() );
        }
        else
        {
            QString suf = e.suffix();
            if ( suf == "qpk" || suf == "qpd" )
                handleFile( suf, e );
        }
    }
    numberOfFiles = fileCount;
}
