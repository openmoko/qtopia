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

#include <QBuffer>
#include <QSettings>

#include "httpfetcher.h"
#include "packageinformationreader.h"
#include "version.h"
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <qdebug.h>

#define DEFAULT_MAX_PACKAGES 100
#define MAX_PACKAGES_LIST_BYTES 50000
#define MAX_FILE_SIZE_BYTES 4194304

HttpFetcher::HttpFetcher( const QString &iurl, QObject *parent )
    : QThread( parent )
    , http( 0 )
    , packageData( 0 )
    , httpRequestAborted( false )
    , url( iurl )
    , fileSize( 0 )
    , curProgValue( 0 )
{
    pkgController = qobject_cast<AbstractPackageController*>(parent);
}

HttpFetcher::~HttpFetcher()
{
    if ( packageData ) delete packageData;
    if ( http ) delete http;
}

const int HttpFetcher::initProgress = 10;
const int HttpFetcher::maxProgress = 100;

void HttpFetcher::run()
{
    QString server;
    QString path;
    QString fetchFile;

    // if no file set, assume fetching package list
    fetchFile = file.isEmpty() ? AbstractPackageController::PACKAGE_SUMMARY_FILE : file;
    if ( url.startsWith( "http://" ))
        url = url.mid( 7 );
    int firstSlash = url.indexOf( "/" );
    if ( firstSlash < 0 )  // no path
    {
        server = url;
        if ( file.isEmpty() ) // assume fetching package list
            path = QString( "/%1" ).arg( fetchFile );
    }
    else
    {
        server = url.left( firstSlash );
        path = url.mid( firstSlash );
        if ( path.endsWith( "/" ))
            path += fetchFile;
        else
            path += QString( "/%1" ).arg( fetchFile );
    }
    if ( http == 0 ) http = new QHttp();
    connect( this, SIGNAL(httpAbort()),
            http, SLOT(abort()) );

    HttpFileReceiver *hr;
    Md5File *md5File = 0;
    const QString tmp="tmp/";
    QDir packagePathDir( Qtopia::packagePath() );
    if ( file.isEmpty() ) // getting package list
    {

        QSettings packageServersConf( "Trolltech", "PackageServers" );
        packageServersConf.beginGroup( "DeviceType" );
        QString device( packageServersConf.value( "device", "unknown" ).toString() );
        path.append("?device=").append(device);
        packageServersConf.endGroup(); 

        if ( packageData == 0 ) packageData = new QBuffer();
        hr = new HttpInfoReceiver;
        connect( packageData, SIGNAL(bytesWritten(qint64)),
                hr, SLOT(packageDataWritten(qint64)));
        
        if ( packagePathDir.exists(tmp) )
        {
            QDir packageTmpDir( Qtopia::packagePath() + tmp );
            QFileInfoList fileList= packageTmpDir.entryInfoList( QDir::Files );
            QFile f;
            foreach( QFileInfo fi, fileList )
            {
                f.setFileName( fi.absoluteFilePath() );
                f.remove();
            }
        }
    }
    else                  // getting a file
    {
        if ( !packagePathDir.exists(tmp) )
            packagePathDir.mkdir( tmp );

        md5File = new Md5File( file.prepend( Qtopia::packagePath() + tmp ) );
        // qDebug() << "http fetching" << md5File->fileName();
        if ( md5File->exists() )
            md5File->remove();
        packageData = md5File;
        hr = new HttpFileReceiver;
        hr->fileSize = fileSize;
    }
    hr->fetcher = this;

    connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),
            hr, SLOT(readResponseHeader(const QHttpResponseHeader&)));
    connect( http, SIGNAL(dataReadProgress(int,int)),
            hr, SLOT( updateDataReadProgress(int,int)));
    connect( http, SIGNAL(requestFinished(int,bool)),
            hr, SLOT(httpRequestFinished(int,bool)));

    // qDebug() << "set host" << server;
    http->setHost( server, 80 );
    packageData->open( QIODevice::WriteOnly );
    // qDebug() << "get data" << path;
    httpGetId = http->get( path, packageData );

    // show some progress now that we've set everything up
    curProgValue = initProgress;
    emit progressValue( initProgress );

    // qDebug() << "event loop start";

    // run threads event loop
    exec();
    
    // qDebug() << "event loop done";

    packageData->close();

    if( md5File )
        md5Sum = md5File->md5Sum();

    delete packageData;
    packageData = 0;
    delete http;
    http = 0;
    delete hr;
}

void HttpFetcher::cancel( const QString &errorReason )
{
    error = errorReason;
    httpRequestAborted = true;
    emit httpAbort();
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpFileReceiver implementation
/////

HttpFileReceiver::HttpFileReceiver( QObject *p )
    : QObject( p )
    , fileSize( 0 )
{
    QSettings serverConf( "Trolltech", "PackageServers" );
    serverConf.clear();
    QStringList servConfList = serverConf.childGroups();
    if ( servConfList.contains( QLatin1String( "Configuration" )))
    {
        serverConf.beginGroup( QLatin1String( "Configuration" ));
            maxFileSize = serverConf.value( QLatin1String( "maxFileSize" ),
                                            MAX_FILE_SIZE_BYTES ).toInt();
        serverConf.endGroup();
    }
    else
    {
        maxFileSize = MAX_FILE_SIZE_BYTES;
    }
}

HttpFileReceiver::~HttpFileReceiver()
{
}

void HttpFileReceiver::httpRequestFinished(int requestId, bool error)
{
    qLog(Package) << "requestId" << requestId << "finished" << ( error ? "in error" : "ok" );

    if ( requestId != fetcher->httpGetId )
        return;

    emit fetcher->progressValue( HttpFetcher::maxProgress );

    if ( error && !fetcher->httpRequestAborted )
    {
        fetcher->error = QLatin1String("HttpFileReceiver::httpRequestFinished:- http error: ") 
                                    + fetcher->http->errorString();
        fetcher->httpRequestAborted = true;
    }
    // terminate the threads loop
    if ( fetcher->httpRequestAborted )
    {
        fetcher->exit(1);
    }
    else
        fetcher->quit();
}

void HttpFileReceiver::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    if ( responseHeader.statusCode() >= 400 )
    {
        fetcher->httpRequestAborted = true;
        QString detailedError( "HttpFileReceiver::readReasponseHeader:- status code: %1 "
                               "reason phrase: %2" );
        detailedError = detailedError.arg( responseHeader.statusCode() )
                                     .arg( responseHeader.reasonPhrase() );
        fetcher->error = detailedError; 
    }
}

