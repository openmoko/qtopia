/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "packagecontroller.h"
#include "httpfetcher.h"
#include "packageinformationreader.h"
#include "packagemodel.h"
#include "installedpackagescanner.h"
#include "targz.h"
#include "utils.h"

#include <QIcon>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QTimer>
#include <QtopiaApplication>
#include <QDesktopWidget>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <QProgressBar>
#include <QTextBrowser>
#include <QBoxLayout>
#include <QPushButton>
#include <QSignalMapper>
#include "packageview.h"

QProgressDialog::QProgressDialog( QWidget *p, Qt::WFlags f )
    : QDialog( p, f )
    , bar( 0 )
    , label( 0 )
    , vb( 0 )
{
    vb = new QVBoxLayout( this );
    vb->setMargin( 6 );
    vb->setSpacing( 6 );
    bar = new QProgressBar( this );
    label = new QTextBrowser( this );
    label->setFocusPolicy( Qt::NoFocus );
    cancelButton = new QPushButton( tr( "Cancel" ), this );
    vb->addWidget( label );
    vb->addWidget( bar );
    vb->addWidget( cancelButton );
    connect( cancelButton, SIGNAL(clicked()),
            this, SIGNAL(canceled()) );
    connect( cancelButton, SIGNAL(clicked()),
            this, SLOT(reject()) );

    QTimer::singleShot( 0, this, SLOT(init()) );
}

void QProgressDialog::init()
{
    //XXX Suspect
    QDesktopWidget *desktop = QApplication::desktop();
    int y = 50;
    int x = 0;
    setGeometry( x, y, desktop->width(), desktop->height() - y );
    setFixedSize( desktop->width(), desktop->height() - y );
}

QProgressDialog::~QProgressDialog()
{
}

QSize QProgressDialog::sizeHint() const
{
    //XXX Wrong.  Should showMaximized() if that's what we want.
    return QApplication::desktop()->availableGeometry().size();
}

void QProgressDialog::setLabelText( const QString &t )
{
    label->setPlainText( t );
}

void QProgressDialog::setMaximum( int m )
{
    bar->setMaximum( m );
}

void QProgressDialog::setValue( int v )
{
    bar->setValue( v );
}

void QProgressDialog::reset()
{
    bar->setValue( 0 );
    hide();
}

////////////////////////////////////////////////////////////////////////
/////
///// AbstractPackageController implementation
/////

const QString AbstractPackageController::INFORMATION_FILE = "control";
const QString AbstractPackageController::PACKAGE_SUMMARY_FILE = "packages.list";

/**
  \internal
  \enum PCType
      local,
      network,
      installed
*/

AbstractPackageController *AbstractPackageController::factory( PCType t, PackageModel *parent )
{
    AbstractPackageController *apc = 0;
    switch ( t )
    {
        case local:
            apc = new LocalPackageController( parent );
            break;
        case network:
            apc = new NetworkPackageController( parent );
            break;
        case installed:
            apc = new InstalledPackageController( parent );
            break;
        default:
            qWarning( "Bad package controller type" );
    }
    apc->installControl = parent->installControl();
    return apc;
}

AbstractPackageController::AbstractPackageController( QObject *parent )
    : QObject( parent )
    , progressDisplay( 0 )
    , installControl( 0 )
{
}

AbstractPackageController::~AbstractPackageController()
{
}

void AbstractPackageController::setPackageFilter( const QList<InstallControl::PackageInfo> &filter )
{
    foreach ( InstallControl::PackageInfo pkg, filteredOutPkgList )
    {
        if ( !filter.contains( pkg ))
        {
            filteredOutPkgList.removeAll( pkg );
            pkgList.append( pkg );
            qSort( pkgList );
            qLog(Package) << pkg.name << "is no longer filtered out";
        }
    }
    foreach ( InstallControl::PackageInfo pkg, pkgList )
    {
        if ( filter.contains( pkg ))
        {
            filteredOutPkgList.append( pkg );
            pkgList.removeAll( pkg );
            qLog(Package) << pkg.name << "is now filtered out";
        }
    }
}

