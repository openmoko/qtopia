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

#ifndef HTTPFETCHER_H
#define HTTPFETCHER_H

#include <qthread.h>

#include <QHttpResponseHeader>
#include <QUrl>

#include "packagecontroller.h"
#include "md5file.h"

class QHttp;
class PackageInformationReader;

class HttpFileReceiver : public QObject
{
    Q_OBJECT
public:
    HttpFileReceiver( QObject *p = 0 );
    virtual ~HttpFileReceiver();
private slots:
    virtual void httpRequestFinished(int, bool);
    virtual void readResponseHeader(const QHttpResponseHeader &);
protected slots:
    virtual void updateDataReadProgress(int, int);
protected:
    HttpFetcher *fetcher;
private:
    int fileSize;
    int maxFileSize;

    friend class HttpFetcher;
};

class HttpInfoReceiver : public HttpFileReceiver
{
    Q_OBJECT
public:
    HttpInfoReceiver( QObject *p = 0 );
    virtual ~HttpInfoReceiver();
protected slots:
    virtual void updateDataReadProgress(int, int);
private slots:
    void packageDataWritten( qint64 );
    void packageComplete();
private:
    PackageInformationReader *reader;
    QString lineString;
    int maxPackagesList;
    int maxPackagesListSize;

    friend class HttpFetcher;
};


class HttpFetcher : public QThread
{
    Q_OBJECT
public:
    static const int initProgress;
    static const int maxProgress;

    HttpFetcher( const QString &url, QObject *parent );
    virtual ~HttpFetcher();
    virtual void run();
    void setFile( const QString &f, int size = 0 ) { file = f; fileSize = size; }
    QString getFile() const { return file; }
    bool httpRequestWasAborted() const { return httpRequestAborted; }
    QString getError() const { return error; }
    QString getMd5Sum() const{ return md5Sum; }
public slots:
    void cancel( const QString &errorReason = "" );
signals:
    void progressValue( int );
    void newPackage(InstallControl::PackageInfo*);
    void httpAbort();
private:
    QHttp *http;
    QIODevice *packageData;
    AbstractPackageController *pkgController;
    bool httpRequestAborted;
    int httpGetId;
    QUrl url;
    QString file;
    int fileSize;
    int curProgValue;
    QString error;
    QString md5Sum;
    friend class HttpInfoReceiver;
    friend class HttpFileReceiver;
    friend class PackageWizard;
};

#endif
