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
#include "version.h"
#ifndef QT_NO_SXE
#include "qpackageregistry.h"
#endif
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

SimpleErrorReporter::SimpleErrorReporter( ReporterType type, const QString &pkgName )
{
    packageName = pkgName;
    switch ( type )
    {
        case ( Install ):
            prefix = QObject::tr("<b>Install Failed</b> for %1: ", "%1 = package name")
                       .arg( packageName ); 
            break;
        case ( Uninstall ):
            prefix = QObject::tr("<b>Uninstall Failed</b> for %1: ", "%1 = package name" ) 
                        .arg( packageName ); 
            break;
        case ( Other ):
        default: 
            prefix = "";
     }

}

void SimpleErrorReporter::doReportError( const QString &simpleError, const QString &detailedError )
{
   
    QString userVisibleError = prefix + simpleError;
    QString logError = userVisibleError + "\n" + detailedError;  
    PackageView::displayMessage( userVisibleError );
    qWarning( qPrintable( logError ) );
}

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
bool InstallControl::installPackage( const InstallControl::PackageInfo &pkg, const QString &md5Sum, ErrorReporter *reporter ) const
{
    if( pkg.md5Sum != md5Sum || md5Sum.length() != 32 )
    {
        qLog(Package) << "MD5 Sum mismatch! Header MD5:" << pkg.md5Sum << "Package MD5:" << md5Sum;

        QString simpleError = PackageView::tr( "Invalid package or package was corrupted. "
                                            "Re-download or contact package supplier");
        QString detailedError = PackageView::tr( "InstallControl::installPackage:- MD5 Sum mismatch, "
                                                    "Descriptor MD5: %1, Package MD5: %2")
                                .arg( pkg.md5Sum ).arg( md5Sum ); 
        if( reporter )
            reporter->reportError( simpleError, detailedError );

        return false;
    }

    SandboxInstallJob job( &pkg, m_installMedia, reporter );
    if ( job.isAborted() )
        return false;

#ifndef QT_NO_SXE
    QObject::connect( &job, SIGNAL(newBinary(SxeProgramInfo &)),
            QPackageRegistry::getInstance(), SLOT(registerBinary(SxeProgramInfo &)) );
#endif

    QString dataTarGz = job.destinationPath() + "/data.tar.gz";

    // install to directory
    QString packageFile = pkg.packageFile;
    packageFile.prepend( Qtopia::tempDir() );

    if( !targz_extract_all( packageFile, job.destinationPath(), false ) )
    {
        if( reporter )
        {
            reporter->reportError( QObject::tr( "Unable to unpack package" ), 
                        QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                            .arg( packageFile ) 
                            .arg( job.destinationPath() ));
        }
        job.removeDestination();
        return false;
    }

    // extract data part
    if( !targz_extract_all( dataTarGz, job.destinationPath(), false ) )
    {
        if( reporter )
        {
            reporter->reportError( QObject::tr( "Unable to unpack package" ), 
                        QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                            .arg( dataTarGz )
                            .arg( job.destinationPath() ));
        }
        job.removeDestination();
        return false;
    }

    // remove data package
    QFile dataPackage( dataTarGz );
    dataPackage.remove();

    QFile tmpPackage( packageFile );
    tmpPackage.remove();

    if ( !verifyPackage( job.destinationPath(), pkg, reporter ))
    {
        job.removeDestination();
        return false;
    }

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
            if ( !job.setupSandbox() )
            {
                job.removeDestination();

                return false;
            }
        }
        else
        {
            qLog(Package) << "promoting to system package:  move from" << job.destinationPath()
                << "to" << systemRootPath;
            if ( !targz_extract_all( dataTarGz, systemRootPath ))
            {
                if( reporter )
                {
                    reporter->reportError( QObject::tr( "Unable to unpack package" ),
                                QString( "InstallControl::installPackage:- Could not untar %1 to %2" )
                                    .arg( dataTarGz )
                                    .arg( job.destinationPath() ));
                }
                job.removeDestination();
                return false;
            }
            job.removeDestination();
        }
    }
    else
    {
        if (!job.setupSandbox())
        {
            job.removeDestination();
            return false;
        }
    }

    if( !job.installContent() )
    {
        job.removeDestination();
        return false;        
    }

    return true;
}
/*!
  Uninstall the package \a pkg off the device.
*/
void InstallControl::uninstallPackage( const InstallControl::PackageInfo &pkg, ErrorReporter *reporter ) const
{
    SandboxUninstallJob job( &pkg, m_installMedia, reporter );
    job.unregisterPackageFiles();
    job.dismantleSandbox();
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
        if( reporter )
        {
            QString simpleError = PackageView::tr("Invalid package, contact package supplier");
            QString detailedError = "InstallControl::verifyPackage:- Error during reading of packgae information file"; 
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }

    if ( infoReader.domain() != pkg.domain )
    {
        if( reporter )
        {
            QString simpleError = PackageView::tr( "Package security domains inconsistent with declared domains,"
                                           "Contact package supplier");
            QString detailedError("InstallControl::verifyPackage:- Declared domain(s): %1, Install domain(s): %2");
            detailedError.arg( pkg.domain ).arg( infoReader.domain() );
            reporter->reportError( simpleError, detailedError );
        }
        return false;
    }

    if ( !infoReader.md5Sum().isEmpty() )
    {
        if ( reporter )
        {
           reporter->reportError( PackageView::tr( "Invalid package, contact package supplier" ),
                PackageView::tr( "InstallControl::verifyPackage:- Control file should not contain md5" ));  
        }
        return false;
    }
       
    if ( !VersionUtil::checkVersionLists( Qtopia::compatibleVersions(), infoReader.qtopiaVersion() ) )
    {
        if ( reporter )
        {
            QString detailedError( "InstallControl::verifyPackage:- Control file's qtopia version incompatible. "
                                   "Package's compatible Qtopia Versions %1, Qtopia Version %2");
            detailedError = detailedError.arg( infoReader.qtopiaVersion(), Qtopia::version() );
            reporter->reportError( PackageView::tr( "Invalid package, contact package supplier" ),
                            detailedError );
        }
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