QString AbstractPackageController::packageDetails( int pkgId ) const
{
    InstallControl::PackageInfo pkg = pkgList[pkgId];

    const char * dummyStr[] ={ QT_TRANSLATE_NOOP("PackageView", "Download Size:"),
                               QT_TRANSLATE_NOOP("PackageView", "Size:" ),
                               QT_TRANSLATE_NOOP("PackageView", "Cost:" ),
                               QT_TRANSLATE_NOOP("PackageView", "Security:" ),
                               QT_TRANSLATE_NOOP("PackageView", "Signed:" ),
                               QT_TRANSLATE_NOOP("PackageView", "Trust:" ),
                               QT_TRANSLATE_NOOP("PackageView", "Trusted:" )
                             };
    Q_UNUSED( dummyStr );

    return  tr( "Name:" ) + QLatin1String(" ") + pkg.name + "<br>" +
            tr( "Description:" ) + QLatin1String(" ")+ pkg.description + "<br>" +
            tr( "Installation Size:" )  + QLatin1String(" ")+ pkg.installedSize + "<br>" +
            tr( "MD5Sum:" ) + QLatin1String(" ")+ pkg.md5Sum + "<br>"
#ifndef QT_NO_SXE
            + tr( "Capabilities:" ) + QLatin1String(" ") + DomainInfo::explain( pkg.domain, pkg.name );
#else
            ;
#endif

}

////////////////////////////////////////////////////////////////////////
/////
///// LocalPackageController implementation
/////

const QString LocalPackageController::LOCAL_PACKAGE_DIRECTORY = "local";

LocalPackageController::LocalPackageController( QObject *parent )
    : AbstractPackageController( parent )
    , reader( 0 )
{
    reader = new PackageInformationReader;
    initialiseLocalPackageInfo();
}

LocalPackageController::~LocalPackageController()
{
    if ( reader )
        delete reader;
    reader = 0;
}

void LocalPackageController::initialiseLocalPackageInfo()
{
    pkgList.clear();
    pkgLoc.clear();
    QStringList paths = Qtopia::installPaths();
    QString tempPackageInfoPath = Qtopia::tempDir() + "packages";
    connect( reader, SIGNAL(packageComplete()),
            this, SLOT(packageComplete()));
    for ( int i = 0; i < paths.count(); ++i )
    {
        currentPackageDirectory = paths[i] + LOCAL_PACKAGE_DIRECTORY;
        QDir packageDirectory( currentPackageDirectory );

        if ( !packageDirectory.exists() )
            continue;
        if ( !packageDirectory.exists( PACKAGE_SUMMARY_FILE ))
        {
            qWarning( "Local package directory %s did not have a package"
                    "summary file %s - no packages here will be processed",
                    qPrintable( paths[i] ), qPrintable( PACKAGE_SUMMARY_FILE ));
            continue;
        }
        QFile summaryFile( packageDirectory.filePath( PACKAGE_SUMMARY_FILE ));
        if ( !summaryFile.open( QIODevice::ReadOnly ))
        {
            qWarning( "Could not open for reading %s",
                    qPrintable( summaryFile.fileName() ));
            continue;
        }
        while ( !summaryFile.atEnd() )
        {
            QString line = summaryFile.readLine();
            reader->readLine( line );
        }
    }
}

void LocalPackageController::packageComplete()
{
    pkgList.append( reader->package() );
    pkgLoc.insert( reader->package(), currentPackageDirectory );
    reader->reset();
}

