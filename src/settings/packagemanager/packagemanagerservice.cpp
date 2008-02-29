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
#include <QUrl>
#include <QDSData>

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
    }
};


PackageServiceInstaller::PackageServiceInstaller( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
    , m_installActive( false )
{
    QVBoxLayout *progressLayout = new QVBoxLayout( this );

    progressLayout->addWidget( m_progressLabel = new QLabel( this ) );
    progressLayout->addWidget( m_progressBar = new QProgressBar( this ) );

    m_progressLabel->setWordWrap( true );

    m_packageDetails = new ServicePackageDetails( this );

    QtopiaApplication::setMenuLike( m_packageDetails, true );

    connect( &m_http, SIGNAL(dataReadProgress(int,int)), this, SLOT(updateProgress(int,int)) );
}

void PackageServiceInstaller::reportError( const QString &error )
{
    m_progressLabel->setText( error );
}

void PackageServiceInstaller::confirmInstall( InstallControl::PackageInfo package )
{
    if( !package.isComplete() || package.url.isEmpty() )
    {
        m_progressLabel->setText( tr( "Package header malformed" ) );

        return;
    }

    m_pendingPackage =  package;

    QString details = tr( "<font color=\"#66CC00\"><b>Installing package</b></font> %1 <b>Go ahead?</b>" )
#ifndef QT_NO_SXE
            .arg( DomainInfo::explain( m_pendingPackage.domain(), m_pendingPackage.name ) );
#else
            .arg( m_pendingPackage.name );
#endif

    m_packageDetails->description->setHtml( details );

    if( QtopiaApplication::execDialog( m_packageDetails ) == QDialog::Accepted )
    {
        installPendingPackage();
    }
    else
    {
        reject();
    }
}

void PackageServiceInstaller::installPendingPackage()
{
    m_packageFile.setFileName( Qtopia::tempDir() + m_pendingPackage.packageFile );

    m_packageFile.open( QIODevice::WriteOnly );

    connect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );

    QUrl url( m_pendingPackage.url );

    m_http.setHost( url.host() );
    m_http.get( url.path(), &m_packageFile );

    m_progressLabel->setText( tr( "Downloading %1...", "%1 = package name" ) );
}

void PackageServiceInstaller::installPackage( const QString &url )
{
    m_installActive = true;

    m_progressLabel->setText( tr( "Downloading package header..." ) );

    QtopiaApplication::setMenuLike( this, true );

    QtopiaApplication::showDialog( this );

    connect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );

    m_headerBuffer.open( QIODevice::ReadWrite );

    QUrl u( url );

    m_http.setHost( u.host() );
    m_http.get( u.path(), &m_headerBuffer );
}

void PackageServiceInstaller::installPackage( const QByteArray &descriptor )
{
    QTextStream stream( descriptor );

    PackageInformationReader reader( stream );

    QtopiaApplication::showDialog( this );

    confirmInstall( reader.package() );
}

void PackageServiceInstaller::headerDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(headerDownloadDone(bool)) );

    if( error )
    {
        m_headerBuffer.close();
        m_headerBuffer.setData( QByteArray() );

        m_progressLabel->setText( tr( "Header download failed with error: %1" ).arg( m_http.errorString() ) );

        QtopiaApplication::setMenuLike( this, false );

        return;
    }

    m_progressBar->setValue( m_progressBar->maximum() );

    m_headerBuffer.seek( 0 );

    QTextStream stream( &m_headerBuffer );

    PackageInformationReader reader( stream );

    confirmInstall( reader.package() );
}

void PackageServiceInstaller::packageDownloadDone( bool error )
{
    disconnect( &m_http, SIGNAL(done(bool)), this, SLOT(packageDownloadDone(bool)) );

    m_packageFile.close();

    if( error )
    {
        m_packageFile.remove();
        // Warn of error condition.
        m_progressLabel->setText( tr( "Package download failed with error: %1" ).arg( m_http.errorString() ) );

        return;
    }

    m_progressBar->setValue( m_progressBar->maximum() );

    if( m_installer.installPackage( m_pendingPackage ), this )
        m_progressLabel->setText( tr( "%1 installed", "%1 = package name" ).arg( m_pendingPackage.name ) );

    QtopiaApplication::setMenuLike( this, false );
}

void PackageServiceInstaller::updateProgress( int done, int total )
{
    if( m_progressBar->maximum() != total )
        m_progressBar->setMaximum( total );

    m_progressBar->setValue( done );
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