void HttpFileReceiver::updateDataReadProgress(int bytesRead, int totalBytes)
{
    qLog(Package) << "update data read - bytes read:" << bytesRead << " total:" << totalBytes;
    if ( fetcher->httpRequestAborted )
        return;

    if ( totalBytes > maxFileSize )
    {
        fetcher->cancel( tr("Downloaded file size, %1, exceed maximum allowed file size, %2 ")
                        .arg( totalBytes ).arg( maxFileSize ) );
    }

    int run = HttpFetcher::maxProgress - HttpFetcher::initProgress;
    if ( totalBytes != 0 )
    {
        fetcher->curProgValue = HttpFetcher::initProgress +
                                ( bytesRead * ( run / totalBytes ));
        if ( fileSize && bytesRead > fileSize )
        {
            qWarning( "Bytes read %d overflowed expected file size %d\n",
                    bytesRead, fileSize );
            fetcher->cancel(tr("Downloaded file exceeds expected file size"));
        }
    }

    emit fetcher->progressValue( fetcher->curProgValue );
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpInfoReceiver implementation
/////

HttpInfoReceiver::HttpInfoReceiver( QObject *p )
    : HttpFileReceiver( p )
    , maxPackagesList( DEFAULT_MAX_PACKAGES )
{
    reader = new PackageInformationReader;
    connect( reader, SIGNAL(packageComplete()),
            this, SLOT(packageComplete()));

    QSettings serverConf( "Trolltech", "PackageServers" );
    serverConf.clear();
    QStringList servConfList = serverConf.childGroups();
    if ( servConfList.contains( QLatin1String( "Configuration" )))
    {
        serverConf.beginGroup( QLatin1String( "Configuration" ));
        if ( serverConf.contains( QLatin1String( "maxPackagesList" )))
            maxPackagesList = serverConf.value( QLatin1String( "maxPackagesList" )).toInt();

        maxPackagesListSize = serverConf.value( QLatin1String( "maxPackagesListSize" ),
                                        QVariant(MAX_PACKAGES_LIST_BYTES) ).toInt();

        serverConf.endGroup();
    } 
    else
    {
        maxPackagesListSize = MAX_PACKAGES_LIST_BYTES;
    }
    
}

HttpInfoReceiver::~HttpInfoReceiver()
{
    delete reader;
}

void HttpInfoReceiver::packageComplete()
{
    /*
    qDebug() << "Package {" << reader->description() << ", "
        << reader->package().fullDescription << ", "
        << reader->package().size << ", "
        << reader->section() << ", "
        << reader->domain() << ", "
        << reader->package().packageFile << "}"
        << ( reader->getIsError() ? reader->getError() : " OK" );
    qDebug() << "\t" << "======= complete ============\n";
    */
    Q_ASSERT( fetcher->pkgController );
    NetworkPackageController *npc = static_cast<NetworkPackageController*>(fetcher->pkgController);
    if ( npc->numberPackages() < maxPackagesList )
    {
        if ( VersionUtil::checkVersion( reader->package().qtopiaVersion ) )
                npc->addPackage( reader->package() );
    }
    else
    {
        // TODO: after string freeze this should display user visible error
        qWarning( "Overflowed package list limit %d\n", maxPackagesList );
        fetcher->cancel(tr("Too many packages available to display from server"));
    }
}

void HttpInfoReceiver::packageDataWritten( qint64 bytes )
{
    // qDebug() << "package data available" << bytes;
    if ( fetcher->httpRequestAborted )
        return;
    fetcher->packageData->close();
    fetcher->packageData->open( QIODevice::ReadOnly );
    while ( true )
    {
        lineString += fetcher->packageData->readLine( bytes );
        if ( !lineString.contains( "\n" ))
        {
            // qDebug( "No return character" );
            goto out_recv_data;
        }
        reader->readLine( lineString );
        lineString = "";
    }
out_recv_data:
    fetcher->packageData->close();

    //if downloading a package list clear buffer
    QBuffer *buf = qobject_cast<QBuffer *>(fetcher->packageData);
    if ( buf )
       buf->buffer().clear();
    fetcher->packageData->open( QIODevice::WriteOnly );
}


void HttpInfoReceiver::updateDataReadProgress(int bytesRead, int totalBytes)
{
    if ( totalBytes > maxPackagesListSize )
    {
        fetcher->cancel( QString("Packages.list exceeds maximum size of %1 bytes.")
                        .arg( maxPackagesListSize ) );
        return;
    }
    HttpFileReceiver::updateDataReadProgress( bytesRead, totalBytes );
}