void LocalPackageController::install( int pkgId )
{
    QString pkgFile = pkgLoc[pkgList[pkgId]] + "/" + pkgList[pkgId].packageFile;
    QString lnkFile = Qtopia::tempDir() + pkgList[pkgId].packageFile;

    QFile::link(pkgFile, lnkFile);
    SimpleErrorReporter errorReporter( SimpleErrorReporter::Install, pkgList[pkgId].name );
    installControl->installPackage( pkgList[pkgId], pkgList[pkgId].md5Sum, &errorReporter );
    QFile::remove(lnkFile);

    //TODO: Error handling
    emit updated();
    emit packageInstalled( pkgList[pkgId] ); 
}

////////////////////////////////////////////////////////////////////////
/////
///// NetworkPackageController implementation
/////
NetworkPackageController::NetworkPackageController( QObject *parent )
    : AbstractPackageController( parent )
    , hf( 0 )
{
    QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
    progressDisplay = new QProgressDialog( topLevelWidgets[0] );
    progressDisplay->setMaximum( HttpFetcher::maxProgress );
    progressDisplay->setWindowTitle( "Downloading" );

    //The signal mapper is used to provide an empty error parameter
    //from the progressDiplay dialog's cancelled signal to the
    //cancel slot of a httpfetcher which is connected later. 
    signalMapper = new QSignalMapper(this);
    connect( progressDisplay, SIGNAL(canceled()),
                signalMapper, SLOT(map()));
    signalMapper->setMapping( progressDisplay, ""); //empty error parameter implies user aborted 
    
    connect( progressDisplay, SIGNAL(rejected()),
             progressDisplay, SIGNAL(canceled()) );
}

NetworkPackageController::~NetworkPackageController()
{
    delete signalMapper;
    signalMapper = 0;
}

void NetworkPackageController::setNetworkServer( const QString &s )
{
    if ( currentNetworkServer == s ) return;
    currentNetworkServer = s;
    int rows = pkgList.count();
    PackageModel *pm = qobject_cast<PackageModel *>( parent() );
    emit rowsAboutToBeRemoved( pm->index( 1, 0, QModelIndex() ), 0, rows - 1 );
    pkgList.clear();
    Q_CHECK_PTR( pm );
    // row 1 is the network controller
    emit rowsRemoved( pm->index( 1, 0, QModelIndex() ), 0, rows - 1 );
    insertNetworkPackageItems();
}

void NetworkPackageController::insertNetworkPackageItems()
{
    hf = new HttpFetcher( currentNetworkServer, this );
    progressDisplay->setLabelText(
            tr( "Getting package list from %1" ).arg( currentNetworkServer ));

    connect( signalMapper, SIGNAL(mapped(QString)), hf, SLOT(cancel(QString)) );

    connect( hf, SIGNAL(progressValue(int)), progressDisplay, SLOT(setValue(int)));
    connect( hf, SIGNAL(finished()), this, SLOT(listFetchComplete()));
    connect( hf, SIGNAL(terminated()), this, SLOT(listFetchComplete()));
   
    QtopiaApplication::setMenuLike( progressDisplay, true );
    QtopiaApplication::showDialog( progressDisplay );
    hf->start();
}

void NetworkPackageController::listFetchComplete()
{
    if ( hf )
    {
        if ( hf->httpRequestWasAborted() )
        {
            if ( !hf->getError().isEmpty() )
            {
                SimpleErrorReporter errorReporter( SimpleErrorReporter::Other );
                QString detailedMessage( "NetworkPackageController::listFetchComplete:- Fetch from %1 failed: %2" );
                detailedMessage = detailedMessage.arg( currentNetworkServer ).arg( hf->getError() );

                errorReporter.reportError( tr( "Error connecting to server, check server URL is correct and/or "
                                         "contact server administrator" ), detailedMessage );
            } else //Note: if getError does return empty, then the user cancelled the package fetch.
            {
               clearPackages();
            }
            emit serverStatus( tr( "Not connected" ) );
        }
        else
        {
            emit updated();
            emit serverStatus( tr( "Connected") + "<br>" + tr("%n program(s) found","%n =# of packages", pkgList.count()));
        }
        delete hf;
        hf = 0;
    }
    else
    {
        qWarning( "HttpFetcher was deleted early!" );
    }
    emit updated();
    progressDisplay->reset();
}

