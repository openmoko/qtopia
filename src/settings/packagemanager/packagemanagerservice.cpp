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

#include "packagemanagerservice.h"
#include "packageinformationreader.h"
#include <qtopianamespace.h>
#include <QtopiaApplication>
#include "ui_packagedetails.h"
#include "packageview.h"
#include "packagemodel.h"
#include "packagecontroller.h" 
#include "domaininfo.h"
#include <QUrl>
#include <QDSData>
#include <QDir>
#include <QScrollArea>
#include "installedpackagescanner.h"

/*!
    \service PackageManagerService PackageManager
    \brief Provides the Package Manager service.

    The \i{Package Manager} service enables package installations to be triggered remotely.

    A package install can be started by invoking the service message \c{installPackageConfirm(QString)} with
    URL of the descriptor of the package to install.  Package manager will then download the descriptor and
    after gaining the users consent download the package and install it.

    \sa QtopiaAbstractService
*/
PackageManagerService::PackageManagerService( PackageView *parent )
    : QtopiaAbstractService( QLatin1String( "PackageManager" ), parent )
    , m_installer( 0 )
    , m_packageView( parent )
{
    publishAll();
}

/*!
    Download a package descriptor from the URL \a url and after obtaining the users consent download
    and install the package it refers to.

    This slot corresponds to the QCop message \c{PackageManager::installPackageConfirm(QString)}.
*/
void PackageManagerService::installPackageConfirm( const QString &url )
{
    if( !m_installer )
    {
        m_installer = new PackageServiceInstaller( m_packageView );

        connect( m_installer, SIGNAL(finished(int)), this, SLOT(installFinished()) );

        m_installer->installPackage( url );
    }
    else if( !m_installer->installActive() )
    {
        m_installer->installPackage( url );
    }
    else
        m_pendingUrls.append( url );
}

/*!
    Initiate the package installation process using the package descriptor embedded within
    \a request.

    This slot corresponds to the QCop message \c{PackageManager::installPackage(QDSActionRequest)}.
*/
void PackageManagerService::installPackage(const QDSActionRequest& request)
{
    QDSActionRequest requestCopy( request );

    if( !m_installer )
    {
        m_installer = new PackageServiceInstaller( m_packageView );

        connect( m_installer, SIGNAL(finished(int)), this, SLOT(installFinished()) );

        m_installer->installPackage( requestCopy.requestData().data() );
    }
    else if( !m_installer->installActive() )
    {
        m_installer->installPackage( requestCopy.requestData().data() );
    }
    else
        m_pendingDescriptors.append( requestCopy.requestData().data() );

    requestCopy.respond();
}

void PackageManagerService::installFinished()
{
    if( !m_pendingUrls.isEmpty() )
        m_installer->installPackage( m_pendingUrls.takeFirst() );
    else if( !m_pendingDescriptors.isEmpty() )
        m_installer->installPackage( m_pendingDescriptors.takeFirst() );
}

class ServicePackageDetails : public QDialog, public Ui::PackageDetails
{
public:
    ServicePackageDetails( QWidget *parent )
        : QDialog( parent )
    {
        setupUi(this);
        reenableButton->setVisible( false );
    }
};


PackageServiceInstaller::PackageServiceInstaller( PackageView *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , m_scanner( 0 )
    , m_installActive( false )
    , m_packageView( parent )
    , m_expectedPackageSize( 0 )
{
    QVBoxLayout *progressLayout = new QVBoxLayout( this );

    progressLayout->addWidget( m_progressLabel = new QLabel( this ) );
    progressLayout->addWidget( m_progressBar = new QProgressBar( this ) );

    m_progressLabel->setWordWrap( true );
    m_progressLabel->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

    m_packageDetails = new ServicePackageDetails( this );

    QtopiaApplication::setMenuLike( m_packageDetails, true );

    QtopiaApplication::setMenuLike( m_packageDetails, true );
}

void PackageServiceInstaller::doReportError( const QString &simpleError, const QString &detailedError )
{
    m_progressLabel->setText( simpleError );
    qWarning( qPrintable(detailedError) );
}

void PackageServiceInstaller::confirmInstall( const InstallControl::PackageInfo &package )
{
    if( !package.isComplete() || package.url.isEmpty() )
    {
        m_progressLabel->setText( tr( "Package header malformed." ) );

        return;
    }

    bool toIntOk = true;

    m_expectedPackageSize = package.size.toInt( &toIntOk );

    if( !toIntOk )
    {
        m_progressLabel->setText( tr( "Cannot determine package size." ) );

        return;
    }

    m_pendingPackage =  package;

    QString details;

#ifndef QT_NO_SXE
    if( m_packageView->model->hasSensitiveDomains( m_pendingPackage.domain ) )
    {
        details = tr( "The package <font color=\"#0000FF\">%1</font> <b>cannot be installed</b> as it utilizes protected resources" )
                .arg( m_pendingPackage.name );

        m_packageDetails->installButton->setVisible( false );
        m_packageDetails->cancelButton->setText( tr( "OK" ) );
    }
    else
#endif
    if ( packageInstalled( m_pendingPackage.md5Sum ) )
    {
        details = tr( "The package <font color=\"#0000FF\">%1</font> has <b>already been installed</b>" )
                    .arg( m_pendingPackage.name );
        m_packageDetails->installButton->setVisible( false );
        m_packageDetails->cancelButton->setText( tr( "OK" ) );
    }
    else
    { 
        details = tr( "<font color=\"#66CC00\"><b>Installing package</b></font> %1 <b>Go ahead?</b>" )
#ifndef QT_NO_SXE
            .arg( DomainInfo::explain( m_pendingPackage.domain, m_pendingPackage.name ) );
#else
            .arg( m_pendingPackage.name );
#endif
        m_packageDetails->installButton->setVisible( true );
        m_packageDetails->cancelButton->setText( tr( "Cancel" ) );
    }

    m_packageDetails->description->setHtml( details );

    if( QtopiaApplication::execDialog( m_packageDetails ) == QDialog::Accepted )
        installPendingPackage();
    else
        reject();
}

void PackageServiceInstaller::installPendingPackage()
{
    m_packageFile.setFileName( Qtopia::tempDir() + m_pendingPackage.packageFile );

    m_packageFile.unsetError();

    if( m_packageFile.open( QIODevice::WriteOnly ) )
    {
        connect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );
        connect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updatePackageProgress(int,int)) );

        QUrl url( m_pendingPackage.url );

        m_http.setHost( url.host(), url.port(80) );
        m_http.get( url.path(), &m_packageFile );

        m_progressLabel->setText( tr( "Downloading %1...", "%1 = package name" ).arg( m_pendingPackage.name ) );
    }
    else
    {
        m_progressLabel->setText(
                tr( "Package download failed due to file error: %1", "%1 = file error description" )
                .arg( m_packageFile.errorString() ) );

        QtopiaApplication::setMenuLike( this, false );
    }
}

