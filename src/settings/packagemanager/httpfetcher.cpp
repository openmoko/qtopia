/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "httpfetcher.h"
#include "packageinformationreader.h"
#include <qtopialog.h>
#include <qtopianamespace.h>

#include <qdebug.h>

HttpFetcher::HttpFetcher( const QString &iurl, QObject *parent )
    : QThread( parent )
    , http( 0 )
    , packageData( 0 )
    , httpRequestAborted( false )
    , url( iurl )
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

    HttpFileReceiver *hr;
    if ( file.isEmpty() ) // getting package list
    {
        if ( packageData == 0 ) packageData = new QBuffer();
        hr = new HttpInfoReceiver;
        connect( packageData, SIGNAL(bytesWritten(qint64)),
                hr, SLOT(packageDataWritten(qint64)));
    }
    else                  // getting a file
    {
        QFile *f = new QFile( file.prepend( Qtopia::tempDir() ) );
        // qDebug() << "http fetching" << f->fileName();
        if ( f->exists() )
            f->remove();
        packageData = f;
        hr = new HttpFileReceiver;
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

    delete packageData;
    packageData = 0;
    delete http;
    http = 0;
    delete hr;
}

void HttpFetcher::cancel()
{
    httpRequestAborted = true;
    http->abort();
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpFileReceiver implementation
/////

HttpFileReceiver::HttpFileReceiver( QObject *p )
    : QObject( p )
{
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

    // terminate the threads loop
    if ( fetcher->httpRequestAborted )
        fetcher->exit(1);
    else
        fetcher->quit();
}

void HttpFileReceiver::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    // qDebug() << "response header - code:" << responseHeader.statusCode();
    if ( responseHeader.statusCode() >= 400 )
    {
        fetcher->httpRequestAborted = true;
    }
}

void HttpFileReceiver::updateDataReadProgress(int bytesRead, int totalBytes)
{
    // qDebug() << "update data read - bytes read:" << bytesRead << " total:" << totalBytes;
    if ( fetcher->httpRequestAborted )
        return;

    int run = HttpFetcher::maxProgress - HttpFetcher::initProgress;
    if ( totalBytes != 0 )
        fetcher->curProgValue = HttpFetcher::initProgress +
                                ( bytesRead * ( run / totalBytes ));

    emit fetcher->progressValue( fetcher->curProgValue );
}

////////////////////////////////////////////////////////////////////////
/////
/////   HttpInfoReceiver implementation
/////

HttpInfoReceiver::HttpInfoReceiver( QObject *p )
    : HttpFileReceiver( p )
{
    reader = new PackageInformationReader;
    connect( reader, SIGNAL(packageComplete()),
            this, SLOT(packageComplete()));
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
    npc->addPackage( reader->package() );
}

void HttpInfoReceiver::packageDataWritten( qint64 bytes )
{
    // qDebug() << "package data available" << bytes;
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
    fetcher->packageData->open( QIODevice::WriteOnly );
}
