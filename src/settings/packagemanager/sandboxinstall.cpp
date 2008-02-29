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

#include "sandboxinstall.h"
#include "packageview.h"

#include <qtopianamespace.h>
#include <qcontent.h>

#ifdef QT_NO_SXE
#define PACKAGE_DIRECTORY "packages"
#else
#include <qpackageregistry.h>
#define PACKAGE_DIRECTORY QPackageRegistry::packageDirectory
#endif

#include <errno.h>
#include <qdebug.h>
#include <qtopialog.h>

#include <QDir>
#include <QProcess>

/*!
  Construct a new sandbox install job object, ensuring the sandbox
  root exists, and creating the destination directory for the
  package to be expanded into.

  The directory \a sys is the path to the Qtopia system root under
  which the sandbox lives (or is created if it doesnt exist) and must
  be a member of Qtopia::installPaths() and be writable.

  The \a pkg points to the package which will be installed.

  If \a m is non-empty it specifies the file-system path mount-point
  of a media card (or other writeable file-system) on which the package
  will be located.

  In this case the destination directory will be a symlink from the
  system root to the location on the media card.
  */
SandboxInstallJob::SandboxInstallJob( const InstallControl::PackageInfo *pkg, const QString &m, ErrorReporter *reporter )
    : package( pkg )
    , media( m )
    , abort( false )
{
    QDir sandboxRoot( Qtopia::packagePath() );
    destination = sandboxRoot.path() + "/" + pkg->md5Sum;
    Q_ASSERT( sandboxRoot.exists() );
    if ( !pkg->isComplete() )   // must have the md5Sum value, plus content!
    {
        QString message = QObject::tr( "Package %1 <font color=\"#CC0000\">is incomplete</font> - install cancelled." )
                .arg( pkg->name );

        if( reporter )
            reporter->reportError( message );
        else
            PackageView::displayMessage( message );

        abort = true;
        return;
    }
    if ( sandboxRoot.exists( pkg->md5Sum ))
    {
        QString message = QObject::tr( "<font color=\"#CC0000\">Package Error</font> "
                "Package %1 already installed" ).arg( pkg->name );

        if( reporter )
            reporter->reportError( message );
        else
            PackageView::displayMessage( message );

        abort = true;
        return;
    }
    if ( media.isEmpty() )
    {
        if ( !sandboxRoot.mkdir( pkg->md5Sum ))
        {
            qWarning( "mkdir failed : %s", strerror( errno ));
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
            qWarning( "### %s (the media path) doesnt exist! ###", qPrintable( media ));
            abort = true;
            return;
        }
        qLog(Package) << "Creating directory" << mediaSandbox.path() << "in path" << media;
        if ( !mediaDir.mkdir( PACKAGE_DIRECTORY ))
        {
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
}

/*!
  Register all the files in the directory tree \a directory with the Qtopia
  system, including SXE and Document systems.  The \a pkgName and \a profile
  are applied to those program binaries discovered.
  */
void SandboxInstallJob::registerPackageFiles( const QString &path )
{
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
                qDebug() << "Adding to dot.desktop files" << fileInfo.path();
                desktopPaths << fileInfo.filePath();
            }
            else if ( fileInfo.isExecutable() )
            {
                if ( !fileInfo.isWritable() )
                {
                    QFile::Permissions perms = QFile::permissions( fileInfo.filePath() );
                    if ( !QFile::setPermissions( fileInfo.filePath(), perms | QFile::WriteOwner ))
                    {
                        qWarning( "%s is not writable and could not change permissions", qPrintable(fileInfo.filePath()) );
                        continue;
                    }
                }
                QString registerPath = directory.path();
                registerPath.remove( Qtopia::qtopiaDir() );
#ifndef QT_NO_SXE
                QPackageRegistry::getInstance()->registerBinary(
                        registerPath, fileInfo.fileName(), package->domain );
                runSandbox( directory.path() + "/" + fileInfo.fileName() );
#endif
            }
        }
    }
}

void SandboxInstallJob::installContent()
{
    if ( desktopPaths.count() == 0 )
    {
        qLog(Package) << "SandboxInstallJob::installContent - No desktop files!";
        return;
    }
    foreach ( QString d, desktopPaths )
    {
        QContent::install( d );
    }
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
void SandboxInstallJob::setupSandbox()
{
    // create links for the icon, binary
    QHash<QString,QSettings*> desktopConf;
    qDebug() << "Desktop files are" << desktopPaths;
    for ( int x = 0; x < desktopPaths.count(); ++x )
    {
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
            desktopConf[exec] = dc;
        }
    }
    QString installSystemBinPath = Qtopia::packagePath() + QLatin1String( "/bin" );
    QDir packageBin( destination + "/" + QLatin1String( "bin" ));
    QFileInfoList bins = packageBin.entryInfoList();
    for ( int b = 0; b < bins.count(); ++b )
    {
        if ( !bins[b].isExecutable() )
            continue;
        if ( bins[b].fileName().startsWith(".") ) // dont do invisible files or dirs
            continue;
        QString origBin = bins[b].fileName();
        QString newBin = package->md5Sum + "_" + origBin;
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
                    QString newIcon = icon;
                    newIcon.replace( origBin, newBin );
                    if ( !icon.isEmpty() )
                    {
                        desktopConf[origBin]->setValue( "Icon", newIcon );
                        qLog(Package) << "Re-writing desktop file Icon from" << origBin << "to" << newBin;
                    }
                }
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
        dc->beginGroup( QLatin1String( "Package Data" ));
        dc->setValue( QLatin1String( "Path" ), destination );
        qLog(Package) << "deleting desktop settings object for" << it.key();
        // delete QSettings object, causes .desktop file to be persisted
        delete it.value();
        ++it;
    }
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
                    qWarning( "While making link %s, in dir %s could not mkdir %s - for %s",
                            qPrintable( link ), qPrintable( prevDir.path() ),
                            qPrintable( linkPath[i] ), qPrintable( dirCheck.path() ));
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
        qWarning( "Could not create link %s pointing to file %s",
                qPrintable( targetBuf ), qPrintable( target ));
        return false;
    }
    qLog(Package) << "Created link" << link << "pointing to" << targetBuf;
    return true;
}
