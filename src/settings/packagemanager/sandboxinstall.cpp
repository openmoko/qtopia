/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
/Q** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "sandboxinstall.h"
#include "packageview.h"

#include <qtopianamespace.h>
#include <qcontent.h>

#ifdef QT_NO_SXE
#define PACKAGE_DIRECTORY "packages"
#else
#include <qpackageregistry.h>
#define PACKAGE_DIRECTORY QPackageRegistry::packageDirectory
#include <qmutex.h>
#endif

#include <errno.h>
#include <qdebug.h>
#include <qtopialog.h>

#include <QDir>
#include <QProcess>

#include <qcontentfilter.h>
#include <qcontentset.h>
#include <qcontent.h>
/*!
  Construct a new sandbox install job object, ensuring the sandbox
  root exists, and creating the destination directory for the
  package to be expanded into.

  The destination directory is below Qtopia::packagePath() and is formed from
  the md5sum of the package.

  The \a pkg points to the package which will be installed.

  If \a m is non-empty it specifies the file-system path mount-point
  of a media card (or other writeable file-system) on which the package
  will be located.

  In this case the destination directory will be a symlink from the
  system root to the location on the media card.
  */
SandboxInstallJob::SandboxInstallJob( const InstallControl::PackageInfo *pkg, const QString &m, ErrorReporter *errorReporter )
    : package( pkg )
    , media( m )
    , abort( false )
    , reporter( errorReporter )
{
    
    QDir sandboxRoot( Qtopia::packagePath() );
    destination = sandboxRoot.path() + "/" + pkg->md5Sum;
    Q_ASSERT( sandboxRoot.exists() );
    if ( !pkg->isComplete() )   // must have the md5Sum value, plus content!
    {
        
        if( reporter )
        {
            QString simpleError =  QObject::tr( "Invalid package, contact package supplier" );
            QString detailedError( "SandboxInstallJob::SandboxInstallJob:- PackageInfo pkg is incomplete" );
            reporter->reportError( simpleError, detailedError );
        }

        abort = true;
        return;
    }
    if ( sandboxRoot.exists( pkg->md5Sum ))
    {
        if( reporter )
        {
            QString simpleError = QObject::tr( "Package already installed " );
            QString detailedError( QString("SandboxInstallJob::SandboxInstallJob:- "
                                       "Package directory matching %1 already exists")
                                        .arg( pkg->md5Sum ));
            reporter->reportError( simpleError, detailedError );
        }

        abort = true;
        return;
    }
    if ( media.isEmpty() )
    {
        if ( !sandboxRoot.mkdir( pkg->md5Sum ))
        {
            if( reporter )
            {
                QString simpleError = QObject::tr( "Unable make destination for package" );
                QString detailedError = QString( "SandboxInstallJob::SandboxInstallJob:- "
                                               "Could not create directory %1").arg( pkg->md5Sum);
                reporter->reportError( simpleError, detailedError );
            }
                
            abort = true;
            return;
        }
    }
    else
    {
        mediaSandboxRoot();
    }
}

/*!
  \fn QString SandboxInstallJob::destinationPath() const

  Return the path within the Qtopia::packagePath() directory where the sandbox
  directories will be set up for this install.

  If this is an install to a media card, then these will be symlinks.

  The name of the directory is based on the md5sum of the package.
*/

/*!
  \fn bool SandboxInstallJob::isAborted() const

  Return true if the install job has been aborted.  A warning will have been
  displayed on stderr, and possibly to the screen of the device.
*/

/*!
  Create a sandbox install directory on the media card for this package
  and if necessary create the media card "package" directory parent.
  The sandbox install directory is symlinked to the main package root.
*/
void SandboxInstallJob::mediaSandboxRoot()
{
    QDir mediaSandbox( media + "/" + PACKAGE_DIRECTORY );
    if ( !mediaSandbox.exists() )  // try creating it from parent
    {
        QDir mediaDir( media );
        if ( !mediaDir.exists() )
        {
            //TODO: Proper error reporting for media card errors
            qWarning( "### %s (the media path) doesnt exist! ###", qPrintable( media ));
            abort = true;
            return;
        }
        qLog(Package) << "Creating directory" << mediaSandbox.path() << "in path" << media;
        if ( !mediaDir.mkdir( PACKAGE_DIRECTORY ))
        {
            //TODO: Proper error reporting for media card errors
            qWarning( "mkdir failed : %s", strerror( errno ));
            abort = true;
            return;
        }
    }
    QString linkTarget = mediaSandbox.path() + "/" + package->md5Sum;
    QFile::link( linkTarget, destination );
    destination = linkTarget;
    if ( mediaSandbox.exists( package->md5Sum ))
    {
        // overwriting with reinstalled package
        removeDestination();
    }
    mediaSandbox.mkdir( package->md5Sum );
}

