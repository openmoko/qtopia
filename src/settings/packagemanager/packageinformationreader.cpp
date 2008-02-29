/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA All rights reserved.
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

#include "packageinformationreader.h"
#include "packagecontroller.h"

#include <QFile>
#include <QTextStream>


/**
  Base constructor
*/
PackageInformationReader::PackageInformationReader()
    : isError( false )
    , hasContent( false )
    , accumulatingFullDesc( false )
{
    reset();
}

/**
  Take a stream onto a control file and return the decoded
  package information.

  Set isError == true if the information format is wrong.
*/
PackageInformationReader::PackageInformationReader( QTextStream &ts )
    : isError( false )
    , hasContent( false )
    , accumulatingFullDesc( false )
{
    PackageInformationReader();
    while (!ts.atEnd())
    {
        QString line = ts.readLine();
        readLine( line );
    }
}

/**
  Take a filename of a control file and return the decoded
  package information.

  Set isError == true if the information format is wrong.
*/
PackageInformationReader::PackageInformationReader(const QString& fName)
    : isError( false )
    , hasContent( false )
    , accumulatingFullDesc( false )
{
    PackageInformationReader();
    QString fileName( fName );
    // open file for reading
    QFile file(fileName);

    int pos = fileName.indexOf( QString( "/" ) + AbstractPackageController::INFORMATION_FILE );
    if ( pos > 0 )  // contains a path to a control file, at least 1 char before the "/"
    {
        int dirNamePos = fileName.lastIndexOf( "/", pos - fileName.length() - 1 );
        // default the package name to the directory above the control file
        pkg.name = fileName.mid( dirNamePos + 1, pos - dirNamePos - 1 );
    }
    if (!file.exists()) {
        pkg.status = (InstallControl::InstallStatus)( InstallControl::PartlyInstalled | InstallControl::Error );
        pkg.description = fileName + " does not exist";
        return;
    }

    file.open(QIODevice::ReadOnly);

    QTextStream textStream(&file);

    while (!textStream.atEnd()) {
        QString line = textStream.readLine();

        readLine( line );
    }
    if ( !pkg.isComplete() )
        pkg.status = (InstallControl::InstallStatus)( InstallControl::PartlyInstalled | InstallControl::Error );
}

void PackageInformationReader::reset()
{
    pkg.description = QString::null;
    pkg.fullDescription = QString::null;
    pkg.size = QString::null;
    pkg.section = QString::null;
    pkg.domain = QString::null;
    pkg.packageFile = QString::null;
    pkg.version = QString::null;
    pkg.trust = QString::null;
    pkg.files.clear();
    error = QString::null;
    isError = false;
    hasContent = false;
}

void PackageInformationReader::checkCompleted()
{
    if ( hasContent && pkg.isComplete() )
    {
        emit packageComplete();
        reset();
    }
}

/**
  Read a line of package information eg:
  \code
      Package: FoobarBaz
  \endcode
*/
void PackageInformationReader::readLine( const QString &line )
{
    if ( line.isEmpty() )
    {
        checkCompleted();
        return;
    }
    bool isDescContinuation = line.startsWith( " " );
    QString lineStr = line.trimmed();
    if ( lineStr.length() == 0 )
    {
        checkCompleted();
        return;
    }
    int colon = line.indexOf(':');
    if ( colon == -1 )
    {
        pkg.name = "corrupted";  // NO TR
        pkg.status =  (InstallControl::InstallStatus)( pkg.status | InstallControl::Error );
        error = "No colon in package information"; // NO TR
        isError = true;
        return;
    }
    colon += 2;
    if ( isDescContinuation && accumulatingFullDesc )
    {
        pkg.fullDescription += "\n";
        pkg.fullDescription += line;
        return;
    }
    accumulatingFullDesc = false;
    if ( lineStr.startsWith( QLatin1String( "Package:" )))
    {
        checkCompleted();
        pkg.name = lineStr.mid( colon ).trimmed();
        if ( !pkg.name.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Description:" ), Qt::CaseInsensitive ))
    {
        pkg.description = lineStr.mid( colon ).trimmed();
        if ( !pkg.description.isEmpty() ) hasContent = true;
        accumulatingFullDesc = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Size:" ), Qt::CaseInsensitive ))
    {
        pkg.size = lineStr.mid( colon ).trimmed();
        if ( !pkg.description.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Section:" ), Qt::CaseInsensitive ))
    {
        pkg.section = lineStr.mid( colon ).trimmed();
        if ( !pkg.section.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Domain:" ), Qt::CaseInsensitive ))
    {
        pkg.domain = lineStr.mid( colon ).trimmed();
        if ( !pkg.domain.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Filename:" ), Qt::CaseInsensitive ))
    {
        pkg.packageFile = lineStr.mid( colon ).trimmed();
        if ( !pkg.packageFile.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "MD5Sum:" ), Qt::CaseInsensitive ))
    {
        pkg.md5Sum = lineStr.mid( colon ).trimmed();
        if ( !pkg.md5Sum.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Trust:" ), Qt::CaseInsensitive ))
    {
        pkg.trust = lineStr.mid( colon ).trimmed();
        if ( !pkg.trust.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Version:" ), Qt::CaseInsensitive ))
    {
        pkg.version = lineStr.mid( colon ).trimmed();
        if ( !pkg.version.isEmpty() ) hasContent = true;
    }
    else if ( lineStr.startsWith( QLatin1String( "Files:" ), Qt::CaseInsensitive ))
    {
        QString fileList = lineStr.mid( colon ).trimmed();
        pkg.files = fileList.split( QLatin1String( " " ), QString::SkipEmptyParts );
        if ( !pkg.files.isEmpty() ) hasContent = true;
    }
    else
    {
        // legacy/irrelevant fields not currently an error
    }
}


