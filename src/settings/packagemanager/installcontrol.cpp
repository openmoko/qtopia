/****************************************************************************
// create links for the icon, binary
 **
 ** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#include "installcontrol.h"
#include "packagecontroller.h"
#include "packageview.h"
#include "sandboxinstall.h"
#include "targz.h"

#include <QDir>
#include <QDebug>
#include <QProcess>

#include "packageinformationreader.h"

#include <qcontent.h>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <qstorage.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <errno.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

/**
  \internal
  All packages (on a particular file system/storage device) are installed
  in the QPackageRegistry::packageDirectory, under a <packagename> directory
  */

InstallControl::InstallControl()
{
}

InstallControl::~InstallControl()
{
}

/*!
  Install the package \a pkg onto the device.

  The package is installed into the Qtopia::packagePath() directory
  which is an internal writable location.

  If the Qtopia::packagePath() directory is not on the file-system of
  the media chosen by setInstallMedia() then after the install to the
  media file-system, links are created in the Qtopia::packagePath()
  so that Qtopia's launching and resource systems will work.

  If the package is a system package then is then re-installed under the
  standard Qtopia directory tree, under the install destination, after the
  system package verification has been done.  This includes certificate
  validation and other checks.
*/
bool InstallControl::installPackage( const InstallControl::PackageInfo &pkg, ErrorReporter *reporter ) const
{
    SandboxInstallJob job( &pkg, m_installMedia, reporter );
    if ( job.isAborted() )
        return false;

    QString dataTarGz = job.destinationPath() + "/data.tar.gz";

    // install to directory
    QString packageFile = pkg.packageFile;
    packageFile.prepend( Qtopia::tempDir() );
    targz_extract_all( packageFile, job.destinationPath() );

    // extract data part
    targz_extract_all( dataTarGz, job.destinationPath() );

    // remove data package
    QFile dataPackage( dataTarGz );
    dataPackage.remove();

    QFile tmpPackage( packageFile );
    tmpPackage.remove();

    if ( !verifyPackage( job.destinationPath(), pkg, reporter ))
        return false;

    job.registerPackageFiles();

    if ( pkg.isSystemPackage() )
    {
        QStorageMetaInfo storage;
        QStringList dirs = Qtopia::installPaths();
        QString systemRootPath;
        for ( int i = 0; i < dirs.count(); ++i )
        {
            const QFileSystem *f = storage.fileSystemOf( dirs[i] );
            if ( f->isWritable() )
                systemRootPath = dirs[i];
        }
        if ( systemRootPath.isEmpty() )
        {
            qWarning( "******* No writeable system path for system package *******" );
            job.setupSandbox();
        }
        else
        {
            qLog(Package) << "promoting to system package:  move from" << job.destinationPath()
                << "to" << systemRootPath;
            if ( !targz_extract_all( dataTarGz, systemRootPath ))
            {
                PackageView::displayMessage( QObject::tr( "<font color=\"#CC0000\">System Package Error</font> "
                            "possible file name collision %1", "%1 = file name" ).arg( pkg.name ));
            }
            job.removeDestination();
        }
    }
    else
    {
        job.setupSandbox();
    }

    job.installContent();

    return true;
}

/*!
  Verify the contents of the unzipped package before finally registering the
  installation.  If the contents fail verification a QMesssageBox is displayed
  to the user, and the package is marked as disabled.

  Checks made in the verification step are:
  \list
  \o Is the MD5 sum and/or GPG signature valid? (Not currently implemented)
  \o Is the control file valid, ie not corrupt?
  \o Do the security domains declared in the package information (received
  at browse time eg via http) match the domains declared in the control file?
  \o If not certified, are the domains within the set allowed for untrusted applications? (Not currently implemented)
  \o If shipped with a certificate, is it valid against our root store? (Not currently implemented)
  \endlist
  */
bool InstallControl::verifyPackage( const QString &packagePath, const InstallControl::PackageInfo &pkg, ErrorReporter *reporter ) const
{
    PackageInformationReader infoReader( packagePath + QDir::separator() +
            AbstractPackageController::INFORMATION_FILE );
    if ( infoReader.getIsError() )
    {
        QString message = PackageView::tr( "<font color=\"#CC0000\">Package Error</font> "
                    "Package %1 info corrupted, will be disabled."
                    "Re-download, or contact the supplier" ).arg( pkg.name );

        if( reporter )
            reporter->reportError( message );
        else
            PackageView::displayMessage( message );

        return false;
    }
    if ( infoReader.domain() != pkg.domain )
    {
        QString message = PackageView::tr( "<font color=\"#CC0000\">Domain Error</font> "
                    "Package %1 has security problem, will be disabled. "
                    "Contact supplier. (Declared: %2, Install: %3)", "%1 = package name, %2 = declared domain, %3 = install domain" )
                .arg( pkg.name ).arg( pkg.domain ).arg( infoReader.domain() );

        if( reporter )
            reporter->reportError( message );
        else
            PackageView::displayMessage( message );

        return false;
    }
    QString certPath = infoReader.trust();
    if ( certPath.isEmpty() || certPath == "Untrusted" ) // untrusted package
        return true;
    QFileInfo certFile( packagePath + QDir::separator() + certPath );
    if ( !certFile.exists() )
    {
        qWarning( "Failed verification: cert file %s not found", qPrintable( certFile.filePath() ));

        return false;
    }
    if ( certFile.isFile() )
    {
        return verifyCertificate( certFile.filePath() );
    }
    else if ( certFile.isDir() )  // directory with cert alternatives, one at least must verify
    {
        QDir certDir( certFile.filePath() );
        QFileInfoList flist = certDir.entryInfoList();
        for ( int f = 0; f < flist.count(); ++f )
        {
            if ( flist[f].isFile() && verifyCertificate( flist[f].filePath() ))
                return true;
        }
        return false;
    }
    else
    {
        // was something other than a file or a directory, that's got to be bad
        return false;
    }

    // TODO - Security Monitor / Launcher needs to mark packages as disabled
    return true;
}

bool InstallControl::verifyCertificate( const QString &certPath ) const
{
    // TODO - Implement certificate verification
    Q_UNUSED( certPath );
    return false;
}