/*!
  Remove the destination directory for the sandboxed package and
  everything below it.

  Use this method with care.
  */
void SandboxInstallJob::removeDestination() const
{
    QDir destDir( destination );
    if ( !destDir.exists() )
    {
        qWarning( "Request to remove non-existent directory %s", qPrintable( destination ));
        return;
    }
    QString cmd( "rm -Rf " );
    cmd += destination;
    Q_ASSERT( !destination.isEmpty() && destination != "/" );
    qLog(Package) << "Removing destination by executing:" << cmd;
    QProcess::execute( cmd );
    
    QDir dir( Qtopia::packagePath() );
    if ( dir.cd("tmp") )
    {
        QFileInfoList fileList= dir.entryInfoList( QDir::Files );
        QFile f;
        foreach( QFileInfo fi, fileList )
        {
            f.setFileName( fi.absoluteFilePath() );
            f.remove();
        }
    }
    clearMiscFiles();
}

/*!
  Register all the files in the directory tree at \a path with the Qtopia
  system, including SXE and Document systems.
  */
bool SandboxInstallJob::registerPackageFiles( const QString &path )
{
#ifndef QT_NO_SXE
    static SxeProgramInfo pi;
    static QMutex piMutex;
    static bool init = false;

    piMutex.lock();
    if ( !init )
    {
        QPackageRegistry::getInstance()->initProgramInfo( pi );
        init = true;
    }
    piMutex.unlock();
#endif

    QDir directory( path.isEmpty() ? destination : path );

    qLog(Package) << "registering package files in" << directory.path();
    QFileInfoList fileList = directory.entryInfoList();
    for (int fileI = 0; fileI < fileList.count(); ++fileI)
    {
        QFileInfo fileInfo = fileList.at(fileI);

        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        // recurse to other directories
        if (fileInfo.isDir()) {
            registerPackageFiles( fileInfo.filePath() );
        }
        else if ( fileInfo.isFile() )
        {
            if ( fileInfo.suffix() == "desktop" )
            {
                qLog(Package) << "Adding to dot.desktop files" << fileInfo.path();
                desktopPaths << fileInfo.filePath();
            }
            else if ( fileInfo.isExecutable() )
            {
                if ( !fileInfo.isWritable() )
                {
                    QFile::Permissions perms = QFile::permissions( fileInfo.filePath() );
                    if ( !QFile::setPermissions( fileInfo.filePath(), perms | QFile::WriteOwner ) )
                    {
                        if ( reporter )
                        {
                            QString detailedError( "SandboxInstallJob::registerPackageFiles:- "
                                                "%1 is not writable and could not change permissions");
                            detailedError = detailedError.arg( qPrintable(fileInfo.filePath()) );
                            reporter->reportError( tr( "Unable to set permissions on package" ),
                                                    detailedError );
                        }
                        return false;
                    }
                }
#ifndef QT_NO_SXE
                QString registerPath = directory.path();
                registerPath.remove( Qtopia::packagePath() );
                
                piMutex.lock();
                pi.fileName = fileInfo.fileName();
                pi.relPath = registerPath;
                pi.runRoot = Qtopia::packagePath();
                pi.domain = package->domain;
                emit newBinary( pi );
                piMutex.unlock();
                runSandbox( directory.path() + "/" + fileInfo.fileName() );
#endif
            }
        }
    }
    return true;
}