void NetworkPackageController::install( int packageI )
{
    SimpleErrorReporter reporter( SimpleErrorReporter::Install, pkgList[packageI].name);

    bool ok;
    qlonglong fileSize = pkgList[packageI].size.toLongLong( &ok );
    if ( !ok )
    {
        QString simpleError =  PackageView::tr( "Invalid package download size supplied, contact package supplier" );
        QString detailedError( "NetworkPackageController::install:- Invalid package size supplied: %1" );
        detailedError = detailedError.arg( pkgList[packageI].size );
        reporter.reportError( simpleError, detailedError );
        return;
    }

    QString installedSizeStr = pkgList[packageI].installedSize;
    qlonglong installedSize = SizeUtils::parseInstalledSize(installedSizeStr);

    if ( installedSize  < 1 )
    {
        QString simpleError =  QObject::tr( "Package did not supply valid installed size, contact package supplier" );
        QString detailedError( "NetworkPackageController::install:- Package supplied invalid size %1" );
        detailedError = detailedError.arg( installedSizeStr );
        reporter.reportError( simpleError, detailedError );
        return;
    }

    if ( fileSize >= installedSize )
    {
        QString simpleError =  QObject::tr( "Invalid package, contact package supplier" );
        QString detailedError( "NetworkPackageController::install:- download file size >= installed size, "
                               "download size = %1, installed size = %2 ");
        detailedError = detailedError.arg( fileSize ).arg( installedSize );
        reporter.reportError( simpleError, detailedError );
        return;
    }

    QString neededSpace;
    if ( !SizeUtils::isSufficientSpace( installedSize, neededSpace) )
    {
        QString simpleError =  QObject::tr( "Not enough space for package, free up %1", "%1= 1MB" );
        simpleError = simpleError.arg( neededSpace );
        QString detailedError( "NetworkPackageController::install:- Insufficient space, need %1" );
        detailedError = detailedError.arg( neededSpace );
        reporter.reportError( simpleError, detailedError );
        return;
    }

    qLog(Package) << "installing network package" << packageI;
    qLog(Package) << "\t:" << pkgList[packageI].packageFile << "bytes";

    if ( hf == NULL )
        hf = new HttpFetcher( currentNetworkServer, this );

    hf->setFile( pkgList[packageI].packageFile, fileSize );
    currentPackageName = pkgList[packageI].name;
    if ( progressDisplay )
    {
        progressDisplay->setLabelText( tr( "Getting package %1 from %2", "%1 package name, %2 server name" )
                .arg( pkgList[packageI].name ).arg( currentNetworkServer ));
        connect( hf, SIGNAL(progressValue(int)), progressDisplay, SLOT(setValue(int)));

        connect( signalMapper, SIGNAL(mapped(QString)), hf, SLOT(cancel(QString)) );
        progressDisplay->show();
    }
    connect( hf, SIGNAL(finished()), this, SLOT(packageFetchComplete()));
    connect( hf, SIGNAL(terminated()), this, SLOT(packageFetchComplete()));
    hf->start();
}

void NetworkPackageController::packageFetchComplete()
{
    if ( hf )
    {
        if ( hf->httpRequestWasAborted() )
        {
            if ( !hf->getError().isEmpty() )  
            {
                SimpleErrorReporter errorReporter( SimpleErrorReporter::Install, currentPackageName );
                QString detailedMessage( "NetworkPackageController::packageFetchComplete:- Fetch from %1 failed: %2" );
                detailedMessage = detailedMessage.arg( currentNetworkServer ).arg( hf->getError() );

                errorReporter.reportError( tr( "Error occurred while downloading package"), detailedMessage);
            } //note: if getError does return empty, then the user has cancelled package download
        }
        else
        {
            InstallControl::PackageInfo pi;
            QString installFile;
            installFile = hf->getFile();
            int i;
            for ( i = 0; i < pkgList.count(); ++i )
                if ( installFile.endsWith( pkgList[i].packageFile ))
                    break;

            SimpleErrorReporter errorReporter( SimpleErrorReporter::Install, pkgList[i].name);
            if( installControl->installPackage( pkgList[i], hf->getMd5Sum(), &errorReporter ) )
            {
                pi = pkgList[i];
                emit updated();
                emit packageInstalled( pi );
            }

        }
        delete hf;
        hf = 0;
    }
    else
    {
        qWarning( "HttpFetcher was deleted early!" );

    }
    progressDisplay->reset();
}