void PackageServiceInstaller::installPackage( const QString &url )
{
    m_installActive = true;

    m_progressLabel->setText( tr( "Downloading package header..." ) );

    QtopiaApplication::setMenuLike( this, true );

    showMaximized();

    connect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );
    connect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updateHeaderProgress(int,int)) );

    m_headerBuffer.open( QIODevice::ReadWrite );

    QUrl u( url );

    m_http.setHost( u.host(), u.port(80) );
    m_http.get( u.path(), &m_headerBuffer );
}

void PackageServiceInstaller::installPackage( const QByteArray &descriptor )
{
    QTextStream stream( descriptor );

    PackageInformationReader reader( stream );

    showMaximized();

    confirmInstall( reader.package() );
}

void PackageServiceInstaller::headerDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );
    disconnect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updateHeaderProgress(int,int)) );

    if( error )
    {
        m_headerBuffer.close();
        m_headerBuffer.setData( QByteArray() );

        if( m_http.error() != QHttp::Aborted )
        {
            m_progressLabel->setText( tr( "Header download failed with error: %1" ).arg( m_http.errorString() ) );
        }

        QtopiaApplication::setMenuLike( this, false );

        return;
    }

    m_progressBar->setValue( m_progressBar->maximum() );

    m_headerBuffer.seek( 0 );

    QTextStream stream( &m_headerBuffer );

    PackageInformationReader reader( stream );

    m_headerBuffer.buffer().clear();
    confirmInstall( reader.package() );
}

void PackageServiceInstaller::packageDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );
    disconnect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updatePackageProgress(int,int)) );

    m_packageFile.close();

    m_progressBar->setValue( m_progressBar->maximum() );

    if( error )
    {
        if( m_http.error() != QHttp::Aborted )
        {
            // Warn of error condition.
            m_progressLabel->setText( tr( "Package download failed with error: %1" ).arg( m_http.errorString() ) );
        }
    }
    else if( m_installer.installPackage( m_pendingPackage, m_packageFile.md5Sum(), this ) )
    {
        m_progressLabel->setText( tr( "%1 installed", "%1 = package name" ).arg( m_pendingPackage.name ) );
        qobject_cast<InstalledPackageController *>(m_packageView->model->installed )
                ->reloadInstalledLocations( QStringList( Qtopia::packagePath() + "controls/" ) );
    }

    m_packageFile.remove();

    QtopiaApplication::setMenuLike( this, false );
}

void PackageServiceInstaller::updateHeaderProgress( int done, int total )
{
    if( m_progressBar->maximum() != total )
        m_progressBar->setMaximum( total );

    m_progressBar->setValue( done );

    if( total > 4096 )
    {
        m_progressLabel->setText(
                tr( "Package header exceeds maximum size of %1 bytes. Download cancelled" ).arg( 4096 ) );

        m_http.abort();
    }
}

void PackageServiceInstaller::updatePackageProgress( int done, int total )
{
    if( m_progressBar->maximum() != total )
        m_progressBar->setMaximum( total );

    m_progressBar->setValue( done );

    if( total > m_expectedPackageSize )
    {
        m_progressLabel->setText(
                tr( "Package size exceeds the expected size of %1 kB. Download cancelled" ).arg( m_expectedPackageSize / 1024 ) );

        m_http.abort();
    }
    else if( m_packageFile.error() != QFile::NoError )
    {
        m_progressLabel->setText(
                tr( "Package downloaded due to file error: %1", "%1 = file error description" ).arg( m_packageFile.errorString() ) );

        m_http.abort();
    }
}

void PackageServiceInstaller::accept()
{
    m_installActive = false;

    QDialog::accept();
}

void PackageServiceInstaller::reject()
{
    m_installActive = false;

    m_http.abort();

    QDialog::reject();
}

bool PackageServiceInstaller::packageInstalled( const QString &md5Sum )
{
    if ( !m_scanner )
        m_scanner = new InstalledPackageScanner( this );

    return m_scanner->isPackageInstalled( md5Sum );
}