bool SandboxInstallJob::installContent()
{
    if ( desktopPaths.count() == 0 )
    {
        qLog(Package) << "SandboxInstallJob::installContent - No desktop files!";
        return true;
    }
    QList<QContentId> cids;
    foreach ( QString d, desktopPaths )
    {
        
        QContentId cid = QContent::install( d );
        if ( cid == QContent::InvalidId )
        {
            if ( reporter )
            {
                QString detailedError( "Unable to install %1 to database" );
                detailedError = detailedError.arg( d );  
                reporter->reportError( QObject::tr("Unable to install package contents into database "),
                                        detailedError ); 
            }

            //uninstall previously install desktop files
            foreach( QContentId id, cids ) 
               QContent::uninstall( id );
            
            return false;
            
        }
        cids << cid;

        QContent content( cid );
        content.setCategories( QStringList("Packages") );
        
        if( !content.commit() )
        {  
            if ( reporter )
            {
                QString detailedError( "Unable to commit %1 to database" );
                detailedError = detailedError.arg( d );  
                reporter->reportError( QObject::tr("Unable to commit package contents to database "),
                                        detailedError ); 
            }
    
            //uninstall previously install desktop files
            foreach( QContentId id, cids ) 
               QContent::uninstall( id );
            
            return false;
        }
    }
    return true;
}

/*!
   Execute the systems MAC kernel sandboxing rule on the binary
   by invoking the "sandbox" shell script, from the qtopia etc directory.

   The \a binPath is passed as an argument to the shell script
*/
void SandboxInstallJob::runSandbox( const QString &binPath )
{
    static QString sandboxCmd;
    static bool commandUnavailable = false;

    if ( commandUnavailable )
        return;

    if ( sandboxCmd.isEmpty() )
    {
        QStringList dirs = Qtopia::installPaths();
        for ( int d = 0; d < dirs.count(); ++d )
            if ( QFile::exists( dirs[d] + "/etc/qtopia_sxe/sandbox" ))
                sandboxCmd = dirs[d] + "/etc/qtopia_sxe/sandbox %1";
        if ( sandboxCmd.isEmpty() )
        {
            commandUnavailable = true;
            return;
        }
    }
    qLog(Package) << "Running command" << ( sandboxCmd.arg( binPath ));
    QProcess::execute( sandboxCmd.arg( binPath ));
}