////////////////////////////////////////////////////////////////////////
/////
///// InstalledPackageController implementation
/////


const QString InstalledPackageController::DISABLED_TAG = "__DISABLED";

InstalledPackageController::InstalledPackageController( QObject *parent )
    : AbstractPackageController( parent )
{
    QTimer::singleShot( 0, this, SLOT(initialize()) );
}

InstalledPackageController::~InstalledPackageController()
{
}

void InstalledPackageController::initialize()
{
    reloadInstalledLocations( QStringList( Qtopia::packagePath() + "controls/" ));
}

/*!
  \internal
  Uninstalls the package, (despite the method name being install)
*/
void InstalledPackageController::install(int pkgId)
{
    SimpleErrorReporter errorReporter( SimpleErrorReporter::Uninstall, pkgList[pkgId].name );
    //TODO: set the media parameter appropriately
    //TODO: setup error reporting for uninstall
    installControl->uninstallPackage( pkgList[pkgId], &errorReporter);
    InstallControl::PackageInfo pi;
    pi = pkgList[pkgId];
    pkgList.removeAt( pkgId );
    emit updated();
    emit packageInstalled( pi );
}

void InstalledPackageController::reloadInstalledLocations( const QStringList &locs )
{
    pkgList.clear();
    installedScanner = new InstalledPackageScanner( this );
    installedScanner->setLocations( locs );
    qLog(Package) << "reloadInstalledLocations" << locs;
    installedScanner->start( QThread::LowestPriority );
    emit updated();
}

QIcon InstalledPackageController::getDataIcon( int pkgId ) const
{
    InstallControl::PackageInfo pi = packageInfo( pkgId );

    if ( pi.isEnabled )
         return QIcon( ":icon/accessories" );
    else
        return QIcon( ":icon/reset" );
}

bool InstalledPackageController::reenable( int pkgId ) 
{
    InstallControl::PackageInfo pi = packageInfo( pkgId );
    QDir installSystemBinPath( Qtopia::packagePath() + "/bin" );

    QFileInfoList links = installSystemBinPath.entryInfoList( QStringList( pi.md5Sum + "*" ), QDir::System );        

    if ( links.count() == 0 )
    {
        qWarning( "InstalledPackageController::reenable: Could not find links to reenable " );
        return false;
    }

    QFile linkFile;
    QString oldTarget;
    QString newTarget;
    for ( int i = 0; i < links.count(); ++i )
    {
        linkFile.setFileName( links[i].absoluteFilePath() );
        oldTarget = linkFile.symLinkTarget();

        if ( oldTarget.endsWith(DISABLED_TAG) )
        {
            if ( !linkFile.remove() )
            {
                qLog(Package) << "InstalledPackageController::reenable:- could not remove symlink during reenable:"
                                << linkFile.fileName() << "  target: "<< oldTarget;
                return false;
            }

            newTarget =  oldTarget.replace(DISABLED_TAG, "" );
            if ( !QFile::link(newTarget, linkFile.fileName() ) )
            {
                qLog(Package) << "InstalledPackageController::reenable:- could not create symlink during reenable: "
                                << linkFile.fileName() << " target:" << newTarget;
                return false;
            }
        }
    }
    pkgList[ pkgId ].isEnabled = true;
    return true;
}
