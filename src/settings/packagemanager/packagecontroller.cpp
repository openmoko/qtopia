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

#include "packagecontroller.h"
#include "httpfetcher.h"
#include "packageinformationreader.h"
#include "packagemodel.h"
#include "installedpackagescanner.h"
#include "targz.h"

#include <QIcon>
#include <QDir>
#include <QDebug>
#include <QList>
#include <QTimer>
#include <QtopiaApplication>
#include <QDesktopWidget>

#include <qtopianamespace.h>
#include <qtopialog.h>

#ifndef Q_WS_QWS
#include <QProgressDialog>
#else

#include <QProgressBar>
#include <QTextBrowser>
#include <QBoxLayout>
#include <QPushButton>

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
#ifdef QTOPIA_PHONE
    //XXX Wrong.  Should showMaximized() if that's what we want.
    return QApplication::desktop()->availableGeometry().size();
#endif
    return QSize();
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

#endif
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
    installControl->installPackage( pkgList[pkgId] );
    QFile::remove(lnkFile);
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
}

NetworkPackageController::~NetworkPackageController()
{
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
    connect( progressDisplay, SIGNAL(canceled()),
            hf, SLOT(cancel()) );
    connect( hf, SIGNAL(progressValue(int)), progressDisplay, SLOT(setValue(int)));
    connect( hf, SIGNAL(finished()), this, SLOT(listFetchComplete()));
    QtopiaApplication::showDialog( progressDisplay );
    hf->start();
}

void NetworkPackageController::listFetchComplete()
{
    // qDebug() << "package list fetch complete";
    if ( hf )
    {
        if ( hf->httpRequestWasAborted() )
            emit packageMessage( tr( "Fetch from %1 failed: %2", "%1=server name, %2=error string" )
                    .arg( currentNetworkServer )
                    .arg( hf->getError() ));
        else
            emit packageMessage( tr( "Fetched details on<br>%1 packages", "%1 = # of packages" )
                    .arg( pkgList.count() ));
        delete hf;
        hf = 0;
    }
    else
    {
        qWarning( "HttpFetcher was deleted early!" );
    }
    // qDebug() << "fetched" << pkgList.count() << "items of package info";
    emit updated();
    progressDisplay->reset();
}

void NetworkPackageController::install( int packageI )
{
    if ( hf == NULL )
        hf = new HttpFetcher( currentNetworkServer, this );
    qLog(Package) << "installing network package" << packageI;
    qLog(Package) << "\t:" << pkgList[packageI].packageFile;
    hf->setFile( pkgList[packageI].packageFile );
    if ( progressDisplay )
    {
        progressDisplay->setLabelText( tr( "Getting package %1 from %2", "%1 package name, %2 server name" )
                .arg( pkgList[packageI].name ).arg( currentNetworkServer ));
        progressDisplay->show();
        connect( hf, SIGNAL(progressValue(int)), progressDisplay, SLOT(setValue(int)));
    }
    connect( hf, SIGNAL(finished()), this, SLOT(packageFetchComplete()));
    hf->start();
}

void NetworkPackageController::packageFetchComplete()
{
    InstallControl::PackageInfo pi;
    QString installFile;
    installFile = hf->getFile();
    // qDebug() << "Package fetch complete";
    int i;
    for ( i = 0; i < pkgList.count(); ++i )
        if ( installFile.endsWith( pkgList[i].packageFile ))
            break;
    installControl->installPackage( pkgList[i] );
    emit updated();
    emit packageInstalled( pkgList[i] );
    progressDisplay->reset();
    delete hf;
    hf = 0;
}

////////////////////////////////////////////////////////////////////////
/////
///// InstalledPackageController implementation
/////

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
    reloadInstalledLocations( QStringList( Qtopia::packagePath() ));
}

void InstalledPackageController::install(int packageI)
{
}

void InstalledPackageController::reloadInstalledLocations( const QStringList &locs )
{
    installedScanner = new InstalledPackageScanner( this );
    installedScanner->setLocations( locs );
    qLog(Package) << "reloadInstalledLocations" << locs;
    installedScanner->start( QThread::LowestPriority );
    emit updated();
}