/*!
  Create the sandbox directory structure and other settings for the untrusted
  package.  The directory structure assumes a Qtopia file system similar to
  the following:

  \image file-system.png

  Note that the installPath() includes the "package" directory, so that eg searches
  for binaries will include "package/bin".

  Here the package "BomberMan" is a game with two binaries, a library, an icon
  and several help files (not shown but assumed to be in the help/html directory).

  It has has the following entries in its bomb.desktop file:

  \code
  Name[]=Bomber Man
  Exec = game
  Icon=BomberMan/bomb
  \endcode

  The Qtopia launcher must be able to find the binary, which therefore must be
  in the installPath() and must not clash with other binary names.

  So that the untrusted packages are found last (preventing an untrusted package
  masquerading as for example "netsetup") they are placed in the "packages"
  directory heierarchy via links.  Here they are returned at the end of the
  installPath() during search for binaries to launch.

  Note also that package authors could easily create names which clash with
  each other - in the example the "game" executable in "BomberMan" clashes with
  an executable from the "CannonMan" package.

  For this reason binaries are linked into the installPath() using the packages md5sum.
  This way unless the packages are actually the same package, the binary names will not clash.
  The value of the "Exec" field is altered accordingly during installation.

  This is also important for IPC messaging since the binary name (and argv[0]) is
  used to create messaging endpoints, and therefore must be unique Qtopia-wide.

  The Qtopia launcher must also be able to find the icon.  Here again the
  installPath() is searched automatically for the value of the "Icon=" field
  so hence the BomberMan directory is symlinked from the "pic" directory.

  The items which the Qtopia system must be able to locate within a sandboxed package
  are as follows, where the paths given are relative to the packages sandbox directory:
  \list
  \o executable binary, in "bin" as listed in the "Exec=" field of .desktop
  \o quicklaunched binary, in  "plugins/applications"
  \o package launcher icon (as listed in the "Icon=" field of the .desktop file)
  \o package help files (not shown in above diagram, usually "help/html")
  \endlist

  During package installation the following files are referenced:
  \list
  \o desktop file, eg "apps/Applications/bomb.desktop"
  \o certificates, eg "etc/crt/somecorp.pem"
  \endlist

  A package can quite likely have more than one .desktop file, so there
  might be a game and a level editor, with a .desktop file each.  This will
  create an icon in the launcher view for each.

  On execution the packaged binary is launched with the following environment,
  where the path to the sandbox directory is represented by \c $SANDBOX:
  \list
  \o $SANDBOX added to its installPath()
  \o $SANDBOX/bin added to PATH
  \o $SANDBOX/lib added to LD_LIBRARY_PATH
  \o chdir() to $SANDBOX
  \endlist

  Note that since the binary is launched via the symlink named after its md5sum,
  this same string must be linked in the sandbox "pic" directory to locate image resources
  The Qtopia resource system will search in "pic/" + argv[0] for images.
*/
bool SandboxInstallJob::setupSandbox()
{
    // create links for the icon, binary
    QHash<QString,QSettings*> desktopConf;
    qDebug() << "Desktop files are" << desktopPaths;

    if ( desktopPaths.count() == 0 )
    {
        if ( reporter )
        {
            QString detailedError( "SandboxInstalLJob::setupSandbox:- "
                            "No desktop files");
            reporter->reportError( tr( "Invalid package, contact package supplier"),
                                    detailedError ) ;
        }
        return false; 
    }

    for ( int x = 0; x < desktopPaths.count(); ++x )
    {
        QFile::Permissions perms = QFile::permissions( desktopPaths[x] );
        if ( !QFile::setPermissions( desktopPaths[x], perms | QFile::WriteOwner ))
        {

            if ( reporter )
            {
                QString detailedError( "SandboxInstallJob::setupSandbox:- "
                                "%1 is not writable and could not change permissions");
                detailedError = detailedError.arg( qPrintable( desktopPaths[x]) );
                reporter->reportError( tr( "Unable to set permissions on package" ),
                                                    detailedError );
            }
            return false;
        }

        QSettings *dc = new QSettings( desktopPaths[x], QSettings::IniFormat );
        dc->beginGroup( QLatin1String( "Desktop Entry" ));
        QString exec = dc->value( QLatin1String( "Exec" )).toString();
        qLog(Package) << "found Exec field" << exec << "in" << desktopPaths[x];
        if ( exec.isEmpty() )
        {
            delete dc;
        }
        else
        {
            if ( desktopConf.contains( exec ) )
            {
                
                if ( reporter ) 
                {
                    QString detailedError( "%1 referenced by multiple desktop files, %2 and %2");
                    detailedError = detailedError.arg( exec ).arg( desktopPaths[x] ).arg( desktopConf[exec]->fileName() );
                    reporter->reportError( tr("Invalid package, contact package supplier "),    
                                            detailedError ); 
                }
                return false;
            }
                
            desktopConf[exec] = dc;
        }
    }
    QString installSystemBinPath = Qtopia::packagePath() + QLatin1String( "/bin" );
    QDir packageBin( destination + "/" + QLatin1String( "bin" ));
    
    if ( !packageBin.exists() )
    {
        if ( reporter )
        {
            reporter->reportError( tr("Invalid package, contact package vendor"),
                            QLatin1String("Package did not contain bin directory " 
                            "at package root level") );
        }
        return false;
    }
        
    QStringList newBins;
    QFileInfoList bins = packageBin.entryInfoList();
    for ( int b = 0; b < bins.count(); ++b )
    {
        if ( !bins[b].isExecutable() )
            continue;
        if ( bins[b].fileName().startsWith(".") ) // dont do invisible files or dirs
            continue;
        QString origBin = bins[b].fileName();
        QString newBin = package->md5Sum + "_" + origBin;
        newBins << newBin;
        QString linkDst = Qtopia::packagePath() + QLatin1String( "/bin/" ) + newBin;
        if ( createLink( bins[b].filePath(), linkDst ))
        {
            if ( desktopConf.contains( origBin ))
            {
                desktopConf[origBin]->setValue( "Exec", newBin );
                qLog(Package) << "Re-writing desktop file Exec from" << origBin << "to" << newBin;
    
                QString icon = desktopConf[origBin]->value( "Icon" ).toString();
                QDir packagePic( destination + "/pics/" + origBin );
                if ( packagePic.exists() ) // there is a pics directory
                {
                    linkDst = Qtopia::packagePath() + QLatin1String( "/pics/" ) + newBin;
                    createLink( packagePic.path(), linkDst );

                    if ( icon.startsWith( origBin + '/' ) )
                    {
                        QString newIcon = newBin + icon.mid( origBin.length() );
                        desktopConf[origBin]->setValue( "Icon", newIcon );
                        qLog(Package) << "Re-writing desktop file Icon from" << origBin << "to" << newBin;
                    }
                }
                desktopConf[origBin]->sync();
            }
            else
            {
                qLog(Package) << "No rewrite for non-Exec binary" << origBin;
            }
        }
    }
    QHash<QString,QSettings*>::iterator it = desktopConf.begin();
    while ( it != desktopConf.end() )
    {
        QSettings *dc = it.value();
        if ( !newBins.contains(dc->value(QLatin1String("Exec")).toString()) ) 
        {
            if ( reporter )
            {
                QString detailedMessage( "SandboxInstallJob::setupSandbox:- desktop file %1 "
                                    "does not reference a existing binary executable" );
                detailedMessage = detailedMessage.arg( dc->fileName() );
                reporter->reportError( tr("Invalid package, contact package supplier" ),
                                    detailedMessage );
            }          
            return false; 
        }
        dc->endGroup();
        dc->beginGroup( QLatin1String( "Package Data" ));
        dc->setValue( QLatin1String( "Path" ), destination );
        qLog(Package) << "deleting desktop settings object for" << it.key();
        // delete QSettings object, causes .desktop file to be persisted
        delete it.value();
        ++it;
    }
    
    //copy control into controls directory and add md5sum entry
    QDir packages( destination + "/../" );
    if ( !packages.exists("controls") && !packages.mkdir( "controls" ) )
    {
        
        if ( reporter )
        {
            reporter->reportError( tr("Unable to index package" ),
                QLatin1String("SandboxInstallJob::setupSandbox:-Could not create controls directory") );
        } 
        return false;
    }

    QString origControl( destination +  QLatin1String( "/control") );
    QString newControl = destination + "/../" + QLatin1String( "controls/" )
                            + package->md5Sum + QLatin1String( ".control" );
    QFile::copy( origControl, newControl );
    QFile newControlFile( newControl );
    if( !newControlFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
    {
        if ( reporter )
        {
            reporter->reportError( tr( "Unable to index package" ),
                QLatin1String("SandboxInstallJob::setupSandbox:-Could not modify control file" ) );
        }
        return false; 
    }
    QString md5Entry("MD5Sum: " );
    md5Entry.append( package->md5Sum + "\n" );
    newControlFile.write( md5Entry.toLatin1() );
     
    return true;
}

/*!
  Create a symlink which points to \a target.  The symlink has the path \a link.

  If possible a relative path, ie ../../path/to/link is used for the target.

  If any of the directories making up the /path/to/link are missing, they
  are created.

  No test for existence of the target file is made - thus a "dangling" link
  can be made.
*/
bool SandboxInstallJob::createLink( const QString &target, const QString &link )
{
    QStringList linkPath = link.split( "/", QString::SkipEmptyParts );
    QStringList targetPath = target.split( "/", QString::SkipEmptyParts );

    // ensure the directory that will contain the link exists
    linkPath.removeLast();
    QDir dirCheck( linkPath.join( "/" ).prepend( "/" ));
    if ( !dirCheck.exists() )
    {
        qLog(Package) << "Building directory path" << dirCheck.path();
        QString linkCheck = "";
        QDir prevDir( "/" );
        for ( int i = 0; i < linkPath.count(); ++i )
        {
            linkCheck = linkCheck + "/" + linkPath[i];
            dirCheck.setPath( linkCheck );
            if ( !dirCheck.exists() )
            {
                if ( !prevDir.mkdir( linkPath[i] ))
                {
                    if ( reporter )
                    {
                        QString detailedError( "SandboxInstallJob::createLink:- "
                                    "While making link %1, in dir %2 could not mkdir %3 - for %4");
                        detailedError = detailedError.arg( link ).arg( prevDir.path() ).arg( linkPath[i] ).arg( dirCheck.path());
                        reporter->reportError( tr("Unable to link to package"), detailedError );
                     
                    }
                    return false;
                }
                qLog(Package) << "Making dir" << linkPath[i] << "in" << prevDir.path();
            }
            prevDir = dirCheck;
        }
    }

    QString targetBuf;
    int upCount = linkPath.count();
    int fullCount = upCount;
    int maxDots = qMin( linkPath.count(), targetPath.count() - 1 );
    int pathDivergePtr = 0;
    for ( ; pathDivergePtr < maxDots; ++pathDivergePtr, --upCount )
        if ( linkPath[pathDivergePtr] != targetPath[pathDivergePtr] )
            break;
    if ( fullCount > upCount ) // target relative path with dots will save space
    {
        for ( int i = 0; i < upCount; ++i )
            targetBuf += "../";
        for ( int i = pathDivergePtr; i < targetPath.count(); ++i )
        {
            targetBuf += targetPath[i];
            if ( i < targetPath.count() - 1 )
                targetBuf += "/";
        }
    }
    else
    {
        targetBuf = target;
    }

    if ( !QFile::link( targetBuf, link ))
    {
        if ( reporter )
        {
            QString detailedError( "SandboxInstallJob::createLink:- "
                    "Could not create link %1 pointing to file %2" );
            detailedError = detailedError.arg( targetBuf ).arg( target );
            reporter->reportError( tr("Unable to link to package"), detailedError ); 
        }
        return false;
    }
    qLog(Package) << "Created link" << link << "pointing to" << targetBuf;
    return true;
}

/*!
  \internal
  Removes links related to  \c package from the Qtopia::packagePath's
  bin, pics and control directories 
*/
void SandboxInstallJob::clearMiscFiles() const
{
    QList<QDir> dirs;
    dirs << Qtopia::packagePath() + "bin";
    dirs << Qtopia::packagePath() + "pics";
    dirs << Qtopia::packagePath() + "controls";
    
    foreach( QDir dir, dirs )
    {
        if ( !dir.exists() )
        {
            qLog( Package ) << "SandboxInstallJob::clearMiscFiles:- dir does not exist: " 
                            << dir.absolutePath(); 
            return;
        }
        
        QFileInfoList fileList= dir.entryInfoList( QDir::System | QDir::NoDotAndDotDot | QDir::Files );
        QFile f;
        foreach( QFileInfo fi, fileList )
        {
            if ( fi.completeBaseName().startsWith( package->md5Sum ) )
            { 
                f.setFileName( fi.absoluteFilePath() );
                f.remove();
            }
        }
    }
}

/*
  \internal
  \class SandboxUninstallJob
  \brief Uninstalls a package

  The SandboxUninstallJob is responsible for the particulars in deleting
  a sandboxed application, these include:
  
  \list
  \o Unregistering binaries from the SXE system 
  \o Unregistering binaries from the Document system 
  \o Deleteing the package's control file in the controls directory
  \o Removing symlinks from the bin and pics directories 
     of Qtopia::packagePath
  \o Removing the package directory
  \o Rolling back the package's MAC sandbox rules
  \endlist

  The following calls should be used to uninstall a package:

  \code
  SandboxUninstallJob job( &pkg, m_installMedia, reporter );
  job.unregisterPackageFiles();
  job.dismantleSandbox();
  \endcode
*/ 

/*!
  \internal
  SandboxUninstallJob constructor
*/ 
SandboxUninstallJob::SandboxUninstallJob( const InstallControl::PackageInfo *pkg, const QString &media, ErrorReporter *reporter)
    : package( pkg )
{
    packagePath = media.isEmpty() ? Qtopia::packagePath() : media + "/" + PACKAGE_DIRECTORY + "/";
    packagePath += package->md5Sum;
    
    if ( !QDir( packagePath).exists() )
    {
            if( reporter )
            {
                reporter->reportError( QObject::tr( "Package does not exist" ) );    
            }
    }
}

/*!
  \internal
  Unregisters a package's binary files from the SXE and Document System.
*/  
void SandboxUninstallJob::unregisterPackageFiles() const
{
#ifndef QT_NO_SXE
    QPackageRegistry::getInstance()->unregisterPackageBinaries( packagePath );
#endif 

    QContentFilter appFilter = QContentFilter( QContentFilter::Role, "Application" ); 
    QContentSet set( appFilter);
    QContentList list = set.items();
    for ( int i = 0; i < list.count(); ++i )
    {
        if ( list.at( i ).executableName().startsWith( package->md5Sum ) )
            QContent::uninstall( list.at(i).id() ); 
    }
}

/*!
  \internal
  Deletes the sandboxed package and any files associated with it
  and rolls back any MAC kernel rules applied to the package's
  binaries    
*/
void SandboxUninstallJob::dismantleSandbox() const
{
#ifndef QT_NO_SXE    
    QStringList binaryPaths = getPackageBinaryPaths();
#endif

    //Deletes the package's control file in the controls directory.  
    QFile control( packagePath + "/../controls/" + package->md5Sum + ".control" );
    if ( !control.remove() )
        qWarning( "SandboxUninstallJob could not remove .control file from controls directory: %s",
                 qPrintable(control.fileName()) );
    

    //Deletes the packages symlinks in the bin and pics directory.
    QList<QDir> dirList;
    dirList << QDir ( Qtopia::packagePath() + QLatin1String( "bin" ) );
    dirList << QDir ( Qtopia::packagePath() + QLatin1String( "pics" ) );
    
    QDir dir;
    QFileInfoList fList;
    foreach ( dir, dirList )
    { 
        fList = dir.entryInfoList( QDir::AllEntries | QDir::System ); 
        for ( int i = 0; i < fList.count(); ++i )
        {
            if ( fList.at(i).fileName().startsWith( package->md5Sum ) )
            {
                if ( !QFile::remove( fList.at(i).absoluteFilePath() ) )
                    qWarning("SandboxInstallJob: cannot uninstall link: %s", 
                            qPrintable(fList.at(i).absoluteFilePath()) ); 
            }
        }
    }
    
    //Delete the package directory
    removePackage();

#ifndef QT_NO_SXE    
    //roll back sandbox rules for each binary in the package
    for ( int i = 0; i < binaryPaths.count(); ++i )
       rollBackSandboxRule( binaryPaths[i] ); 
#endif
}

/*!
  \internal
  Rolls back the MAC sandbox rules for a single binary
*/
void SandboxUninstallJob::rollBackSandboxRule( const QString &binPath ) const
{
    static QString unsandboxCmd;
    static bool commandUnavailable = false;

    if ( commandUnavailable )
        return;

    if ( unsandboxCmd.isEmpty() )
    {
        QStringList dirs = Qtopia::installPaths();
        for ( int d = 0; d < dirs.count(); ++d )
            if ( QFile::exists( dirs[d] + "/etc/qtopia_sxe/unsandbox" ))
                unsandboxCmd = dirs[d] + "/etc/qtopia_sxe/unsandbox %1";
        if ( unsandboxCmd.isEmpty() )
        {
            commandUnavailable = true;
            return;
        }
    }
    qLog(Package) << "Running command" << ( unsandboxCmd.arg( binPath ));
    QProcess::execute( unsandboxCmd.arg( binPath ));
}

/*!
  \internal
  Obtains the paths to all the binaries in a directory and it's sub-directories
*/
QStringList SandboxUninstallJob::getPackageBinaryPaths( const QString &path) const
{
    QDir directory( path.isEmpty() ? packagePath : path );
    QStringList ret;

    QFileInfoList fileList = directory.entryInfoList();
    for ( int i = 0; i < fileList.count(); ++i )
    {
        QFileInfo fileInfo = fileList.at(i);

        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        // recurse to other directories
        if (fileInfo.isDir()) 
            ret += getPackageBinaryPaths( fileInfo.filePath() );
        
        if ( fileInfo.isFile() && fileInfo.isExecutable() )
            ret += fileInfo.absoluteFilePath();
    }
    return ret;
}

/*!
  \internal
  Removes the package directory and everything below it
*/
void SandboxUninstallJob::removePackage() const
{
    QDir destDir( packagePath );
    if ( !destDir.exists() )
    {
        qWarning( "Request to remove non-existent directory %s", qPrintable( packagePath ));
        return;
    }
    QString cmd( "rm -Rf " );
    cmd += packagePath;
    Q_ASSERT( !packagePath.isEmpty() && packagePath != "/" );
    qLog(Package) << "Removing destination by executing:" << cmd;
    QProcess::execute( cmd );
}
