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

#include "packagecontroller.h"

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
    virtual void updateDataReadProgress(int, int);
protected:
    HttpFetcher *fetcher;

    friend class HttpFetcher;
};

class HttpInfoReceiver : public HttpFileReceiver
{
    Q_OBJECT
public:
    HttpInfoReceiver( QObject *p = 0 );
    virtual ~HttpInfoReceiver();
private slots:
    void packageDataWritten( qint64 );
    void packageComplete();
private:
    PackageInformationReader *reader;
    QString lineString;

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
    void setFile( const QString &f ) { file = f; }
    QString getFile() const { return file; }
    bool httpRequestWasAborted() const { return httpRequestAborted; }
    QString getError() const { return error; }
public slots:
    void cancel();
signals:
    void progressValue( int );
    void newPackage(InstallControl::PackageInfo*);
private:
    QHttp *http;
    QIODevice *packageData;
    AbstractPackageController *pkgController;
    bool httpRequestAborted;
    int httpGetId;
    QString url;
    QString file;
    int curProgValue;
    QString error;

    friend class HttpInfoReceiver;
    friend class HttpFileReceiver;
    friend class PackageWizard;
};

#endif
